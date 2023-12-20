#pragma once
#include "propertytreetokeniser.h"
#include "ValueAny.h"

namespace nugget::expressions {
	using namespace properties;
	struct Expression_imp;
	class Expression {
	public:
		Expression_imp& imp;
		Expression();
		void AddToken(const Token& token);
		ValueAny Evaluate();
		static ValueAny ConvertType(nugget::ValueAny a, nugget::ValueAny::Type type);

	};
}
