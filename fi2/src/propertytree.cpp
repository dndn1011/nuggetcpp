

#include <vector>
#include <string>
#include <iostream>
#include <coroutine>
#include <regex>
#include <unordered_map>
#include <map>
#include <functional>
#include <regex>
#include <array>
#include <assert.h>
#include <charconv>
#include <fstream>
#include <filesystem>
#include <format>

#include "propertytree.h"
#include "propertytreetokeniser.h"

#include "filereader.h"
#include "debug.h"

#include "expressions.h"

#include "identifier.h"
#include "notice.h"
#include "expressions.h"
#include "utils/TimeSampler.h"
#include "utils/StableVector.h"

//#pragma optimize("", off)

namespace nugget::properties {
    struct ConversionPair {
        ConversionPair(Token::Type from, Token::Type to) : from(from), to(to) {}
        ConversionPair(const ConversionPair& other) : from(other.from), to(other.to) {}
        bool operator==(const ConversionPair& b) const {
            return from == b.from && to == b.to;
        }
        Token::Type from;
        Token::Type to;
    };
}

//////////////////////////////////////////
namespace std {
    template <>
    struct hash<nugget::properties::ConversionPair> {
        std::size_t operator()(const nugget::properties::ConversionPair& cp) const {
            // Use a combination of hash values for 'from' and 'to'
            return hash<nugget::properties::Token::Type>{}(cp.from) ^ hash<nugget::properties::Token::Type>{}(cp.to);
        }
    };

    template <>
    struct equal_to<nugget::properties::ConversionPair> {
        bool operator()(const nugget::properties::ConversionPair& lhs, const nugget::properties::ConversionPair& rhs) const {
            return lhs == rhs; // Assuming you've already defined 'operator=='
        }
    };
}
//////////////////////////////////////////


namespace nugget::properties {
    using namespace identifier;
    using namespace expressions;
    struct GrammarParseData {
        ParseState& parseState;

        static int64_t ParseInteger(const std::string& str) {
            int64_t result;
            auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
            if (ec == std::errc()) {
                return result;
            } else {
                // Throw an exception for the error
                throw std::invalid_argument("Error parsing integer");
            }
        }

        static float ParseFloat(const std::string& str) {
            float result;
            if (str == "") {
                return 0.0f;
            } else {
                auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
                if (ec == std::errc()) {
                    return result;
                }
            }
            assert(0);
            return 0;
        }

        static std::string ParseString(const std::string& str) {
            // strip off beginning and end double quotes
            return str.substr(1, str.size() - 2);
        }

        static IDType ParseQualifiedName(const std::string& str) {
            return IDR(str);
        }

        std::unordered_map<ConversionPair, std::function<void(const std::string& from, const std::string& path)>> storers =
        {
            {
                {
                    Token::Type::integer, Token::Type::float_
                },
                [](const std::string& from, const std::string& path) {
                    auto fromval = ParseInteger(from);
                    Notice::Set(IDR(path), (float)fromval);
                },
            },
            {
                {
                    Token::Type::float_, Token::Type::dimension
                },
                [](const std::string& from, const std::string& path) {
                // raw float dimension
                auto fromval = ParseFloat(from);
                Notice::Set(IDR(path), nugget::ui::Dimension{ fromval,nugget::ui::Dimension::Units::none });
            },
        },
        {
            {
                Token::Type::integer, Token::Type::dimension
            },
            [](const std::string& from, const std::string& path) {
                // raw float dimension
                auto fromval = ParseInteger(from);
                Notice::Set(IDR(path), nugget::ui::Dimension{ (float)fromval,nugget::ui::Dimension::Units::none });
            },
        },
            {
                {
                    Token::Type::percent, Token::Type::dimension
                },
                [](const std::string& from, const std::string& path) {
                // raw float dimension
                auto fromval = ParseFloat(from.substr(0, from.size() - 1));
                Notice::Set(IDR(path), nugget::ui::Dimension{ (float)fromval,nugget::ui::Dimension::Units::percent });
            },
        },
        };

        std::unordered_map<std::string, std::function<void()>> objectInitialisers = {
            {
                "Color",[&]() {
                    assert(initaliserList.size() == 4);
                    float r = Expression::ConvertType(initaliserList[0], ValueAny::Type::float_).GetValueAsFloat();
                    float g = Expression::ConvertType(initaliserList[1], ValueAny::Type::float_).GetValueAsFloat();
                    float b = Expression::ConvertType(initaliserList[2], ValueAny::Type::float_).GetValueAsFloat();
                    float a = Expression::ConvertType(initaliserList[3], ValueAny::Type::float_).GetValueAsFloat();
                    IDType id = IDR(IDR(currentPathName), currentValueName);
                    Notice::Set(id,Color(r, g, b, a));
                },
            },
            {
                "ColorList",[&]() {
                    size_t size = initaliserList.size();
                    assert(size / 4 * 4 == size);
                    ColorList cols;
                    for (int i = 0; i < size; i += 4) {
                        auto v0 = initaliserList[i + 0];
                        auto v1 = initaliserList[i + 1];
                        auto v2 = initaliserList[i + 2];
                        auto v3 = initaliserList[i + 3];
                        cols.data.push_back(Color {
                            Expression::ConvertType(v0,ValueAny::Type::float_).GetValueAsFloat(),
                            Expression::ConvertType(v1,ValueAny::Type::float_).GetValueAsFloat(),
                            Expression::ConvertType(v2,ValueAny::Type::float_).GetValueAsFloat(),
                            Expression::ConvertType(v3,ValueAny::Type::float_).GetValueAsFloat()
                            });
                    }
                    IDType id = IDR(IDR(currentPathName), currentValueName);
                    Notice::Set(id, cols);
                },
            },
            {
                "Vector3f",[&]() {
                    assert(initaliserList.size() == 3);
                    float x = Expression::ConvertType(initaliserList[0], ValueAny::Type::float_).GetValueAsFloat();
                    float y = Expression::ConvertType(initaliserList[1], ValueAny::Type::float_).GetValueAsFloat();
                    float z = Expression::ConvertType(initaliserList[2], ValueAny::Type::float_).GetValueAsFloat();
                    IDType id = IDR(IDR(currentPathName), currentValueName);
                    Notice::Set(id,Vector3f(x, y, z));
                },
            },
            {
                "Vector3fList",[&]() {
                    size_t size = initaliserList.size();
                    assert(size / 3 * 3 == size);
                    Vector3fList verts;
                    for (int i = 0; i < size; i += 3) {
                        auto v0 = initaliserList[i + 0];
                        auto v1 = initaliserList[i + 1];
                        auto v2 = initaliserList[i + 2];
                        verts.data.push_back(Vector3f{
                            Expression::ConvertType(v0,ValueAny::Type::float_).GetValueAsFloat(),
                            Expression::ConvertType(v1,ValueAny::Type::float_).GetValueAsFloat(),
                            Expression::ConvertType(v2,ValueAny::Type::float_).GetValueAsFloat()
                            });
                    }
                    IDType id = IDR(IDR(currentPathName), currentValueName);
                    Notice::Set(id,verts);
                }
            },
            {
                "Vector2f",[&]() {
                    assert(initaliserList.size() == 3);
                    float x = Expression::ConvertType(initaliserList[0], ValueAny::Type::float_).GetValueAsFloat();
                    float y = Expression::ConvertType(initaliserList[1], ValueAny::Type::float_).GetValueAsFloat();
                    IDType id = IDR(IDR(currentPathName), currentValueName);
                    Notice::Set(id,Vector2f(x, y));
                }
            },
            {
                "Vector2fList",[&]() {
                    size_t size = initaliserList.size();
                    assert(size / 2 * 2 == size);
                    Vector2fList verts;
                    for (int i = 0; i < size; i += 2) {
                        auto v0 = initaliserList[i + 0];
                        auto v1 = initaliserList[i + 1];
                        verts.data.push_back(Vector2f{
                            Expression::ConvertType(v0,ValueAny::Type::float_).GetValueAsFloat(),
                            Expression::ConvertType(v1,ValueAny::Type::float_).GetValueAsFloat()
                            });
                    }
                    IDType id = IDR(IDR(currentPathName), currentValueName);
                    Notice::Set(id,verts);
                }
            }
        };
        
        [[nodiscard]] bool ParseInitialiserList() {
            // get the current type in order to know which parser to use
            if (currentTypeName == "") {
                // we need to check if the value already exists and we can get the type from that
                IDType id = IDR(IDR(currentPathName), currentValueName);
                if (Notice::KeyExists(id)) {
                    auto value = Notice::GetValueAny(id);
                    currentTypeName = Notice::GetValueTypeAsString(value);
                    assert(currentTypeName != "");
                } else {
                    parseState.description =
                        std::format("Do not have type information for initialiser list for {}", IDToString(id));
                    SetLineNumberToToken();
                    return false;
                }
            }
//            output("-----------------> {} {}\n", IDToString(IDR(IDR(currentPathName), currentValueName)), currentTypeName);
            check(objectInitialisers.contains(currentTypeName), "type not supported for initialiser list\n");
            objectInitialisers[currentTypeName]();

            initaliserList.SetSize(0);
            return true;
        }

        ValueAny ParseValue() {
            switch (currentValueType) {
                case Token::Type::integer: {
                    assert(currentTypeName == "" || currentTypeName == "int");
                    return ValueAny(ParseInteger(currentValue));
                } break;
                case Token::Type::float_: {
                    assert(currentTypeName == "" || currentTypeName == "float");
                    return ValueAny(ParseFloat(currentValue));
                } break;
                case Token::Type::string: {
                    assert(currentTypeName == "" || currentTypeName == "string");
                    return ValueAny(ParseString(currentValue));
                } break;
                case Token::Type::identifier: {
                    assert(currentTypeName == "");
                    return ValueAny(IDR(currentValue));
                } break;
                default:
                    assert(0);
            }
            return ValueAny();
        }
        void StoreValue() {
            std::string path = IDCombineStrings(currentPathName, currentValueName);
            ValueAny any = ParseValue();
            Notice::Set(IDR(path), any);
        }

        GrammarParseData(const std::string& rootName, std::vector<Token>& list, ParseState& parseState) :
            currentPathName(rootName), rootName(rootName), list(list), parseState(parseState) {
        }

        bool AtEnd() {
            return point >= endLength;
        }

        const Token &GetCurrentToken() {
            assert(point > 0);
            return list[point - 1];
        }

        bool NextToken() {
            point++;
            if (point > list.size()) {   // off end
                SetLineNumberToToken();
                parseState.description = std::format("unexpected end of input");
                return false;
            }
            return true;
        }

        Token::Type NextTokenType() {
            if (NextToken()) {
                auto& a = list[point - 1];
                return a.type;
            }
            else {
                return Token::Type::error;
            }
        }

        void NestWithCurrentBlock() {
            currentPathName = IDCombineStrings(currentPathName, currentBlockName);
            IDType id = identifier::Register(currentPathName);
            if (nestLevel > 0) {   // we do not bother counting the children of the root
                childCounts.back()++;
                Notice::Set(IDR(id, IDR("_seq")), childCounts.back());
            }
            Notice::SetVoid(id);
            nestLevel++;
            childCounts.push_back(0);
        }

        void SetLineNumberToToken() {
            parseState.lineNumber = list[point - 1].lineNumber;
        }

        [[nodiscard]] bool UnNest() {
            if (nestLevel > 0) {
                currentPathName = IDRemoveLeaf(currentPathName);
                nestLevel--;
                childCounts.pop_back();
                return true;
            } else {
                parseState.description = std::format("extra close brace");
                SetLineNumberToToken();
                return false;
            }
        }

        void SetCurrentValueName() {
            currentValueName = list[point - 1].text;
        }

        void SetUniqueName() {
            currentValueName = std::format("__anon_{}", uniqueNameCounter);
            uniqueNameCounter++;
        }

        void AddValueToInitialiserList(const ValueAny &val) {
            initaliserList.emplace_back(val);
        }

        void ClearType() {
            currentPossibleType = currentTypeName = "";
            currentValueType = Token::Type::unset;
        }
        void ClearCurrentDerivedName() {
            currentDerivedName = "";
        }
        void ExtendCurrentValue() {
            currentValue += list[point - 1].text;
        }
        void ExtendCurrentValue(const std::string &str) {
            currentValue += str;
        }
        [[nodiscard]] bool ConfirmCurrentType() {
            currentTypeName = currentPossibleType;
            currentValueType = Token::ValueTypeFromString(currentPossibleType);
            if (currentValueType == Token::Type::error) {
                SetLineNumberToToken();
                return false;
            } else {
                return true;
            }
        }
        void SetCurrentDerivationOrType() {
            currentDerivedName = IDCombineStrings(rootName, list[point - 1].text);
            currentPossibleType = list[point - 1].text;
        }
        void SetCurrentValue() {
            currentValue = list[point - 1].text;
        }

        void BlockNameFromValueName() {
            currentBlockName = currentValueName;
        }

        bool CurrentDerivedNameSet() {
            return currentDerivedName != "";
        }

        Token::Type GetTypeOfValue(IDType id) {
            if (Notice::IsValueTypeInteger32(id)) return Token::Type::integer;
            if (Notice::IsValueTypeInteger64(id)) return Token::Type::integer;
            if (Notice::IsValueTypeFloat(id)) return Token::Type::float_;
            if (Notice::IsValueTypeString(id)) return Token::Type::string;
            if (Notice::IsValueTypeIdentifier(id)) return Token::Type::identifier;
            if (Notice::IsValueTypeColor(id)) return Token::Type::Color;
            if (Notice::IsValueTypeDimension(id)) return Token::Type::dimension;
            assert(0);
            return Token::Type::unset;
        }
        
    std::unordered_map<IDType, std::function<nugget::ValueAny(IDType id)>> expandParseVars = {
        {
            ID("here"),
            [&](IDType id) {
                return ValueAny(IDR(currentPathName));
            }
        }
    };

    ValueAny ExpandParseVariable(IDType id) {
        if (expandParseVars.contains(id)) {
            return expandParseVars.at(id)(id);
        } else {
            if (parseVariables.contains(id)) {
                return parseVariables.at(id);
            }
        }
        return ValueAny(Exception{ .description = std::format("Parse variable not set: {}\n",IDToString(id)) });
        return {};
    }

    ValueAny ParseInitialiserExpressionToValue() {
        Expression ex;
        while (GetCurrentToken().type != Token::Type::comma && GetCurrentToken().type != Token::Type::closeBrace) {
            ex.AddToken(GetCurrentToken());
            if (!NextToken()) {
                return ValueAny(Exception{ .description = std::format("Could not parse initialiser expression\n") });
            }
        }
        // must not consume the next token, "push back"
        point--;
        return ex.Evaluate([&](IDType id) {
            return ExpandParseVariable(id);
            }
        );
    }

    bool ParseExpression() {
        Expression ex;
        while (GetCurrentToken().type != Token::Type::semicolon) {
            ex.AddToken(GetCurrentToken());
            if (!NextToken()) {
                return false;
            }
        }
        ValueAny any = ex.Evaluate([&](IDType id) {
            return ExpandParseVariable(id);
            }
        );
        if (any.IsException()) {
            auto e = any.GetValueAsException();
            parseState.description = std::format("Could not evaluate expression: {}", e.description);
            return false;
        }
        std::string path = IDCombineStrings(currentPathName, currentValueName);
        IDType toid = IDR(path);
        Notice::Set<ValueAny>(toid, any);
        return true;
    }

    ValueAny ParseExpressionToValue() {
        Expression ex;
        while (GetCurrentToken().type != Token::Type::semicolon) {
            ex.AddToken(GetCurrentToken());
            if (!NextToken()) {
                return {};
            }
        }
        return ex.Evaluate([&](IDType id) {
            return ExpandParseVariable(id);
            }
        );
    }
#if 0
        [[nodiscard]] bool ParseAssignment() {
            currentValue = "";
            for (;;) {
                if (currentValue == "@") {
                    point++;
                    if (point >= list.size()) {   // off end
                        SetLineNumberToToken();
                        parseState.description = std::format("unexpected end of input");
                        return false;
                    }
                    ExtendCurrentValue(parseVariables.at(list[point - 1].text));
                }
                if (point >= list.size()) {   // off end
                    SetLineNumberToToken();
                    parseState.description = std::format("unexpected end of input");
                    return false;
                }
                if (list[point].type == Token::Type::doubleColon) {
                    point++;
                    ExtendCurrentValue();
                }
                else if (list[point].type == Token::Type::doubleColon) {
                    point++;
                    ExtendCurrentValue();
                }
                else {
                    break;
                }
                if (point >= list.size()) {   // off end
                    SetLineNumberToToken();
                    parseState.description = std::format("unexpected end of input");
                    return false;
                }
                if (list[point].type == Token::Type::identifier) {
                    point++;
                    ExtendCurrentValue();
                }
                else {
                    SetLineNumberToToken();
                    parseState.description = std::format("malformed qualified name");
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] bool TokenIsAssignment() {
            std::string path = IDCombineStrings(currentPathName, currentValueName);
            if (!ParseAssignment()) {
                return false;
            }

            if (currentValueType == Token::Type::unset) {
                // we don't know the type from the syntax, check if data already exists and has a type
                if (Notice::KeyExists(IDR(path))) {
                    currentValueType = GetTypeOfValue(IDR(path));
                } else {
                    currentValueType = list[point - 1].type;
                }
            }
            if (currentValueType != list[point - 1].type) {
                if (storers.contains({ list[point - 1].type, currentValueType })) {
                    auto& storer = storers[{ list[point - 1].type, currentValueType }];
                    storer(currentValue, path);
                } else {
                    SetLineNumberToToken();
                    parseState.description = std::format("type mismatch in assignment: cannot assign {} to {}",
                        list[point - 1].TypeAsString(), Token::TypeAsString(currentValueType));
                    return false;
                }
            } else {
                if (parseVariableMode) {
                    SetCurrentValue();
                    parseVariables[currentValueName] = currentValue;
                    parseVariableMode = false;
                } else {
                    StoreValue();
                }
            }
            return true;
        }
        bool NextTokenIsAssignment() {
            NextTokenType();
            return TokenIsAssignment();
        }
#endif

        [[nodiscard]] bool ExpectSemicolon() {
            if (NextTokenType() != Token::Type::semicolon) {
                SetLineNumberToToken();
                parseState.description = std::format("Expected semicolon, got {}", GetCurrentToken().TypeAsString());
                return false;
            } else {
                return true;
            }
        }


        // expects a block nodes
        bool CopyNodes(const std::string& fromstr, const std::string& tostr) {
            IDType fromId = IDR(fromstr);
            IDType toId = IDR(tostr);
            std::vector<IDType> children;
            Notice::SetVoid(toId);

            if (auto r = !Notice::GetChildren(fromId, children /*fill*/)) {
                parseState.description = std::format("The class to Derive from could not be found or is empty: {}", fromstr);
                SetLineNumberToToken();
                return false;
            }

            std::string toPath;
            std::string fromPath;

            for (auto& fromPathId : children) {
                fromPath = IDToString(fromPathId);
                auto leaf = IDKeepLeaf(IDToString(fromPathId));
                IDCombineStringsInPlace(tostr, leaf.data(), toPath);

                auto value = Notice::GetValueAny(fromPathId);
                if (!value.IsVoid()) {
                    auto& fromValue = Notice::GetValueAny(IDR(fromPath));
                    Notice::Set(IDR(toPath), fromValue);
//                    output("Inherit value {}->{}\n", fromPath, toPath);
                } else {
                    auto r = CopyNodes(IDCombineStrings(fromstr, leaf.data()), IDCombineStrings(tostr, leaf.data()));
                    if (!r) {
                        return false;
                    }
                }
            }
            return true;
        }

        bool CopyDerivation() {
//            output("Inherit {}, {}\n", currentDerivedName, currentPathName);
            if (CurrentDerivedNameSet()) {
                return CopyNodes(currentDerivedName, currentPathName);
            }
            return true;
        }

        void ClearLiteralLines() {
            literalLines.clear();
        }

        void AddLiteralLine() {
            literalLines.push_back(list[point - 1].text);
        }

        void AssignLiteralLines() {
            std::stringstream ss;
            for (auto&& x : literalLines) {
                ss << x;
            }
            currentValue = ss.str();
            currentValueType = Token::Type::string;
            currentTypeName = "string";
            StoreValue();
        }
        const std::string& GetCurrentTypeName() {
            return currentTypeName;
        }
        const std::string& GetCurrentValueName() {
            return currentValueName;
        }

        std::unordered_map<IDType, ValueAny> parseVariables;
    private:
        std::vector<Token>& list;
        std::string currentPathName = rootName;
        std::string rootName;

        size_t point = 0;
        size_t endLength = list.size();
        size_t nestLevel = 0;
        std::string currentValueName;
        std::string currentTypeName;
        std::string currentBlockName;
        std::string currentValue;
        std::string currentDerivedName;
        std::string currentPossibleType;
        Token::Type currentValueType = Token::Type::unset;
        std::vector<int64_t> childCounts;

        size_t uniqueNameCounter = 0;


        StableVector<ValueAny,100> initaliserList;
        std::vector<std::string> literalLines;
    };
}

namespace nugget::properties {
    bool GrammarParse(std::string rootName, std::vector<Token>& list,ParseState &parseState) {
        Notice::SetVoid(IDR(rootName));
        GrammarParseData pdata(rootName, list,parseState);
        while (!pdata.AtEnd()) {
            /////////////////////////////////////////////
        expectInitialisation__:
            pdata.ClearCurrentDerivedName();
            switch (pdata.NextTokenType()) {
                /////////////////////////////////////////////
                case Token::Type::identifier: {
                    pdata.SetCurrentValueName();
                    switch (pdata.NextTokenType()) {
                        /////////////////////////////////////////////
                        case Token::Type::equals: {
                            pdata.ClearType();
                            goto assignmentAfterType__;
                        } break;
                            /////////////////////////////////////////////
                        case Token::Type::colon: {
                        nameOfNodeIsSet__:
                            switch (pdata.NextTokenType()) {
                                /////////////////////////////////////////////
                                case Token::Type::equals: {
                                    goto assignmentAfterType__;
                                } break;
                                case Token::Type::identifier: {
                                    pdata.SetCurrentDerivationOrType();
                                    switch (pdata.NextTokenType()) {
                                        /////////////////////////////////////////////
                                        case Token::Type::equals: {
                                            if (!pdata.ConfirmCurrentType()) {
                                                output("vvvvvvvvvvvvvvvvvvv\n");
                                                pdata.SetLineNumberToToken(); 
                                                parseState.description = std::format("Type error, probably unknown: {}", pdata.GetCurrentTypeName());
                                                return false;
                                            }
                                        assignmentAfterType__:
                                            switch (pdata.NextTokenType()) {
                                                case Token::Type::openBrace: {
                                                    // object initialiser
                                                expectInitialiserValue__:
                                                    pdata.NextToken();
                                                    if (pdata.GetCurrentToken().type == Token::Type::closeBrace) {
                                                        // blank value, ignore
                                                        goto endOfInitialiser__;
                                                    } else {
                                                        ValueAny any = pdata.ParseInitialiserExpressionToValue();
                                                        if (any.IsException()) {
                                                            return false;
                                                        }
                                                        pdata.AddValueToInitialiserList(any);
                                                    }
#if 0
                                                    switch (pdata.NextTokenType()) {
                                                        case Token::Type::integer: {
                                                            pdata.AddValueToInitialiserList();
                                                        } break;
                                                        case Token::Type::float_: {
                                                            pdata.AddValueToInitialiserList();
                                                        } break; 
                                                        default: {
                                                            output("vvvvvvvvvvvvvvvvvvv\n");
                                                            parseState.description = std::format("Unexpected token: {} {}", pdata.GetCurrentToken().TypeAsString(), pdata.GetCurrentToken().text);
                                                            return false;
                                                        }
                                                    }
#endif
                                                    switch (pdata.NextTokenType()) {
                                                        case Token::Type::comma: {
                                                            goto expectInitialiserValue__;
                                                        } break;
                                                        case Token::Type::closeBrace: {
                                                            endOfInitialiser__:
                                                            if (!pdata.ParseInitialiserList()) {
                                                                return false;
                                                            }
                                                            if (!pdata.ExpectSemicolon()) {
                                                                pdata.SetLineNumberToToken();
                                                                output("vvvvvvvvvvvvvvvvvvv\n");
                                                                return false;
                                                            }
                                                            goto expectInitialisation__;
                                                        } break;
                                                        default: {
                                                            output("vvvvvvvvvvvvvvvvvvv\n");
                                                            pdata.SetLineNumberToToken(); 
                                                            parseState.description = std::format("Unexpected token: {}", pdata.GetCurrentToken().TypeAsString());
                                                            return false;
                                                        }
                                                    }
                                                } break;
                                                default: {
                                                    ///////////////////////////////////////////////
                                                    // assume expression, note this consumes the ending semicolon
                                                    if (!pdata.ParseExpression()) {
                                                        output("vvvvvvvvvvvvvvvvvvv\n");
                                                        pdata.SetLineNumberToToken(); 
                                                        return false;
                                                    }
                                                    goto expectInitialisation__;
                                                }
                                            }
                                        } break;
                                            /////////////////////////////////////////////
                                        case Token::Type::openBrace: {
                                            pdata.BlockNameFromValueName();
                                            pdata.NestWithCurrentBlock();
                                            if (!pdata.CopyDerivation()) {
                                                return false;
                                            }
                                            goto expectInitialisation__;
                                        } break;
                                            /////////////////////////////////////////////
                                        case Token::Type::semicolon: {  // derive as is
                                            pdata.BlockNameFromValueName();
                                            pdata.NestWithCurrentBlock();
                                            auto r = pdata.CopyDerivation();
                                            if (!r) {
                                                return false;
                                            }
                                            auto ok = pdata.UnNest();
                                            if (!ok) {
                                                return false;
                                            }
                                            goto expectInitialisation__;
                                        } break;
                                        default:
                                            output("vvvvvvvvvvvvvvvvvvv\n");
                                            pdata.SetLineNumberToToken();
                                            parseState.description = std::format("Unexpected token: {}", pdata.GetCurrentToken().TypeAsString());
                                            return false;
                                    }
                                } break;
                                default:
                                    output("vvvvvvvvvvvvvvvvvvv\n");
                                    pdata.SetLineNumberToToken();
                                    parseState.description = std::format("Unexpected token: {}", pdata.GetCurrentToken().TypeAsString());
                                    return false;
                            }
                        } break;
                            /////////////////////////////////////////////
                        case Token::Type::openBrace: {
                            pdata.BlockNameFromValueName();
                            pdata.NestWithCurrentBlock();
                            goto expectInitialisation__;
                        } break;
                        case Token::Type::doubleLess: {
                            pdata.ClearLiteralLines();
                        nextLiteralLine__:
                            switch (pdata.NextTokenType()) {
                                case Token::Type::literalLine: {
                                    pdata.AddLiteralLine();
                                    goto nextLiteralLine__;
                                } break;
                                case Token::Type::doubleMore: {
                                } break;
                                default: {
                                    assert(0);
                                }
                            }
                            if (!pdata.ExpectSemicolon()) {
                                output("vvvvvvvvvvvvvvvvvvv\n");
                                return false;
                            }
                            pdata.AssignLiteralLines();
                        } break;
                        default:
                            output("vvvvvvvvvvvvvvvvvvv\n");
                            parseState.description = std::format("Unexpected token: {}", pdata.GetCurrentToken().TypeAsString());
                            return false;
                    }
                } break;
                    /////////////////////////////////////////////
                case Token::Type::closeBrace: {
                    //output("Nesting - 1\n");
                    if (!pdata.UnNest()) {
                        return false;
                    }
                } break;
                case Token::Type::colon: {
                    pdata.SetUniqueName();
                    goto nameOfNodeIsSet__;
                } break;
                case Token::Type::at: {
                    switch (pdata.NextTokenType()) {
                        case Token::Type::identifier: {
                            pdata.SetCurrentValueName();
                            switch (pdata.NextTokenType()) {
                                case Token::Type::equals: {
                                    pdata.NextToken();
                                    pdata.parseVariables[IDR(pdata.GetCurrentValueName())] = pdata.ParseExpressionToValue();
                                } break;
                                default: {
                                    output("vvvvvvvvvvvvvvvvvvv\n");
                                    parseState.description = std::format("Unexpected token: {} {}", pdata.GetCurrentToken().TypeAsString(), pdata.GetCurrentToken().text);
                                    return false;
                                }
                            }
                        } break;
                        default: {
                            output("vvvvvvvvvvvvvvvvvvv\n");
                            parseState.description = std::format("Unexpected token: {} {}", pdata.GetCurrentToken().TypeAsString(), pdata.GetCurrentToken().text);
                            return false;
                        }
                    }
                } break;
                case Token::Type::eof: {
                    // we are done, correctly
                    return true;
                } break;
                case Token::Type::error: {
                    output("vvvvvvvvvvvvvvvvvvv\n");
                    pdata.SetLineNumberToToken();
                    parseState.description = std::format("could not get next token, eof?");
                    return false;
                } break;
                default:
                    output("vvvvvvvvvvvvvvvvvvv\n");
                    pdata.SetLineNumberToToken();
                    parseState.description = std::format("Unexpected token: {}", pdata.GetCurrentToken().TypeAsString());
                    return false;
            }
        }
        return true;
    }

    ParseState LoadPropertyTree(const std::string &where,const std::string filename) {
        ParseState parseState;
        auto t = TimeSampler("parse", true);

        output("LoadPropertyTree: START\n");
        FileReader fileReader(filename);
        fileReader.Open();
        std::vector<Token> tokenList;
        auto tokeniseOK = Tokenise([&fileReader](size_t point) {
            std::string s = fileReader.readBytesFromPosition(point);
            return s;
            }, parseState, tokenList);

        if (!tokeniseOK) {
            return parseState;
        }

        auto r = GrammarParse(where, tokenList, parseState);
        if (!r) {
            return parseState;
        }

        output("LoadPropertyTree: END\n");


        parseState.successful = true;

        return parseState;
    }

}
