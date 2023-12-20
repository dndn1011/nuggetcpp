#pragma once
namespace nugget::properties {

    class Token {
    public:
        enum class Type {
#define __PTREE_TOKEN_ENUMDEF
#include "ptree_tokens.h"
            size,
        };

        enum Type type;
        std::string text;
        size_t lineNumber = 0;

        static std::unordered_map<enum Type, std::string>typeToString;
        static std::unordered_map<std::string, enum Token::Type> valueTypeFromString;

        static Token::Type ValueTypeFromString(std::string str) {
            if (valueTypeFromString.contains(str)) {
                return valueTypeFromString[str];
            } else {
                return Token::Type::error;
            }
        }

        std::string TypeAsString() const {
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

    bool Tokenise(std::function<std::string(size_t)> readFunc, ParseState& parseState, std::vector<Token>& tokenList);

}