#pragma once
#include "identifier.h"
#include <functional>
#include "types.h"
#include "../scene/scene.h"

namespace nugget::renderer {
	using namespace identifier;
	using namespace scene;

	void RenderModel(IDType nodeID,const Transform& transform, const Matrix4f& viewMatrix);

	void ConfigureRenderModel(IDType idNode);
}
