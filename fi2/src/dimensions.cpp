#include <format>
#include <unordered_map>

#include "dimensions.h"

namespace nugget::ui {

	static const std::unordered_map<Dimension::Units, std::string> unitToString = {
		{ Dimension::Units::none, "none" },
		{ Dimension::Units::pixel, "pixel" },
		{ Dimension::Units::percent, "percent" },
		{ Dimension::Units::scale, "scale" },
	};

	std::string Dimension::ToString() const {
		return std::format("({}){}", unitToString.at(unit), f);
	}

}