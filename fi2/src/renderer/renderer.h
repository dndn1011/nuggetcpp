#pragma once
#include "identifier.h"
#include <functional>
#include "types.h"

namespace nugget::renderer {
	using namespace identifier;

	void RenderModel(IDType nodeID,const Matrix4f& modelMatrix, const Matrix4f& viewMatrix);

	void ConfigureRenderModel(IDType idNode);
}
