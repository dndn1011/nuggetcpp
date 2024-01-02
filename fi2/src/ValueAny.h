#pragma once
#include <unordered_map>
#include "dimensions.h"
#include "types.h"
#include "identifier.h"

namespace nugget {
	struct ValueAny {
		ValueAny();
		explicit ValueAny(int32_t v);
		explicit ValueAny(int64_t v);
		explicit ValueAny(uint64_t v);
		explicit ValueAny(float v);
		explicit ValueAny(const Color& v);
		explicit ValueAny(const std::string& v);
		explicit ValueAny(identifier::IDType v);
		explicit ValueAny(void* v);
		explicit ValueAny(const nugget::ui::Dimension& v);
		explicit ValueAny(const Exception& v);
		explicit ValueAny(const Vector3fList& v);
		explicit ValueAny(const Vector2fList& v);
		explicit ValueAny(const ColorList& v);
		explicit ValueAny(const Vector3f& v);
		explicit ValueAny(const Vector2f& v);

		ValueAny(const ValueAny& other);
		ValueAny& operator=(const ValueAny& other);
		bool operator==(const ValueAny& other) const;
		ValueAny(ValueAny&& other) noexcept;

		enum class Type {
			void_,
			int32_t_,
			int64_t_,
			uint64_t_,
			float_,
			factor_,
			Color,
			string,
			IDType,
			pointer,
			deleted,
			dimension,
			Vector3fList,
			Vector2fList,
			ColorList,
			Vector2f,
			Vector3f,
			Exception,
		};

		static std::string TypeAsString(ValueAny::Type type);
		std::string TypeAsString() const;
		std::string AsString() const;
		Color AsColor() const;
		identifier::IDType AsIDType() const;
		int32_t AsInt32() const;
		int64_t AsInt64() const;
		uint64_t AsUint64() const;
		float AsFloat() const;
		void* AsPointer() const;
		nugget::ui::Dimension AsDimension() const;
		const Vector3fList& AsVector3fList() const;
		const Vector2fList& AsVector2fList() const;
		const ColorList& AsColorList() const;
		const Vector2f& AsVector2f() const;
		const Vector3f& AsVector3f() const;
		const Exception& AsException() const;

		void Set(std::string val);
		void Set(int32_t val);
		void Set(int64_t val);
		void Set(uint64_t val);
		void Set(float val);
		void Set(const Color& colorIn);
		void Set(identifier::IDType id);
		void Set(const ValueAny& val);
		void Set(void* ptr);
		void Set(const nugget::ui::Dimension& dim);
		void Set(const Vector3fList& vects);
		void Set(const Vector2fList& vects);
		void Set(const ColorList& vects);
		void Set(const Vector3f& vect);
		void Set(const Vector2f& vect);
		void Set(const Exception& exception);
		void SetVoid();

		bool IsVoid() const;
		bool IsException() const;

		bool NotDeleted() const;

		void MarkDeleted();

		~ValueAny();

		Type GetType() const;

	private:
		union Data {
			int32_t int32_t_;
			int64_t int64_t_;
			uint64_t uint64_t_;
			float float_;
			Color* colorPtr;
			Exception* exceptionPtr;
			std::string* stringPtr;
			identifier::IDType idType;
			void* ptr;
			nugget::ui::Dimension* dimensionPtr;

			Vector3fList* vector3fListPtr;
			Vector2fList* vector2fListPtr;
			ColorList* colorListPtr;
			Vector3f* vector3fPtr;
			Vector2f* vector2fPtr;
		} data = {};

		Type type = Type::void_;

		static const std::unordered_map<Type, std::string> typeStringLookup;

		void CopyFrom(const ValueAny& other);
	};
}