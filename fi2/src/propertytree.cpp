#pragma optimize("", off)

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

#define NOTICE_INCLUDE_VARIANTS
#include "identifier.h"
#include "notice.h"

#include "utils/TimeSampler.h"

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
                    Token::Type::qualifiedName, Token::Type::identifier
                },
                [](const std::string& from, const std::string& path) {
                    Notice::Set(IDR(path), IDR(from));
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
                        std::string rs = initaliserList[0];
                        std::string gs = initaliserList[1];
                        std::string bs = initaliserList[2];
                        std::string as = initaliserList[3];
                        float r = ParseFloat(rs);
                        float g = ParseFloat(gs);
                        float b = ParseFloat(bs);
                        float a = ParseFloat(as);
                        IDType id = IDR(IDR(currentPathName), currentValueName);
                        Notice::Set(id,Color(r, g, b, a));
                    },
                },
                {
                    "Vector3fList",[&]() {
                        size_t size = initaliserList.size();
                        assert(size / 3 * 3 == size);
                        Vector3fList verts;
                        for (int i = 0; i < size; i += 3) {
                            auto v0 = ParseFloat(initaliserList[i+0]);
                            auto v1 = ParseFloat(initaliserList[i+1]);
                            auto v2 = ParseFloat(initaliserList[i+2]);
                            verts.data.push_back(Vector3f{v0,v1,v2});
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
                    parseState.description = std::format("Do not have type information for initialiser list");
                    SetLineNumberToToken();
                    return false;
                }
            }
            output("-----------------> {}\n", currentTypeName);
            check(objectInitialisers.contains(currentTypeName), "type not supported for initialiser list\n");
            objectInitialisers[currentTypeName]();

            initaliserList.clear();
            return true;
        }

        void StoreValueImp() {
            switch (currentValueType) {
                case Token::Type::integer: {
                    assert(currentTypeName == "" || currentTypeName == "int");
                    std::string path = IDCombineStrings(currentPathName, currentValueName);
                    Notice::Set(IDR(path), ParseInteger(currentValue));
                } break;
                case Token::Type::float_: {
                    assert(currentTypeName == "" || currentTypeName == "float");
                    std::string path = IDCombineStrings(currentPathName, currentValueName);
                    Notice::Set(IDR(path), ParseFloat(currentValue));
                } break;
                case Token::Type::string: {
                    assert(currentTypeName == "" || currentTypeName == "string");
                    std::string path = IDCombineStrings(currentPathName, currentValueName);
                    Notice::Set(IDR(path), ParseString(currentValue));
                } break;
                case Token::Type::qualifiedName: {
                    assert(currentTypeName == "");
                    std::string path = IDCombineStrings(currentPathName, currentValueName);
                    Notice::Set(IDR(path), ParseQualifiedName(currentValue));
                } break;
                case Token::Type::identifier: {
                    assert(currentTypeName == "");
                    std::string path = IDCombineStrings(currentPathName, currentValueName);
                    Notice::Set(IDR(path), IDR(currentValue));
                } break;
                default:
                    assert(0);
            }
        }


        GrammarParseData(const std::string& rootName, std::vector<Token>& list, ParseState& parseState) :
            currentPathName(rootName), rootName(rootName), list(list), parseState(parseState) {
        }
        bool AtEnd() {
            return point >= endLength;
        }
        Token::Type TokenType() {
            assert(point > 0);
            return list[point - 1].type;
        }
        Token GetCurrentToken() {
            assert(point > 0);
            return list[point - 1];
        }
        Token::Type NextTokenType() {
            auto& a = list[point++];
            //output("New Token: %s->%s<-\n", a.TypeAsString().c_str(), a.text.c_str());
            return a.type;
        }
        void SetCurrentBlockName() {
            currentBlockName = list[point - 1].text;
        }
        void Next() {
            point++;
        }

        size_t GetChildCount() {
            return childCounts.back();
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

        void AddValueToInitialiserList() {
            initaliserList.push_back(list[point - 1].text);
        }

        void ClearType() {
            currentPossibleType = currentTypeName = "";
            currentValueType = Token::Type::unset;
        }
        void ClearCurrentDerivedName() {
            currentDerivedName = "";
        }
        void SetCurrentDerivedName() {
            currentDerivedName = IDCombineStrings(rootName, list[point - 1].text);
        }
        [[nodiscard]] bool SetCurrentType() {
            currentValueType = Token::ValueTypeFromString(list[point - 1].text);
            if (currentValueType == Token::Type::error) {
                SetLineNumberToToken();
                return false;
            } else {
                return true;
            }
        }
        void SetCurrentType(Token::Type type) {
            currentValueType = type;
        }
        void ExtendCurrentValue() {
            currentValue += list[point - 1].text;
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
        void StoreValue() {
            StoreValueImp();
        }

        void BlockNameFromValueName() {
            currentBlockName = currentValueName;
        }

        void ClearValue() {
            currentValue = "";
        }

        const bool GetChildrenOfDerivation(std::vector<IDType>& children) {
            auto r = Notice::GetChildren(IDR(currentDerivedName), children /*fill*/);
            assert(r);
            return r;
        }
        bool CurrentDerivedNameSet() {
            return currentDerivedName != "";
        }
        const std::string& GetCurrentDerivedName() {
            return currentDerivedName;
        }

        const std::string& GetCurrentPathName() {
            return currentPathName;
        }

        bool IsAtRoot() {
            return nestLevel == 0;
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

        [[nodiscard]] bool TokenIsAssignment() {
            SetCurrentValue();
            // check for qualified names
            for (;;) {
                if (point >= list.size()) {   // off end
                    SetLineNumberToToken();
                    parseState.description = std::format("unexpected end of input");
                    return false;
                }
                if (list[point].type == Token::Type::doubleColon) {
                    point++;
                    ExtendCurrentValue();
                } else {
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
                } else {
                    SetLineNumberToToken();
                    parseState.description = std::format("malformed qualified name");
                    return false;
                }
            }
            std::string path = IDCombineStrings(currentPathName, currentValueName);
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
                StoreValue();
            }
            return true;
        }
        bool NextTokenIsAssignment() {
            NextTokenType();
            return TokenIsAssignment();
        }

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
                auto leaf = IDKeepLeafCStr(IDToString(fromPathId));
                IDCombineStringsInPlace(tostr, leaf, toPath);

                auto value = Notice::GetValueAny(fromPathId);
                if (!value.IsVoid()) {
                    auto& fromValue = Notice::GetValueAny(IDR(fromPath));
                    Notice::Set(IDR(toPath), fromValue);
                } else {
                    auto r = CopyNodes(IDCombineStrings(fromstr, leaf), IDCombineStrings(tostr, leaf));
                    if (!r) {
                        return false;
                    }
                }
            }
            return true;
        }

        bool CopyDerivation() {
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

        std::vector<std::string> initaliserList;
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
                                case Token::Type::identifier: {
                                    pdata.SetCurrentDerivationOrType();
                                    switch (pdata.NextTokenType()) {
                                        /////////////////////////////////////////////
                                        case Token::Type::equals: {
                                            if (!pdata.ConfirmCurrentType()) {
                                                output("vvvvvvvvvvvvvvvvvvv\n");
                                                parseState.description = std::format("Type error, probably unknown: {}", pdata.GetCurrentTypeName());
                                                return false;
                                            }
                                        assignmentAfterType__:
                                            switch (pdata.NextTokenType()) {
                                                case Token::Type::openBrace: {
                                                    // object initialiser
                                                expectInitialiserValue__:
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
                                                    switch (pdata.NextTokenType()) {
                                                        case Token::Type::comma: {
                                                            goto expectInitialiserValue__;
                                                        } break;
                                                        case Token::Type::closeBrace: {
                                                            if (!pdata.ParseInitialiserList()) {
                                                                return false;
                                                            }
                                                            if (!pdata.ExpectSemicolon()) {
                                                                output("vvvvvvvvvvvvvvvvvvv\n");
                                                                return false;
                                                            }
                                                            goto expectInitialisation__;
                                                        } break;
                                                        default: {
                                                            output("vvvvvvvvvvvvvvvvvvv\n");
                                                            parseState.description = std::format("Unexpected token: {}", pdata.GetCurrentToken().TypeAsString());
                                                            return false;
                                                        }
                                                    }
                                                } break;
                                                default: {
                                                    if (!pdata.TokenIsAssignment()) {
                                                        return false;
                                                    }
                                                    if (!pdata.ExpectSemicolon()) {
                                                        output("vvvvvvvvvvvvvvvvvvv\n");
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
                case Token::Type::eof: {
                    // we are done, correctly
                    return true;
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
