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
            case Token::Type::colon:
                return 100;
            default:
                return 0;
            }
        }

        int IsValue(Token::Type type) {
            switch (type) {
            case Token::Type::integer:
            case Token::Type::float_:
            case Token::Type::identifier:
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
            for (const auto& token : input) {
                switch (token.type) {
                    case Token::Type::integer:
                    case Token::Type::float_:
                    case Token::Type::identifier:
                    case Token::Type::string: {
                        output.push_back(token);
                    } break;
                    case Token::Type::colon:
                    case Token::Type::comma:
                    case Token::Type::plus:
                    case Token::Type::minus:
                    case Token::Type::multiply:
                    case Token::Type::divide: {
                        ProcessOperator(token);
                    } break;
                    case Token::Type::openParen: {
                        operatorStack.push(token);
                    } break;
                    case Token::Type::closeParen: {
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

        static inline std::unordered_map<IDType, std::function<ValueAny(const std::span<ValueAny>& args)>> creators = {
            {
                ID("Color"),
                [](const std::span<ValueAny>& args) {
                    check(args.size() == 4,"Incorrect number of arguments");
                    float r = Expression::ConvertType(args[0], ValueAny::Type::float_).GetValueAsFloat();
                    float g = Expression::ConvertType(args[1], ValueAny::Type::float_).GetValueAsFloat();
                    float b = Expression::ConvertType(args[2], ValueAny::Type::float_).GetValueAsFloat();
                    float a = Expression::ConvertType(args[3], ValueAny::Type::float_).GetValueAsFloat();
                    return ValueAny(Color(r, g, b, a));
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
                    return ValueAny((float)from.GetValueAsInt64());
                },
            },
            {
                {
                    ValueAny::Type::float_, ValueAny::Type::dimension
                },
                [](const ValueAny& from) {
                    // raw float dimension
                    return ValueAny(Dimension(from.GetValueAsFloat()));
                }
            },
            {
                {
                    ValueAny::Type::int64_t_, ValueAny::Type::dimension
                },
                [](const ValueAny& from) {
                    return ValueAny(Dimension((float)from.GetValueAsInt64()));
                }
            },
            {
                {
                    ValueAny::Type::int64_t_, ValueAny::Type::string
                },
                [](const ValueAny& from) {
                    return ValueAny(std::format("{}",from.GetValueAsInt64()));
                }
            },
        };

        ValueAny ParseToken(const Token& token) {
            switch (token.type) {
                case Token::Type::identifier: {
                    return ValueAny(IDR(token.text));
                } break;
                case Token::Type::dimension: {
                    check(0, "not implemented");
                } break;
                case Token::Type::float_: {
                    float val;
                    if (ParseFloat(token.text, val)) {
                        return ValueAny(val);
                    }
                    else {
                        check(0, "could not parse integer");
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
                check(0, "cannot convert {} to {}\n", a.GetTypeAsString(), ValueAny::GetTypeAsString(type));
            }
            return {};
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

#define EXPR_OPERATOR +
#define EXPR_NAME Plus
#define EXPR_OP EXPR_PLUS
#include "expressions_operators.h"

#define EXPR_OPERATOR *
#define EXPR_NAME Multiply
#define EXPR_OP EXPR_MULTIPLY
#include "expressions_operators.h"

#define EXPR_OPERATOR /
#define EXPR_NAME Divide
#define EXPR_OP EXPR_DIVIDE
#include "expressions_operators.h"

        ValueAny EvalRpn() {
            size_t point = 0; 
            size_t outSize = output.size();
            StableVector<ValueAny, 10> accumulation;

            while (point < outSize) {
                switch (output[point].type) {
                    case Token::Type::identifier: {
                        const ValueAny& v = ParseToken(output[point++]);
                        accumulation.emplace_back(v);
                    } break;
                    case Token::Type::integer: {
                        const ValueAny& v = ParseToken(output[point++]);
                        accumulation.emplace_back(v);
                    } break;
                    case Token::Type::string: {
                        const ValueAny& v = ParseToken(output[point++]);
                        accumulation.emplace_back(v);
                    } break;
                    case Token::Type::float_: {
                        const ValueAny& v = ParseToken(output[point++]);
                        accumulation.emplace_back(v);
                    } break;
                    case Token::Type::colon: {
                        IDType className = accumulation[0].GetValueAsIDType();
                        if (!creators.contains(className)) {
                            check(0, "No constructor available for this class");
                        } else {
                            ValueAny any = creators.at(className)(accumulation.GetArrayFrom(1));
                            accumulation.SetSize(0);
                            accumulation.emplace_back(any);
                            point++;
                        }
                    } break;
                    case Token::Type::comma: {
                        point++;
                    } break;
                    case Token::Type::plus: {
                        const auto &r = Plus(accumulation.GetArrayLast(2));
                        accumulation.Pop(2);
                        accumulation.emplace_back(r);
                        point++;
                    } break;
                    case Token::Type::multiply: {
                        const auto& r = Multiply(accumulation.GetArrayLast(2));
                        accumulation.Pop(2);
                        accumulation.emplace_back(r);
                        point++;
                    } break;
                    case Token::Type::divide: {
                        const auto& r = Divide(accumulation.GetArrayLast(2));
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

        ValueAny Evaluate() {
            InfixToPostfix();
            return EvalRpn();
        }

        void PrintPostfix() {
            for (const auto& token : output) {
                std::cout << token.text << " ";
            }
            std::cout << std::endl;
        }
    };

    struct Expression_imp {
        Rpn rpn;
    };

    Expression::Expression() : imp(*new Expression_imp()) {
    }
    void Expression::AddToken(const Token& token) {
        imp.rpn.AddToken(token);
    }

    ValueAny Expression::Evaluate() {
        return imp.rpn.Evaluate();
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
