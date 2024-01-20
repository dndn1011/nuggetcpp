#include "UIEntities.h"
#include <unordered_map>
#include <functional>
#include <assert.h>
#include "notice.h"
#include "debug.h"
#include "utils/utils.h"
#include "propertytree.h"

namespace nugget::ui::entity {
	using namespace identifier;
	using namespace Notice;
	using namespace nugget::properties;
	struct EntityInfo {
		IDType node = {};
		CreateLambda createFunc = {};
		std::function<void()> selfGeomFunc = {};
		std::function<void(IDType)> manageGeomFunc = {};
		SimpleLambda drawFunc = {};
	};
	static inline std::unordered_map<IDType,EntityInfo> registeredEntities;

	void CreateEntityRecursive(const Notice::Board &board,IDType id) {
		auto classNode = IDRCheck(id, "class");
		if (classNode!=IDType::null) {
			assert(board.KeyExists(classNode));
			auto classId = board.GetID(classNode);
			check(registeredEntities.contains(classId), "the entity constructor is not defined: {}\n",
				IDToString(classId));

			registeredEntities.at(classId).createFunc(id);

			std::vector<IDType> children;
			if (auto r = board.GetChildren(IDR(id, "sub"),children)) {
				for (auto& x : children) {
					IDType classNodeHash = IDRCheck(x, "class");
					if (classNodeHash!=IDType::null && board.KeyExists(classNodeHash)) {
						CreateEntityRecursive(board,x);
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
	void UpdateEntityRecursive(Notice::Board &to,const Notice::Board &from,IDType node) {

		//output("checking: {}\n", IDToString(node));

		if (node != IDType::null) {
			if (GetLeaf(node) == ID("_internal")) {
				return;
			}
			if (from.KeyExists(node)) {
				// both exist
				ValueAny valueTo = to.GetValueAny(node);
				ValueAny valueFrom = from.GetValueAny(node);
				if (valueTo == valueFrom) {
					//	output("SAME!\n");
				} else {
//					output("updating: {}\n", IDToString(node));
//					output("      values: {} <- {}\n", valueTo.AsString(), valueFrom.AsString());
					to.Set(node, valueFrom);
				}
			} else {
				output("Removed {}\n", IDToString(node));
				to.Remove(node);
			}
		}
		std::vector<IDType> children;
		if (auto r = to.GetChildren(node,children /*fill*/)) {
			for (auto& x : children) {
				UpdateEntityRecursive(to /*fill*/,from,x);
			}
		}
	}

	void UpdateEntityRecursive2(Notice::Board& to, const Notice::Board& from, IDType node) {
//		output("checking: %s <- %s : %zu %zu\n", IDToString(to).c_str(), IDToString(from).c_str(),to,from);
		if (node != IDType::null) {
			if (to.KeyExists(node)) {
				// both exist
			} else {
				//			output("MISSING!!!! {}\n", IDToString(from).c_str());
				ValueAny valueTo = from.GetValueAny(node);
				to.Set(node, valueTo);
			}
		}
		std::vector<IDType> children;
		if (auto r = from.GetChildren(node, children /*fill*/)) {
			for (auto& x : children) {
				UpdateEntityRecursive2(to /*fill*/,from, x);
			}
		}
	}

	void CreateEntity(Notice::Board &board,IDType id) {
		CreateEntityRecursive(board,id);
	}

	void UpdateEntity(Notice::Board &to, const Notice::Board &from) {
		UpdateEntityRecursive(to,from, IDType::null);
		UpdateEntityRecursive2(to, from, IDType::null);
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


	void ManageGeometry(Notice::Board& board,IDType nodeId) {
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
