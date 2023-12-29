
ValueAny EXPR_NAME(const std::span<ValueAny> values) {
    ValueAny v = values[0];
    ValueAny::Type vt = v.GetType();
    for (size_t i = 1; i < values.size(); i++) {
        ValueAny w = values[i];
        auto wt = w.GetType();
        if (wt != vt) {
            auto key = ConversionPair(wt, vt);
            if (converters.contains(key)) {
                w = converters.at(key)(w);
            } else {
                ValueAny::Type promote = GetPromotedType(vt, wt);
                if (vt == promote) {
                    w = ConvertType(w, vt);
                    if (w.IsException()) {
                        return ValueAny(Exception{ .description = std::format("Could not convert '{}' to '{}'\n",ValueAny::TypeAsString(wt),ValueAny::TypeAsString(vt)) });
                    }
                } else {
                    v = ConvertType(v, promote);
                    if (v.IsException()) {
                        assert(0);
                    }
                }
                //                check(0, "mismatch of types: need to convert {} and {} promotion should be to {}\n",w.GetTypeAsString(),v.GetTypeAsString(),ValueAny::GetTypeAsString(promote));
            }
        }
        switch (v.GetType()) {
#if EXPR_OP == EXPR_DOT || EXPR_OP == EXPR_DOUBLE_COLON
            case ValueAny::Type::IDType: {
                return ValueAny(IDR(v.AsIDType(), w.AsIDType()));
            }
#else
            case ValueAny::Type::int64_t_: {
                return ValueAny(v.AsInt64() EXPR_OPERATOR w.AsInt64());
            }
            case ValueAny::Type::Vector3f: {
                return ValueAny(v.AsVector3f() EXPR_OPERATOR w.AsVector3f());
            }
#if EXPR_OP == EXPR_PLUS
            case ValueAny::Type::string: {
                return ValueAny(v.AsString() EXPR_OPERATOR w.AsString());
            }
#endif
            case ValueAny::Type::float_: {
                return ValueAny(v.AsFloat() EXPR_OPERATOR w.AsFloat());
            }
#endif
#if EXPR_OP == EXPR_MULTIPLY
            case ValueAny::Type::Color: {
                return ValueAny(v.AsColor() EXPR_OPERATOR w.AsColor());
            }
#endif
            default: {
                check(0, "not implemented for types: EXPR_OP={}, type: {}\n", EXPR_OP, v.TypeAsString());
            }
        }
    }
    return {};
}

#undef EXPR_OPERATOR
#undef EXPR_NAME
#undef EXPR_OP
