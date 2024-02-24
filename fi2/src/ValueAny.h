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
        explicit ValueAny(const Matrix4f& v);
        explicit ValueAny(const Matrix3f& v);
        explicit ValueAny(const Vector4f& v);
        explicit ValueAny(const Int64List& v);
        explicit ValueAny(const IdentifierList& v);

        operator int64_t() const { return AsInt64(); }
        operator int32_t() const { return AsInt32(); }
        operator uint64_t() const { return AsUint64(); }
        operator float() const { return AsFloat(); }
        operator Color() const { return AsColor(); }
        operator std::string() const { return AsString(); }
        operator identifier::IDType() const { return AsIDType(); }
        operator nugget::ui::Dimension() const { return AsDimension(); }
        operator Exception() const { return AsException(); }
        operator Vector3fList() const { return AsVector3fList(); }
        operator Vector2fList() const { return AsVector2fList(); }
        operator ColorList() const { return AsColorList(); }
        operator Vector3f() const { return AsVector3f(); }
        operator Vector2f() const { return AsVector2f(); }
        operator Matrix4f() const { return AsMatrix4f(); }
        operator Matrix3f() const { return AsMatrix3f(); }
        operator Vector4f() const { return AsVector4f(); }
        operator Int64List() const { return AsInt64List(); }
        operator IdentifierList() const { return AsIdentifierList(); }

        ValueAny(const ValueAny& other);
        ValueAny& operator=(const ValueAny& other);

        ValueAny(ValueAny&& other) noexcept;

        bool operator==(const ValueAny& other) const;

        ~ValueAny();

        enum class Type {
            void_,
            parent_,
            int32_t_,
            int64_t_,
            uint64_t_,
            float_,
            Color,
            string,
            IDType,
            pointer,
            dimension,
            Exception,
            Vector3fList,
            Vector2fList,
            ColorList,
            Vector3f,
            Vector2f,
            Matrix4f,
            Matrix3f,
            Vector4f,
            Int64List,
            IdentifierList,

            deleted
        };

        static std::string TypeAsString(ValueAny::Type type);
        std::string TypeAsString() const;
        std::string AsString() const;
        const Color& AsColor() const;
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
        const Matrix4f& AsMatrix4f() const;
        const Matrix3f& AsMatrix3f() const;
        const Vector4f& AsVector4f() const;
        const Int64List& AsInt64List() const;
        const IdentifierList& AsIdentifierList() const;
        const std::string& AsStringRef() const;

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
        void Set(const Vector4f& vect);
        void Set(const Vector2f& vect);
        void Set(const Exception& exception);
        void Set(const Matrix4f& mat);
        void Set(const Matrix3f& mat);
        void Set(const Int64List& ints);
        void Set(const IdentifierList& ints);

        void SetAsParent();
        bool IsVoid() const;
        bool IsException() const;
        bool NotDeleted() const;
        void MarkDeleted();
        Type GetType() const;

    private:
        union Data {
            int32_t int32_t_;
            int64_t int64_t_;
            uint64_t uint64_t_;
            float float_;
            Color* colorPtr;
            std::string* stringPtr;
            identifier::IDType idType;
            void* ptr;
            nugget::ui::Dimension* dimensionPtr;
            Exception* exceptionPtr;
            Vector3fList* vector3fListPtr;
            Vector2fList* vector2fListPtr;
            ColorList* colorListPtr;
            Vector4f* vector4fPtr;
            Vector3f* vector3fPtr;
            Vector2f* vector2fPtr;
            Matrix4f* matrix4fPtr;
            Matrix3f* matrix3fPtr;
            Int64List* int64ListPtr;
            IdentifierList* identifierListPtr;
        } data = {};

        Type type = Type::void_;

        static const std::unordered_map<Type, std::string> typeStringLookup;

        void CopyFrom(const ValueAny& other);
        void FreeCurrentValue();
    };
}
