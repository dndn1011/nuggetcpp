#pragma once
#include <string>
#include "notice.h"

namespace nugget::properties {
	struct [[nodiscard]] ParseState {
		std::string description;
		bool successful=false;
		size_t lineNumber=1;
	};
	ParseState LoadPropertyTree(Notice::Board& board /*fill*/, const std::string filename);

	extern Notice::Board gNotice;

}