#include "system.h"
#include <vector>
#include <algorithm>
#include "identifier.h"
#include "debug.h"

namespace nugget::system {
	namespace {
		using namespace identifier;
		struct ModuleReg {
			RegisterFunction func;
			int64_t order;
			std::string label;
		};
		struct RegisterSingleton;
		extern RegisterSingleton* staticEntityRegistrations;
		struct RegisterSingleton {
			std::unordered_map<IDType, std::vector<ModuleReg>> moduleList;
			static RegisterSingleton* GetInstance() {
				if (staticEntityRegistrations) {
					return staticEntityRegistrations;
				} else {
					return staticEntityRegistrations = new RegisterSingleton();  // never deleted
				}
			}
		};
		RegisterSingleton* staticEntityRegistrations = RegisterSingleton::GetInstance();

		size_t RegisterModule(RegisterFunction func, int64_t order, IDType phase) {
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

		bool Update() {
			auto& list = RegisterSingleton::GetInstance()->moduleList[ID("update")];
			if (list.size() == 0) {
				return false;
			}
			std::ranges::sort(list, [](const ModuleReg& a, const ModuleReg& b) {
				return a.order < b.order;
				});
			for (auto&& x : list) {
				x.func();
			}
			return true;
		}

		std::unordered_map<IDType, FunctionByIDLambda> registeredFunctions;

	}
	size_t RegisterModule(RegisterFunction func, int64_t order, IDType phase,const std::string &label) {
		RegisterSingleton::GetInstance()->moduleList[phase].push_back(ModuleReg{ func, order, label });
		return size_t();
	}

	void Init() {
		auto& list = RegisterSingleton::GetInstance()->moduleList[ID("init")];

		std::ranges::sort(list, [](const ModuleReg& a, const ModuleReg& b) {
			return a.order < b.order;
			});
		for (auto&& x : list) {
			output("{}\n", x.label);
			x.func();
		}
	}

	bool Update() {
		auto& list = RegisterSingleton::GetInstance()->moduleList[ID("update")];
		if (list.size() == 0) {
			return false;
		}
		std::ranges::sort(list, [](const ModuleReg& a, const ModuleReg& b) {
			return a.order < b.order;
			});
		for (auto&& x : list) {
			x.func();
		}
		return true;
	}

	void Shutdown() {
		auto& list = RegisterSingleton::GetInstance()->moduleList[ID("shutdown")];
		if (list.size() != 0) {
			std::ranges::sort(list, [](const ModuleReg& a, const ModuleReg& b) {
				return a.order < b.order;
				});
			for (auto&& x : list) {
				x.func();
			}
		}
	}

	void RegisterFunctionByID(IDType node, FunctionByIDLambda func) {
		registeredFunctions[node] = func;
	}
	void CallFunctionByID(IDType node,IDType parent) {
		check(registeredFunctions.contains(node),"Node {} does not exist",IDToString(node));
		registeredFunctions.at(node)(node,parent);
	}

}