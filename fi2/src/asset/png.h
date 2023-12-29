#pragma once

namespace nugget::asset {
	struct PNGImage {
		unsigned char* data;
		int width;
		int height;
		int channels;
		~PNGImage();
	};

	bool LoadPNG(std::string filename, PNGImage& png);
}