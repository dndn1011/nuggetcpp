#include "../../external/stb_image.h"

#include <string>
#include "png.h"

namespace nugget::asset {
	bool LoadPNG(std::string filename, TextureData& tex) {
		//stbi_set_flip_vertically_on_load(true);
		tex.data = stbi_load(filename.c_str(), &tex.width, &tex.height, &tex.channels, STBI_rgb);
		if (tex.data != nullptr) {
			return true;
		} else {
			return false;
		}
	}
}