#include <iostream>
#include <filesystem>
#include <assert.h>
#include <memory>
#include <unordered_set>
#include <functional>

#include <sstream>

#include "types.h"
#include "identifier.h"

#include "Notice.h"
#include "debug.h"



namespace nugget {
	namespace Notice {
		using namespace identifier;

		bool notifyLock = false;

		struct ValueEntry {
			ValueEntry() {}

			ValueEntry(const ValueEntry& other) = delete;
			ValueEntry(const ValueEntry&& other) = delete;

			ValueEntry& operator=(const ValueEntry& other) {
				value = other.value;
				return *this;
			};
			
			ValueEntry& operator=(ValueEntry&& other) noexcept {
				value = std::move(other.value);
				return *this;
			}

			ValueEntry(const ValueAny& any) {
				value = any;
			}

			ValueAny value;
			std::vector<Handler> handlers;
		};
		 
		struct Data {
			std::unordered_map<IDType, ValueEntry> valueEntries;
			std::unordered_set<IDType> changes;
		};

		static Data data;

		// pass in both id and the entry to save another lookup
		void Notify(IDType id, const ValueEntry&entry) {
			if (!notifyLock) {
				for (auto& x : entry.handlers) {
					x.func(x.changeId);
				}
			} else {
				data.changes.insert(id);
			}
		}

		void Remove(IDType id)
		{
			if (KeyExists(id)) {
				const auto& entry = data.valueEntries.at(id);
				if (entry.value.GetType() == ValueAny::Type::void_) {  // if it is a parent node
					std::vector<IDType> children;
					if (GetChildren(id, children/*fill*/)) {
						// mark all children deleted first
						for (const auto& x : children) {
							Remove(x);
						}
						// notify parent then mark parent deleted
						Notify(id, entry);
						data.valueEntries.at(id).value.MarkDeleted();
					} else {
						assert(0);
					}
				} else {
					data.valueEntries.at(id).value.MarkDeleted();
				}
			}
		}

		void LockNotifications() {
		}
		void UnlockNotifications() {
			// when unlocking, fire off the pending notifications
			for (auto& x : data.changes) {
				assert(KeyExists(x));
				auto& entry = data.valueEntries[x];
				for (auto& y : entry.handlers) {
					y.func(y.changeId);
				}
			}
			data.changes.clear();
			notifyLock = false;
		}

		std::string GetValueAsString(IDType id) {
			if (KeyExists(id)) {
				auto &v = data.valueEntries.at(id);
				return v.value.GetValueAsString();
			} else {
				return "<undefined>";
			}
		}

		std::string GetValueTypeAsString(const ValueAny& var) {
			return  var.GetTypeAsString();
		}

		bool IsValueTypeInteger64(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetType() == ValueAny::Type::int64_t_;
		}
		bool IsValueTypeUnsignedInteger64(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetType() == ValueAny::Type::uint64_t_;
		}
		bool IsValueTypeInteger32(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetType() == ValueAny::Type::int32_t_;
		}
		bool IsValueTypeFloat(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetType() == ValueAny::Type::float_;
		}
		bool IsValueTypeString(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetType() == ValueAny::Type::string;
		}
		bool IsValueTypeIdentifier(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetType() == ValueAny::Type::IDType;
		}
		bool IsValueTypeColor(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetType() == ValueAny::Type::Color;
		}
		bool IsValueTypeDimension(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetType() == ValueAny::Type::dimension;
		}
		std::string GetString(IDType id) {
			assert(KeyExists(id));
			auto &v = data.valueEntries.at(id);
			return v.value.GetValueAsString();
		}
		int32_t GetInt32(IDType id) {
			assert(KeyExists(id));
			auto &v = data.valueEntries.at(id);
			return v.value.GetValueAsInt32();
		}
		int64_t GetInt64(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetValueAsInt64();
		}
		uint64_t GetUint64(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetValueAsUint64();
		}
		float GetFloat(IDType id) {
			assert(KeyExists(id));
			auto &v = data.valueEntries.at(id);
			return v.value.GetValueAsFloat();
		}
		IDType GetID(IDType id) {
			assert(KeyExists(id));
			auto &v = data.valueEntries.at(id);
			return v.value.GetValueAsIDType();
		}
		void* GetPointer(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return  v.value.GetValueAsPointer();
		}
		Dimension GetDimension(IDType id)
		{
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetValueAsDimension();
		}
		const Vector3fList& GetVector3fList(IDType id)
		{
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.value.GetValueAsVector3fList();
		}
		bool GetVector3fList(IDType id, Vector3fList& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::Vector3fList) {
					result = v.GetValueAsVector3fList();
					return true;
				}
			}
			return false;
		}
		bool nugget::Notice::GetVector2fList(IDType id, Vector2fList& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::Vector2fList) {
					result = v.GetValueAsVector2fList();
					return true;
				}
			}
			return false;
		}
		bool nugget::Notice::GetColorList(IDType id, ColorList& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::ColorList) {
					result = v.GetValueAsColorList();
					return true;
				}
			}
			return false;
		}
		const ValueAny& GetValueAny(IDType id) {
			if (KeyExists(id)) {
				return data.valueEntries[id].value;
			}
			assert(0);
			return data.valueEntries[id].value;
		}
		nugget::Color GetColor(IDType id) {
			if (KeyExists(id)) {
				auto &v = data.valueEntries[id];
				assert(v.value.GetType() == ValueAny::Type::Color);
				return v.value.GetValueAsColor();
			} else {
				return nugget::Color(0.5, 0.5, 0.5, 1);
			}
		}

		void SetVoid(IDType id)
		{
			auto& entry = data.valueEntries[id];
			entry.value.SetValueVoid();
		}

		template <typename T>
		void Set(IDType id, const T& value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != ValueAny(value)) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(ValueAny(value));
			}
		}

		template void Set<int64_t>(IDType id, const int64_t& value);
		template void Set<uint64_t>(IDType id, const uint64_t& value);
		template void Set<std::string>(IDType id, const std::string& value);
		template void Set<int32_t>(IDType id, const int32_t& value);
		template void Set<Color>(IDType id, const Color& value);
		template void Set<float>(IDType id, const float& value);
		template void Set<ValueAny>(IDType id, const ValueAny& value);
		template void Set<Dimension>(IDType id, const Dimension& value);
		template void Set<IDType>(IDType id, const IDType& value);
		template void Set<Vector3fList>(IDType id, const Vector3fList& value);
		template void Set<Vector3f>(IDType id, const Vector3f& value);
		template void Set<Vector2fList>(IDType id, const Vector2fList& value);
		template void Set<ColorList>(IDType id, const ColorList& value);
		template void Set<Vector2f>(IDType id, const Vector2f& value);

		void RegisterHandler(const Handler &handler) {
			check(KeyExists(handler.changeId), "Could not find node for handler registration: {}\n", IDToString(handler.changeId));
			data.valueEntries.at(handler.changeId).handlers.push_back(handler);
		}
		void RegisterHandlerOnChildren(const Handler &handler,std::vector<Handler> &out) {
			std::vector<IDType> children;
			auto r = GetChildren(handler.notifyId,children /*fill*/);
			assert(r);
			for (auto& x : children) {
				auto h = Handler{ handler.notifyId, x, handler.func };
				out.push_back(h);
				RegisterHandler(h);
			}
		}

		// Check key exists and is not marked deleted
		bool KeyExists(IDType id) {
			return data.valueEntries.contains(id) && data.valueEntries.at(id).value.NotDeleted();
		}

		bool GetChildren(IDType id, std::vector<IDType>& fill) {
			if (KeyExists(id)) {
				if (auto set = identifier::IDGetChildren(id)) {
					for (const auto& x : *set) {
						if (KeyExists(x)) {
							fill.push_back(x);
						}
					}
				}
				return true;
			} else {
				return false;
			}
		}

		bool GetChildrenOfType(IDType id, ValueAny::Type type,std::vector<IDType>& fill) {
			if (KeyExists(id)) {
				if (auto set = identifier::IDGetChildren(id)) {
					for (const auto& x : *set) {
						if (KeyExists(x)) {
							if (Notice::GetValueAny(x).GetType() == type) {
								fill.push_back(x);
							}
						}
					}
				}
				return true;
			} else {
				return false;
			}
		}

		bool GetChildrenWithNodeExisting(IDType id, IDType leaf, std::vector<IDType>& fill) {
			if (KeyExists(id)) {
				if (auto set = identifier::IDGetChildren(id)) {
					for (const auto& x : *set) {
						check(KeyExists(x), "Key should exist: {}", IDToString(x));
						if (KeyExists(IDR(x, leaf))) {
							fill.push_back(x);
						}
					}
				}
				return true;
			} else {
				return false;
			}
		}

		bool GetChildrenWithNodeOfValue(IDType id, IDType leaf, ValueAny value,std::vector<IDType>& fill) {
			if (KeyExists(id)) {
				if (auto set = identifier::IDGetChildren(id)) {
					for (const auto& x : *set) {
						check(KeyExists(x), "Key should exist: {}", IDToString(x));
						auto xleaf = IDR(x, leaf);
						if (KeyExists(xleaf) && Notice::GetValueAny(xleaf) == value) {
							fill.push_back(x);
						}
					}
				}
				return true;
			} else {
				return false;
			}
		}


	}
}
