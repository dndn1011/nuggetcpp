
#pragma once
#include <functional>
#include "identifier.h"

namespace nugget::system {
	using namespace identifier;

	using RegisterFunction = std::function<void()>;
	
	using FunctionByIDLambda = std::function<void(IDType id,IDType parent)>;

	size_t RegisterModule(RegisterFunction func,int64_t order,identifier::IDType phase = identifier::ID("init"),const std::string &label = "<no label>");
	void Init();
	bool Update();

	void RegisterFunctionByID(IDType node, FunctionByIDLambda func);
	void CallFunctionByID(IDType node,IDType parent);

	void Shutdown();

}
