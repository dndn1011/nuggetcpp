#pragma once
#include <functional>
#include "types.h"
#include "identifier.h"
#include "dimensions.h"

namespace nugget {
	namespace Notice {
		using namespace identifier;
		using Dimension = nugget::ui::Dimension;

		struct ValueAny {
			ValueAny();

			explicit ValueAny(int32_t v);
			explicit ValueAny(int64_t v);
			explicit ValueAny(uint64_t v);
			explicit ValueAny(float v);
			explicit ValueAny(const Color& v);
			explicit ValueAny(const std::string& v);
			explicit ValueAny(IDType v);
			explicit ValueAny(void* v);
			explicit ValueAny(const Dimension& v);
			explicit ValueAny(const Vector3fList& v);

			ValueAny(const ValueAny & other);

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
			};

			std::string GetTypeAsString() const;
			std::string GetValueAsString() const;
			Color GetValueAsColor() const;
			IDType GetValueAsIDType() const;
			int32_t GetValueAsInt32() const;
			int64_t GetValueAsInt64() const;
			uint64_t GetValueAsUint64() const;
			float GetValueAsFloat() const;
			void* GetValueAsPointer() const;
			Dimension GetValueAsDimension() const;
			const Vector3fList& GetValueAsVector3fList() const;
			
			void SetValue(std::string val);
			void SetValue(int32_t val);
			void SetValue(int64_t val);
			void SetValue(uint64_t val);
			void SetValue(float val);
			void SetValue(const Color& colorIn);
			void SetValue(identifier::IDType id);
			void SetValue(const ValueAny& val);
			void SetValue(void* ptr);
			void SetValue(const Dimension& dim);
			void SetValue(const Vector3fList& verts);
			void SetValueVoid();

			bool IsVoid();
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
				std::string* stringPtr;
				IDType idType;
				void* ptr;
				Dimension* dimensionPtr;

				Vector3fList* vector3fPtr;
			} data = {};

			Type type = Type::void_;

			static const std::unordered_map<Type, std::string> typeStringLookup;

			void CopyFrom(const ValueAny& other);
		};

		using HandlerFunc = std::function<void(IDType changedID)>;

		struct Handler {
			Handler(IDType a, IDType b, HandlerFunc func) : notifyId(a), changeId(b), func(func), uid(++uniqueId) {
			}
			Handler(IDType a, HandlerFunc func) : notifyId(a), changeId(a), func(func), uid(++uniqueId) {
			}
			IDType notifyId;
			IDType changeId;
			HandlerFunc func;
			size_t uid;
		private:
			static inline size_t uniqueId;
		};



		std::string		GetString(IDType id);
		int32_t			GetInt32(IDType id);
		int64_t			GetInt64(IDType id);
		uint64_t		GetUint64(IDType id);
		float			GetFloat(IDType id);
		nugget::Color	GetColor(IDType id);
		IDType			GetID(IDType id);
		const ValueAny& GetValueAny(IDType id);
		void*           GetPointer(IDType id);
		Dimension       GetDimension(IDType id);
		const Vector3fList& GetVector3fList(IDType id);

		std::string GetValueAsString(IDType id);
		std::string GetValueTypeAsString(const ValueAny& var);

		template <typename T>
		void Set(IDType id, const T& value);

		void SetVoid(IDType id);

		void Remove(IDType id);

		bool IsValueTypeInteger64(IDType id);
		bool IsValueTypeUnsignedInteger64(IDType id);
		bool IsValueTypeInteger32(IDType id);
		bool IsValueTypeFloat(IDType id);
		bool IsValueTypeString(IDType id);
		bool IsValueTypeIdentifier(IDType id);
		bool IsValueTypeColor(IDType id);
		bool IsValueTypeDimension(IDType id);
		bool IsValueTypeVertices(IDType id);

		void RegisterHandler(const Handler& handler);
		void UnregisterHandler(const Handler& handler);
			
		void RegisterHandlerOnChildren(const Handler& handler, std::vector<Handler>& out);

		void LockNotifications();
		void UnlockNotifications();
		bool KeyExists(IDType id);

		bool GetChildren(IDType, std::vector<IDType>& fill);
		bool GetChildrenWithNodeExisting(IDType id, IDType leaf, std::vector<IDType>& fill);
		bool GetChildrenWithNodeOfValue(IDType id, IDType leaf, Notice::ValueAny value, std::vector<IDType>& fill);
	}
}