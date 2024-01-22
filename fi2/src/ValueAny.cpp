#include "types.h"
#include "identifier.h"
#include "ValueAny.h"
#include "dimensions.h"
#include <sstream>
#include <format>

namespace nugget {
	using namespace identifier;

	ValueAny::ValueAny() : type(Type::void_) {}

	ValueAny::ValueAny(int32_t v) : type(Type::int32_t_), data{ .int32_t_ = v } {}
	ValueAny::ValueAny(int64_t v) : type(Type::int64_t_), data{ .int64_t_ = v } {}
	ValueAny::ValueAny(uint64_t v) : type(Type::uint64_t_), data{ .uint64_t_ = v } {}
	ValueAny::ValueAny(float v) : type(Type::float_), data{ .float_ = v } {}
	ValueAny::ValueAny(const Color& v) : type(Type::Color), data{ .colorPtr = new Color(v) } {}
	ValueAny::ValueAny(const std::string& v) : type(Type::string), data{ .stringPtr = new std::string(v) } {}
	ValueAny::ValueAny(IDType v) : type(Type::IDType), data{ .idType = v } {}
	ValueAny::ValueAny(void* ptr) : type(Type::pointer), data{ .ptr = ptr } {}
	ValueAny::ValueAny(const nugget::ui::Dimension& v) : type(Type::dimension), data{ .dimensionPtr = new nugget::ui::Dimension(v) } {}
	ValueAny::ValueAny(const Vector3fList& v) : type(Type::Vector3fList), data{ .vector3fListPtr = new Vector3fList(v) } {}
	ValueAny::ValueAny(const Vector2fList& v) : type(Type::Vector2fList), data{ .vector2fListPtr = new Vector2fList(v) } {}
	ValueAny::ValueAny(const ColorList& v) : type(Type::ColorList), data{ .colorListPtr = new ColorList(v) } {}
	ValueAny::ValueAny(const Vector3f& v) : type(Type::Vector3f), data{ .vector3fPtr = new Vector3f(v) } {}
	ValueAny::ValueAny(const Vector2f& v) : type(Type::Vector2f), data{ .vector2fPtr = new Vector2f(v) } {}
	ValueAny::ValueAny(const Matrix4f& v) : type(Type::Matrix4f), data{ .matrix4fPtr = new Matrix4f(v) } {}
	ValueAny::ValueAny(const Vector4f& v) : type(Type::Vector4f), data{ .vector4fPtr = new Vector4f(v) } {}
	ValueAny::ValueAny(const Int64List& v) : type(Type::Int64List), data{ .int64ListPtr = new Int64List(v) } {}

	ValueAny::ValueAny(const Exception& v) : type(Type::Exception), data{ .exceptionPtr = new Exception(v) } {}

	ValueAny::ValueAny(const ValueAny& other) {
		assert(other.NotDeleted());
		CopyFrom(other);
	}

	ValueAny::ValueAny(ValueAny&& other) noexcept {
		assert(other.NotDeleted());
		CopyFrom(other);
	}

	ValueAny& ValueAny::operator=(const ValueAny& other) {
		assert(other.NotDeleted() && NotDeleted());
		CopyFrom(other);
		return *this;
	}

	bool ValueAny::operator==(const ValueAny& other) const {
		assert(other.NotDeleted() && NotDeleted());
		if (type != other.type) {
			return false;
		}
		else {
			switch (type) {
			case ValueAny::Type::int32_t_: {
				return data.int32_t_ == other.data.int32_t_;
			} break;
			case ValueAny::Type::int64_t_: {
				return data.int64_t_ == other.data.int64_t_;
			} break;
			case ValueAny::Type::uint64_t_: {
				return data.uint64_t_ == other.data.uint64_t_;
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
			case ValueAny::Type::parent_: {
				// yeh, comparing a parent to a parent does not count the children
				return true;
			} break;
			case ValueAny::Type::dimension: {
				return *data.dimensionPtr == *other.data.dimensionPtr;
			} break;
			case ValueAny::Type::Vector3fList: {
				return *data.vector3fListPtr == *other.data.vector3fListPtr;
			} break;
			case ValueAny::Type::Vector2fList: {
				return *data.vector2fListPtr == *other.data.vector2fListPtr;
			} break;
			case ValueAny::Type::Vector3f: {
				return *data.vector3fPtr == *other.data.vector3fPtr;
			} break;
			case ValueAny::Type::Vector4f: {
				return *data.vector4fPtr == *other.data.vector4fPtr;
			} break;
			case ValueAny::Type::Int64List: {
				return *data.int64ListPtr == *other.data.int64ListPtr;
			} break;
			case ValueAny::Type::Vector2f: {
				return *data.vector2fPtr == *other.data.vector2fPtr;
			} break;
			case ValueAny::Type::Matrix4f: {
				return *data.matrix4fPtr == *other.data.matrix4fPtr;
			} break;
			case ValueAny::Type::ColorList: {
				return *data.colorListPtr == *other.data.colorListPtr;
			} break;
			case ValueAny::Type::void_: {
				check(0, "should not compare a void!");
				return false;
			} break;
			case ValueAny::Type::deleted: {
				check(0, "should not compare deleted!");
				return false;
			} break;
			case ValueAny::Type::pointer: {
				return data.ptr == other.data.ptr;
			} break;
			case ValueAny::Type::Exception: {
				check(0, "unimplemented");
				return false;
			} break;
			default: {
				assert(0);
			} break;
			}
		}
		return false;
	}

	std::string ValueAny::TypeAsString() const {
		assert(NotDeleted());
		assert(typeStringLookup.contains(type));
		return typeStringLookup.at(type);
	}
	/*static*/
	std::string ValueAny::TypeAsString(ValueAny::Type type) {
		assert(typeStringLookup.contains(type));
		return typeStringLookup.at(type);
	}
	std::string ValueAny::AsString() const {
		assert(NotDeleted());
		switch (type) {
		case ValueAny::Type::int32_t_: {
			return std::to_string(data.int32_t_);
		} break;
		case ValueAny::Type::int64_t_: {
			return std::to_string(data.int64_t_);
		} break;
		case ValueAny::Type::uint64_t_: {
			return std::to_string(data.uint64_t_);
		} break;
		case ValueAny::Type::float_: {
			return std::to_string(data.float_);
		} break;
		case ValueAny::Type::string: {
			return *data.stringPtr;
		} break;
		case ValueAny::Type::IDType: {
			return IDToString(data.idType);
		} break;
		case ValueAny::Type::Color: {
			return data.colorPtr->to_string();
		} break;
		case ValueAny::Type::dimension: {
			return data.dimensionPtr->ToString();
		} break;
		case ValueAny::Type::Vector3fList: {
			return data.vector3fListPtr->to_string();
		} break;
		case ValueAny::Type::Vector2fList: {
			return data.vector2fListPtr->to_string();
		} break;
		case ValueAny::Type::ColorList: {
			return data.colorListPtr->to_string();
		} break;
		case ValueAny::Type::parent_: {
			return "<parent>";
		} break;
		case ValueAny::Type::deleted: {
			return "<deleted>";
		} break;
		case ValueAny::Type::pointer: {
			return std::format("{:16x}", (size_t)data.ptr);
		} break;
		case ValueAny::Type::Exception: {
			return data.exceptionPtr->description;
		} break;
		case ValueAny::Type::Vector4f: {
			return data.vector4fPtr->to_string();
		} break;
		case ValueAny::Type::Int64List: {
			return data.int64ListPtr->to_string();
		} break;
		case ValueAny::Type::Vector3f: {
			return data.vector3fPtr->to_string();
		} break;
		case ValueAny::Type::Vector2f: {
			return data.vector2fPtr->to_string();
		} break;
		case ValueAny::Type::Matrix4f: {
			return data.matrix4fPtr->to_string();
		} break;
		case ValueAny::Type::void_: {
			return "<void>";
		} break;
		default: {
			assert(0);
		} break;
		}
		return "<undefined>";
	}
	const Color& ValueAny::AsColor() const {
		assert(NotDeleted());
		assert(type == Type::Color);
		return *data.colorPtr;
	}
	const Exception &ValueAny::AsException() const {
		assert(NotDeleted()); 
		assert(type == Type::Exception);
		return *data.exceptionPtr;
	}
	const Matrix4f& ValueAny::AsMatrix4f() const
	{
		assert(NotDeleted());
		assert(type == Type::Matrix4f);
		return *data.matrix4fPtr;
	}
	const Vector4f& ValueAny::AsVector4f() const
	{
		assert(NotDeleted());
		assert(type == Type::Vector4f);
		return *data.vector4fPtr;
	}
	IDType ValueAny::AsIDType() const {
		assert(NotDeleted()); 
		assert(type == Type::IDType);
		return data.idType;
	}
	int32_t ValueAny::AsInt32() const {
		assert(NotDeleted());
		assert(type == Type::int32_t_);
		return data.int32_t_;
	}
	int64_t ValueAny::AsInt64() const {
		assert(NotDeleted());
		assert(type == Type::int64_t_);
		return data.int64_t_;
	}
	uint64_t ValueAny::AsUint64() const {
		assert(NotDeleted());
		assert(type == Type::int64_t_);
		return data.int64_t_;
	}
	float ValueAny::AsFloat() const {
		assert(NotDeleted());
		assert(type == Type::float_);
		return data.float_;
	}
	void* ValueAny::AsPointer() const {
		assert(NotDeleted());
		assert(type == Type::pointer);
		return data.ptr;
	}
	nugget::ui::Dimension ValueAny::AsDimension() const
	{
		assert(NotDeleted());
		assert(type == Type::dimension);
		return *data.dimensionPtr;
	}
	
	const Vector3fList& ValueAny::AsVector3fList() const {
		assert(NotDeleted());
		assert(type == Type::Vector3fList);
		return *data.vector3fListPtr;
	}
	const Vector2fList& ValueAny::AsVector2fList() const {
		assert(NotDeleted());
		assert(type == Type::Vector2fList);
		return *data.vector2fListPtr;
	}
	const ColorList& ValueAny::AsColorList() const {
		assert(NotDeleted());
		assert(type == Type::ColorList);
		return *data.colorListPtr;
	}
	const Vector3f& ValueAny::AsVector3f() const {
		assert(NotDeleted());
		assert(type == Type::Vector3f);
		return *data.vector3fPtr;
	}
	const Vector2f& ValueAny::AsVector2f() const {
		assert(NotDeleted());
		assert(type == Type::Vector3f);
		return *data.vector2fPtr;
	}
	void ValueAny::Set(std::string val) {
		assert(type == Type::string);
		if (data.stringPtr != nullptr) {
			delete 	data.stringPtr;
		}
		data.stringPtr = new std::string(val);
	}
	void ValueAny::Set(int32_t val) {
		assert(type == Type::int32_t_);
		data.int32_t_ = val;
	}
	void ValueAny::Set(int64_t val) {
		if (type == Type::deleted) {
			type = Type::int64_t_;
		}
		assert(type == Type::int64_t_);
		data.int64_t_ = val;
	}
	void ValueAny::Set(uint64_t val) {
		if (type == Type::deleted) {
			type = Type::uint64_t_;
		}
		assert(type == Type::uint64_t_);
		data.uint64_t_ = val;
	}
	void ValueAny::Set(float val) {
		if (type == Type::deleted) {
			type = Type::float_;
		}
		assert(type == Type::float_);
		data.float_ = val;
	}
	void ValueAny::Set(const Color& colorIn) {
		if (type == Type::deleted) {
			type = Type::Color;
		}
		assert(type == Type::Color);
		if (data.colorPtr != nullptr) {
			delete 	data.colorPtr;
		}
		data.colorPtr = new Color(colorIn);
	}
	void ValueAny::Set(identifier::IDType id) {
		if (type == Type::deleted) {
			type = Type::IDType;
		}
		assert(type == Type::IDType);
		data.idType = id;
	}
	void ValueAny::Set(void* val) {
		if (type == Type::deleted) {
			type = Type::pointer;
		}
		assert(type == Type::pointer);
		data.ptr = val;
	}
	void ValueAny::Set(const nugget::ui::Dimension& dim)
	{
		if (type == Type::deleted) {
			type = Type::dimension;
		}
		assert(type == Type::dimension);
		if (data.dimensionPtr != nullptr) {
			delete 	data.dimensionPtr;
		}
		data.dimensionPtr = new nugget::ui::Dimension(dim);
	}
	void ValueAny::Set(const Vector3fList& verts) {
		if (type == Type::deleted) {
			type = Type::Vector3fList;
		}
		assert(type == Type::Vector3fList);
		if (data.vector3fListPtr != nullptr) {
			delete 	data.vector3fListPtr;
		}
		data.vector3fListPtr = new Vector3fList(verts);
	}
	void ValueAny::Set(const Vector3f& v) {
		if (type == Type::deleted) {
			type = Type::Vector3f;
		}
		assert(type == Type::Vector3f);
		if (data.vector3fListPtr != nullptr) {
			delete 	data.vector3fPtr;
		}
		data.vector3fPtr = new Vector3f(v);
	}
	void ValueAny::Set(const Vector2fList& verts) {
		if (type == Type::deleted) {
			type = Type::Vector2fList;
		}
		assert(type == Type::Vector2fList);
		if (data.vector2fListPtr != nullptr) {
			delete 	data.vector2fListPtr;
		}
		data.vector2fListPtr = new Vector2fList(verts);
	}
	void ValueAny::Set(const ColorList& verts) {
		if (type == Type::deleted) {
			type = Type::ColorList;
		}
		assert(type == Type::ColorList);
		if (data.colorListPtr != nullptr) {
			delete 	data.colorListPtr;
		}
		data.colorListPtr = new ColorList(verts);
	}
	void ValueAny::Set(const Vector2f& v) {
		if (type == Type::deleted) {
			type = Type::Vector2f;
		}
		assert(type == Type::Vector2f);
		if (data.vector2fListPtr != nullptr) {
			delete 	data.vector2fPtr;
		}
		data.vector2fPtr = new Vector2f(v);
	}
	void ValueAny::Set(const Matrix4f& v) {
		if (type == Type::deleted) {
			type = Type::Matrix4f;
		}
		assert(type == Type::Matrix4f);
		if (data.matrix4fPtr != nullptr) {
			delete 	data.matrix4fPtr;
		}
		data.matrix4fPtr = new Matrix4f(v);
	}
	void ValueAny::Set(const Vector4f& v) {
		if (type == Type::deleted) {
			type = Type::Vector4f;
		}
		assert(type == Type::Vector4f);
		if (data.matrix4fPtr != nullptr) {
			delete 	data.vector4fPtr;
		}
		data.vector4fPtr = new Vector4f(v);
	}
	void ValueAny::Set(const Int64List& v) {
		if (type == Type::deleted) {
			type = Type::Int64List;
		}
		assert(type == Type::Int64List);
		if (data.int64ListPtr != nullptr) {
			delete 	data.int64ListPtr;
		}
		data.int64ListPtr = new Int64List(v);
	}
	void ValueAny::SetAsParent() {
		type = Type::parent_;
	}
	void ValueAny::Set(const ValueAny& val) {
		CopyFrom(val);
	}

	bool ValueAny::IsVoid() const {
		assert(NotDeleted());
		return type == Type::parent_;
	}

	bool ValueAny::IsException() const {
		assert(NotDeleted());
		return type == Type::Exception;
	}

	bool ValueAny::NotDeleted() const
	{
		return type != Type::deleted;;
	}

	void ValueAny::MarkDeleted() {
		FreeCurrentValue();
		type = Type::deleted;
	}

	ValueAny::~ValueAny() {
		FreeCurrentValue();
	}

	ValueAny::Type ValueAny::GetType() const {
		return type;
	}

	void ValueAny::FreeCurrentValue() {
		switch (type) {
			case Type::Color: {
				if (data.colorPtr) {
					delete data.colorPtr;
					data.colorPtr = nullptr;
				}
			} break;
			case Type::string: {
				if (data.stringPtr) {
					delete data.stringPtr;
					data.stringPtr = nullptr;
				}
			} break;
			case Type::dimension: {
				if (data.dimensionPtr) {
					delete data.dimensionPtr;
					data.dimensionPtr = nullptr;
				}
			} break;
			case Type::Vector3fList: {
				if (data.vector3fListPtr) {
					delete data.vector3fListPtr;
					data.vector3fListPtr = nullptr;
				}
			} break;
			case Type::Vector3f: {
				if (data.vector3fPtr) {
					delete data.vector3fPtr;
					data.vector3fPtr = nullptr;
				}
			} break;
			case Type::Vector2f: {
				if (data.vector2fPtr) {
					delete data.vector2fPtr;
					data.vector2fPtr = nullptr;
				}
			} break;
			case Type::ColorList: {
				if (data.colorListPtr) {
					delete data.colorListPtr;
					data.colorListPtr = nullptr;
				}
			} break;
			case Type::Vector2fList: {
				if (data.vector2fListPtr) {
					delete data.vector2fListPtr;
					data.vector2fListPtr = nullptr;
				}
			} break;
			case Type::Exception: {
				if (data.exceptionPtr) {
					delete data.exceptionPtr;
					data.exceptionPtr = nullptr;
				}
			} break;
			case Type::Matrix4f: {
				if (data.matrix4fPtr) {
					delete data.matrix4fPtr;
					data.matrix4fPtr = nullptr;
				}
			} break;
			case Type::Vector4f: {
				if (data.vector4fPtr) {
					delete data.vector4fPtr;
					data.vector4fPtr = nullptr;
				}
			} break;
			case Type::void_:
			case Type::deleted:
			case Type::parent_:
			case Type::int64_t_:
			case Type::uint64_t_:
			case Type::int32_t_:
			case Type::IDType:
			case Type::pointer:
			case Type::float_: {
				// no action needed
			} break;
			default: {
				assert(0);
			}
		}
	}

	void ValueAny::CopyFrom(const ValueAny& other) {
		assert(other.NotDeleted());
		
		FreeCurrentValue();

		type = other.type;
		switch (type) {
			case Type::int64_t_: {
				data.int64_t_ = other.data.int64_t_;
			} break;
			case Type::uint64_t_: {
				data.uint64_t_ = other.data.uint64_t_;
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
				data.colorPtr = new Color(*other.data.colorPtr);
			} break;
			case Type::Exception: {
				data.exceptionPtr = new Exception(*other.data.exceptionPtr);
			} break;
			case Type::string: {
				data.stringPtr = new std::string(*other.data.stringPtr);
			} break;
			case Type::dimension: {
				data.dimensionPtr = new nugget::ui::Dimension(*other.data.dimensionPtr);
			} break;
			case Type::Vector3fList: {
				data.vector3fListPtr = new Vector3fList(*other.data.vector3fListPtr);
			} break;
			case Type::Vector3f: {
				data.vector3fPtr = new Vector3f(*other.data.vector3fPtr);
			} break;
			case Type::Vector2fList: {
				data.vector2fListPtr = new Vector2fList(*other.data.vector2fListPtr);
			} break;
			case Type::Vector2f: {
				data.vector2fPtr = new Vector2f(*other.data.vector2fPtr);
			} break;
			case Type::ColorList: {
				data.colorListPtr = new ColorList(*other.data.colorListPtr);
			} break;
			case Type::Matrix4f: {
				data.matrix4fPtr = new Matrix4f(*other.data.matrix4fPtr);
			} break;
			case Type::Vector4f: {
				data.vector4fPtr = new Vector4f(*other.data.vector4fPtr);
			} break;
			case Type::Int64List: {
				data.int64ListPtr = new Int64List(*other.data.int64ListPtr);
			} break;
			case Type::pointer: {
				data.ptr = other.data.ptr;
			} break;
			case Type::parent_: {
				/* do nothing */
			} break;
			case Type::void_: {
				check(0, "We really should not be trying to copy voids...");
			} break;
			case Type::deleted: {
				check(0, "We really should not be trying to copy deleted...");
			} break;
		}
	}
	/*static*/
	const std::unordered_map<ValueAny::Type, std::string> ValueAny::typeStringLookup = {
		{ValueAny::Type::void_,"void_"},
		{ValueAny::Type::parent_,"parent_"},
		{ValueAny::Type::int32_t_,"int32_t_"},
		{ValueAny::Type::int64_t_,"int64_t_"},
		{ValueAny::Type::uint64_t_,"uint64_t_"},
		{ValueAny::Type::Color,"Color"},
		{ValueAny::Type::dimension,"Dimension"},
		{ValueAny::Type::float_,"float"},
		{ValueAny::Type::string,"string"},
		{ValueAny::Type::IDType,"IDType"},
		{ValueAny::Type::Vector3fList,"Vector3fList"},
		{ValueAny::Type::Vector3f,"Vector3f"},
		{ValueAny::Type::Vector2fList,"Vector2fList"},
		{ValueAny::Type::ColorList,"ColorList"},
		{ValueAny::Type::Vector2f,"Vector2f"},
		{ValueAny::Type::Exception,"Exception"},
		{ValueAny::Type::Matrix4f,"Matrix4f"},
		{ValueAny::Type::Matrix4f,"Vector4f"},
	};


}
