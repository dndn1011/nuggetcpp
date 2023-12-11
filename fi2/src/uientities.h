#pragma once
#include "identifier.h"
#include <functional>
#include "../utils/utils.h"

namespace nugget::ui::entity {
	using namespace identifier;
	
	using CreateLambda = std::function<void(IDType id)>;

	void CallFunction(IDType functionId,IDType entityId);
	void CreateEntity(IDType id, IDType type);
	void CreateEntity(IDType id);
	void UpdateEntity(IDType to, IDType from);
	void RegisterFunction(IDType type, CreateLambda func);
	void Init();
	void ManageGeometry(IDType nodeId);
}