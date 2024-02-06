#include "../renderer/graphicsapi.h"
#include "nuggetgl/glinternal.h"
#include "graphicsapi_private.h"
#include "propertytree.h"
#include "system.h"

namespace nugget::renderer {
    using namespace nugget::gl;
    using namespace properties;

    std::unordered_map<IDType, RenderModelInfo> renderModels;
    std::unordered_map<IDType, RenderSetupCallback> renderSetupCallbacks;

    void RenderModel(IDType nodeID) {
        RenderModelInfo& renderModel = renderModels.at(nodeID);
        for (auto&& x : renderModel.renderSectionCallbacks) {
            x();
        }
    }

    void ConfigureModel(IDType nodeID,const std::vector<IDType> sections) {
        RenderModelInfo& modelInfo = renderModels[nodeID];
        if (modelInfo.nodeID == IDType::null) {
            modelInfo.nodeID = nodeID;
            check(modelInfo.VAOHandle == 0, "This should have been uninitialised (zero)");
            glGenVertexArrays(1, &modelInfo.VAOHandle);
        }
        glBindVertexArray(modelInfo.VAOHandle);
        for (auto&& node : sections) {
            IDType funcID = gProps.GetID(IDR(node, ID("function")));
            renderSetupCallbacks.at(funcID)(node, modelInfo);
        }

        glBindVertexArray(0);
    }

    RenderModelInfo &GetRenderModelInfo(IDType nodeID) {
        return renderModels.at(nodeID);
    }

    void RegisterRenderSetupCallback(IDType nodeID, const RenderSetupCallback& renderCallback) {
        renderSetupCallbacks[nodeID] = renderCallback;
    }


#if 0
        for (auto&& section : sections) {
            RenderSectionInfo& renderSectionInfo = modelInfo.renderSections[section];
            if (renderSectionInfo.nodeID == IDType::null) {
                renderSectionInfo.nodeID = nodeID;
                check(renderSectionInfo.VBOHandle == 0, "This should have been uninitialised (zero)");
                glGenBuffers(1, &renderSectionInfo.VBOHandle);
            }
            glBindBuffer(GL_ARRAY_BUFFER, renderSectionInfo.VBOHandle);
            glBufferData(GL_ARRAY_BUFFER, bufferSizeBytes, data, GL_STATIC_DRAW);
        }
#endif

/*    
    void BeginModelSection(IDType nodeID, size_t bufferSizeBytes, void* data, std::function<void(IDType)> renderCallback) {
        RenderModelData currentModel = *recent;
        RenderSection& section = currentModel.renderSections[nodeID];
        glBindBuffer(GL_ARRAY_BUFFER, section.VBOHandle);
        glBufferData(GL_ARRAY_BUFFER, bufferSizeBytes, data, GL_STATIC_DRAW);
    }
	void EndModelSection() {
	}

    void RenderModel(IDType nodeID) {
        RenderModelData& data = renderModels.at(nodeID);
        for (auto&& y : data.renderSections) {
            renderLambdas[y](y);
        }
    }
    */
}