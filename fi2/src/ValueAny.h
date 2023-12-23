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
		explicit ValueAny(const Vector3fList& v);
		explicit ValueAny(const Exception& v);

		ValueAny(const ValueAny& other);

		ValueAny& operator=(const ValueAny& other);
		
		bool operator==(const ValueAny& other) const;

		ValueAny(const ValueAny&& other) = delete;

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
			Exception,
		};
		static std::string GetTypeAsString(ValueAny::Type type);
		std::string GetTypeAsString() const;
		std::string GetValueAsString() const;
		Color GetValueAsColor() const;
		identifier::IDType GetValueAsIDType() const;
		int32_t GetValueAsInt32() const;
		int64_t GetValueAsInt64() const;
		uint64_t GetValueAsUint64() const;
		float GetValueAsFloat() const;
		void* GetValueAsPointer() const;
		nugget::ui::Dimension GetValueAsDimension() const;
		const Vector3fList& GetValueAsVector3fList() const;
		const Exception& GetValueAsException() const;

		void SetValue(std::string val);
		void SetValue(int32_t val);
		void SetValue(int64_t val);
		void SetValue(uint64_t val);
		void SetValue(float val);
		void SetValue(const Color& colorIn);
		void SetValue(identifier::IDType id);
		void SetValue(const ValueAny& val);
		void SetValue(void* ptr);
		void SetValue(const nugget::ui::Dimension& dim);
		void SetValue(const Vector3fList& verts);
		void SetValue(const Exception& exception);
		void SetValueVoid();

		bool IsVoid() const;
		bool IsException() const;

		bool NotDeleted();

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

			Vector3fList* vector3fPtr;
		} data = {};

		Type type = Type::void_;

		static const std::unordered_map<Type, std::string> typeStringLookup;

		void CopyFrom(const ValueAny& other);
	};
}