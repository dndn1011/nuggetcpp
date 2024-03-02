#pragma once
#include "identifier.h"
#include <functional>
#include "glinternal.h"

namespace nugget::renderer {
	using namespace identifier;

	struct TextureService {
		IDType requiredTextureHash = IDType::null;
		GLuint textureHandle = 0;

		IDType suppliedTextureHash = IDType::null;
	};

	void RequestTextureService(TextureService& service);
	
	void TexturesAreDirty();
}