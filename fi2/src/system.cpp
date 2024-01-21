#include "system.h"
#include <vector>
#include <algorithm>
#include "identifier.h"
namespace nugget::system {
	using namespace identifier;
	struct ModuleReg {
		RegisterFunction func;
		int64_t order;
	};
	struct RegisterSingleton;
	extern RegisterSingleton* staticEntityRegistrations;
	struct RegisterSingleton {
		std::unordered_map<IDType,std::vector<ModuleReg>> moduleList;
		static RegisterSingleton* GetInstance() {
			if (staticEntityRegistrations) {
				return staticEntityRegistrations;
			} else {
				return staticEntityRegistrations = new RegisterSingleton();  // never deleted
			}
		}
	};
	RegisterSingleton* staticEntityRegistrations = RegisterSingleton::GetInstance();

	size_t RegisterModule(RegisterFunction func, int64_t order,IDType phase) {
		RegisterSingleton::GetInstance()->moduleList[phase].push_back(ModuleReg{ func, order });
		return size_t();
	}

	void Init() {
		auto& list = RegisterSingleton::GetInstance()->moduleList[ID("init")];
		std::ranges::sort(list, [](const ModuleReg& a, const ModuleReg& b) {
			return a.order < b.order;
			});
		for (auto&& x : list) {
			x.func();
		}
	}

	void Update() {
		auto& list = RegisterSingleton::GetInstance()->moduleList[ID("update")];
		std::ranges::sort(list, [](const ModuleReg& a, const ModuleReg& b) {
			return a.order < b.order;
			});
		for (auto&& x : list) {
			x.func();
		}
	}
}
