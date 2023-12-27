#include "system.h"
#include <vector>
#include <algorithm>

namespace nugget::system {
	struct ModuleReg {
		RegisterFunction func;
		int64_t order;
	};
	struct RegisterSingleton;
	extern RegisterSingleton* staticEntityRegistrations;
	struct RegisterSingleton {
		std::vector<ModuleReg> moduleList;
		static RegisterSingleton* GetInstance() {
			if (staticEntityRegistrations) {
				return staticEntityRegistrations;
			} else {
				return staticEntityRegistrations = new RegisterSingleton();  // never deleted
			}
		}
	};
	RegisterSingleton* staticEntityRegistrations = RegisterSingleton::GetInstance();

	size_t RegisterModule(RegisterFunction func, int64_t order) {
		RegisterSingleton::GetInstance()->moduleList.push_back(ModuleReg{ func, order });
		return size_t();
	}

	void Init() {
		std::ranges::sort(RegisterSingleton::GetInstance()->moduleList, [](const ModuleReg& a, const ModuleReg& b) {
			return a.order < b.order;
			});
		for (auto&& x : RegisterSingleton::GetInstance()->moduleList) {
			x.func();
		}
	}
}
