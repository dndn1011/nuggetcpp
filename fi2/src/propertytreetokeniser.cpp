#include <string>
#include <unordered_map>
#include <regex>
#include "debug.h"
#include <vector>
#include <functional>
#include "propertytree.h"
#include "identifier.h"
#include "notice.h"
#include "propertytreetokeniser.h"

namespace nugget::properties {
    using namespace identifier;
#if 0
    struct ParseExpression {
        std::string regex;
        std::function<Token(const std::string&)> func;
    };

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

    Generator<KeyValue> parseKeyValues(std::string_view input) {
        // Implement parsing logic here
        KeyValue kv;
        kv.key = "key";
        kv.type = "type";
        kv.value = "value";
        tokenList.push_back(kv;
    }

    Generator<IdentifierBlock> parseIdentifierBlock(std::string_view input) {
        // Implement parsing logic here
        IdentifierBlock identifierBlock;
        identifierBlock.identifier = "my_identifier";
        identifierBlock.parent = "parent_identifier";
        tokenList.push_back(identifierBlock;
    }
#endif

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
        {"dimension",Type::dimension},
        {"vertices",Type::vertices},
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

    size_t ParseIdentifier(const char* str, size_t len) {
        if (str[0] == '\0' || (!std::isalpha(str[0]) && str[0] != '_')) {
            return 0;
        }
        const char* p = str + 1;
        for (p = str; (size_t)(p - str) < len && (std::isalnum(*p) || *p == '_' || *p=='.'); ++p);
        if ((size_t)(p - str) < len) {
            return (size_t)(p - str);
        } else {
            check(0, ("Error parsing identifier\n"));
            return 0;
        }
    }

    Token TokenisePercentage(Token::Type type, const std::string& next, size_t& point, bool& ok, size_t lineNumber) {
        // Check if the 'next' string starts with the specified pattern.
        auto start = next.find_first_of("-0123456789");
        if (start != 0) {
            return { Token::Type::error, "invalid first character for TokenisePercentage", lineNumber };
        }
        auto found = next.find_first_not_of("0123456789.",1);
        if (found == std::string::npos) {
            ok = false;
            return { Token::Type::error, "Unexpected end of input", lineNumber };
        }
        if (next[found] != '%') {
            ok = false;
            return { Token::Type::error, "", lineNumber };
        }
        point += found + 1;
        ok = true;
        return Token{ type, next.substr(0, found + 1), lineNumber };
    }

    Token TokeniseFloat(Token::Type type, const std::string& next, size_t& point, bool& ok, size_t lineNumber) {
        // Check if the 'next' string starts with the specified pattern.
        auto start = next.find_first_of("-0123456789");
        if (start != 0) {
            return { Token::Type::error, "invalid first character for TokeniseFloat", lineNumber };
        }
        auto found = next.find_first_not_of("0123456789",1);
        if (found == std::string::npos) {
            ok = false;
            return { Token::Type::error, "Unexpected end of input", lineNumber };
        }
        if (next[found] != '.') {
            ok = false;
            return { Token::Type::error, "", lineNumber };
        }
        found++;
        auto found2 = next.substr(found).find_first_not_of("0123456789");
        if (found2 == std::string::npos) {
            ok = false;
            return { Token::Type::error, "Unexpected end of input", lineNumber };
        }
        point += found + found2;
        ok = true;
        return Token{ type, next.substr(0, found + found2), lineNumber };
    }
    Token TokeniseInteger(Token::Type type, const std::string& next, size_t& point, bool& ok, size_t lineNumber) {
        // Check if the 'next' string starts with the specified pattern.
        auto start = next.find_first_of("-0123456789");
        if (start != 0) {
            return { Token::Type::error, "invalid first character for TokeniseInteger", lineNumber };
        }
        auto found = next.find_first_not_of("0123456789",1);
        if (found == std::string::npos) {
            ok = false;
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
}

namespace nugget::properties {

    bool Tokenise(std::function<std::string(size_t)> readFunc, ParseState& parseState, std::vector<Token>& tokenList) {
        size_t point = 0;
        bool literalMode = false;
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
            if (literalMode) {
                //////////////////////////////////////////////
                // end here doc
                if (next[0] == '>' && next[1] == '>') {
                    point += 2;
                    ok = true;
                    literalMode = false;
                    tokenList.push_back(Token{ Token::Type::doubleMore,std::string(next.substr(0, 2)),parseState.lineNumber });
                } else {
                    //////////////////////////////////////////////
                    // build here doc
                    // find end of line
                    size_t i = 0;
                    for (i = 0; i < next.size() && next[i] != '\n'; i++);
                    check(next.size(), "Exceeded maximum line length");
                    parseState.lineNumber++;
                    point += i + 1;
                    ok = true;
                    tokenList.push_back(Token{ Token::Type::literalLine,std::string(next.substr(0, i + 1)),parseState.lineNumber });
                }
            } else if (next[0] == '\n') {
                //////////////////////////////////////////////
                // line count
                parseState.lineNumber++;
                point += 1;
                ok = true;
                //tokenList.push_back(Token{ Token::Type::whitespace,std::string(next.substr(0, 1)),parseState.lineNumber });
            } else if (std::isspace(next[0]) && next[0] != '\n') {
                //////////////////////////////////////////////
                // new line whitespace
                int i = 1;
                while (std::isspace(next[i]) && next[i] != '\n') { ++i; }
                ok = true;
                point += i;
                //tokenList.push_back(Token{ Token::Type::whitespace,std::string(next.substr(0, i)),parseState.lineNumber });
            } else if (next[0] == '{') {
                //////////////////////////////////////////////
                // open brace
                point += 1;
                ok = true;
                tokenList.push_back(Token{ Token::Type::openBrace,std::string(next.substr(0, 1)),parseState.lineNumber });
            } else if (next[0] == '/' && next[1] == '/') {
                //////////////////////////////////////////////
                // line comment
                int i = 2;
                while (next[i] != '\n') { ++i; }
                ok = true;
                point += i;
                //tokenList.push_back(Token{ Token::Type::whitespace,std::string(next.substr(0, i)), parseState.lineNumber });
            } else if (std::isalpha(next[0]) || next[0] == '_') {
                //////////////////////////////////////////////
                // identifier
                size_t i = ParseIdentifier(next.c_str(), next.size());
                point += i;
                ok = true;
                tokenList.push_back(Token{ Token::Type::identifier,std::string(next.substr(0, i)), parseState.lineNumber });
            } else if (next[0] == ':') {
                if (next[1] == ':') {
                    //////////////////////////////////////////////
                    // double colon
                    point += 2;
                    ok = true;
                    tokenList.push_back(Token{ Token::Type::doubleColon,std::string(next.substr(0, 2)),parseState.lineNumber });
                } else {
                    //////////////////////////////////////////////
                    // single colon
                    point += 1;
                    ok = true;
                    tokenList.push_back(Token{ Token::Type::colon,std::string(next.substr(0, 1)),parseState.lineNumber });
                }
            } else if (next[0] == ',') {
                //////////////////////////////////////////////
                // comma
                point += 1;
                ok = true;
                tokenList.push_back(Token{ Token::Type::comma,std::string(next.substr(0, 1)),parseState.lineNumber });
            } else if (next[0] == '=') {
                //////////////////////////////////////////////
                // equals
                point += 1;
                ok = true;
                tokenList.push_back(Token{ Token::Type::equals,std::string(next.substr(0, 1)),parseState.lineNumber });
            } else if (next[0] == ';') {
                //////////////////////////////////////////////
                // semicolon
                point += 1;
                ok = true;
                tokenList.push_back(Token{ Token::Type::semicolon,std::string(next.substr(0, 1)),parseState.lineNumber });
            } else if (next[0] == '}') {
                //////////////////////////////////////////////
                // close brace
                point += 1;
                ok = true;
                tokenList.push_back(Token{ Token::Type::closeBrace,std::string(next.substr(0, 1)),parseState.lineNumber });
            } else if (std::isdigit(next[0]) || next[0]=='-') {
                //////////////////////////////////////////////
                // precentage?
                auto tpc = TokenisePercentage(Token::Type::percent, next, point, ok, parseState.lineNumber);
                if (ok) {
                    tokenList.push_back(tpc);
                } else {
                    //////////////////////////////////////////////
                    // float?
                    auto t = TokeniseFloat(Token::Type::float_, next, point, ok, parseState.lineNumber);
                    if (ok) {
                        tokenList.push_back(t);
                    } else {
                        //////////////////////////////////////////////
                        // integer
                        tokenList.push_back(TokeniseInteger(Token::Type::integer, next, point, ok, parseState.lineNumber));
                    }
                }
            } else if (next[0] == '"') {
                //////////////////////////////////////////////
                // double quote
                tokenList.push_back(TokenisebyRegex(Token::Type::string, R"(^["][^"\\]*(\\.[^"\\]*)*["])", next, point, ok, parseState.lineNumber));
            } else if (next[0] == '<' && next[1] == '<') {
                //////////////////////////////////////////////
                // start here doc
                point += 2;
                ok = true;
                literalMode = true;
                tokenList.push_back(Token{ Token::Type::doubleLess,std::string(next.substr(0, 2)),parseState.lineNumber });
            }
            if (!ok) {
                parseState.description = std::format("Could not tokenise: ->{}<-\n", next);
                tokenList.push_back(Token({ Token::Type::error, std::string(next.substr(0, 32)), parseState.lineNumber }));
                return false;
            }
        }
        tokenList.push_back(Token({ Token::Type::eof,"" }));
        return true;
    }
}