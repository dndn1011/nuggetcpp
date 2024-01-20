#pragma once


#include <string>
#include <fstream>
#include <vector>

namespace nugget {
	struct Color;
	struct Color {
		float r;
		float g;
		float b;
		float a;

		bool operator==(const Color& other) const;
		Color operator+(const Color& other) const;
		Color operator*(const Color& other) const;
		Color operator/(const Color& other) const;
		std::string to_string() const;

		static Color defaultValue;

	private:
		static std::string to_string_imp(const nugget::Color& obj);
	};

	struct Vector3f {
		float x, y, z;

		bool operator==(const Vector3f& other) const;
		Vector3f operator+(const Vector3f& other) const;
		Vector3f operator-(const Vector3f& other) const;
		Vector3f operator*(const Vector3f& other) const;
		Vector3f operator*(float other) const;

		std::string to_string() const;

	private:
		static std::string to_string_imp(const Vector3f& obj);
	};

	struct Vector2f {
		float x, y;

		bool operator==(const Vector2f& other) const;
		Vector2f operator+(const Vector2f& other) const;
		Vector2f operator-(const Vector2f& other) const;
		Vector2f operator*(const Vector2f& other) const;

		std::string to_string() const;

	private:
		static std::string to_string_imp(const Vector2f& obj);
	};

	struct Vector3fList {
		std::vector<Vector3f> data;

		bool operator==(const Vector3fList& other) const;
		std::string to_string() const;

	private:
		static std::string to_string_imp(const Vector3fList& obj);
	};

	struct ColorList {
		std::vector<Color> data;

		bool operator==(const ColorList& other) const;
		std::string to_string() const;

	private:
		static std::string to_string_imp(const ColorList& obj);
	};

	struct Vector2fList {
		std::vector<Vector2f> data;

		bool operator==(const Vector2fList& other) const;
		std::string to_string() const;

	private:
		static std::string to_string_imp(const Vector2fList& obj);
	};

	struct Exception {
		std::string description;
	};

	struct Vector4f;
	class Matrix4f {
		float data[16];

	public:
		Matrix4f();
		Matrix4f(const float(&values)[16]);
		Matrix4f(const Matrix4f& other);

		Matrix4f& operator=(const Matrix4f& other);
		bool operator==(const Matrix4f& other) const;
		bool operator!=(const Matrix4f& other) const;

		Matrix4f operator+(const Matrix4f& other) const; // Addition
		Matrix4f operator-(const Matrix4f& other) const; // Subtraction
		Matrix4f operator*(const Matrix4f& other) const; // Matrix multiplication
		Vector4f operator*(const Vector4f& vector) const; // Matrix-vector multiplication

		// Utility Functions
		float determinant() const; // Determinant of the matrix
		Matrix4f transpose() const; // Transpose of the matrix
		Matrix4f inverse() const; // Inverse of the matrix

		std::string to_string() const;
		float& operator[](int index);
		const float& operator[](int index) const;

		const float(&GetArray() const) [16];
	};

	struct Vector4f {
		// Member variables
		float x, y, z, w;

		Vector4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

		// Method declarations
		bool operator==(const Vector4f& other) const;
		Vector4f operator+(const Vector4f& other) const;
		Vector4f operator-(const Vector4f& other) const;
		Vector4f operator*(const Vector4f& other) const;
		std::string to_string() const;

		static Vector4f defaultValue;

	private:
		static std::string to_string_imp(const Vector4f& obj);
	};

}
