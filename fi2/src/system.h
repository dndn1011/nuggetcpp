
#pragma once
#include <functional>
#include "identifier.h"

namespace nugget::system {
	using RegisterFunction = std::function<void()>;
	size_t RegisterModule(RegisterFunction func,int64_t order,identifier::IDType phase = identifier::ID("init"));
	void Init();
	bool Update();
}
