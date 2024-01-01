#include "../../external/stb_image.h"

#include <string>
#include "png.h"

namespace nugget::asset {
	bool Load(std::string filename, TextureData& tex) {
		tex.data = stbi_load(filename.c_str(), &tex.width, &tex.height, &tex.channels, 0);
		if (tex.data != nullptr) {
			return true;
		} else {
			return false;
		}
	}
}