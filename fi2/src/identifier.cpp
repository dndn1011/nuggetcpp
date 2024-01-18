#include <string>
#include <unordered_map>
#include "identifier.h"
#include <assert.h>
#include <sstream>
#include <unordered_set>
#include <format>
#include "debug.h"
//#pragma optimize("", off)

namespace nugget {
	namespace identifier {
		static struct Data {
			std::unordered_map<IDType, std::string> toStringMap;
			std::unordered_map<IDType, std::unordered_set<IDType>> childMap;
			std::unordered_map<IDType, IDType> parentMap;
			std::unordered_map<IDType, IDType> selfMap;
		} data;

		static std::uint64_t CombineHashesRT(std::uint64_t hash1, std::uint64_t hash2) {
			if (hash1 == 0) {
				return hash2;
			} else {
				return (hash1 ^ hash2) * 1099511628211ULL;
			}
		}

		static std::pair<uint64_t, uint64_t> HashRT_Rercursive(
			const char* str, std::size_t index = 0,
			std::uint64_t hash = 14695981039346656037ULL,
			uint64_t combo = 0) {
			if (str[index] == '\0') {
				combo = CombineHashesRT(combo, hash);
				return { hash, combo };
			} else if (str[index] == '.') {
				combo = CombineHashesRT(combo, hash);
				return HashRT_Rercursive(str, index + 1, 14695981039346656037ULL, combo);
			} else {
				return HashRT_Rercursive(str, index + 1, (hash ^ static_cast<std::uint64_t>(str[index])) * 1099511628211ULL, combo);
			}
		}

		static IDType HashRT(const std::string &str) {
			return (IDType)HashRT_Rercursive(str.c_str()).second;
		}

		std::string IDToString(IDType id) {
			assert(("Null id used", +id));
			auto &s = data.toStringMap.at(id);
			return(s);
		}

		void SetReverseLookup(IDType id,const std::string& str) {
			static int c = 0;
//			output("@@@@@@@@@@@@ {} : {}\n", c++, str);
			if (data.toStringMap.contains(id)) {
				const std::string &curval = data.toStringMap.at(id);
				if (str != curval) {
					assert(("hash clash!!!!!!!!!!!!!!!!!!!!", 0));
				}
			} else {
				data.toStringMap.emplace(id,str);
			}
		}

		IDType Register(std::string pathString) {
			assert(("hash request on empty string", pathString != ""));
			
			bool firstTime = true;
			IDType returnHash = IDType::null;

			std::string leafString;
			std::string remainingPathString;
			
			for (;;) {
				remainingPathString = pathString;
				IDSeparateLeafAndPathInPlace(pathString, leafString);
				IDType fullHash = HashRT(remainingPathString);
				if (firstTime) {
					returnHash = fullHash;
					firstTime = false;
				}
				if (pathString.size() == 0) { 
					IDType leafHash = HashRT(leafString);
					SetReverseLookup(leafHash, leafString);
					data.selfMap[fullHash] = leafHash;
					break;
				} else {
					IDType pathHash = HashRT(pathString);
					IDType leafHash = HashRT(leafString);
					SetReverseLookup(fullHash, remainingPathString);
					SetReverseLookup(leafHash, leafString);
					data.parentMap[fullHash] = pathHash;
					data.childMap[pathHash].insert(fullHash);
					data.selfMap[fullHash] = leafHash;
				}
			}

			return returnHash;
		}

		const std::unordered_set<IDType>* IDGetChildren(IDType id) {
			if (data.childMap.contains(id)) {
				return std::addressof(data.childMap[id]);
			} else {
				return nullptr;
			}
		}

		IDType GetParent(IDType child) {
			assert(("No parent?", data.parentMap.contains(child)));
			return data.parentMap.at(child);
		}

		IDType GetParentTry(IDType child) {
			if (data.parentMap.contains(child)) {
				return data.parentMap.at(child);
			} else {
				return IDType::null;
			}
		}

		IDType GetLeaf(IDType child) {
			assert(("No self leaf?", data.selfMap.contains(child)));
			return data.selfMap.at(child);
		}

		IDType IDR(const char* cstr) {
			return Register(cstr);
		}
		IDType IDR(const char* cstr1, const char* cstr2) {
			std::string combo = IDCombineStrings( cstr1,cstr2 );
			return Register(combo);
		}
		IDType IDR(IDType hash1, const char* cstr2) {
			assert(data.toStringMap.contains(hash1));
			std::string str1 = IDToString(hash1);
			std::string combo = IDCombineStrings( str1,cstr2 );
			return Register(combo);
		}
		IDType IDR(const char* cstr1, IDType hash2) {
			assert(data.toStringMap.contains(hash2));
			std::string str2 = IDToString(hash2);
			std::string combo = IDCombineStrings( cstr1,str2 );
			return Register(combo);
		}
		IDType IDR(IDType hash1, IDType hash2) {
			assert(data.toStringMap.contains(hash1));
			assert(data.toStringMap.contains(hash2));
			std::string str1 = IDToString(hash1);
			std::string str2 = IDToString(hash2);
			std::string combo = IDCombineStrings( str1,str2 );
			return Register(combo);
		}
		IDType IDR(IDType id, const std::string_view str) {
			return IDR(id, str.data());
		}
		IDType IDR(const std::string_view str) {
			return IDR(str.data());
		}

		// this is run time hash, but it does not register anything because we are checking if a node exists
		// returns 0 if not found
		IDType IDRCheck(IDType hash1, const std::string_view str2) {
			// check we have reverse lookup of id
			if (data.toStringMap.contains(hash1)) {
				std::string str1 = data.toStringMap[hash1];
				std::string combo = IDCombineStrings( str1,str2.data() );
				IDType id = HashRT(combo);
				if (data.toStringMap.contains(id)) {
					return id;
				} else {
					return IDType::null;
				}
			} else {
				return IDType::null;
			}
		}

		IDType IDR(const std::vector<std::string>& strings) {
			IDType id = IDType::null;
			bool first = true;
			for (const auto& x : strings) {
				if (first) {
					id = IDR(x);
					first = false;
				} else {
					id = IDR(id, x);
				}
			}
			return id;
		}

#if 0
		// n.b. does not register the combined hash
		IDType IDR(const std::vector<IDType>& ids) {
			IDType id = IDType::null;
			bool first = true;
			for (const auto& x : ids) {
				if (first) {
					id = x;
					first = false;
				} else {
#if defined(NDEBUG) || 1
					id = (IDType)CombineHashesRT(+id, +x);
#else
					id = IDR(id, x);
#endif
				}
			}
			return id;
		}
#endif

		std::string IDCombineStrings(const std::string& a, const std::string& b) {
			if (a.size() > 0) {
				std::stringstream ss;
				ss << a << "." << b;
				return ss.str();
			} else {
				return b;
			}
		}

		void IDCombineStringsInPlace(const std::string& a, const std::string& b,std::string &result) {
			result.clear();
			result.append(a).append(".").append(b);
		}

		[[nodiscard]] std::string IDRemoveLeaf(const std::string& path) {
			// Find the position of the last '.' character
			auto lastDotPosition = path.find_last_of('.');

			// Check if a '.' character was found
			if (lastDotPosition != std::string::npos) {
				// Strip off everything after the last '.' character
				return path.substr(0, lastDotPosition);
			} else {
				// If no '.' character was found, return the original string
				return path;
			}
		}

		bool IDRemoveLeafInPlace(std::string& path) {
			// Find the position of the last '.' character
			auto lastDotPosition = path.find_last_of('.');

			// Check if a '.' character was found
			if (lastDotPosition != std::string::npos) {
				// Strip off everything after the last '.' character
				path.resize(lastDotPosition);
				return true;
			} else {
				return false;
			}
		}

		void IDSeparateLeafAndPathInPlace(std::string& path /*fill*/,std::string &leaf /*fill*/) {
			// Find the position of the last '.' character
			auto lastDotPosition = path.find_last_of('.');

			// Check if a '.' character was found
			if (lastDotPosition != std::string::npos) {
				// copy over the leaf
				leaf = path.substr(lastDotPosition+1);
				// Strip off everything after the last '.' character
				path.resize(lastDotPosition);
			} else {
				leaf = path;
				path.resize(0);
			}
		}

		std::string IDKeepLeaf(const std::string& path) {
			// Find the position of the last '.' character
			auto lastDotPosition = path.find_last_of('.');

			// Check if a '.' character was found
			if (lastDotPosition != std::string::npos) {
				return path.substr(lastDotPosition + 1);
			} else {
				return path;
			}
		}
	}
}

const char* DebugLookupHash(nugget::identifier::IDType hash) {
	static std::string str;
	if (nugget::identifier::data.toStringMap.contains(hash)) {
		str = nugget::identifier::data.toStringMap.at(hash);
		return str.c_str();
	} else {
		return "<not found";
	}
}

void PrintHashTree(nugget::identifier::IDType hash) {
	std::string str = nugget::identifier::data.toStringMap.at(hash);
	output(":{}\n", str.c_str());
	auto list = nugget::identifier::IDGetChildren(hash);                   
	if (list) {
		for (nugget::identifier::IDType x : *list) {
			PrintHashTree(x);
		}
	}
}
