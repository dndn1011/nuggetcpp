#pragma once


#include <string>
#include <fstream>
#include <vector>
#include "identifier.h"

namespace nugget {
	using namespace identifier;
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

		float Dot(const Vector3f& o) const {
			return o.x * x + o.y * y + o.z * z;
		}

		Vector3f Cross(const Vector3f& o) {
			return Vector3f{
				y * o.z - z * o.y,
				z * o.x - x * o.z,
				x * o.y - y * o.x
			};
		}

		float Length() const;

		Vector3f Normalized() const;

		std::string to_string() const;

		static Vector3f defaultValue;

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

		void ToFloats(std::vector<float>& out) const;
	private:
		static std::string to_string_imp(const Vector3fList& obj);
	};

	struct Int64List {
		std::vector<int64_t> data;

		bool operator==(const Int64List& other) const;
		std::string to_string() const;

	private:
		static std::string to_string_imp(const Int64List& obj);
	};

	struct IdentifierList {
		std::vector<IDType> data;

		bool operator==(const IdentifierList& other) const;
		std::string to_string() const;

	private:
		static std::string to_string_imp(const IdentifierList& obj);
	};

	struct ColorList {
		std::vector<Color> data;

		bool operator==(const ColorList& other) const;
		std::string to_string() const;

		void ToFloats(std::vector<float>& out) const;

	private:
		static std::string to_string_imp(const ColorList& obj);
	};

	struct Vector2fList {
		std::vector<Vector2f> data;

		bool operator==(const Vector2fList& other) const;
		std::string to_string() const;
		void ToFloats(std::vector<float>& out) const;

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

		const float(&GetArray() const)[16];

		static void SetFromEulers(float radX, float radY, float radZ, Matrix4f& outMatrix);
		static void CreateProjectionMatrix(float screenWidth, float screenHeight, float fov, float nearPlane, float farPlane, Matrix4f& matrix);
		static void LookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& normalisedUp, Matrix4f& outMatrix);
	};

	class Matrix3f {
		float data[9];

	public:
		Matrix3f();
		Matrix3f(const float(&values)[9]);
		Matrix3f(const Matrix3f& other);

		Matrix3f& operator=(const Matrix3f& other);
		bool operator==(const Matrix3f& other) const;
		bool operator!=(const Matrix3f& other) const;

		Matrix3f operator+(const Matrix3f& other) const; // Addition
		Matrix3f operator-(const Matrix3f& other) const; // Subtraction
		Matrix3f operator*(const Matrix3f& other) const; // Matrix multiplication
		Vector3f operator*(const Vector3f& vector) const; // Matrix-vector multiplication

		// Utility Functions
		float determinant() const; // Determinant of the matrix
		Matrix3f transpose() const; // Transpose of the matrix
		Matrix3f inverse() const; // Inverse of the matrix

		std::string to_string() const;
		float& operator[](int index);
		const float& operator[](int index) const;

		const float(&GetArray() const)[9];

		static void SetFromEulers(float radX, float radY, float radZ, Matrix3f& outMatrix);
		static void LookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& normalisedUp, Matrix3f& outMatrix);
	};


	struct Vector4f {
		// Member variables
		float x=0, y=0, z=0, w=0;

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
