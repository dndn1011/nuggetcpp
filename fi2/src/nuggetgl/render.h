#pragma once
#include "identifier.h"

namespace nugget::gl {
	using namespace identifier;
	GLenum GLPrimitiveFromIdentifier(IDType ID);
	const GLfloat *GLCameraProjectionMatrix();
	void GLCameraSetProjectionFromProperties(IDType nodeID);
}
