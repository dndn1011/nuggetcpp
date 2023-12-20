
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
                } else {
                    v = ConvertType(v, promote);
                }
                //                check(0, "mismatch of types: need to convert {} and {} promotion should be to {}\n",w.GetTypeAsString(),v.GetTypeAsString(),ValueAny::GetTypeAsString(promote));
            }
        }
        switch (v.GetType()) {
            case ValueAny::Type::int64_t_: {
                return ValueAny(v.GetValueAsInt64() EXPR_OPERATOR w.GetValueAsInt64());
            }
#if EXPR_OP != EXPR_MULTIPLY && EXPR_OP != EXPR_DIVIDE
            case ValueAny::Type::string: {
                return ValueAny(v.GetValueAsString() EXPR_OPERATOR w.GetValueAsString());
            }
#endif
            case ValueAny::Type::float_: {
                return ValueAny(v.GetValueAsFloat() EXPR_OPERATOR w.GetValueAsFloat());
            }
            default: {
                check(0, "Plus not implemented for types");
            }
        }
    }
    return {};
}

#undef EXPR_OPERATOR
#undef EXPR_NAME
#undef EXPR_OP
