#pragma once
#include <functional>
#include "types.h"
#include "identifier.h"
#include "ValueAny.h"

namespace nugget {
	namespace Notice {
		using namespace identifier;
		using Dimension = nugget::ui::Dimension;

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
		bool GetChildrenWithNodeOfValue(IDType id, IDType leaf, ValueAny value, std::vector<IDType>& fill);
	}
}