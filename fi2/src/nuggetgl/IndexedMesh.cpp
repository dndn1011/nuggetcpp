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
        std::vector<GLuint> glTextureHandles;
        std::vector<GLuint> textureUniforms;

        IDType modelID=IDType::null;

        GLuint IBO=0;
        GLuint VBO=0;

        const std::string shaderTexturePrefix = "texture";
        GLenum primitive=0;

//        Matrix4f modelMatrix;
//        Matrix4f viewMatrix;

        Vector3f cameraPos = {};
        Vector3f lookAtPos = {};
        Vector3f lookAtUp = {};

        void InitUniforms() {

            // get the locations of the texture uniforms for the shader
            for (GLuint i = 0; i < glTextureHandles.size(); i++) {
                auto loc = glGetUniformLocation(shader,
                    std::format("{}{}", shaderTexturePrefix, i).c_str());
                textureUniforms.push_back(loc);
            }
        }

        Vector3f lightPos = { 100,50,100 };

        void Render(const Transform &transform,const Matrix4f &viewMatrix) {

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
            glEnable(GL_FRAMEBUFFER_SRGB);

            glBindVertexArray(renderModelInfo.VAOHandle);

            GLint lightPosHandle = glGetUniformLocation(shader, "lightPos");
            glUniform3f(lightPosHandle, lightPos.x, lightPos.y, lightPos.z);

            float k = 32.0f;
            lightPos.x += lightPos.z / k;
            lightPos.z -= lightPos.x / k;

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
                glUniformMatrix4fv(modMatLocation, 1, GL_FALSE, transform.Matrix().GetArray());
            }

            GLint viewMatLocation = glGetUniformLocation(shader, "viewMatrix");
            if (viewMatLocation >= 0) {
                glUniformMatrix4fv(viewMatLocation, 1, GL_FALSE, viewMatrix.GetArray());
            }

            // we are setting for each section, but in reality this is often shared, so will need some optimisation later
            GLint modRotLocation = glGetUniformLocation(shader, "rotationMatrix");
            if (modRotLocation >= 0) {
                glUniformMatrix3fv(modRotLocation, 1, GL_FALSE, transform.Rot().GetArray());
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
            for (GLuint i = 0; i < glTextureHandles.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, glTextureHandles[i]);
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

            // todo handle reloading
            assert(glTextureHandles.size() == 0);

            // Texture
            {
                // texture allocation

                IDType texturesNode = IDR(nodeID, "textures");
                const IdentifierList& list = gProps.GetIdentifierList(texturesNode);

                for (size_t i = 0; i < list.data.size(); ++i) {
                    const nugget::asset::TextureData& textureData = nugget::asset::GetTexture(list.data[i]);
                    GLuint glTexID;
                    glGenTextures(1, &glTexID);
                    glBindTexture(GL_TEXTURE_2D, glTexID);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, textureData.width, textureData.height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data);
                    glBindTexture(GL_TEXTURE_2D, 0);

                    glTextureHandles.push_back(glTexID);
                }
            }

#define PROP(a,t1,t2) IDType a##ID = IDR(nodeID,#a);a = (t2)gProps.Get##t1(a##ID)
            // put it together
            {
                PROP(cameraPos, Vector3f, Vector3f);
                PROP(lookAtPos, Vector3f, Vector3f);
                PROP(lookAtUp, Vector3f, Vector3f);

                IDType primitiveID = IDR(nodeID, ID("primitive"));
                primitive = GLPrimitiveFromIdentifier(gProps.GetID(primitiveID));

                IDType shaderNodeHash = IDR(nodeID, "shader");
                IDType usedShaderNode = gProps.GetID(shaderNodeHash);
          
                {   // get the shader handle and update the shader handle when notified
                    shader = GetShaderHandle(usedShaderNode);

                    Notice::gBoard.RegisterHandler(
                        Notice::Handler(
                            IDR("nugget.gl.shaders", usedShaderNode),
                            [this, usedShaderNode](IDType id) {
                                shader = GetShaderHandle(usedShaderNode);
                            }),
                        regsteredHandlers);
                }

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
            renderModelInfo.renderSectionCallbacks.push_back([this](const Transform &transform, const Matrix4f& viewMatrix) {
                Render(transform, viewMatrix);
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

