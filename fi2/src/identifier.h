#pragma once
#include <string>
#include "assert.h"

#include <cstdint>
#include <optional>
#include <vector>
#include <unordered_set>

#include <cctype>


namespace nugget {
    namespace identifier {
        enum class IDType : uint64_t {
            null = 0,
        };

//        typedef uint64_t IDType;

        // hash a string and register it in the identrifier db
        IDType Register(const std::string& str);

        consteval std::uint64_t __combineHashes(std::uint64_t hash1, std::uint64_t hash2) {
            if (hash1 == 0) {
                return hash2;
            } else {
                return (hash1 ^ hash2) * 1099511628211ULL;
            }
        }

        consteval std::pair<uint64_t,uint64_t> __fnv1a_64_hash(
            const char* str, std::size_t index = 0,
            std::uint64_t hash = 14695981039346656037ULL,
            uint64_t combo=0) {
            if (str[index] == '\0') {
                combo = __combineHashes(combo, hash);
                return { hash, combo };
            } else if(str[index] == '.') {
                combo = __combineHashes(combo, hash);
                return __fnv1a_64_hash(str, index + 1, 14695981039346656037ULL, combo);
            } else {
                return __fnv1a_64_hash(str, index + 1, (hash ^ static_cast<std::uint64_t>(str[index])) * 1099511628211ULL, combo);
            }
        }
        
        /////////////////////////////////////// public

        consteval IDType ID(const char* cstr) {
            assert(("This hash function is being executed at run time. Please use alterative function.", std::is_constant_evaluated()));
            return (IDType)__fnv1a_64_hash(cstr).second;
        }
        consteval IDType ID(const char* cstr1, const char* cstr2) {
            assert(("This hash function is being executed at run time. Please use alterative function.", std::is_constant_evaluated()));
            return (IDType)__combineHashes(__fnv1a_64_hash(cstr1).second, __fnv1a_64_hash(cstr2).second);
        }
        consteval IDType ID(IDType hash1, const char* cstr2) {
            assert(("This hash function is being executed at run time. Please use alterative function.", std::is_constant_evaluated()));
            return (IDType)__combineHashes((uint64_t)hash1, __fnv1a_64_hash(cstr2).second);
        }
        consteval IDType ID(const char *cstr1, IDType hash2) {
            assert(("This hash function is being executed at run time. Please use alterative function.", std::is_constant_evaluated()));
            return (IDType)__combineHashes(__fnv1a_64_hash(cstr1).second, (uint64_t)hash2);
        }
        consteval IDType ID(IDType hash1,IDType hash2) {
            assert(("This hash function is being executed at run time. Please use alterative function.", std::is_constant_evaluated()));
            return (IDType)__combineHashes((uint64_t)hash1, (uint64_t)hash2);
        }

        IDType IDR(const char* cstr); 
        IDType IDR(const char* cstr1, const char* cstr2);
        IDType IDR(IDType hash1, const char* cstr2);
        IDType IDR(const char* cstr1, IDType hash2);
        IDType IDR(IDType hash1, IDType hash2);
        IDType IDR(IDType id, const std::string_view str);
        IDType IDR(const std::string_view str);
        IDType IDRCheck(IDType hash1, const std::string_view str2);

        IDType IDR(const std::vector<std::string>& strings);
        IDType IDR(const std::vector<IDType>& ids);
        
        std::string IDCombineStrings(const std::string &a, const std::string& b);
        void IDCombineStringsInPlace(const std::string& a, const std::string& b, std::string &result);
        
        std::string IDRemoveLeaf(const std::string& path);
        void IDRemoveLeafInPlace(std::string& path);

        //std::string IDKeepLeaf(const std::string& path);
        std::string IDKeepLeaf(const std::string& path);

        std::string IDToString(IDType id);

        const std::unordered_set<IDType>* IDGetChildren(IDType id);

        IDType GetParent(IDType child);
        IDType GetLeaf(IDType child);
        
    }
}

void PrintHashTree(uint64_t hash);

inline uint64_t operator+(const nugget::identifier::IDType& value) {
    return uint64_t(value);
}
