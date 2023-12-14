//#include <SFML/Graphics.hpp>

#include "types.h"
#include <sstream>

	std::string nugget::Color::to_string_imp(const nugget::Color& obj) {
		std::stringstream ss;
		ss
			<< "{"
			<< std::to_string(obj.r)
			<< ","
			<< std::to_string(obj.g)
			<< ","
			<< std::to_string(obj.b)
			<< ","
			<< std::to_string(obj.a)
			<< "}";
		return ss.str();
	};

namespace sf {
	uint8_t ConvColRange(float in) {
		int32_t c = (int)(in * 256);
		if (c > 255) {
			c = 255;
		}
		if (c < 0) {
			c = 0;
		}
		return (uint8_t)c;
	}
//	Color ToPlatform(nugget::Color in) {
// 		return sf::Color(
//			ConvColRange(in.r),
//			ConvColRange(in.g),
//			ConvColRange(in.b),
//			ConvColRange(in.a));
//	}
}