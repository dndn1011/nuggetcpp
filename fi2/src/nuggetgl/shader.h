#pragma once
//#include "glinternal.h"
#include "identifier.h"

namespace nugget::gl {
	using namespace nugget::identifier;

	GLuint GetShaderHandle(IDType);

	void CompileShaderFromProperties(nugget::identifier::IDType node);

	void SetAllUniforms(const char* name, const float* matrix);

}