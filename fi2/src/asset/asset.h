#pragma once

#include "identifier.h"
#include "../utils/utils.h"

namespace nugget::asset {
	using namespace identifier;
	struct TextureData {
		APPLY_RULE_OF_MINUS_4(TextureData);
		TextureData() {}
		unsigned char* data=nullptr;
		int width=0;
		int height=0;
		int channels=0;
		~TextureData();
	};

	struct ModelData {
		APPLY_RULE_OF_MINUS_4(ModelData);
		ModelData() {}
		std::vector<float> loadBuffer;
		std::vector<uint16_t> indexBuffer;
		~ModelData();
	};

	const TextureData& GetTexture(IDType id);

	const ModelData& GetModel(IDType id);

}
             

