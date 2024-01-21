#include <iostream>
#include <stack>
#include <functional>
#include <unordered_map>
#include "propertytree.h"
#include "propertytreetokeniser.h"
#include "expressions.h"
#include "identifier.h"
#include "dimensions.h"
#include "utils/StableVector.h"
#include "types.h"
#include <span>
#include "notice.h"
#include <format>

namespace nugget::expressions {
    using namespace nugget::properties;
    using namespace nugget::identifier;
    using namespace nugget::ui;
        struct ConversionPair {
        ConversionPair(ValueAny::Type from, ValueAny::Type to) : from(from), to(to) {}
        ConversionPair(const ConversionPair& other) : from(other.from), to(other.to) {}
        bool operator==(const ConversionPair& b) const {
            return from == b.from && to == b.to;
        }
        ValueAny::Type from;
        ValueAny::Type to;
    };
}
//////////////////////////////////////////
namespace std {
    template <>
    struct hash<nugget::expressions::ConversionPair> {
        std::size_t operator()(const nugget::expressions::ConversionPair& cp) const {
            // Use a combination of hash values for 'from' and 'to'
            return hash<nugget::ValueAny::Type>{}(cp.from) ^ hash<nugget::ValueAny::Type>{}(cp.to);
        }
    };

    template <>
    struct equal_to<nugget::expressions::ConversionPair> {
        bool operator()(const nugget::expressions::ConversionPair& lhs, const nugget::expressions::ConversionPair& rhs) const {
            return lhs == rhs; // Assuming you've already defined 'operator=='
        }
    };
}

namespace nugget::expressions {
    using namespace nugget::properties;
    using namespace nugget::identifier;
    using namespace nugget::ui;

    struct Rpn {
        std::stack<Token> operatorStack;
        std::vector<Token> output;
        std::vector<Token> input;
        Notice::Board& board;

        Rpn(Notice::Board& boardIn) : board(boardIn) {}

        void AddToken(const Token& token) {
            input.emplace_back(token);
        }

        int GetPrecedence(Token::Type type) {
            switch (type) {
            case Token::Type::plus:
            case Token::Type::minus:
                return 1;
            case Token::Type::multiply:
            case Token::Type::divide:
                return 2;
            case Token::Type::uMinus:
                return 100;
            case Token::Type::at:
                return 200;
            default:
                return 0;
            }
        }

        int IsUnaryCheck(Token::Type type) {  // check previous token
            switch (type) {
                case Token::Type::divide:
                case Token::Type::minus:
                case Token::Type::plus:
                case Token::Type::multiply:
                return true;
            default:
                return false;
            }
        }
        int IsOperator(Token::Type type) {
            switch (type) {
            case Token::Type::plus:
            case Token::Type::minus:
            case Token::Type::multiply:
            case Token::Type::divide:
                return true;
            default:
                return false;
            }
        }

        void ProcessOperator(const Token& op) {
            while (!operatorStack.empty() &&
                GetPrecedence(operatorStack.top().type) >= GetPrecedence(op.type) &&
                operatorStack.top().type != Token::Type::openParen) {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }
            operatorStack.push(op);
        }

        void InfixToPostfix() {
            std::vector<bool> inFunction;
            for(size_t i = 0;i<input.size();i++) {
                const auto& token = input[i];
                switch (token.type) {
                    case Token::Type::integer:
                    case Token::Type::percent:
                    case Token::Type::float_:
                    case Token::Type::string: {
                        output.push_back(token);
                    } break;
                    case Token::Type::identifier: {
                        output.push_back(token);
                    } break;
                    case Token::Type::dollar: {
                        ProcessOperator(token);
                    } break;
                    case Token::Type::at: {
                        ProcessOperator(token);
                    } break;
                    case Token::Type::minus: {
                        if (i == 0 || IsUnaryCheck(input[i-1].type)) {
                            ProcessOperator(Token{ .type = Token::Type::uMinus,.text = "-" });
                        } else {
                            ProcessOperator(token);
                        }
                    } break;
                    case Token::Type::comma:
                    case Token::Type::plus:
                    case Token::Type::dot:
                    case Token::Type::doubleColon:
                    case Token::Type::multiply:
                    case Token::Type::divide: {
                        ProcessOperator(token);
                    } break;
                    case Token::Type::openParen: {
                        // if previous is an identifier, this us a function call
                        if (output.size() > 0) {
                            auto t = output.back();
                            if (t.type == Token::Type::identifier) {
                                output.pop_back();
                                output.push_back(Token{ .type = Token::Type::function,.text = t.text });
                                inFunction.push_back(true);
                            } else {
                                inFunction.push_back(false);
                            }
                            operatorStack.push(token);
                        } else {
                            inFunction.push_back(false);
                            operatorStack.push(token);
                        }
                    } break;
                    case Token::Type::closeParen: {
                        if (inFunction.size() < 1) {
                            assert(0);
                        }
                        if (inFunction.back()) {
                            ProcessOperator(token);
                            inFunction.pop_back();
                        }
                        while (!operatorStack.empty() &&
                            operatorStack.top().type != Token::Type::openParen) {
                            output.push_back(operatorStack.top());
                            operatorStack.pop();
                        }
                        operatorStack.pop(); // Pop the open paren
                    } break;
                    default: {
                        check(0, "Unhandled type: {}\n", token.TypeAsString());
                    } break;
                }
            }
            while (!operatorStack.empty()) {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }
        }
        static bool ParseInteger(const std::string& str,int64_t &result) {
            auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
            if (ec == std::errc()) {
                return true;
            }
            else {
                return false;
            }
        }
        static bool ParseFloat(const std::string& str,float &result) {
            if (str == "") {
                result = 0.0f;
            }
            else {
                auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
                if (ec == std::errc()) {
                    return true;
                }
            }
            return false;
        }

        static std::string ParseString(const std::string& str) {
            // strip off beginning and end double quotes
            return str.substr(1, str.size() - 2);
        }

        static IDType ParseQualifiedName(const std::string& str) {
            return IDR(str);
        }

        std::unordered_map<IDType, std::function<ValueAny(const std::span<ValueAny>& args)>> functions = {
            {
                ID("Color"),
                [](const std::span<ValueAny>& args) {
                    check(args.size() == 4,"Incorrect number of arguments");
                    float r = Expression::ConvertType(args[0], ValueAny::Type::float_).AsFloat();
                    float g = Expression::ConvertType(args[1], ValueAny::Type::float_).AsFloat();
                    float b = Expression::ConvertType(args[2], ValueAny::Type::float_).AsFloat();
                    float a = Expression::ConvertType(args[3], ValueAny::Type::float_).AsFloat();
                    return ValueAny(Color(r, g, b, a));
                }
            },
            {
                ID("vector3f"),
                [](const std::span<ValueAny>& args) {
                    check(args.size() == 3,"Incorrect number of arguments");
                    float x = Expression::ConvertType(args[0], ValueAny::Type::float_).AsFloat();
                    float y = Expression::ConvertType(args[1], ValueAny::Type::float_).AsFloat();
                    float z = Expression::ConvertType(args[2], ValueAny::Type::float_).AsFloat();
                    return ValueAny(Vector3f(x,y,z));
                }
            },
            {
                ID("vector2f"),
                [](const std::span<ValueAny>& args) {
                    check(args.size() == 2,"Incorrect number of arguments");
                    float x = Expression::ConvertType(args[0], ValueAny::Type::float_).AsFloat();
                    float y = Expression::ConvertType(args[1], ValueAny::Type::float_).AsFloat();
                    return ValueAny(Vector2f(x,y));
                }
            },
            {
                ID("ref"),
                [&](const std::span<ValueAny>& args) {
                    check(args.size() == 1,"Incorrect number of arguments");
                    IDType id = args[0].AsIDType();
                    if (board.KeyExists(id)) {
                        return board.GetValueAny(id);
                    } else {
                        return ValueAny(Exception{ .description = std::format("Could not find property '{}'",IDToString(id)) });
                    }
                }
            },
            {
                ID("concat"),
                [&](const std::span<ValueAny>& args) {
                    check(args.size() > 0,"no arguments?");
                    switch (args[0].GetType()) {
                        case ValueAny::Type::Vector3fList: {
                            Vector3fList c(args[0].AsVector3fList());
                            if (args.size() > 1) {
                                switch (args[1].GetType()) {
                                    case ValueAny::Type::Vector3fList: {
                                        for (int i = 1; i < args.size(); i++) {
                                            for (auto&& x : args[i].AsVector3fList().data) {
                                                c.data.push_back(x);
                                            }
                                        }
                                    } break;
                                    default: {
                                        return ValueAny(Exception{ .description = std::format("Unsupported types for concat '{}' and '{}'",args[0].TypeAsString(),args[1].TypeAsString()) });
                                    } break;
                                }
                            }
                            return ValueAny(c);
                        } break;
                        case ValueAny::Type::ColorList: {
                            ColorList c(args[0].AsColorList());
                            if (args.size() > 1) {
                                switch (args[1].GetType()) {
                                    case ValueAny::Type::ColorList: {
                                        for (int i = 1; i < args.size(); i++) {
                                            for (auto&& x : args[i].AsColorList().data) {
                                                c.data.push_back(x);
                                            }
                                        }
                                    } break;
                                    default: {
                                        return ValueAny(Exception{ .description = std::format("Unsupported types for concat '{}' and '{}'",args[0].TypeAsString(),args[1].TypeAsString()) });
                                    } break;
                                }
                            }
                            return ValueAny(c);
                        } break;
                        case ValueAny::Type::Vector2fList: {
                            Vector2fList c(args[0].AsVector2fList());
                            if (args.size() > 1) {
                                switch (args[1].GetType()) {
                                    case ValueAny::Type::Vector2fList: {
                                        for (int i = 1; i < args.size(); i++) {
                                            for (auto&& x : args[i].AsVector2fList().data) {
                                                c.data.push_back(x);
                                            }
                                        }
                                    } break;
                                    default: {
                                        return ValueAny(Exception{ .description = std::format("Unsupported types for concat '{}' and '{}'",args[0].TypeAsString(),args[1].TypeAsString()) });
                                    } break;
                                }
                            }
                            return ValueAny(c);
                        } break;
                        default: {
                            if (args.size() > 1) {
                                return ValueAny(Exception{ .description = std::format("Unsupported types for concat '{}' and '{}'",args[0].TypeAsString(),args[1].TypeAsString()) });
                            } else {
                                return ValueAny(Exception{ .description = std::format("Unsupported type for concat '{}'",args[0].TypeAsString()) });
                            }
                        } break;
                    }
                }
            }

        };

        static inline std::unordered_map<ConversionPair, std::function<ValueAny(const ValueAny &from)>> converters =
        {
            {
                {
                    ValueAny::Type::int64_t_, ValueAny::Type::float_
                },
                [](const ValueAny& from) {
                    return ValueAny((float)from.AsInt64());
                },
            },
            {
                {
                    ValueAny::Type::float_, ValueAny::Type::dimension
                },
                [](const ValueAny& from) {
                    // raw float dimension
                    return ValueAny(Dimension(from.AsFloat()));
                }
            },
            {
                {
                    ValueAny::Type::int64_t_, ValueAny::Type::dimension
                },
                [](const ValueAny& from) {
                    return ValueAny(Dimension((float)from.AsInt64()));
                }
            },
            {
                {
                    ValueAny::Type::int64_t_, ValueAny::Type::string
                },
                [](const ValueAny& from) {
                    return ValueAny(std::format("{}",from.AsInt64()));
                }
            },
            {
                {
                    ValueAny::Type::int64_t_, ValueAny::Type::Color
                },
                [](const ValueAny& from) {
                float v = (float)from.AsInt64();
                    return ValueAny(Color{v,v,v,v});
                }
            },
            {
                {
                    ValueAny::Type::float_, ValueAny::Type::Vector3f
                },
                [](const ValueAny& from) {
                float v = (float)from.AsFloat();
                    return ValueAny(Vector3f{v,v,v});
                }
            },
        };

        ValueAny ParseToken(const Token& token) {
            switch (token.type) {
                case Token::Type::identifier: {
                    return ValueAny(IDR(token.text));
                } break;
                case Token::Type::function: {
                    return ValueAny(IDR(token.text));
                } break;
                case Token::Type::dimension: {
                    check(0, "not implemented");
                } break;
                case Token::Type::percent: {
                    float val;
                    if(ParseFloat(token.text,val)) {
                        return ValueAny(nugget::ui::Dimension{ val,nugget::ui::Dimension::Units::none });
                    } else {
                        check(0, "could not parse percent");
                    }
                } break;
                case Token::Type::float_: {
                    float val;
                    if (ParseFloat(token.text, val)) {
                        return ValueAny(val);
                    }
                    else {
                        check(0, "could not parse float");
                    }
                } break;
                case Token::Type::integer: {
                    int64_t val;
                    if (ParseInteger(token.text, val)) {
                        return ValueAny(val);
                    }
                    else {
                        check(0, "could not parse integer");
                    }
                } break;
                case Token::Type::string: {
                    return ValueAny(ParseString(token.text));
                } break;
                case Token::Type::Vector3fList: {
                    check(0, "not implemented");
                } break;
                case Token::Type::Color: {
                    check(0, "not implemented");
                } break;
                default: {
                    check(0, "unhandled");
                } break;
            }
            check(0, "cannot parse token");
            return {};
        }

        static ValueAny ConvertType(ValueAny a, ValueAny::Type type) {
            if (a.GetType() == type) {
                return ValueAny(a);
            }
            ConversionPair cp(a.GetType(), type);
            if (converters.contains(cp)) {
                return converters.at(cp)(a);
            } else {
                return ValueAny(Exception{ .description = format("cannot convert {} to {}\n", a.TypeAsString(), ValueAny::TypeAsString(type)) });
            }
        }

        ValueAny::Type GetPromotedType(ValueAny::Type a, ValueAny::Type b) {
            // Determine the order of types in the promotion hierarchy
            const std::vector<ValueAny::Type> promotionOrder = {
                ValueAny::Type::int32_t_,
                ValueAny::Type::uint64_t_,
                ValueAny::Type::int64_t_,
                ValueAny::Type::float_,
                ValueAny::Type::Color,  
                ValueAny::Type::string,
                ValueAny::Type::IDType,
                ValueAny::Type::pointer,
                ValueAny::Type::dimension,
                ValueAny::Type::Vector3f,
                ValueAny::Type::Vector3fList,
            };

            // Find the common type in the promotion hierarchy
            auto itA = std::find(promotionOrder.begin(), promotionOrder.end(), a);
            auto itB = std::find(promotionOrder.begin(), promotionOrder.end(), b);

            if (itA != promotionOrder.end() && itB != promotionOrder.end()) {
                // Both types are in the promotion hierarchy
                return std::max(*itA, *itB);
            }
            else {
                // One or both types are not in the promotion hierarchy, default to the larger type
                return std::max(a, b);
            }
        }

#define EXPR_PLUS 1
#define EXPR_MINUS 2
#define EXPR_MULTIPLY 3
#define EXPR_DIVIDE 4
#define EXPR_DOT 5
#define EXPR_DOUBLE_COLON 6

#define EXPR_OPERATOR +
#define EXPR_NAME Plus
#define EXPR_OP EXPR_PLUS
#include "expressions_operators.h"

#define EXPR_OPERATOR -
#define EXPR_NAME Minus
#define EXPR_OP EXPR_MINUS
#include "expressions_operators.h"

#define EXPR_OPERATOR *
#define EXPR_NAME Multiply
#define EXPR_OP EXPR_MULTIPLY
#include "expressions_operators.h"

#define EXPR_OPERATOR /
#define EXPR_NAME Divide
#define EXPR_OP EXPR_DIVIDE
#include "expressions_operators.h"

#define EXPR_NAME Dot
#define EXPR_OP EXPR_DOT
#include "expressions_operators.h"

#define EXPR_NAME DoubleColon
#define EXPR_OP EXPR_DOUBLE_COLON
#include "expressions_operators.h"

        /////////////////////////
        // specialist
        ValueAny Vector3ListOp(const std::span<ValueAny> values,int operation) {
            check(values.size() == 2, "Should be two args");
            ValueAny a = values[0];
            ValueAny b = values[1];
            check(a.GetType() == ValueAny::Type::Vector3fList || b.GetType() == ValueAny::Type::Vector3fList, "one type needs to be Vector3fList");
            if (a.GetType() != ValueAny::Type::Vector3fList) {
                auto c = a;   // a always the main type
                a = b;
                b = c;
            }

            float bf = 0;
            switch (b.GetType()) {
                case ValueAny::Type::float_:
                    bf = b.AsFloat();
                    goto scalar_product__;
                case ValueAny::Type::int64_t_:
                    bf = (float)b.AsInt64();
                    scalar_product__:
                {
                    const auto& al = a.AsVector3fList();
                    Vector3fList out;
                    for (size_t i = 0; i < al.data.size(); i++) {
                        switch (operation) {
                            case EXPR_MULTIPLY:
                                out.data.push_back(al.data[i] * bf);
                                break;
                            default:
                                assert(0);
                        }
                    }
                    return ValueAny(out);
                }
                case ValueAny::Type::Vector3fList: {
                    auto& al = a.AsVector3fList();
                    auto& bl = b.AsVector3fList();
                    check(al.data.size() == bl.data.size(), "sizes must be the same");
                    Vector3fList out;
                    for (size_t i = 0; i < al.data.size(); i++) {
                        switch (operation) {
                            case EXPR_PLUS:
                            out.data.push_back(al.data[i] + bl.data[i]);
                            break;
                            case EXPR_MULTIPLY:
                            out.data.push_back(al.data[i] * bl.data[i]);
                            break;
                            default:
                            assert(0);
                        }
                    }
                    return ValueAny(out);
                } break;
                case ValueAny::Type::Vector3f: {
                    auto& al = a.AsVector3fList();
                    auto& bv = b.AsVector3f();
                    Vector3fList out;
                    for (size_t i = 0; i < al.data.size(); i++) {
                        switch (operation) {
                            case EXPR_PLUS:
                                out.data.push_back(al.data[i] + bv);
                                break;
                            case EXPR_MINUS:
                                out.data.push_back(al.data[i] - bv);
                                break;
                            case EXPR_MULTIPLY:
                                out.data.push_back(al.data[i] * bv);
                                break;
                            default:
                                assert(0);
                        }
                    }
                    return ValueAny(out);
                } break;
                default: {
                    return ValueAny(Exception{ .description = std::format("Unsupprted operation, EXPR_OP={}, types: {} and {}\n",
                        operation, values[0].TypeAsString(), values[1].TypeAsString()) });
                }
            }
        }

        // unary
        ValueAny IndirectValue(const std::span<ValueAny> values) {
            const ValueAny& v = values[0];
            check(v.GetType() == ValueAny::Type::IDType, "Must be an IDType");
            const ValueAny& w = board.GetValueAny(v.AsIDType());
            const IDType wid = w.AsIDType();
            if (board.KeyExists(wid)) {
                return board.GetValueAny(wid);
            }
            return {};
        }

        // unary
        ValueAny ExpandParseVariable(const std::span<ValueAny> values, std::function<ValueAny(IDType)> expandParseVars) {
            const ValueAny& v = values[0];
//            output("{}\n", v.GetValueAsString());
            auto id = v.AsIDType();
            return expandParseVars(id);
        }

        ValueAny EvalRpn(std::function<ValueAny(IDType)> expandParseVars) {
            size_t point = 0; 
            size_t outSize = output.size();
            StableVector<ValueAny, 100> accumulation;
            std::vector<size_t> functionMarkers;

            while (point < outSize) {
//                output("token: {}\n", output[point].ToString());
                switch (output[point].type) {
                    case Token::Type::float_:
                    case Token::Type::percent:
                    case Token::Type::identifier:
                    case Token::Type::string:
                    case Token::Type::integer: {
                        const ValueAny& v = ParseToken(output[point++]);
                        if (v.IsException()) {
                            assert(0);
                        }
                        accumulation.emplace_back(v);
                    } break;
                    case Token::Type::function: {
                        const ValueAny& v = ParseToken(output[point++]);
                        if (v.IsException()) {
                            assert(0);
                        }
                        functionMarkers.push_back(accumulation.size());
                        accumulation.emplace_back(v);
                    } break;
                    case Token::Type::dollar: {
                        const auto& r = IndirectValue(accumulation.GetArrayLast(1));
                        if (r.IsException()) {
                            assert(0);
                        }
                        accumulation.Pop(1);
                        accumulation.emplace_back(r);
                        point++;
                    } break;
                    case Token::Type::uMinus: {
                        const ValueAny& v = accumulation[accumulation.size()-1];
                        ValueAny r;
                        switch (v.GetType()) { 
                            case ValueAny::Type::float_: {
                                r = ValueAny(-v.AsFloat());
                            } break;
                            case ValueAny::Type::int32_t_: {
                                r = ValueAny(-v.AsInt32());
                            } break;
                            case ValueAny::Type::int64_t_: {
                                r = ValueAny(-v.AsInt64());
                            } break;
                            case ValueAny::Type::Vector3f: {
                                auto& w = v.AsVector3f();
                                r = ValueAny(Vector3f(-w.x, -w.y, -w.z));
                            } break;
                            default: {
                                r = ValueAny(Exception{ .description = std::format("Could not apply unary minus to type '{}'",v.TypeAsString()) });
                            } break;
                        }
                        if (r.IsException()) {
                            return ValueAny(r);
                        }
                        accumulation.Pop(1);
                        accumulation.emplace_back(r);
                        point++;
                    } break;
                    case Token::Type::at: {
//                        output("at: {} : {}", accumulation.GetArrayLast(1)[0].GetTypeAsString(), accumulation.GetArrayLast(1)[0].GetValueAsString());
                        const auto& r = ExpandParseVariable(accumulation.GetArrayLast(1),expandParseVars);
                        if (r.IsException()) {
                            return r;
                        }
                        accumulation.Pop(1);
                        accumulation.emplace_back(r);
                        point++;
                    } break;
                    case Token::Type::closeParen: {
                        size_t i = functionMarkers.back();
                        functionMarkers.pop_back();
                        IDType className = accumulation[i].AsIDType();
                        if (!functions.contains(className)) {
                            return ValueAny(Exception{ .description = std::format("Could not find function '{}'",IDToString(className)) });
                        } else {
                            ValueAny any = functions.at(className)(accumulation.GetArrayLast(accumulation.size()-i-1));
                            if (any.IsException()) {
                                return ValueAny(any);
                            }
                            accumulation.SetSize(i);
                            accumulation.emplace_back(any);
                            point++;
                        }
                    } break;
                    case Token::Type::comma: {
                        point++;
                    } break;
                    case Token::Type::doubleColon: {
                        const auto& r = DoubleColon(accumulation.GetArrayLast(2));
                        if (r.IsException()) {
                            assert(0);
                        }
                        accumulation.Pop(2);
                        accumulation.emplace_back(r);
                        point++;
                    } break;
                    case Token::Type::dot: {
                        const auto& r = Dot(accumulation.GetArrayLast(2));
                        if (r.IsException()) {
                            return r;
                        }
                        accumulation.Pop(2);
                        accumulation.emplace_back(r);
                        point++;
                    } break;
                    case Token::Type::plus: {
                        if (accumulation.size() > 1 && accumulation[accumulation.size() - 2].GetType() == ValueAny::Type::Vector3fList
                            ||
                            accumulation[accumulation.size() - 1].GetType() == ValueAny::Type::Vector3fList) {
                            const auto& r = Vector3ListOp(accumulation.GetArrayLast(2), EXPR_PLUS);
                            if (r.IsException()) {
                                return r;
                            }
                            accumulation.Pop(2);
                            accumulation.emplace_back(r);
                        } else {
                            const auto& r = Plus(accumulation.GetArrayLast(2));
                            if (r.IsException()) {
                                assert(0);
                            }
                            accumulation.Pop(2);
                            accumulation.emplace_back(r);
                        }
                        point++;
                        } break;
                    case Token::Type::minus: {
                        if (accumulation.size()>1 && accumulation[accumulation.size() - 2].GetType() == ValueAny::Type::Vector3fList
                            ||
                            accumulation[accumulation.size() - 1].GetType() == ValueAny::Type::Vector3fList) {
                            const auto& r = Vector3ListOp(accumulation.GetArrayLast(2),EXPR_MINUS);
                            if (r.IsException()) {
                                return r;
                            }
                            accumulation.Pop(2);
                            accumulation.emplace_back(r);
                        } else {
                            const auto& r = Minus(accumulation.GetArrayLast(2));
                            if (r.IsException()) {
                                assert(0);
                            }
                            accumulation.Pop(2);
                            accumulation.emplace_back(r);
                        }
                        point++;
                    } break;
                    case Token::Type::multiply: {
                        if (accumulation.size()> 1 && accumulation[accumulation.size() - 2].GetType() == ValueAny::Type::Vector3fList
                            ||
                            accumulation[accumulation.size() - 1].GetType() == ValueAny::Type::Vector3fList) {
                            const auto& r = Vector3ListOp(accumulation.GetArrayLast(2),EXPR_MULTIPLY);
                            if (r.IsException()) {
                                assert(0);
                            }
                            accumulation.Pop(2);
                            accumulation.emplace_back(r);
                        } else {
                            const auto& r = Multiply(accumulation.GetArrayLast(2));
                            if (r.IsException()) {
                                return r;
                            }
                            accumulation.Pop(2);
                            accumulation.emplace_back(r);
                        }
                        point++;
                    } break;
                    case Token::Type::divide: {
                        const auto& r = Divide(accumulation.GetArrayLast(2));
                        if (r.IsException()) {
                            return r;
                        }
                        accumulation.Pop(2);
                        accumulation.emplace_back(r);
                        point++;
                    } break;
                    default: {
                        check(0, "What do I do now?");
                    } break;
                } 
            }
            return accumulation[0];
        }

        ValueAny Evaluate(std::function<ValueAny(IDType)> expandParseVars) {
            InfixToPostfix();
            return EvalRpn(expandParseVars);
        }

        void PrintPostfix() {
            for (const auto& token : output) {
                std::cout << token.text << " ";
            }
            std::cout << std::endl;
        }
    };

    struct Expression_imp {
        Expression_imp(Notice::Board& boardIn) : rpn(boardIn) {}
        Rpn rpn;
    };

    Expression::Expression(Notice::Board& boardIn) : imp(*new Expression_imp(boardIn)) {
    }
    void Expression::AddToken(const Token& token) {
        imp.rpn.AddToken(token);
    }

    ValueAny Expression::Evaluate(std::function<nugget::ValueAny(IDType)> expandParseVars) {
        return imp.rpn.Evaluate(expandParseVars);
    }

    /*static*/
    ValueAny Expression::ConvertType(nugget::ValueAny a, nugget::ValueAny::Type type) {
        return Rpn::ConvertType(a, type);
    }
}
#if 0
void test() {
    using namespace nugget::properties;
    using namespace nugget::expressions;

    Rpn rpn;

    std::vector<Token> infixExpression = {
        {Token::Type::openBrace, "("},
        {Token::Type::integer, "8"},
        {Token::Type::minus, "-"},
        {Token::Type::integer, "3"},
        {Token::Type::closeBrace, ")"},
        {Token::Type::multiply, "*"},
        {Token::Type::openBrace, "("},
        {Token::Type::integer, "2"},
        {Token::Type::plus, "+"},
        {Token::Type::integer, "5"},
        {Token::Type::closeBrace, ")"},
        {Token::Type::divide, "/"},
        {Token::Type::integer, "4"},
    };

    rpn.InfixToPostfix(infixExpression);
    rpn.PrintPostfix();

}
#endif
