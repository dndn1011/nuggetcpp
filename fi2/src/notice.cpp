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

		Notice::Board gBoard;

		bool notifyLock = false;
		 
		struct Data {
			std::unordered_map<IDType, ValueAny> valueEntries;
			std::unordered_map<IDType, std::vector<Handler>> handlers;
			std::unordered_set<IDType> changes;
			void Clear() {
				valueEntries.clear();
				handlers.clear();
				changes.clear();
			}
		};

		Board::Board() : data(*(new nugget::Notice::Data())) {
		}

		Board::~Board() {
			delete& data;
		}

		// pass in both id and the entry to save another lookup
		void Board::Notify(IDType id) {
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

		void Board::Remove(IDType id)
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

		void Board::LockNotifications() {
		}
		void Board::UnlockNotifications() {
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

		std::string Board::AsString(IDType id) {
			if (KeyExists(id)) {
				auto &v = data.valueEntries.at(id);
				return v.AsString();
			} else {
				return "<undefined>";
			}
		}

		std::string Board::GetValueTypeAsString(const ValueAny& var) {
			return  var.TypeAsString();
		}

		bool Board::IsValueTypeInteger64(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::int64_t_;
		}
		bool Board::IsValueTypeUnsignedInteger64(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::uint64_t_;
		}
		bool Board::IsValueTypeInteger32(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::int32_t_;
		}
		bool Board::IsValueTypeFloat(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::float_;
		}
		bool Board::IsValueTypeString(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::string;
		}
		bool Board::IsValueTypeIdentifier(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::IDType;
		}
		bool Board::IsValueTypeColor(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::Color;
		}
		bool Board::IsValueTypeDimension(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::dimension;
		}
		bool Board::IsValueTypeMatrix4f(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.GetType() == ValueAny::Type::Matrix4f;
		}
		bool Board::IsValueTypeParent(IDType id) {
			if (!KeyExists(id)) {
				return false;   // the parent node should exist and be of type 'parent'
			} else {
				auto& v = data.valueEntries.at(id);
				return v.GetType() == ValueAny::Type::parent_;
			}
		}
		std::string Board::GetString(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto &v = data.valueEntries.at(id);
			return v.AsString();
		}
		int32_t Board::GetInt32(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto &v = data.valueEntries.at(id);
			return v.AsInt32();
		}
		int64_t Board::GetInt64(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.AsInt64();
		}
		uint64_t Board::GetUint64(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.AsUint64();
		}
		float Board::GetFloat(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto &v = data.valueEntries.at(id);
			return v.AsFloat();
		}
		IDType Board::GetID(IDType id) const {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto &v = data.valueEntries.at(id);
			return v.AsIDType();
		}
		void* Board::GetPointer(IDType id) {
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return  v.AsPointer();
		}

		Dimension Board::GetDimension(IDType id)
		{
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.AsDimension();
		}
		const Vector3fList& Board::GetVector3fList(IDType id)
		{
			check(KeyExists(id),"Key not found: {}\n",IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.AsVector3fList();
		}
		bool Board::GetVector3fList(IDType id, Vector3fList& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::Vector3fList) {
					result = v.AsVector3fList();
					return true;
				}
			}
			return false;
		}
		bool Board::GetInt64List(IDType id, Int64List& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::Int64List) {
					result = v.AsInt64List();
					return true;
				}
			}
			return false;
		}
		bool Board::GetVector2fList(IDType id, Vector2fList& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::Vector2fList) {
					result = v.AsVector2fList();
					return true;
				}
			}
			return false;
		}
		const Vector2fList& Board::GetVector2fList(IDType id) {
			check(KeyExists(id), "Key not found: {}\n", IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.AsVector2fList();
		}
		const ColorList& Board::GetColorList(IDType id) {
			check(KeyExists(id), "Key not found: {}\n", IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.AsColorList();
		}
		bool Board::GetColorList(IDType id, ColorList& result) {
			if (KeyExists(id)) {
				auto& v = GetValueAny(id);
				if (v.GetType() == ValueAny::Type::ColorList) {
					result = v.AsColorList();
					return true;
				}
			}
			return false;
		}
		const ValueAny& Board::GetValueAny(IDType id) const {
			if (KeyExists(id)) {
				return data.valueEntries[id];
			}
			assert(0);
			return data.valueEntries[id];
		}
		const nugget::Color& Board::GetColor(IDType id) {
			if (KeyExists(id)) {
				auto& v = data.valueEntries[id];
				assert(v.GetType() == ValueAny::Type::Color);
				return v.AsColor();
			} else {
				return Color::defaultValue;
			}
		}
		const nugget::Vector4f& Board::GetVector4f(IDType id) {
			if (KeyExists(id)) {
				auto& v = data.valueEntries[id];
				assert(v.GetType() == ValueAny::Type::Vector4f);
				return v.AsVector4f();
			} else {
				return Vector4f::defaultValue;
			}
		}
		const nugget::Vector3f& Board::GetVector3f(IDType id) {
			if (KeyExists(id)) {
				auto& v = data.valueEntries[id];
				assert(v.GetType() == ValueAny::Type::Vector3f);
				return v.AsVector3f();
			} else {
				return Vector3f::defaultValue;
			}
		}
		const Matrix4f& Board::GetMatrix4f(IDType id) {
			check(KeyExists(id), "Key not found: {}\n", IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.AsMatrix4f();  // Assuming ValueAny has an AsMatrix4f method
		}
		const Matrix3f& Board::GetMatrix3f(IDType id) {
			check(KeyExists(id), "Key not found: {}\n", IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.AsMatrix3f();  // Assuming ValueAny has an AsMatrix3f method
		}
		const IdentifierList& Board::GetIdentifierList(IDType id) {
			check(KeyExists(id), "Key not found: {}\n", IDToString(id));
			auto& v = data.valueEntries.at(id);
			return v.AsIdentifierList();  // Assuming ValueAny has an AsMatrix4f method
		}

		void Board::SetAsParent(IDType id)
		{
			auto& entry = data.valueEntries[id];
			entry.SetAsParent();
		}

		template <typename T>
		void Board::Set(IDType id, const T& value) {
			if (data.valueEntries.contains(id)) {
				const auto& currentValue = data.valueEntries.at(id);
				if (currentValue.GetType() != ValueAny::Type::deleted) {
					bool diff = (currentValue != ValueAny(value));
					data.valueEntries[id].Set(value);
					if (diff) {
						static int count = 0;
						count++;
//						output("-------------------> {} {}\n", count, IDToString(id));
						Notify(id);
					}
				} else {
					data.valueEntries[id].Set(value);
				}
			} else {
//				auto parent = Notice::GetParentTry(id);
//				check(parent != IDType::null && IsValueTypeParent(parent), "Cannot make a child of a leaf node, or parent does not exist: {}\n",IDToString(id));
				auto r = data.valueEntries.emplace(id,value);
				check(r.second, ("Emplace failed"));
			}
		}

		template void Board::Set<int64_t>(IDType id, const int64_t& value);
		template void Board::Set<uint64_t>(IDType id, const uint64_t& value);
		template void Board::Set<std::string>(IDType id, const std::string& value);
		template void Board::Set<int32_t>(IDType id, const int32_t& value);
		template void Board::Set<Color>(IDType id, const Color& value);
		template void Board::Set<float>(IDType id, const float& value);
		template void Board::Set<ValueAny>(IDType id, const ValueAny& value);
		template void Board::Set<Dimension>(IDType id, const Dimension& value);
		template void Board::Set<IDType>(IDType id, const IDType& value);
		template void Board::Set<Vector3fList>(IDType id, const Vector3fList& value);
		template void Board::Set<Vector3f>(IDType id, const Vector3f& value);
		template void Board::Set<Vector2fList>(IDType id, const Vector2fList& value);
		template void Board::Set<ColorList>(IDType id, const ColorList& value);
		template void Board::Set<Vector2f>(IDType id, const Vector2f& value);
		template void Board::Set<Matrix4f>(IDType id, const Matrix4f& value);
		template void Board::Set<Matrix3f>(IDType id, const Matrix3f& value);
		template void Board::Set<Vector4f>(IDType id, const Vector4f& val0ue);
		template void Board::Set<Int64List>(IDType id, const Int64List& value);
		template void Board::Set<IdentifierList>(IDType id, const IdentifierList& value);

		void Board::RegisterHandler(const Handler& handler) {
			if (!data.handlers.contains(handler.changeId)) {
				data.handlers[handler.changeId] = {};
			}
			data.handlers.at(handler.changeId).push_back(handler);
		}
		void Board::RegisterHandler(const Handler& handler, std::vector<Handler>& out) {
			if (!data.handlers.contains(handler.changeId)) {
				data.handlers[handler.changeId] = {};
			}
			data.handlers.at(handler.changeId).push_back(handler);
			out.push_back(handler);
		}
		void Board::RegisterHandlerOnChildren(const Handler &handler,std::vector<Handler> &out) {
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
		bool Board::KeyExists(IDType id) const {
			if (!data.valueEntries.contains(id)) {
				return false;
			}
			if (!data.valueEntries.at(id).NotDeleted()) {
//				output("Node deleted: {}\n", IDToString(id));
				return false;
			}
			return true;
		}

		// The Children Of Null
		// can save humanity

		bool Board::GetChildren(IDType id, std::vector<IDType>& fill) const {
			if (KeyExists(id) || id == IDType::null) {
				if (auto set = identifier::IDGetChildren(id)) {
					for (const auto& x : *set) {
						if (KeyExists(x)) {
							fill.push_back(x);
						} else {
							int a =0;
						}
					}
				}
				return true;
			} else {
				return false;
			}
		}

		bool Board::GetChildrenOfType(IDType id, ValueAny::Type type,std::vector<IDType>& fill) {
			if (KeyExists(id)) {
				if (auto set = identifier::IDGetChildren(id)) {
					for (const auto& x : *set) {
						if (KeyExists(x)) {
							if (GetValueAny(x).GetType() == type) {
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

		bool Board::GetChildrenWithNodeExisting(IDType id, IDType leaf, std::vector<IDType>& fill) {
			if (KeyExists(id)) {
				if (auto set = identifier::IDGetChildren(id)) {
					for (const auto& x : *set) {
//						check(KeyExists(x), "Key should exist: {}", IDToString(x));
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

		bool Board::GetChildrenWithNodeOfValue(IDType id, IDType leaf, ValueAny value,std::vector<IDType>& fill) {
			if (KeyExists(id)) {
				if (auto set = identifier::IDGetChildren(id)) {
					for (const auto& x : *set) {
						check(KeyExists(x), "Key should exist: {}", IDToString(x));
						auto xleaf = IDR(x, leaf);
						if (KeyExists(xleaf) && GetValueAny(xleaf) == value) {
							fill.push_back(x);
						}
					}
				}
				return true;
			} else {
				return false;
			}
		}

		void Board::Clear() {
			data.Clear();
		}

	}
}
