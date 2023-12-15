#pragma once
#include <string>
#include <fstream>
#include <vector>

namespace std {
	template <class T, size_t N>
	class array;
}

namespace nugget {
	struct TEST;
	struct TEST {
		std::string to_string(const nugget::TEST& obj);
	};
}

namespace nugget {
	struct Color;
	struct Color {
		float r;
		float g;
		float b;
		float a;
		bool operator==(const Color& other) const {
			return r == other.r &&
				g == other.g &&
				b == other.b &&
				a == other.a;
		}
		std::string to_string() {
			return to_string_imp(*this);
		}
	private:
		static std::string to_string_imp(const nugget::Color& obj);
	};

	struct Vector3f {
		float x, y, z;
	};
	struct Vertices {
		std::vector<Vector3f> data;
	};

}

namespace sf {
	class Color;
	Color ToPlatform(nugget::Color in);
}
