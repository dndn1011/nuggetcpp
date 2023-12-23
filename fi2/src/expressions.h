#pragma once
#include "propertytreetokeniser.h"
#include "ValueAny.h"

namespace nugget::expressions {
	using namespace properties;
	using namespace identifier;
	struct Expression_imp;
	class Expression {
	public:
		Expression_imp& imp;
		Expression();
		void AddToken(const Token& token);
		ValueAny Evaluate(std::function<nugget::ValueAny(nugget::identifier::IDType)> expandParseVars);
		static ValueAny ConvertType(nugget::ValueAny a, nugget::ValueAny::Type type);
	};
}
