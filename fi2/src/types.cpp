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

std::string nugget::ColorList::to_string_imp(const ColorList& obj) {
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

std::string nugget::Vector2fList::to_string_imp(const Vector2fList& obj) {
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
	return std::format("{}{},{},{}{}", "{", obj.x, obj.y, obj.z, "}");
}

std::string nugget::Vector4f::to_string_imp(const Vector4f& obj) {
	return std::format("{}{},{},{},{}{}", "{", obj.x, obj.y, obj.z, obj.w, "}");
}

std::string nugget::Vector2f::to_string_imp(const Vector2f& obj) {
	return std::format("{}{},{}{}", "{", obj.x, obj.y, "}");
}


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

namespace nugget {

	// Equality operator for Color
	bool Color::operator==(const Color& other) const {
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}

	// Addition operator for Color
	Color Color::operator+(const Color& other) const {
		return Color{ r + other.r, g + other.g, b + other.b, a + other.a };
	}

	// Multiplication operator for Color
	Color Color::operator*(const Color& other) const {
		return Color{ r * other.r, g * other.g, b * other.b, a * other.a };
	}

	// Division operator for Color
	Color Color::operator/(const Color& other) const {
		// Ensure to handle division by zero in a real implementation
		return Color{ r / other.r, g / other.g, b / other.b, a / other.a };
	}

	// to_string method for Color
	std::string Color::to_string() const {
		return to_string_imp(*this);
	}

	// Equality operator for Vector3f
	bool Vector3f::operator==(const Vector3f& other) const {
		return x == other.x && y == other.y && z == other.z;
	}

	// Addition operator for Vector3f
	Vector3f Vector3f::operator+(const Vector3f& other) const {
		return Vector3f{ x + other.x, y + other.y, z + other.z };
	}

	// Subtraction operator for Vector3f
	Vector3f Vector3f::operator-(const Vector3f& other) const {
		return Vector3f{ x - other.x, y - other.y, z - other.z };
	}

	// Multiplication operator for Vector3f
	Vector3f Vector3f::operator*(const Vector3f& other) const {
		return Vector3f{ x * other.x, y * other.y, z * other.z };
	}

	Vector3f Vector3f::operator*(float other) const {
		return Vector3f{ x * other, y * other, z * other };
	}

	// to_string method for Vector3f
	std::string Vector3f::to_string() const {
		return to_string_imp(*this);
	}

	// Equality operator for Vector2f
	bool Vector2f::operator==(const Vector2f& other) const {
		return x == other.x && y == other.y;
	}

	// Addition operator for Vector2f
	Vector2f Vector2f::operator+(const Vector2f& other) const {
		return Vector2f{ x + other.x, y + other.y };
	}

	// Subtraction operator for Vector2f
	Vector2f Vector2f::operator-(const Vector2f& other) const {
		return Vector2f{ x - other.x, y - other.y };
	}

	// Multiplication operator for Vector2f
	Vector2f Vector2f::operator*(const Vector2f& other) const {
		return Vector2f{ x * other.x, y * other.y };
	}

	// to_string method for Vector2f
	std::string Vector2f::to_string() const {
		return to_string_imp(*this);
	}

	// Equality operator for Vector3fList
	bool Vector3fList::operator==(const Vector3fList& other) const {
		if (data.size() != other.data.size()) {
			return false;
		}
		for (size_t i = 0; i < data.size(); ++i) {
			if (!(data[i] == other.data[i])) {
				return false;
			}
		}
		return true;
	}

	// to_string method for Vector3fList
	std::string Vector3fList::to_string() const {
		return to_string_imp(*this);
	}

	// Equality operator for ColorList
	bool ColorList::operator==(const ColorList& other) const {
		if (data.size() != other.data.size()) {
			return false;
		}
		for (size_t i = 0; i < data.size(); ++i) {
			if (!(data[i] == other.data[i])) {
				return false;
			}
		}
		return true;
	}

	// to_string method for ColorList
	std::string ColorList::to_string() const {
		return to_string_imp(*this);
	}

	// Equality operator for Vector2fList
	bool Vector2fList::operator==(const Vector2fList& other) const {
		if (data.size() != other.data.size()) {
			return false;
		}
		for (size_t i = 0; i < data.size(); ++i) {
			if (!(data[i] == other.data[i])) {
				return false;
			}
		}
		return true;
	}

	// to_string method for Vector2fList
	std::string Vector2fList::to_string() const {
		return to_string_imp(*this);
	}

	// Default constructor for Matrix4f
	Matrix4f::Matrix4f() : data{} {
		data[0] = 1.0f;
		data[5] = 1.0f;
		data[10] = 1.0f;
		data[15] = 1.0f;
	}

	// Constructor to initialize with values
	Matrix4f::Matrix4f(const float(&values)[16]) {
		for (int i = 0; i < 16; ++i) {
			data[i] = values[i];
		}
	}

	// Copy constructor
	Matrix4f::Matrix4f(const Matrix4f& other) {
		for (int i = 0; i < 16; ++i) {
			data[i] = other.data[i];
		}
	}

	// Assignment operator
	Matrix4f& Matrix4f::operator=(const Matrix4f& other) {
		if (this != &other) {
			for (int i = 0; i < 16; ++i) {
				data[i] = other.data[i];
			}
		}
		return *this;
	}

	// Equality operator
	bool Matrix4f::operator==(const Matrix4f& other) const {
		for (int i = 0; i < 16; ++i) {
			if (data[i] != other.data[i]) {
				return false;
			}
		}
		return true;
	}

	// Inequality operator
	bool Matrix4f::operator!=(const Matrix4f& other) const {
		return !(*this == other);
	}

	// Method to return the matrix as a string
	std::string Matrix4f::to_string() const {
		std::stringstream ss;
		for (int i = 0; i < 16; ++i) {
			ss << data[i];
			if ((i + 1) % 4 == 0)
				ss << "\n";
			else
				ss << " ";
		}
		return ss.str();
	}

	// Accessor for the elements
	float& Matrix4f::operator[](int index) {
		// No bounds checking
		return data[index];
	}

	// Const accessor for the elements
	const float& Matrix4f::operator[](int index) const {
		// No bounds checking
		return data[index];
	}

	bool Vector4f::operator==(const Vector4f& other) const {
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	Vector4f Vector4f::operator+(const Vector4f& other) const {
		return Vector4f{ x + other.x, y + other.y, z + other.z, w + other.w };
	}

	Vector4f Vector4f::operator-(const Vector4f& other) const {
		return Vector4f{ x - other.x, y - other.y, z - other.z, w - other.w };
	}

	Vector4f Vector4f::operator*(const Vector4f& other) const {
		return Vector4f{ x * other.x, y * other.y, z * other.z, w * other.w };
	}

	std::string Vector4f::to_string() const {
		return to_string_imp(*this);
	}

	Matrix4f Matrix4f::operator+(const Matrix4f& other) const {
		Matrix4f result;
		for (int i = 0; i < 16; ++i) {
			result.data[i] = this->data[i] + other.data[i];
		}
		return result;
	}

	Matrix4f Matrix4f::operator-(const Matrix4f& other) const {
		Matrix4f result;
		for (int i = 0; i < 16; ++i) {
			result.data[i] = this->data[i] - other.data[i];
		}
		return result;
	}

	Matrix4f Matrix4f::operator*(const Matrix4f& other) const {
		Matrix4f result;
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.data[col * 4 + row] = 0;
				for (int k = 0; k < 4; ++k) {
					result.data[col * 4 + row] += this->data[k * 4 + row] * other.data[col * 4 + k];
				}
			}
		}
		return result;
	}

	float Matrix4f::determinant() const {
		// Calculating determinant for a 4x4 matrix
		// det(A) = a11 * det(M11) - a12 * det(M12) + a13 * det(M13) - a14 * det(M14)
		// where M11, M12, M13, M14 are 3x3 submatrices

		auto det3x3 = [](float a, float b, float c, float d, float e, float f, float g, float h, float i) {
			return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
			};

		return data[0] * det3x3(data[5], data[6], data[7], data[9], data[10], data[11], data[13], data[14], data[15])
			- data[1] * det3x3(data[4], data[6], data[7], data[8], data[10], data[11], data[12], data[14], data[15])
			+ data[2] * det3x3(data[4], data[5], data[7], data[8], data[9], data[11], data[12], data[13], data[15])
			- data[3] * det3x3(data[4], data[5], data[6], data[8], data[9], data[10], data[12], data[13], data[14]);
	}

	Matrix4f Matrix4f::transpose() const {
		Matrix4f result;
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.data[row * 4 + col] = this->data[col * 4 + row];
			}
		}
		return result;
	}

	Matrix4f Matrix4f::inverse() const {
		float det = this->determinant();
		if (std::fabs(det) < 1e-6) {
			throw std::runtime_error("Matrix is singular and cannot be inverted");
		}

		Matrix4f cofactor;

		// Lambda function to calculate the determinant of a 3x3 submatrix
		auto det3x3 = [this](int a, int b, int c, int d, int e, int f, int g, int h, int i) {
			return data[a] * (data[e] * data[i] - data[f] * data[h])
				- data[b] * (data[d] * data[i] - data[f] * data[g])
				+ data[c] * (data[d] * data[h] - data[e] * data[g]);
			};

		// Calculating the cofactor matrix
		cofactor.data[0] = det3x3(5, 6, 7, 9, 10, 11, 13, 14, 15);
		cofactor.data[1] = -det3x3(4, 6, 7, 8, 10, 11, 12, 14, 15);
		cofactor.data[2] = det3x3(4, 5, 7, 8, 9, 11, 12, 13, 15);
		cofactor.data[3] = -det3x3(4, 5, 6, 8, 9, 10, 12, 13, 14);

		cofactor.data[4] = -det3x3(1, 2, 3, 9, 10, 11, 13, 14, 15);
		cofactor.data[5] = det3x3(0, 2, 3, 8, 10, 11, 12, 14, 15);
		cofactor.data[6] = -det3x3(0, 1, 3, 8, 9, 11, 12, 13, 15);
		cofactor.data[7] = det3x3(0, 1, 2, 8, 9, 10, 12, 13, 14);

		cofactor.data[8] = det3x3(1, 2, 3, 5, 6, 7, 13, 14, 15);
		cofactor.data[9] = -det3x3(0, 2, 3, 4, 6, 7, 12, 14, 15);
		cofactor.data[10] = det3x3(0, 1, 3, 4, 5, 7, 12, 13, 15);
		cofactor.data[11] = -det3x3(0, 1, 2, 4, 5, 6, 12, 13, 14);

		cofactor.data[12] = -det3x3(1, 2, 3, 5, 6, 7, 9, 10, 11);
		cofactor.data[13] = det3x3(0, 2, 3, 4, 6, 7, 8, 10, 11);
		cofactor.data[14] = -det3x3(0, 1, 3, 4, 5, 7, 8, 9, 11);
		cofactor.data[15] = det3x3(0, 1, 2, 4, 5, 6, 8, 9, 10);

		// Transposing the cofactor matrix to get the adjugate matrix
		Matrix4f adjugate = cofactor.transpose();

		// Dividing the adjugate matrix by the determinant
		Matrix4f inverse;
		for (int i = 0; i < 16; ++i) {
			inverse.data[i] = adjugate.data[i] / det;
		}

		return inverse;
	}

	Vector4f Matrix4f::operator*(const Vector4f& vec) const {
		return Vector4f(
			data[0] * vec.x + data[1] * vec.y + data[2] * vec.z + data[3] * vec.w,
			data[4] * vec.x + data[5] * vec.y + data[6] * vec.z + data[7] * vec.w,
			data[8] * vec.x + data[9] * vec.y + data[10] * vec.z + data[11] * vec.w,
			data[12] * vec.x + data[13] * vec.y + data[14] * vec.z + data[15] * vec.w
		);
	}

	/*static*/
	Color Color::defaultValue = { 0.5,0.5,0.5,0.5 };
	/*static*/
	Vector4f Vector4f::defaultValue = { 0,0,0,0 };

	const float(&nugget::Matrix4f::GetArray() const)[16] {
		return data;
	}
}