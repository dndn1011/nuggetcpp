#include "UIEntities.h"
#include <unordered_map>
#include <functional>
#include <assert.h>
#include "uicontainer.h"
#include "uicircle.h"
#include "uibutton.h"
#include "uitextbox.h"
#include "notice.h"
#include "debug.h"
#include "../utils/utils.h"

namespace nugget::ui::entity {
	using namespace identifier;
	using namespace Notice;

	static std::unordered_map<IDType, CreateLambda> functionMap;

	void CreateEntity(IDType id, IDType class_) {
		assert(0);
		output("CreateEntity: {}\n", IDToString(id));
		assert(functionMap.contains(class_));
		functionMap[class_](id);
	}
	
	void CreateEntityRecursive(IDType id) {
//		output("CreateEntity: {}\n", IDToString(id));
		auto classNode = IDRCheck(id, "class");
		if (classNode) {
			assert(Notice::KeyExists(classNode));
			auto classId = Notice::GetID(classNode);
			assert(("the entity constructor is not defined", functionMap.contains(classId)));

//			printid(id);
			functionMap[classId](id);

			std::vector<IDType> children;
			if (auto r = Notice::GetChildren(IDR(id, "sub"),children)) {
				for (auto& x : children) {
					IDType classNodeHash = IDRCheck(x, "class");
					if (classNodeHash && Notice::KeyExists(classNodeHash)) {
						CreateEntityRecursive(x);
					}
				}
			}
		} else {
			assert(("Could not find entity node", 0));
		}
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
		return 0;
	}

	// compare the state of the property tree with the current instances of entities
	// and sync
	void UpdateEntityRecursive(IDType to, IDType from) {
		//output("checking: %s <- %s : %zu %zu\n", IDToString(to).c_str(), IDToString(from).c_str(),to,from);
		if (Notice::KeyExists(from)) {
			// both exist
			Notice::ValueAny valueTo = Notice::GetValueAny(to);
			Notice::ValueAny valueFrom = Notice::GetValueAny(from);
			if (valueTo == valueFrom) {
			//	output("SAME!\n");
			} else {
				//output("updating: %s <- %s\n", IDToString(to).c_str(), IDToString(from).c_str());
				//output("      values: %s <- %s\n", valueTo.GetValueAsString().c_str(), valueFrom.GetValueAsString().c_str());
				Notice::Set(to, valueFrom);
			}
		} else {
			Notice::Remove(to);
//			output("Removed {}\n", IDToString(to).c_str());
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
			Notice::ValueAny valueTo = Notice::GetValueAny(from);
			Notice::Set(to, valueTo);
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

	void ManageGeometry(IDType nodeId) {
		nugget::ui::container::ManageGeometry(nodeId);
		nugget::ui::textBox::ManageGeometry(nodeId);
	}
	 
	void RegisterFunction(IDType type, CreateLambda func) {
		functionMap[type]=func;
	}

	void CallFunction(IDType functionId,IDType entityId) {
		check(functionMap.contains(functionId),"no such function: {}",IDToString(functionId));
		functionMap.at(functionId)(entityId);
	}


	void Init() {
		nugget::ui::entity::RegisterFunction(IDR("ui::Container"), ui::container::Create);
		nugget::ui::entity::RegisterFunction(IDR("ui::Circle"), ui::circle::Create);
		nugget::ui::entity::RegisterFunction(IDR("ui::Button"), ui::button::Create);
		nugget::ui::entity::RegisterFunction(IDR("ui::TextBox"), ui::textBox::Create);
	}

	
}
