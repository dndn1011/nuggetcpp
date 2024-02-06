#pragma once
#include "propertytree.h"
// this is a cross platform header to a platform specific implmentation

namespace nugget::renderer {
	using namespace identifier;

	void ConfigureModel(IDType nodeID, const std::vector<IDType> sections);

#if 0
	void BeginModelSection(IDType nodeID, size_t bufferSizeBytes, void* data);
	void EndModelSection();

	void BeginModel(IDType nodeID);
	void EndModel();

	void RegisterRenderCallback(IDType idNode, std::function<void(IDType)> lambda);
#endif
}