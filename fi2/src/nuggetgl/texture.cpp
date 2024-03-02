#include <unordered_map>
#include "glinternal.h"
#include "asset/asset.h"
#include "texture.h"
#include <functional>

namespace nugget::renderer {
	using namespace nugget::identifier;
	using namespace nugget::asset;

	std::unordered_map<IDType, GLuint> handlerMap;

	void LoadTexture(IDType hash,GLuint glTexID) {
		const nugget::asset::TextureData& textureData = nugget::asset::GetTexture(hash);
		glBindTexture(GL_TEXTURE_2D, glTexID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, textureData.width, textureData.height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data);
		glBindTexture(GL_TEXTURE_2D, 0);
		handlerMap[hash] = glTexID;
	}

	GLuint TextureHandleByHash(IDType hash) {
		if (handlerMap.contains(hash)) {
			return handlerMap.at(hash);
		} else {
			GLuint glTexID;
			glGenTextures(1, &glTexID);
			LoadTexture(hash, glTexID);
			return glTexID;
		}
	}

	void RequestTextureService(TextureService& service) {
		service.textureHandle = TextureHandleByHash(service.requiredTextureHash);
		service.suppliedTextureHash = service.requiredTextureHash;
	}

	void TexturesAreDirty() {
		for (auto&& x : handlerMap) {
			if (IsTextureOutOfDate(x.first)) {
				LoadTexture(x.first, x.second);
			}
		}
	}



}