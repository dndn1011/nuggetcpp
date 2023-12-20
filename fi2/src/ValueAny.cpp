#include "types.h"
#include "identifier.h"
#include "ValueAny.h"
#include "dimensions.h"
namespace nugget {
	using namespace identifier;

	ValueAny::ValueAny() {}

	ValueAny::ValueAny(int32_t v) : type(Type::int32_t_), data{ .int32_t_ = v } {}
	ValueAny::ValueAny(int64_t v) : type(Type::int64_t_), data{ .int64_t_ = v } {}
	ValueAny::ValueAny(uint64_t v) : type(Type::uint64_t_), data{ .uint64_t_ = v } {}
	ValueAny::ValueAny(float v) : type(Type::float_), data{ .float_ = v } {}
	ValueAny::ValueAny(const Color& v) : type(Type::Color), data{ .colorPtr = new Color(v) } {}
	ValueAny::ValueAny(const std::string& v) : type(Type::string), data{ .stringPtr = new std::string(v) } {}
	ValueAny::ValueAny(IDType v) : type(Type::IDType), data{ .idType = v } {}
	ValueAny::ValueAny(void* ptr) : type(Type::pointer), data{ .ptr = ptr } {}
	ValueAny::ValueAny(const nugget::ui::Dimension& v) : type(Type::dimension), data{ .dimensionPtr = new nugget::ui::Dimension(v) } {}
	ValueAny::ValueAny(const Vector3fList& v) : type(Type::Vector3fList), data{ .vector3fPtr = new Vector3fList(v) } {}

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
			case ValueAny::Type::void_: {
				return true;
			} break;
			case ValueAny::Type::dimension: {
				return *data.dimensionPtr == *other.data.dimensionPtr;
			} break;
			case ValueAny::Type::Vector3fList: {
				return *data.vector3fPtr == *other.data.vector3fPtr;
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
	/*static*/
	std::string ValueAny::GetTypeAsString(ValueAny::Type type) {
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
			return std::to_string(+data.idType);
		} break;
		case ValueAny::Type::Color: {
			return data.colorPtr->to_string();
		} break;
		case ValueAny::Type::dimension: {
			return data.dimensionPtr->ToString();
		} break;
		case ValueAny::Type::Vector3fList: {
			return "<unsupported>";
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
	uint64_t ValueAny::GetValueAsUint64() const {
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
	nugget::ui::Dimension ValueAny::GetValueAsDimension() const
	{
		assert(type == Type::dimension);
		return *data.dimensionPtr;
	}
	const Vector3fList& ValueAny::GetValueAsVector3fList() const
	{
		assert(type == Type::Vector3fList);
		return *data.vector3fPtr;
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
	void ValueAny::SetValue(uint64_t val) {
		assert(type == Type::uint64_t_);
		data.uint64_t_ = val;
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
	void ValueAny::SetValue(const nugget::ui::Dimension& dim)
	{
		assert(type == Type::dimension);
		if (data.dimensionPtr != nullptr) {
			delete 	data.dimensionPtr;
		}
		data.dimensionPtr = new nugget::ui::Dimension(dim);
	}
	void ValueAny::SetValue(const Vector3fList& verts)
	{
		assert(type == Type::Vector3fList);
		if (data.vector3fPtr != nullptr) {
			delete 	data.vector3fPtr;
		}
		data.vector3fPtr = new Vector3fList(verts);
	}
	void ValueAny::SetValueVoid() {
		type = Type::void_;
	}
	void ValueAny::SetValue(const ValueAny& val) {
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
		switch (type) {
			case Type::Color: {
				if (data.colorPtr) {
					delete data.colorPtr;
				}
			} break;
			case Type::string: {
				if (data.stringPtr) {
					delete data.stringPtr;
				}
			} break;
			case Type::dimension: {
				if (data.dimensionPtr) {
					delete data.dimensionPtr;
				}
			} break;
			case Type::Vector3fList: {
				if (data.vector3fPtr) {
					delete data.vector3fPtr;
				}
			} break;
		}

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
			case Type::string: {
				data.stringPtr = new std::string(*other.data.stringPtr);
			} break;
			case Type::dimension: {
				data.dimensionPtr = new nugget::ui::Dimension(*other.data.dimensionPtr);
			} break;
			case Type::Vector3fList: {
				data.vector3fPtr = new Vector3fList(*other.data.vector3fPtr);
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
	const std::unordered_map<ValueAny::Type, std::string> ValueAny::typeStringLookup = {
		{ValueAny::Type::void_,"void"},
		{ValueAny::Type::int32_t_,"int32_t_"},
		{ValueAny::Type::int64_t_,"int64_t_"},
		{ValueAny::Type::uint64_t_,"uint64_t_"},
		{ValueAny::Type::Color,"Color"},
		{ValueAny::Type::Vector3fList,"Vertices"},
		{ValueAny::Type::dimension,"Dimension"},
		{ValueAny::Type::float_,"float"},
		{ValueAny::Type::string,"string"},
	};
}
