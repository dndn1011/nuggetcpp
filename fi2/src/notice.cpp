#include <iostream>
#include <filesystem>
#include <assert.h>
#include <memory>
#include <unordered_set>
#include <functional>

#include <SFML/Graphics.hpp>
#include <sstream>

#include "types.h"

#include "identifier.h"

#include "Notice.h"
#include "debug.h"



namespace nugget {
	namespace Notice {
		using namespace identifier;
		using namespace identifier;

		bool notifyLock = false;

		ValueAny::ValueAny() {}

		ValueAny::ValueAny(int32_t v) : type(Type::int32_t_), data{ .int32_t_ = v } {}
		ValueAny::ValueAny(int64_t v) : type(Type::int64_t_), data{ .int64_t_ = v } {}
		ValueAny::ValueAny(float v) : type(Type::float_), data{ .float_ = v } {}
		ValueAny::ValueAny(const Color& v) : type(Type::Color), data{ .colorPtr = new Color(v) } {}
		ValueAny::ValueAny(const std::string& v) : type(Type::string), data{ .stringPtr = new std::string(v) } {}
		ValueAny::ValueAny(IDType v) : type(Type::IDType), data{ .idType = v } {}
		ValueAny::ValueAny(void *ptr) : type(Type::pointer), data{ .ptr = ptr } {}
		ValueAny::ValueAny(const Dimension& v) : type(Type::dimension), data{ .dimensionPtr = new Dimension(v) } {}

		ValueAny::ValueAny(const ValueAny& other) {
			CopyFrom(other);
		}

		ValueAny& ValueAny::operator=(const ValueAny& other) {
			CopyFrom(other);
			return *this;
		}

		bool ValueAny::operator==(const ValueAny& other) const {
			if (type != other.type) {
				return false;
			} else {
				switch (type) {
					case ValueAny::Type::int32_t_: {
						return data.int32_t_ == other.data.int32_t_;
					} break;
					case ValueAny::Type::int64_t_: {
						return data.int32_t_ == other.data.int32_t_;
					} break;
					case ValueAny::Type::float_: {
						return data.float_ == other.data.float_;
					} break;
					case ValueAny::Type::string: {
						return *data.stringPtr == *other.data.stringPtr;
					} break;
					case ValueAny::Type::IDType: {
						return data.idType == other.data.idType;
					} break;
					case ValueAny::Type::Color: {
						return *data.colorPtr == *other.data.colorPtr;
					} break;
					case ValueAny::Type::void_: {
						return true;
					} break;
					case ValueAny::Type::dimension: {
						return *data.dimensionPtr == *other.data.dimensionPtr;
					} break;
					default: {
						assert(0);
					} break;
				}
			}
			return false;
		}

		std::string ValueAny::GetTypeAsString() const {
			assert(typeStringLookup.contains(type));
			return typeStringLookup.at(type);
		}
		std::string ValueAny::GetValueAsString() const {
			switch (type) {
				case ValueAny::Type::int32_t_: {
					return std::to_string(data.int32_t_);
				} break;
				case ValueAny::Type::int64_t_: {
					return std::to_string(data.int64_t_);
				} break;
				case ValueAny::Type::float_: {
					return std::to_string(data.float_);
				} break;
				case ValueAny::Type::string: {
					return *data.stringPtr;
				} break;
				case ValueAny::Type::IDType: {
					return std::to_string(data.idType);
				} break;
				case ValueAny::Type::Color: {
					return data.colorPtr->to_string();
				} break;
				case ValueAny::Type::dimension: {
					return data.dimensionPtr->ToString();
				} break;
				case ValueAny::Type::void_: {
					return "<void>";
				} break;
				default: {
					assert(0);
				} break;
			}
			return "<undefined";
		}
		Color ValueAny::GetValueAsColor() const {
			assert(type == Type::Color);
			return *data.colorPtr;
		}
		IDType ValueAny::GetValueAsIDType() const {
			assert(type == Type::IDType);
			return data.idType;
		}
		int32_t ValueAny::GetValueAsInt32() const {
			assert(type == Type::int32_t_);
			return data.int32_t_;
		}
		int64_t ValueAny::GetValueAsInt64() const {
			assert(type == Type::int64_t_);
			return data.int64_t_;
		}
		float ValueAny::GetValueAsFloat() const {
			assert(type == Type::float_);
			return data.float_;
		}
		void* ValueAny::GetValueAsPointer() const {
			assert(type == Type::pointer);
			return data.ptr;
		}
		Dimension ValueAny::GetValueAsDimension() const
		{
			assert(type == Type::dimension);
			return *data.dimensionPtr;
		}
		void ValueAny::SetValue(std::string val) {
			assert(type == Type::string);
			if (data.stringPtr != nullptr) {
				delete 	data.stringPtr;
			}
			data.stringPtr = new std::string(val);
		}
		void ValueAny::SetValue(int32_t val) {
			assert(type == Type::int32_t_);
			data.int32_t_ = val;
		}
		void ValueAny::SetValue(int64_t val) {
			assert(type == Type::int64_t_);
			data.int64_t_ = val;
		}
		void ValueAny::SetValue(float val) {
			assert(type == Type::float_);
			data.float_ = val;
		}
		void ValueAny::SetValue(const Color& colorIn) {
			assert(type == Type::Color);
			if (data.colorPtr != nullptr) {
				delete 	data.colorPtr;
			}
			data.colorPtr = new Color(colorIn);
		}
		void ValueAny::SetValue(identifier::IDType id) {
			assert(type == Type::IDType);
			data.idType = id;
		}
		void ValueAny::SetValue(void* val) {
			assert(type == Type::pointer);
			data.ptr = val;
		}
		void ValueAny::SetValue(const Dimension& dim)
		{
			assert(type == Type::dimension);
			if (data.dimensionPtr != nullptr) {
				delete 	data.dimensionPtr;
			}
			data.dimensionPtr = new Dimension(dim);
		}
		void ValueAny::SetValueVoid() {
			type = Type::void_;
		}
		void ValueAny::SetValue(const ValueAny &val) {
			CopyFrom(val);
		}

		bool ValueAny::IsVoid() {
			return type == Type::void_;
		}

		bool ValueAny::NotDeleted()
		{
			return type != Type::deleted;;
		}

		void ValueAny::MarkDeleted() {
			type = Type::deleted;
		}


		ValueAny::~ValueAny() {
			switch (type) {
				case Type::string: {
					delete data.stringPtr;
				} break;
				case Type::Color: {
					delete data.colorPtr;
				} break;
			}
		}

		ValueAny::Type ValueAny::GetType() const {
			return type;
		}

		void ValueAny::CopyFrom(const ValueAny& other) {
			type = other.type;
			switch (type) {
				case Type::int64_t_: {
					data.int64_t_ = other.data.int64_t_;
				} break;
				case Type::int32_t_: {
					data.int32_t_ = other.data.int32_t_;
				} break;
				case Type::float_: {
					data.float_ = other.data.float_;
				} break;
				case Type::IDType: {
					data.idType = other.data.idType;
				} break;
				case Type::Color: {
					if (data.colorPtr) {
						delete data.colorPtr;
					}
					data.colorPtr = new Color(*other.data.colorPtr);
				} break;
				case Type::string: {
					if (data.stringPtr) {
						delete data.stringPtr;
					}
					data.stringPtr = new std::string(*other.data.stringPtr);
				} break;
				case Type::dimension: {
					if (data.dimensionPtr) {
						delete data.dimensionPtr;
					}
					data.dimensionPtr = new Dimension(*other.data.dimensionPtr);
				} break;
				case Type::pointer: {
					data.ptr = other.data.ptr;
				} break;
				case Type::void_: {
					/* do nothing */
				} break;
				default:
					assert(("unhandled type copy", 0));
			}
		}
		/*static*/
		const std::unordered_map<ValueAny::Type,std::string> ValueAny::typeStringLookup = {
			{ValueAny::Type::void_,"void"},
			{ValueAny::Type::int32_t_,"int32_t_"},
			{ValueAny::Type::int64_t_,"int64_t_"},
			{ValueAny::Type::Color,"Color"},
			{ValueAny::Type::float_,"float"},
		};

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
			auto &v = data.valueEntries.at(id);
			return v.value.GetValueAsInt64();
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

		void Set(IDType id, const std::string& value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				std::string current = entry.value.GetValueAsString();
				assert(("Cannot change type of an existing entry", entry.value.GetType() == ValueAny::Type::string));
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != value) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(ValueAny(value));
			}
		}
		void Set(IDType id, int32_t value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				int32_t current = entry.value.GetValueAsInt32();
				assert(("Cannot change type of an existing entry", entry.value.GetType() == ValueAny::Type::int32_t_));
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != value) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(ValueAny(value));
			}
		}
		void Set(IDType id, int64_t value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				int64_t current = entry.value.GetValueAsInt64();
				assert(("Cannot change type of an existing entry", entry.value.GetType() == ValueAny::Type::int64_t_));
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != value) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(ValueAny(value));
			}
		}
		void Set(IDType id, float value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				float current = entry.value.GetValueAsFloat();
				assert(("Cannot change type of an existing entry", entry.value.GetType() == ValueAny::Type::float_));
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != value) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(ValueAny(value));
			}
		}
		void Set(IDType id, Color value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				Color current = entry.value.GetValueAsColor();
				assert(("Cannot change type of an existing entry", entry.value.GetType() == ValueAny::Type::Color));
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != value) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(ValueAny(value));
			}
		}
		void Set(IDType id, IDType value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				IDType current = entry.value.GetValueAsIDType();
				assert(("Cannot change type of an existing entry", entry.value.GetType() == ValueAny::Type::IDType));
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != value) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(ValueAny(value));
			}
		}

		void Set(IDType id, const ValueAny& value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				auto currentType = entry.value.GetType();
				assert(("Cannot change the type of an existing entry", entry.value.GetType() == currentType));
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != value) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(value);
			}
		}

		void Set(IDType id, void* value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				auto currentType = entry.value.GetType();
				assert(("Cannot change the type of an existing entry", entry.value.GetType() == currentType));
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != value) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(value);
			}
		}

		void Set(IDType id, const Dimension &value) {
			if (KeyExists(id)) {
				auto& entry = data.valueEntries[id];
				auto currentType = entry.value.GetType();
				assert(("Cannot change the type of an existing entry", entry.value.GetType() == currentType));
				auto currentValue = entry.value;
				entry.value.SetValue(value);
				if (currentValue != value) {
					Notify(id, entry);
				}
			} else {
				data.valueEntries[id] = ValueEntry(value);
			}
		}

		void UnregisterHandler(const Handler& handler) {
			assert(data.valueEntries.contains(handler.changeId));
			auto& list = data.valueEntries.at(handler.changeId).handlers;
			for (int i = 0; i < list.size();i++) {
				if (list[i].uid == handler.uid) {
					list.erase(list.begin()+i);
					break;
				}
			}
		}

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

		bool GetChildrenWithNodeOfValue(IDType id, IDType leaf, Notice::ValueAny value,std::vector<IDType>& fill) {
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
