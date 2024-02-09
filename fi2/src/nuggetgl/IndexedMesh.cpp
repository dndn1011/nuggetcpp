#include "system.h"
#include "propertytree.h"
#include "glinternal.h"
#include "debug.h"
#include "render.h"
#include "shader.h"
#include "asset/asset.h"
#include "utils/StableVector.h"
#include "../renderer/renderer.h"
#include "../renderer/graphicsapi.h"
#include "../nuggetgl/graphicsapi_private.h"
#include <format>
/*
* rcrnstn: yeah, what do you have there? Usually there is a way
* in the context creation api to specify that you want an
* sRGB-capable framebuffer. GLFW has `glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE)`,
* SDL has `SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE)`
* 
*/

namespace nugget::gl::indexedMesh {
    using namespace identifier;
    using namespace properties;
    using namespace renderer;


    std::vector<Notice::Handler> regsteredHandlers;

    struct RenderableSection {

        APPLY_RULE_OF_MINUS_4(RenderableSection);

        RenderModelInfo& renderModelInfo;

        inline static RenderModelInfo defaultValue;

        RenderableSection() : renderModelInfo(defaultValue) {
            assert(0);
        }

        RenderableSection(RenderModelInfo& rmiIn) : renderModelInfo(rmiIn) {
        }

        GLuint shader = 0;
        std::vector<GLuint> textures;
        std::vector<GLuint> textureUniforms;

        IDType modelID=IDType::null;

        GLuint IBO=0;
        GLuint VBO=0;

        const std::string shaderTexturePrefix = "texture";
        GLenum primitive=0;
        GLint start=0;
        GLsizei length=0;

//        Matrix4f modelMatrix;
//        Matrix4f viewMatrix;

        Vector3f cameraPos = {};
        Vector3f lookAtPos = {};
        Vector3f lookAtUp = {};

        void InitUniforms() {

            // get the locations of the texture uniforms for the shader
            for (GLuint i = 0; i < textures.size(); i++) {
                auto loc = glGetUniformLocation(shader,
                    std::format("{}{}", shaderTexturePrefix, i).c_str());
                textureUniforms.push_back(loc);
            }

#if 0
            GLint viewMatLocation = glGetUniformLocation(shader, "viewMatrix");
            if (viewMatLocation >= 0) {
                glUniformMatrix4fv(viewMatLocation, 1, GL_FALSE, viewMatrix.GetArray());
            }
#endif
        }

        void Render(const Matrix4f &modelMatrix) {

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
            glEnable(GL_FRAMEBUFFER_SRGB);

            glBindVertexArray(renderModelInfo.VAOHandle);

#if 0
            // move this to scene
            // TODO get rid of this hardwired thingy
            Vector3f RV = gProps.GetVector3f(ID("pomegranate.rotation"));
            Matrix4f R;
            Matrix4f::SetFromEulers(RV.x, RV.y, RV.z, R);
            modelMatrix = R * modelMatrix;
#endif

            // we are setting for each section, but in reality this is often shared, so will need some optimisation later
            GLint modMatLocation = glGetUniformLocation(shader, "modelMatrix");
            if (modMatLocation >= 0) {
                glUniformMatrix4fv(modMatLocation, 1, GL_FALSE, modelMatrix.GetArray());
            }

#if 0
            // move this to start of entire scene render (shared by all render objects)
            Matrix4f::LookAt(cameraPos, lookAtPos, lookAtUp, viewMatrix);
            GLint viewMatLocation = glGetUniformLocation(shader, "viewMatrix");
            if (viewMatLocation >= 0) {
                glUniformMatrix4fv(viewMatLocation, 1, GL_FALSE, viewMatrix.GetArray());
            }
#endif

            check(shader, "shader is not set");
            glUseProgram(shader);
            for (GLuint i = 0; i < textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, textures[i]);
                auto loc = textureUniforms[i];
                glUniform1i(loc, i);
            }

            const nugget::asset::ModelData& modelData = nugget::asset::GetModel(modelID);

            glDrawElements(GL_TRIANGLES, (GLsizei)modelData.indexBuffer.size(), GL_UNSIGNED_SHORT, 0);

            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                int a = 0;
            }
            glBindVertexArray(0);
        }

        void UpdateFromPropertyTree(IDType nodeID) {

            static const GLsizei layerNumFloats[] = {
                    3,
                    2,
                    4,
                    3,
            };

            static const size_t bufferNumFloats = std::size(layerNumFloats);

            size_t stride = 0;
            for (auto&& x : layerNumFloats) {
                stride += x;
            }

            // Vertex buffer
            if(VBO == 0) {
                IDType modelNodeID = IDR(nodeID, ID("model"));
                modelID = gProps.GetID(modelNodeID);
                const nugget::asset::ModelData& modelData = nugget::asset::GetModel(modelID);

                ////////////////
                // modelData
                //

                size_t VBOsize = modelData.loadBuffer.size() * sizeof(float);
                void* VBOdata = (void*)(modelData.loadBuffer.data());

                glGenBuffers(1, &VBO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, VBOsize, VBOdata, GL_STATIC_DRAW);
 
                for (size_t step = 0, i = 0; i < bufferNumFloats; step += layerNumFloats[i], ++i) {
                    // Set up vertex attributes
                    glVertexAttribPointer((GLuint)i, (GLuint)layerNumFloats[i], GL_FLOAT, GL_FALSE, (GLsizei)(stride * sizeof(float)), (void*)(step * sizeof(float)));
                    glEnableVertexAttribArray((GLuint)i);
                }

                glGenBuffers(1, &IBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    modelData.indexBuffer.size() * sizeof(uint16_t),
                    modelData.indexBuffer.data(),
                    GL_STATIC_DRAW);
            }

            // Texture
            if(textures.size() == 0) {
                // texture allocation
                if (textures.size()==0) {
                    textures.push_back({});
                }
                GLuint& texID = textures[0];
//                if (texID != 0) {
//                    glDeleteTextures(1, &texID);
//                }
                glGenTextures(1, &texID);

                // texture load
                IDType textureNode = IDR(nodeID, "texture");
                IDType textureName = gProps.GetID(textureNode);
                const nugget::asset::TextureData& texture = nugget::asset::GetTexture(textureName);
                glBindTexture(GL_TEXTURE_2D, texID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.data);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

#define PROP(a,t1,t2) IDType a##ID = IDR(nodeID,#a);a = (t2)gProps.Get##t1(a##ID)
            // put it together
            {
                PROP(start, Int64, GLuint);
                PROP(length, Int64, GLsizei);
//                PROP(modelMatrix, Matrix4f, Matrix4f);
//                PROP(viewMatrix, Matrix4f, Matrix4f);
                PROP(cameraPos, Vector3f, Vector3f);
                PROP(lookAtPos, Vector3f, Vector3f);
                PROP(lookAtUp, Vector3f, Vector3f);

                IDType primitiveID = IDR(nodeID, ID("primitive"));
                primitive = GLPrimitiveFromIdentifier(gProps.GetID(primitiveID));

                IDType shaderNodeHash = IDR(nodeID, "shader");
                IDType usedShaderNode = gProps.GetID(shaderNodeHash);
                shader = GetShaderHandle(usedShaderNode);
                check(shader != 0, "Failed to get shader handle for {}", IDToString(usedShaderNode));
                InitUniforms();
            }
        }

        void RegisterHotReloadHandlers(IDType nodeID) {
            gProps.RegisterHandlerOnChildren(Notice::Handler(nodeID,
                [this, nodeID](IDType id) {
                    UpdateFromPropertyTree(nodeID);
                }), regsteredHandlers);
            gProps.RegisterHandler(Notice::Handler(IDR(nodeID, "uvs"),
                [this, nodeID](IDType uid) {
                    UpdateFromPropertyTree(nodeID);
                }), regsteredHandlers);
            gProps.RegisterHandler(Notice::Handler(IDR(nodeID, "colors"),
                [this, nodeID](IDType cid) {
                    UpdateFromPropertyTree(nodeID);
                }), regsteredHandlers);

            IDType shaderNoideHash = IDR(nodeID, "shader");
            IDType shaderProgramNoticeID = IDR({ IDToString(gProps.GetID(shaderNoideHash)), "_internal", "_pglid" });
            gProps.RegisterHandler(Notice::Handler(shaderProgramNoticeID, [&, shaderProgramNoticeID](IDType id) {
                shader = (GLuint)gProps.GetInt64(shaderProgramNoticeID);
                }), regsteredHandlers);
        }

        void SetupFromPropertyTree(IDType nodeID) {
            UpdateFromPropertyTree(nodeID);

            RegisterHotReloadHandlers(nodeID);

            // add to list of render functions
            renderModelInfo.renderSectionCallbacks.push_back([this](const Matrix4f &modelMatrix) {
                Render(modelMatrix);
                });
        }

    };

    static std::unordered_map<IDType, RenderableSection> renderableSections;


//    static void RenderSection(IDType node) {
//
//    }

    static void Init() {
        RegisterRenderSetupCallback(IDR("CreateIndexedMesh"), 
            [](IDType sectionNode, RenderModelInfo& modelInfo) {
            auto [iterator,succeed] = 
                renderableSections.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(sectionNode),
                    std::forward_as_tuple(modelInfo));
            check(succeed,"could not add section");
            (*iterator).second.SetupFromPropertyTree(sectionNode);
            });
    }


//    static void Render(IDType sect ionNode) {
//        sections[sectionNode].Render();
//    }

    static size_t init_dummy[] =
    {
        {
            nugget::system::RegisterModule([]() {
            if (!Notice::gBoard.KeyExists(ID("commandLine.assignments.runHeadless"))) {
                Init();
            }
            return 0;
            }, 200)
        }
    };

}

