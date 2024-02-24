#include <unordered_map>
#include "glinternal.h"
#include "asset/asset.h"
#include "texture.h"

namespace nugget::renderer {
	using namespace nugget::identifier;

	std::unordered_map<IDType, GLuint> handlerMap;

	GLuint TextureHandleByHash(IDType node) {
		if (handlerMap.contains(node)) {
			return handlerMap.at(node);
		} else {
			GLuint glTexID;
			const nugget::asset::TextureData& textureData = nugget::asset::GetTexture(node);
			glGenTextures(1, &glTexID);

			glBindTexture(GL_TEXTURE_2D, glTexID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, textureData.width, textureData.height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data);
			glBindTexture(GL_TEXTURE_2D, 0);
			handlerMap[node] = glTexID;
			return glTexID;
		}
	}

}