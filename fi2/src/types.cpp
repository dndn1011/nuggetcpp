#include "types.h"
#include <format>
#include <sstream>

std::string nugget::Vector3fList::to_string_imp(const Vector3fList& obj) {
	std::stringstream ss;
	bool first = true;
	ss << "[";
	for (auto&& x : obj.data) {
		if (!first) {
			ss << ",";
		}
		first = false;
		ss << x.to_string();
	}
	ss << "]";
	return ss.str();
}

std::string nugget::Vector3f::to_string_imp(const Vector3f& obj) {
    return std::format("{}{},{},{}{}","{", obj.x, obj.y, obj.z, "}");
}
