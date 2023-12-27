#include "UIEntities.h"
#include <unordered_map>
#include <functional>
#include <assert.h>
#include "notice.h"
#include "debug.h"
#include "utils/utils.h"

namespace nugget::ui::entity {
	using namespace identifier;
	using namespace Notice;

	struct EntityInfo {
		IDType node = {};
		CreateLambda createFunc = {};
		std::function<void()> selfGeomFunc = {};
		std::function<void(IDType)> manageGeomFunc = {};
		SimpleLambda drawFunc = {};
	};
	static inline std::unordered_map<IDType,EntityInfo> registeredEntities;

	void CreateEntityRecursive(IDType id) {
		auto classNode = IDRCheck(id, "class");
		if (classNode!=IDType::null) {
			assert(Notice::KeyExists(classNode));
			auto classId = Notice::GetID(classNode);
			check(registeredEntities.contains(classId), "the entity constructor is not defined: {}\n",
				IDToString(classId));

			registeredEntities.at(classId).createFunc(id);

			std::vector<IDType> children;
			if (auto r = Notice::GetChildren(IDR(id, "sub"),children)) {
				for (auto& x : children) {
					IDType classNodeHash = IDRCheck(x, "class");
					if (classNodeHash!=IDType::null && Notice::KeyExists(classNodeHash)) {
						CreateEntityRecursive(x);
					}
				}
			}
		}
		//else {
		//	assert(("Could not find entity node", 0));
		//}
	}

	IDType HashRemap(IDType path, IDType newRoot) {
		std::string strPath = IDToString(path);
		auto lastDotPosition = strPath.find_first_of('.');
		// Check if a '.' character was found
		if (lastDotPosition != std::string::npos) {
			IDType newPath = IDR(newRoot, IDR(strPath.substr(lastDotPosition + 1)));
			return newPath;
		} else {
			assert(0);
		}
		return IDType::null;
	}

	// compare the state of the property tree with the current instances of entities
	// and sync
	void UpdateEntityRecursive(IDType to, IDType from) {
		//output("checking: %s <- %s : %zu %zu\n", IDToString(to).c_str(), IDToString(from).c_str(),to,from);
		if (Notice::KeyExists(from)) {
			// both exist
			ValueAny valueTo = Notice::GetValueAny(to);
			ValueAny valueFrom = Notice::GetValueAny(from);
			if (valueTo == valueFrom) {
			//	output("SAME!\n");
			} else {
				//output("updating: %s <- %s\n", IDToString(to).c_str(), IDToString(from).c_str());
				//output("      values: %s <- %s\n", valueTo.GetValueAsString().c_str(), valueFrom.GetValueAsString().c_str());
				Notice::Set(to, valueFrom);
			}
		} else {
//			output("Removed {}\n", IDToString(to).c_str());
			Notice::Remove(to);
		}
		std::vector<IDType> children;
		if (auto r = Notice::GetChildren(to,children /*fill*/)) {
			for (auto& x : children) {
				IDType newFrom = IDR(from, GetLeaf(x));
				UpdateEntityRecursive(x,newFrom);
			}
		}
	}

	void UpdateEntityRecursive2(IDType to, IDType from) {
//		output("checking: %s <- %s : %zu %zu\n", IDToString(to).c_str(), IDToString(from).c_str(),to,from);
		if (Notice::KeyExists(to)) {
			// both exist
		} else {
//			output("MISSING!!!! {}\n", IDToString(from).c_str());
			ValueAny valueTo = Notice::GetValueAny(from);
			Set(to, valueTo);
		}
		std::vector<IDType> children;
		if (auto r = Notice::GetChildren(from, children /*fill*/)) {
			for (auto& x : children) {
				IDType newTo = IDR(to, GetLeaf(x));
				UpdateEntityRecursive2(newTo, x /*fill*/);
			}
		}
	}

	void CreateEntity(IDType id) {
		CreateEntityRecursive(id);
	}

	void UpdateEntity(IDType to, IDType from) {
		UpdateEntityRecursive(to,from);
		UpdateEntityRecursive2(to, from);
	}

	struct RegisterSingleton;
	extern RegisterSingleton* staticEntityRegistrations;
	struct RegisterSingleton {
		std::vector<std::function<void()>> initList;
		static RegisterSingleton* GetInstance() {
			if (staticEntityRegistrations) {
				return staticEntityRegistrations;
			} else {
				return staticEntityRegistrations = new RegisterSingleton();  // never deleted
			}
		}
	};
	RegisterSingleton* staticEntityRegistrations = RegisterSingleton::GetInstance();


	void ManageGeometry(IDType nodeId) {
		for (auto &&x : registeredEntities) {
			check(x.second.selfGeomFunc, ("The function pointer is null\n"));
			x.second.selfGeomFunc();
		}


		for (auto&& x : registeredEntities) {
			if (x.second.manageGeomFunc) {
				x.second.manageGeomFunc(nodeId);
			}
		}
	}
	 
#if 0
	void CallFunction(IDType functionId,IDType entityId) {
		check(registeredEntities.contains(functionId),"no such function: {}",IDToString(functionId));
		registeredEntities.at(functionId).(entityId);
	}
#endif

	void Init() {
		for (auto&& x : staticEntityRegistrations->initList) {
			x();
		}
	}

	size_t RegisterEntityInit(std::function<void()> func) {
		auto ptr = RegisterSingleton::GetInstance();
		ptr->initList.push_back(func);
		return ptr->initList.size();
	}

	void RegisterEntityCreator(IDType typeID, CreateLambda func) {
		if (registeredEntities.contains(typeID)) {
			auto& x = registeredEntities.at(typeID);
			x.createFunc = func;
		} else {
			registeredEntities[typeID] = EntityInfo{ .createFunc = func };
		}
	}

	void RegisterEntityDraw(IDType typeID, SimpleLambda func) {
		if (registeredEntities.contains(typeID)) {
			auto& x = registeredEntities.at(typeID);
			x.drawFunc = func;
		} else {
			registeredEntities[typeID] = EntityInfo{ .drawFunc = func };
		}
	}

	void RegisterSelfGeomFunction(IDType typeID, std::function<void()> func) {
		if (registeredEntities.contains(typeID)) {
			auto& x = registeredEntities.at(typeID);
			x.selfGeomFunc = func;
		} else {
			registeredEntities[typeID] = EntityInfo{ .selfGeomFunc = func };
		}
	}

	void RegisterPostSelfGeomFunction(IDType node, std::function<void(IDType)> func) {
		if (registeredEntities.contains(node)) {
			auto& x = registeredEntities.at(node);
			x.manageGeomFunc = func;
		} else {
			registeredEntities[node] = EntityInfo{ .manageGeomFunc = func };
		}
	}

	void DrawAll() {
		// one call for each class
		for (auto&& x : registeredEntities) {
			if (x.second.drawFunc) {
				x.second.drawFunc();
			}
		}
	}

	
}
