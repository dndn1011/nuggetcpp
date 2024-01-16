#pragma once
#include <string>

namespace nugget::properties {
	struct [[nodiscard]] ParseState {
		std::string description;
		bool successful=false;
		size_t lineNumber=1;
	};
	ParseState LoadPropertyTree(const std::string &where, const std::string filename);
	ParseState LoadPropertyTree(const std::string& where, const std::string& as, const std::string filename);
}