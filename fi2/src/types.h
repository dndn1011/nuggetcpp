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
		Color operator+(const Color& other) const {
			return Color{ other.r + r,other.g + g,other.b + b,other.a + a };
		}
		Color operator*(const Color& other) const {
			return Color{ other.r * r,other.g * g,other.b * b,other.a * a };
		}
		Color operator/(const Color& other) const {
			return Color{ other.r / r,other.g / g,other.b / b,other.a / a };
		}
		std::string to_string() const {
			return to_string_imp(*this);
		}
	private:
		static std::string to_string_imp(const nugget::Color& obj);
	};

	struct Vector3f {
		float x, y, z;
		bool operator==(const Vector3f& other) const {
			return x == other.x && y == other.y && z == other.z;
		}
		Vector3f operator+(const Vector3f& other) const {
			return Vector3f(x + other.x, y + other.y, z + other.z);
		}
		Vector3f operator-(const Vector3f& other) const {
			return Vector3f(x - other.x, y - other.y, z - other.z);
		}
		Vector3f operator*(const Vector3f& other) const {
			return Vector3f(x * other.x, y * other.y, z * other.z);
		}
		std::string to_string() const {
			return to_string_imp(*this);
		}
	private:
		static std::string to_string_imp(const Vector3f& obj);
	};
	struct Vector2f {
		float x, y;
		bool operator==(const Vector2f& other) const {
			return x == other.x && y == other.y;
		}
		Vector2f operator+(const Vector2f& other) const {
			return Vector2f(x + other.x, y + other.y);
		}
		Vector2f operator-(const Vector2f& other) const {
			return Vector2f(x - other.x, y - other.y);
		}
		Vector2f operator*(const Vector2f& other) const {
			return Vector2f(x * other.x, y * other.y);
		}
		std::string to_string() const {
			return to_string_imp(*this);
		}
	private:
		static std::string to_string_imp(const Vector2f& obj);
	};
	struct Vector3fList {
		bool operator==(const Vector3fList& other) const {
			if (other.data.size() != data.size()) {
				return false;
			} else {
				for (int i = 0; i < data.size(); i++) {
					if (data[i] != other.data[i]) {
						return false;
					}
				}
				return true;
			}
		}
		std::vector<Vector3f> data;
		std::string to_string() const {
			return to_string_imp(*this);
		}
	private:
		static std::string to_string_imp(const Vector3fList& obj);
	};

	struct ColorList {
		bool operator==(const ColorList& other) const {
			if (other.data.size() != data.size()) {
				return false;
			} else {
				for (int i = 0; i < data.size(); i++) {
					if (data[i] != other.data[i]) {
						return false;
					}
				}
				return true;
			}
		}
		std::vector<Color> data;
		std::string to_string() const {
			return to_string_imp(*this);
		}
	private:
		static std::string to_string_imp(const ColorList& obj);
	};

	struct Vector2fList {
		bool operator==(const Vector2fList& other) const {
			if (other.data.size() != data.size()) {
				return false;
			} else {
				for (int i = 0; i < data.size(); i++) {
					if (data[i] != other.data[i]) {
						return false;
					}
				}
				return true;
			}
		}
		std::vector<Vector2f> data;
		std::string to_string() const {
			return to_string_imp(*this);
		}
	private:
		static std::string to_string_imp(const Vector2fList& obj);
	};

	struct Exception {
		std::string description;
	};

	class Matrix4f {
		// 4x4 matrix represented as a single array of 16 floats
		float data[16];

	public:
		// Default constructor
		Matrix4f() {
			for (auto& val : data) {
				val = 0.0f;
			}
		}

		// Constructor to initialize with values
		Matrix4f(const float(&values)[16]) {
			for (int i = 0; i < 16; ++i) {
				data[i] = values[i];
			}
		}

		// Copy constructor
		Matrix4f(const Matrix4f& other) {
			for (int i = 0; i < 16; ++i) {
				data[i] = other.data[i];
			}
		}

		// Assignment operator
		Matrix4f& operator=(const Matrix4f& other) {
			if (this != &other) {
				for (int i = 0; i < 16; ++i) {
					data[i] = other.data[i];
				}
			}
			return *this;
		}

		// Equality operator
		bool operator==(const Matrix4f& other) const {
			for (int i = 0; i < 16; ++i) {
				if (data[i] != other.data[i]) {
					return false;
				}
			}
			return true;
		}

		// Inequality operator
		bool operator!=(const Matrix4f& other) const {
			return !(*this == other);
		}

		// Method to return the matrix as a string (for consistency with other structures)
		std::string to_string() const {
			std::string result;
			for (int i = 0; i < 16; ++i) {
				result += std::to_string(data[i]) + " ";
				if ((i + 1) % 4 == 0) {
					result += "\n";
				}
			}
			return result;
		}

		// Accessor for the elements
		float& operator[](int index) {
			// No bounds checking
			return data[index];
		}

		// Const accessor for the elements
		const float& operator[](int index) const {
			// No bounds checking
			return data[index];
		}

		// Additional matrix-specific methods can be added here (e.g., multiplication, addition)
	};
}
