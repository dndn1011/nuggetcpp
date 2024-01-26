#pragma once

#include "identifier.h"
#include "../utils/utils.h"

namespace nugget::asset {
	using namespace identifier;
	struct TextureData {
		APPLY_RULE_OF_MINUS_4(TextureData);
		TextureData() {}
		unsigned char* data;
		int width;
		int height;
		int channels;
		~TextureData();
	};

	struct ModelData {
		APPLY_RULE_OF_MINUS_4(ModelData);
		ModelData() {}
		std::vector<char> data;
		int width;
		int height;
		int channels;
		~ModelData();
	};

	const TextureData& GetTexture(IDType id);

	const ModelData& GetModel(IDType id);

}
             

