#pragma once

#include "../renderer/graphicsapi.h"
#include "nuggetgl/glinternal.h"
#include "identifier.h"

namespace nugget::renderer {

    struct RenderSectionInfo;
    using SectionRenderCallback = std::function<void(const Matrix4f &modelMatrix, const Matrix4f& viewMatrix)>;

    struct RenderModelInfo;
    using RenderSetupCallback = std::function<void(IDType sectionNode,RenderModelInfo& model)>;
    struct RenderModelInfo {
        IDType nodeID = IDType::null;
        GLuint VAOHandle = 0;
        RenderSetupCallback renderSetupCallback;
        std::vector<SectionRenderCallback> renderSectionCallbacks;
    };
    void RegisterRenderSetupCallback(IDType nodeID,const RenderSetupCallback &renderCallback);

}
