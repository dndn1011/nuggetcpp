#pragma once
#include <functional>

namespace nugget::system {
	using RegisterFunction = std::function<void()>;
	size_t RegisterModule(RegisterFunction func,int64_t order);
	void Init();
}
