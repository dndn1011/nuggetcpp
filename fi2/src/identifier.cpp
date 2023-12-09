#include <string>
#include <unordered_map>
#include "identifier.h"
#include <assert.h>
#include <sstream>
#include <unordered_set>
#include <format>
#include "debug.h"

namespace nugget {
	namespace identifier {
		static struct Data {
			std::unordered_map<IDType, std::string> toStringMap;
			std::unordered_map<IDType, std::unordered_set<IDType>> childMap;
			std::unordered_map<IDType, IDType> parentMap;
			std::unordered_map<IDType, IDType> selfMap;
		} data;

		std::string IDToString(IDType id) {
			assert(("Null id used", id));

			auto s = data.toStringMap[id];
			assert(("Unable to do a reverse lookup",s != ""));
			return(s);
		}

		void SetReverseLookup(IDType id,const std::string& str) {
			if (data.toStringMap.contains(id)) {
				std::string curval = data.toStringMap[id];
				if (str != curval) {
					assert(("hash clash!!!!!!!!!!!!!!!!!!!!", 0));
				}
			} else {
				data.toStringMap[id] = str;
			}
		}

		IDType Register(const std::string& str) {
			assert(("hash request on empty string", str != ""));
			std::string s = IDRemoveLeaf(str);
			std::string leaf = IDKeepLeaf(str);

			IDType parent = __fnv1a_64_hash(s.c_str()).second;
			IDType child = __fnv1a_64_hash(str.c_str()).second;
			IDType strId = child;
			SetReverseLookup(child,str);
			data.parentMap[child] = parent;
			IDType self = __fnv1a_64_hash(leaf.c_str()).second;
			data.selfMap[child] = self;
			SetReverseLookup(self,leaf);
			while (parent != child) {
				SetReverseLookup(parent,s);
				data.childMap[parent].insert(child);
				s = IDRemoveLeaf(s);
				child = parent;
				parent = __fnv1a_64_hash(s.c_str()).second;
			}
			return strId;
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

		IDType GetLeaf(IDType child) {
			assert(("No self leaf?", data.selfMap.contains(child)));
			return data.selfMap.at(child);
		}

		IDType IDR(const char* cstr) {
			return Register(cstr);
		}
		IDType IDR(const char* cstr1, const char* cstr2) {
			std::string combo = IDCombineStrings({ cstr1,cstr2 });
			return Register(combo);
		}
		IDType IDR(IDType hash1, const char* cstr2) {
			assert(data.toStringMap.contains(hash1));
			std::string str1 = IDToString(hash1);
			std::string combo = IDCombineStrings({ str1,cstr2 });
			return Register(combo);
		}
		IDType IDR(const char* cstr1, IDType hash2) {
			assert(data.toStringMap.contains(hash2));
			std::string str2 = IDToString(hash2);
			std::string combo = IDCombineStrings({ cstr1,str2 });
			return Register(combo);
		}
		IDType IDR(IDType hash1, IDType hash2) {
			assert(data.toStringMap.contains(hash1));
			assert(data.toStringMap.contains(hash2));
			std::string str1 = IDToString(hash1);
			std::string str2 = IDToString(hash2);
			std::string combo = IDCombineStrings({ str1,str2 });
			return Register(combo);
		}
		IDType IDR(IDType id, const std::string_view str) {
			return IDR(id, str.data());
		}
		IDType IDR(const std::string_view str) {
			return IDR(str.data());
		}

		// this is run time hash, but it dows not register anything because we are checking if a node exists
		// returns 0 if not found
		IDType IDRCheck(IDType hash1, const std::string_view str2) {
			// check we have reverse lookup of id
			if (data.toStringMap.contains(hash1)) {
				std::string str1 = data.toStringMap[hash1];
				std::string combo = IDCombineStrings({ str1,str2.data() });
				IDType id = __fnv1a_64_hash(combo.c_str()).second;
				if (data.toStringMap.contains(id)) {
					return id;
				} else {
					return 0;
				}
			} else {
				return 0;
			}
		}

		IDType IDR(const std::vector<std::string>& strings) {
			IDType id = 0;
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

		// n.b. does not register the combined hash
		IDType IDR(const std::vector<IDType>& ids) {
			IDType id = 0;
			bool first = true;
			for (const auto& x : ids) {
				if (first) {
					id = x;
					first = false;
				} else {
#ifdef NDEBUG
					id = __combineHashes(id, x);
#else
					id = IDR(id, x);
#endif
				}
			}
			return id;
		}

		std::string IDCombineStrings(const std::vector<std::string>& strings) {
			std::stringstream ss;
			bool first = true;
			for (const auto& x : strings) {
				if (!first) {
					ss << ".";
					ss << x;
				} else {
					first = false;
					ss << x;
				}
			}
			return ss.str();
		}

		std::string IDRemoveLeaf(const std::string& path) {
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

const char* DebugLookupHash(uint64_t hash) {
	static std::string str;
	if (nugget::identifier::data.toStringMap.contains(hash)) {
		str = nugget::identifier::data.toStringMap.at(hash);
		return str.c_str();
	} else {
		return "<not found";
	}
}

void PrintHashTree(uint64_t hash) {
	std::string str = nugget::identifier::data.toStringMap.at(hash);
	output(":{}\n", str.c_str());
	auto list = nugget::identifier::IDGetChildren(hash);                   
	if (list) {
		for (nugget::identifier::IDType x : *list) {
			PrintHashTree(x);
		}
	}
}
