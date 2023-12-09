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
#include "filereader.h"
#include "debug.h"

#define NOTICE_INCLUDE_VARIANTS
#include "identifier.h"
#include "notice.h"

#include "../utils/TimeSampler.h"


namespace nugget::properties {
    using namespace identifier;

    struct KeyValue {
        std::string key;
        std::string type;
        std::string value;
    };

    struct IdentifierBlock {
        std::string identifier;
        std::string parent;
        std::vector<KeyValue> keyValues;
    };

    template <typename T>
    struct Generator {
        struct promise_type;
        using handle_type = std::coroutine_handle<promise_type>;
        handle_type h;

        explicit Generator(handle_type h) : h(h) {}

        ~Generator() {
            if (h)
                h.destroy();
        }

        struct promise_type {
            T current_value;
            std::coroutine_handle<> waiting_coroutine;

            auto initial_suspend() { return std::suspend_always{}; }
            auto final_suspend() noexcept { return std::suspend_always{}; }

            auto get_return_object() {
                return Generator{ handle_type::from_promise(*this) };
            }

            void return_void() {
                std::suspend_never{};
            }

            auto yield_value(T value) {
                current_value = value;
                return yield();
            }

            auto yield() {
                waiting_coroutine = std::coroutine_handle<promise_type>::from_promise(*this);
                return std::suspend_always{};
            }

            void unhandled_exception() {
                std::terminate();
            }

            auto await_ready() const {
                return waiting_coroutine == nullptr;
            }

            auto await_resume() const {
                return current_value;
            }

            void await_suspend(std::coroutine_handle<> coroutine) {
                waiting_coroutine = coroutine;
            }
        };

        auto next() {
            h.resume();
            return h.promise().current_value;
        }

        bool has_next() const {
            return !h.done();
        }
    };

    Generator<KeyValue> parseKeyValues(std::string_view input) {
        // Implement parsing logic here
        KeyValue kv;
        kv.key = "key";
        kv.type = "type";
        kv.value = "value";
        co_yield kv;
    }

    Generator<IdentifierBlock> parseIdentifierBlock(std::string_view input) {
        // Implement parsing logic here
        IdentifierBlock identifierBlock;
        identifierBlock.identifier = "my_identifier";
        identifierBlock.parent = "parent_identifier";
        co_yield identifierBlock;
    }

    class Token {
    public:
        enum class Type {
#define __PTREE_TOKEN_ENUMDEF
#include "ptree_tokens.h"
            size,
        };

        enum Type type;
        std::string text;
        size_t lineNumber=0;

        static std::unordered_map<enum Type, std::string>typeToString;
        static std::unordered_map<std::string, enum Token::Type> valueTypeFromString;

        static Token::Type ValueTypeFromString(std::string str) {
            if (valueTypeFromString.contains(str)) {
                return valueTypeFromString[str];
            } else {
                return Token::Type::error;
            }
        }

        std::string TypeAsString() {
            return typeToString[type];
        }

        static std::string TypeAsString(Type type) {
            return typeToString[type];
        }

        bool isError() {
            return type == Type::error;
        }
        bool isEof() {
            return type == Type::eof;
        }
    };

    /*static*/
    std::unordered_map<enum Token::Type, std::string> Token::typeToString = {
    #define __PTREE_TOKEN_MAPDEF
    #include "ptree_tokens.h"
    };

    std::unordered_map<std::string, enum Token::Type> Token::valueTypeFromString = {
        {"int",Type::integer},
        {"float",Type::float_},
        {"string",Type::string},
        {"Color",Type::Color},
        {"dimension",Type::dimension}
    };

    static std::vector<std::string> parserRegexOrder;

    struct ParseExpression {
        std::string regex;
        std::function<Token(const std::string&)> func;
    };

    std::vector<ParseExpression> parsers = {
        /*   {
               "^//[^\n]*",   // comment
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::whitespace,toParse };
               }
           },
           {
               "^[a-zA-Z_][a-zA-Z_0-9]+(::[a-zA-Z_][a-zA-Z_0-9]*)+",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::qualifiedName,toParse };
               }
           },
           {
               "^[_a-zA-z][_0-9a-zA-Z]*",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::identifier,toParse };
               }
           },
           {
               "^[{]",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::openBrace,toParse };
               }
           },
           {
               "^[ \t\n\r]+",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::whitespace,toParse };
               }
           },
           {
               "^:",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::colon,toParse };
               }
           },
           {
               "^=",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::equals,toParse };
               }
           },
           {
               "^;",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::semicolon,toParse };
               }
           },
           {
               "^[}]",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::closeBrace,toParse };
               }
           },
           {
               "^[+]?[0-9]+[.][0-9]*",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::float_,toParse };
               }
           },
           {
               "^[+]?[0-9]+",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::integer,toParse };
               }
           },
           {
               R"(^["][^"\\]*(\\.[^"\\]*)*["])",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::string,toParse };
               }
           },
           {
               R"(^,)",
               [](const std::string& toParse) -> Token {
                   return Token{ Token::Type::comma,toParse };
               }
           },
           */
    };

    Token TokenisebyRegex(Token::Type type, const std::string regex, const std::string& next, size_t& point, bool& ok, size_t lineNumber) {
        std::smatch match;
        std::regex pattern(regex);
        if (std::regex_search(next, match, pattern) &&
            match.position() == 0) {
            size_t length = match[0].length();
            point += length;
            ok = true;
            return Token{ type,std::string(next.substr(0, length)),lineNumber };
        } else {
            ok = false;
            return { Token::Type::error ,"",lineNumber };
        }
    }

    Token TokenisePercentage(Token::Type type, const std::string& next, size_t& point, bool& ok, size_t lineNumber) {
        // Check if the 'next' string starts with the specified pattern.
        auto found = next.find_first_not_of("0123456789.");
        if (found == std::string::npos) {
            ok = false;
            assert(0);
            return { Token::Type::error, "Unexpected end of input", lineNumber };
        }
        if (found == 0) {
            ok = false;
            return { Token::Type::error, "", lineNumber };
        }
        if (next[found] != '%') {
            ok = false;
            return { Token::Type::error, "", lineNumber };
        }
        point += found+1;
        ok = true;
        return Token{ type, next.substr(0, found+1), lineNumber };
    }

    Token TokeniseInteger(Token::Type type, const std::string& next, size_t& point, bool& ok, size_t lineNumber) {
        // Check if the 'next' string starts with the specified pattern.
        auto found = next.find_first_not_of("+0123456789");
        if (found == std::string::npos) {
            ok = false;
            assert(0);
            return { Token::Type::error, "Unexpected end of input", lineNumber };
        }
        if (found == 0) {
            ok = false;
            return { Token::Type::error, "", lineNumber };
        }
        point += found;
        ok = true;
        return Token{ type, next.substr(0, found), lineNumber };
    }

    size_t ParseIdentifier(const char *str,size_t len) {
        if (str[0]=='\0' || (!std::isalpha(str[0]) && str[0] != '_')) {
            return 0;
        }
        const char* p = str+1;
        for (p = str; (size_t)(p-str)<len && (std::isalnum(*p) || *p == '_'); ++p);
        if ((size_t)(p - str) < len) {
            return (size_t)(p - str);
        } else {
            assert(0);
            return 0;
        }
    }

    Token TokeniseFloat(Token::Type type, const std::string& next, size_t& point, bool& ok, size_t lineNumber) {
        // Check if the 'next' string starts with the specified pattern.
        auto found = next.find_first_not_of("+0123456789");
        if (found == std::string::npos) {
            ok = false;
            assert(0);
            return { Token::Type::error, "Unexpected end of input", lineNumber };
        }
        if (found == 0) {
            ok = false;
            return { Token::Type::error, "", lineNumber };
        }
        if (next[found] != '.') {
            ok = false;
            return { Token::Type::error, "", lineNumber };
        }
        found++;
        auto found2 = next.substr(found).find_first_not_of("0123456789");
        if (found2 == std::string::npos) {
            ok = false;
            assert(0);
            return { Token::Type::error, "Unexpected end of input", lineNumber };
        }
        point += found + found2;
        ok = true;
        return Token{ type, next.substr(0, found + found2), lineNumber };
    }

    Generator<Token> Tokenise(std::function<std::string(size_t)> readFunc,ParseState &parseState) {
        size_t point = 0;
        while (true) {
            std::string view(readFunc(point));
            size_t endLength = view.length();
            if (endLength == 0) {
                break;
            }
            //output("%s\n", std::string(view.substr(0, endLength)).c_str());
            bool ok = false;
            size_t forward = std::min({ (size_t)512,endLength });
            std::string next = std::string(view.substr(0, forward));
            if (next[0] == '\n') {
                parseState.lineNumber++;
                point += 1;
                ok = true;
                co_yield Token{ Token::Type::whitespace,std::string(next.substr(0, 1)),parseState.lineNumber };
            } else if (std::isspace(next[0]) && next[0]!='\n') {
                int i = 1;
                while (std::isspace(next[i]) && next[i] != '\n') { ++i; }
                ok = true;
                point += i;
                co_yield Token{ Token::Type::whitespace,std::string(next.substr(0, i)),parseState.lineNumber };
            } else if (next[0] == '{') {
                point += 1;
                ok = true;
                co_yield Token{ Token::Type::openBrace,std::string(next.substr(0, 1)),parseState.lineNumber };
            } else if (next[0] == '/' && next[1] == '/') {
                int i = 2;
                while (next[i]!='\n') { ++i; }
                ok = true;
                point += i;
                co_yield Token{ Token::Type::whitespace,std::string(next.substr(0, i)), parseState.lineNumber };
            } else if (std::isalpha(next[0]) || next[0] == '_') {
                size_t i = ParseIdentifier(next.c_str(),next.size());
                point += i;
                ok = true;
                co_yield Token{ Token::Type::identifier,std::string(next.substr(0, i)), parseState.lineNumber };
            } else if (next[0] == ':') {
                if (next[1] == ':') {
                    point += 2;
                    ok = true;
                    co_yield Token{ Token::Type::doubleColon,std::string(next.substr(0, 2)),parseState.lineNumber };
                } else {
                    point += 1;
                    ok = true;
                    co_yield Token{ Token::Type::colon,std::string(next.substr(0, 1)),parseState.lineNumber };
                }
            } else if (next[0] == ',') {
                point += 1;
                ok = true;
                co_yield Token{ Token::Type::comma,std::string(next.substr(0, 1)),parseState.lineNumber };
            } else if (next[0] == '=') {
                point += 1;
                ok = true;
                co_yield Token{ Token::Type::equals,std::string(next.substr(0, 1)),parseState.lineNumber };
            } else if (next[0] == ';') {
                point += 1;
                ok = true;
                co_yield Token{ Token::Type::semicolon,std::string(next.substr(0, 1)),parseState.lineNumber };
            } else if (next[0] == '}') {
                point += 1;
                ok = true;
                co_yield Token{ Token::Type::closeBrace,std::string(next.substr(0, 1)),parseState.lineNumber };
            } else if (std::isdigit(next[0])) {
                //auto tpc = TokenisebyRegex(Token::Type::percent, "^[+]?[0-9]+([.][0-9]*)?%", next, point, ok, parseState.lineNumber);
                auto tpc = TokenisePercentage(Token::Type::percent, next, point, ok, parseState.lineNumber);
                if (ok) {
                    co_yield tpc;
                } else {
//                    auto t = TokenisebyRegex(Token::Type::float_, "^[+]?[0-9]+[.][0-9]*", next, point, ok, parseState.lineNumber);
                   auto t = TokeniseFloat(Token::Type::float_, next, point, ok, parseState.lineNumber);
                    if (ok) {
                        co_yield t;
                    } else {
//                        co_yield TokenisebyRegex(Token::Type::integer, "^[+]?[0-9]+", next, point, ok, parseState.lineNumber);
                          co_yield TokeniseInteger(Token::Type::integer, next, point, ok, parseState.lineNumber);
                    }
                }
            } else if (next[0] == '"') {
                co_yield TokenisebyRegex(Token::Type::string, R"(^["][^"\\]*(\\.[^"\\]*)*["])", next, point, ok, parseState.lineNumber);
            } else {
                for (auto& x : parsers) {
                    std::smatch match;
                    std::regex pattern(x.regex);
                    if (std::regex_search(next, match, pattern) &&
                        match.position() == 0) {
                        size_t length = match[0].length();
                        co_yield x.func(std::string(view.substr(0, length)));
                        point += length;
                        ok = true;
                        break;
                    }
                }
            }
            if (!ok) {
                parseState.description = std::format("Could not tokenise: ->{}<-\n", next);
                co_yield Token({ Token::Type::error, "" ,parseState.lineNumber });
                co_return;
            }
        }
        co_yield Token({ Token::Type::eof,"" });
    }

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

    static IDType ParseQualifiedName(const std::string& str) {
        return IDR(str);
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
/////////////////////////////////////////

namespace nugget::properties {
    struct GrammarParseData {
        ParseState &parseState;

        GrammarParseData(const std::string& rootName, std::vector<Token>& list,ParseState &parseState) :
            currentPathName(rootName), rootName(rootName), list(list),parseState(parseState) {
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
            currentPathName = IDCombineStrings({ currentPathName, currentBlockName });
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
            assert(objectInitialisers.contains(currentTypeName));
            objectInitialisers[currentTypeName]();

            initaliserList.clear();
            return true;
        }

        void ClearType() {
            currentPossibleType = currentTypeName = "";
            currentValueType = Token::Type::unset;
        }
        void ClearCurrentDerivedName() {
            currentDerivedName = "";
        }
        void SetCurrentDerivedName() {
            currentDerivedName = IDCombineStrings({ rootName, list[point - 1].text });
        }
        void SetCurrentType() {
            currentValueType = Token::ValueTypeFromString(list[point - 1].text);
        }
        void SetCurrentType(Token::Type type) {
            currentValueType = type;
        }
        void ExtendCurrentValue() {
            currentValue += list[point - 1].text;
        }
        bool ConfirmCurrentType() {
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
            currentDerivedName = IDCombineStrings({ rootName, list[point - 1].text });
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

        const bool GetChildrenOfDerivation(std::vector<IDType> &children) {
            auto r = Notice::GetChildren(IDR(currentDerivedName),children /*fill*/);
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
            std::string path = IDCombineStrings({ currentPathName, currentValueName });
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
                parseState.description = std::format("The class to Derive from could not be found or is empty: {}",fromstr);
                SetLineNumberToToken();
                return false;
            }

            for (auto& x : children) {
                auto leaf = IDKeepLeaf(IDToString(x));
                auto toPath = IDCombineStrings({ tostr, leaf });
                auto fromPath = IDCombineStrings({ fromstr, leaf });
                auto fromPathId = IDR(fromPath);
                assert(Notice::KeyExists(fromPathId));
                auto value = Notice::GetValueAny(fromPathId);
                if (!value.IsVoid()) { 
                    auto& fromValue = Notice::GetValueAny(IDR(fromPath));
                    Notice::Set(IDR(toPath), fromValue);
                    auto& sourceValueVariant = Notice::GetValueAny(IDR(fromPath));
                    auto& targetValueVariant = Notice::GetValueAny(IDR(toPath));
                    std::string sourceValue = Notice::GetValueAsString(IDR(fromPath));
                    std::string targetValue = Notice::GetValueAsString(IDR(toPath));
                    if (0) {
                        output("----------->FROM(%s : %s = %s) TO(%s : %s = %s)\n",
                            fromPath.c_str(),
                            Notice::GetValueTypeAsString(sourceValueVariant).c_str(),
                            sourceValue.c_str(),
                            toPath.c_str(),
                            Notice::GetValueTypeAsString(targetValueVariant).c_str(),
                            targetValue.c_str()
                        );
                    }
                } else {
                    auto r = CopyNodes(IDCombineStrings({ fromstr,leaf }), IDCombineStrings({ tostr,leaf }));
                    if (!r) {
                        return false;
                    }
                }
                //    output("%s %s %s\n", IDToString(x).c_str(),leaf.c_str(),toPath.c_str());
            }
            return true;
        }

        bool CopyDerivation() {
            if (CurrentDerivedNameSet()) {
                return CopyNodes(currentDerivedName, currentPathName);
            }
            return true;
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

        size_t uniqueNameCounter=0;

        std::vector<std::string> initaliserList;

        std::unordered_map<std::string, std::function<void()>> objectInitialisers = {
            {"Color",[&]() {
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
            }},
        };


        void StoreValueImp() {
            switch (currentValueType) {
                case Token::Type::integer: {
                    assert(currentTypeName == "" || currentTypeName == "int");
                    std::string path = IDCombineStrings({ currentPathName, currentValueName });
                    Notice::Set(IDR(path), ParseInteger(currentValue));
                } break;
                case Token::Type::float_: {
                    assert(currentTypeName == "" || currentTypeName == "float");
                    std::string path = IDCombineStrings({ currentPathName, currentValueName });
                    Notice::Set(IDR(path), ParseFloat(currentValue));
                } break;
                case Token::Type::string: {
                    assert(currentTypeName == "" || currentTypeName == "string");
                    std::string path = IDCombineStrings({ currentPathName, currentValueName });
                    Notice::Set(IDR(path), ParseString(currentValue));
                } break;
                case Token::Type::qualifiedName: {
                    assert(currentTypeName == "");
                    std::string path = IDCombineStrings({ currentPathName, currentValueName });
                    Notice::Set(IDR(path), ParseQualifiedName(currentValue));
                } break;
                case Token::Type::identifier: {
                    assert(currentTypeName == "");
                    std::string path = IDCombineStrings({ currentPathName, currentValueName });
                    Notice::Set(IDR(path), IDR(currentValue));
                } break;
                default:
                    assert(0);
            }
        }



    };


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
                                            pdata.ConfirmCurrentType();
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
                                                            parseState.description = std::format("Unexpected token: {}", pdata.GetCurrentToken().TypeAsString());
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
                                            if(!pdata.CopyDerivation()) {
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
                default:
                    output("vvvvvvvvvvvvvvvvvvv\n");
                    pdata.SetLineNumberToToken();
                    parseState.description = std::format("Unexpected token: {}", pdata.GetCurrentToken().TypeAsString());
                    return false;
            }
        }
        //output("DONE!\n");
        return true;
    }

    ParseState LoadPropertyTree(const std::string &where,const std::string filename) {
        ParseState parseState;
        auto t = TimeSampler("parse", true);

        output("LoadPropertyTree: START\n");
        FileReader fileReader("config.pt");
        fileReader.Open();
        std::vector<Token> tokenList;
        auto tokenise = Tokenise([&fileReader](size_t point) {
            std::string s = fileReader.readBytesFromPosition(point);
            return s;
            }, parseState);


        while (tokenise.has_next()) {
            Token token = tokenise.next();
            if (token.isError()) {
                return parseState;
                break;
            }
            if (token.isEof()) {
                break;
            }
            if (token.type != Token::Type::whitespace) {
                tokenList.push_back(token);
            }
//            output("%s ->%s\n\n\n", token.TypeAsString().c_str(), token.text.c_str());
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

#if 0

















#endif

