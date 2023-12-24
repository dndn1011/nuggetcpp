
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
                        return ValueAny(Exception{ .description = std::format("Could not convert '{}' to '{}'\n",ValueAny::GetTypeAsString(wt),ValueAny::GetTypeAsString(vt)) });
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
                return ValueAny(IDR(v.GetValueAsIDType(), w.GetValueAsIDType()));
            }
#else
            case ValueAny::Type::int64_t_: {
                return ValueAny(v.GetValueAsInt64() EXPR_OPERATOR w.GetValueAsInt64());
            }
            case ValueAny::Type::Vector3f: {
                return ValueAny(v.GetValueAsVector3f() EXPR_OPERATOR w.GetValueAsVector3f());
            }
#if EXPR_OP == EXPR_PLUS
            case ValueAny::Type::string: {
                return ValueAny(v.GetValueAsString() EXPR_OPERATOR w.GetValueAsString());
            }
#endif
            case ValueAny::Type::float_: {
                return ValueAny(v.GetValueAsFloat() EXPR_OPERATOR w.GetValueAsFloat());
            }
#endif
#if EXPR_OP == EXPR_MULTIPLY
            case ValueAny::Type::Color: {
                return ValueAny(v.GetValueAsColor() EXPR_OPERATOR w.GetValueAsColor());
            }
#endif
            default: {
                check(0, "not implemented for types: EXPR_OP={}, type: {}\n", EXPR_OP, v.GetTypeAsString());
            }
        }
    }
    return {};
}

#undef EXPR_OPERATOR
#undef EXPR_NAME
#undef EXPR_OP
