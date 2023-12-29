#include "../../external/stb_image.h"

#include <string>
#include "png.h"

namespace nugget::asset {
	bool LoadPNG(std::string filename, PNGImage& png) {
		png.data = stbi_load(filename.c_str(), &png.width, &png.height, &png.channels, 0);
		if (png.data != nullptr) {
			return true;
		} else {
			return false;
		}
	}
	PNGImage::~PNGImage() {
		if (data) {
			stbi_image_free(data);
		}
	}
}