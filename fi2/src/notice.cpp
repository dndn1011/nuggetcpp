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
#include "utils/utils.h"


namespace nugget {
	namespace Notice {
		using namespace identifier;

		bool notifyLock = false;
		 
		struct Data {
			std::unordered_map<IDType, ValueAny> valueEntries;
			std::unordered_map<IDType, std::vector<Handler>> handlers;
			std::unordered_set<IDType> changes;
		};

		static Data data;

		// pass in both id and the entry to save another lookup
		void Notify(IDType id) {
			if (!notifyLock) {
				if (data.handlers.contains(id)) {
					auto& list = data.handlers.at(id);
					for (auto& x : list) {
						x.func(x.changeId);
					}
				}
			} else {
				data.changes.insert(id);
			}
		}

		void Remove(IDType id)
		{
			if (KeyExists(id)) {
				const auto& entry = data.valueEntries.at(id);
				if (entry.GetType() == ValueAny::Type::parent_) {  // if it is a parent node
					std::vector<IDType> children;
					if (GetChildren(id, children/*fill*/)) {
						// mark all children deleted first
						for (const auto& x : children) {
							Remove(x);
						}
						// notify parent then mark parent deleted
						Notify(id);
						data.valueEntries.at(id).MarkDeleted();
					} else {
						assert(0);
					}
				} else {
					data.valueEntries.at(id).MarkDeleted();
				}
			}
		}

		void LockNotifications() {
		}
		void UnlockNotifications() {
			// when unlocking, fire off the pending notifications
			for (auto& x : data.changes) {
				assert(KeyExists(x));
				auto& entry = data.handlers[x];
				for (auto& y : entry) {
					y.func(y.changeId);
				}
			}
			data.changes.clear();
			notifyLock = false;
		}

		std::string AsString(IDType id) {
			if (KeyExists(id)) {
				auto &v = data.valueEntries.at(id);
				return v.AsString();
			} else {
				return "<undefined>";
			}
		}

		std::string GetValueTypeAsString(const ValueAny& var) {
			return  var.TypeAsString();
		}

		bool IsValueTypeInteger64(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::int64_t_;
		}
		bool IsValueTypeUnsignedInteger64(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::uint64_t_;
		}
		bool IsValueTypeInteger32(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::int32_t_;
		}
		bool IsValueTypeFloat(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::float_;
		}
		bool IsValueTypeString(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::string;
		}
		bool IsValueTypeIdentifier(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::IDType;
		}
		bool IsValueTypeColor(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::Color;
		}
		bool IsValueTypeDimension(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::dimension;
		}
		bool IsValueTypeMatrix4f(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::Matrix4f;
		}
		bool IsValueTypeParent(IDType id) {
			if (!KeyExists(id)) {
				return false;   // the parent node should exist and be of type 'parent'
			} else {
				auto& v = data.valueEntries.at(id);
				return v.GetType() == ValueAny::Type::parent_;
			}
		}
		std::string GetString(IDType id) {
			assert(KeyExists(id));
			auto &v = data.valueEntries.at(id);
			return v.AsString();
		}
		int32_t GetInt32(IDType id) {
			assert(KeyExists(id));
			auto &v = data.valueEntries.at(id);
			return v.AsInt32();
		}
		int64_t GetInt64(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.AsInt64();
		}
		uint64_t GetUint64(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.AsUint64();
		}
		float GetFloat(IDType id) {
			assert(KeyExists(id));
			auto &v = data.valueEntries.at(id);
			return v.AsFloat();
		}
		IDType GetID(IDType id) {
			assert(KeyExists(id));
			auto &v = data.valueEntries.at(id);
			return v.AsIDType();
		}
		void* GetPointer(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return  v.AsPointer();
		}
		Dimension GetDimension(IDType id)
		{
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.AsDimension();
		}
		const Vector3fList& GetVector3fList(IDType id)
		{
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.AsVector3fList();
		}
		bool GetVector3fList(IDType id, Vector3fList& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::Vector3fList) {
					result = v.AsVector3fList();
					return true;
				}
			}
			return false;
		}
		bool nugget::Notice::GetVector2fList(IDType id, Vector2fList& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::Vector2fList) {
					result = v.AsVector2fList();
					return true;
				}
			}
			return false;
		}
		bool nugget::Notice::GetColorList(IDType id, ColorList& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::ColorList) {
					result = v.AsColorList();
					return true;
				}
			}
			return false;
		}
		const ValueAny& GetValueAny(IDType id) {
			if (KeyExists(id)) {
				return data.valueEntries[id];
			}
			assert(0);
			return data.valueEntries[id];
		}
		const nugget::Color &GetColor(IDType id) {
			if (KeyExists(id)) {
				auto& v = data.valueEntries[id];
				assert(v.GetType() == ValueAny::Type::Color);
				return v.AsColor();
			} else {
				return Color::defaultValue;
			}
		}
		const nugget::Vector4f& GetVector4f(IDType id) {
			if (KeyExists(id)) {
				auto& v = data.valueEntries[id];
				assert(v.GetType() == ValueAny::Type::Vector4f);
				return v.AsVector4f();
			} else {
				return Vector4f::defaultValue;
			}
		}
		const Matrix4f& GetMatrix4f(IDType id) {
			assert(KeyExists(id));
			auto& v = data.valueEntries.at(id);
			return v.AsMatrix4f();  // Assuming ValueAny has an AsMatrix4f method
		}
		void SetAsParent(IDType id)
		{
			auto& entry = data.valueEntries[id];
			entry.SetAsParent();
		}

		template <typename T>
		void Set(IDType id, const T& value) {
			if (data.valueEntries.contains(id)) {
				const auto& currentValue = data.valueEntries.at(id);
				if (currentValue.GetType() != ValueAny::Type::deleted) {
					bool diff = (currentValue != ValueAny(value));
					data.valueEntries[id].Set(value);
					if (diff) {
						Notify(id);
					}
				} else {
					data.valueEntries[id].Set(value);
				}
			} else {
				auto parent = Notice::GetParentTry(id);
				check(parent != IDType::null && Notice::IsValueTypeParent(parent), "Cannot make a child of a leaf node, or parent does not exist: {}\n",IDToString(id));
				auto r = data.valueEntries.emplace(id,value);
				check(r.second, ("Emplace failed"));
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
		template void Set<Matrix4f>(IDType id, const Matrix4f& value);
		template void Set<Vector4f>(IDType id, const Vector4f& value);

		void RegisterHandler(const Handler &handler) {
			if (!data.handlers.contains(handler.changeId)) {
				data.handlers[handler.changeId] = {};
			}
			data.handlers.at(handler.changeId).push_back(handler);
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
			if (!data.valueEntries.contains(id)) {
				return false;
			}
			if (!data.valueEntries.at(id).NotDeleted()) {
//				output("Node deleted: {}\n", IDToString(id));
				return false;
			}
			return true;
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
