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
        {"Vector2fList",Type::Vector2fList},
        {"Vector3fList",Type::Vector3fList},
        {"ColorList",Type::ColorList},
        {"Vector2f",Type::Vector2f},
        {"Vector3f",Type::Vector3f},
        {"Vector4f",Type::Vector4f},
        {"Matrix4f",Type::Matrix4f},
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
        for (p = str; (size_t)(p - str) < len && (std::isalnum(*p) || *p == '_'); ++p);
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
            } else {
                switch (next[0]) {
                    case '\n': {
                        //////////////////////////////////////////////
                        // line count
                        parseState.lineNumber++;
                        point += 1;
                        ok = true;
                    } break;
                    case ' ':case '\t':case '\r':case '\f':case '\v': {
                        //////////////////////////////////////////////
                        // new line whitespace
                        int i = 1;
                        while (std::isspace(next[i]) && next[i] != '\n') { ++i; }
                        ok = true;
                        point += i;
                    } break;
                    case '{': {
                        //////////////////////////////////////////////
                        // open brace
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::openBrace,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '(': {
                        //////////////////////////////////////////////
                        // open brace
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::openParen,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case ')': {
                        //////////////////////////////////////////////
                        // open brace
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::closeParen,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '.': {
                        //////////////////////////////////////////////
                        // +
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::dot,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '+': {
                        //////////////////////////////////////////////
                        // +
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::plus,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '*': {
                        //////////////////////////////////////////////
                        // +
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::multiply,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '$': {
                        //////////////////////////////////////////////
                        // +
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::dollar,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '/': {
                        if (next[1] == '/') {
                            //////////////////////////////////////////////
                            // line comment
                            int i = 2;
                            while (next[i] != '\n') { ++i; }
                            ok = true;
                            point += i;
                            //tokenList.push_back(Token{ Token::Type::whitespace,std::string(next.substr(0, i)), parseState.lineNumber });
                            break;
                        }
                        else {
                            point += 1;
                            ok = true;
                            tokenList.push_back(Token{ Token::Type::divide,std::string(next.substr(0, 1)), parseState.lineNumber });
                            break;
                        }
                    }
                    case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':case 'g':case 'h':
                    case 'i':case 'j':case 'k':case 'l':case 'm':case 'n':case 'o':case 'p':
                    case 'q':case 'r':case 's':case 't':case 'u':case 'v':case 'w':case 'x':
                    case 'y':case 'z':
                    case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':case 'G':case 'H':
                    case 'I':case 'J':case 'K':case 'L':case 'M':case 'N':case 'O':case 'P':
                    case 'Q':case 'R':case 'S':case 'T':case 'U':case 'V':case 'W':case 'X':
                    case 'Y':case 'Z':case'_':
                    {
                        //////////////////////////////////////////////
                        // identifier
                        size_t i = ParseIdentifier(next.c_str(), next.size());
                        point += i;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::identifier,std::string(next.substr(0, i)), parseState.lineNumber });
                    } break;
                    case ':': {
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
                    } break;
                    case ',': {
                        //////////////////////////////////////////////
                        // comma
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::comma,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '=': {
                        //////////////////////////////////////////////
                        // equals
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::equals,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case ';': {
                        //////////////////////////////////////////////
                        // semicolon
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::semicolon,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '}': {
                        //////////////////////////////////////////////
                        // close brace
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::closeBrace,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '@': {
                        //////////////////////////////////////////////
                        // close brace
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::at,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '-': {
                        //////////////////////////////////////////////
                        // minus operator
                        point += 1;
                        ok = true;
                        tokenList.push_back(Token{ Token::Type::minus,std::string(next.substr(0, 1)),parseState.lineNumber });
                    } break;
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9': {    
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
                    } break;
                    case '"': {
                        tokenList.push_back(TokenisebyRegex(Token::Type::string, R"(^["][^"\\]*(\\.[^"\\]*)*["])", next, point, ok, parseState.lineNumber));
                    } break;
                    case '<': {
                        if (next[1] == '<') {
                            //////////////////////////////////////////////
                            // start here doc
                            point += 2;
                            ok = true;
                            literalMode = true;
                            tokenList.push_back(Token{ Token::Type::doubleLess,std::string(next.substr(0, 2)),parseState.lineNumber });
                            break;
                        }
                    } // pass through
                    default: {
                    } // pass through
                }
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

    std::string Token::ToString() {
        return std::format("Token::Type::{} = {}", TypeAsString(type), text);
    }

}