#include "system.h"
#include "propertytree.h"
#include "glinternal.h"
#include "debug.h"
#include "render.h"
#include "asset/asset.h"
#include "utils/StableVector.h"

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


    std::vector<Notice::Handler> regsteredHandlers;
    
    struct RenderableSection {
        struct VBOInfo {
            GLuint slot;
            size_t size;
        };

        APPLY_RULE_OF_MINUS_4(RenderableSection);

        RenderableSection() {}

        GLuint shader;
        std::vector<VBOInfo> VBOs;
        std::vector<GLuint> textures;
        std::vector<GLuint> textureUniforms;

        IDType modelID;

        GLuint IBO;

        const std::string shaderTexturePrefix = "texture";
        GLenum primitive;
        GLint start;
        GLsizei length;
        Matrix4f modelMatrix;
        Matrix4f viewMatrix;

        Vector3f cameraPos;
        Vector3f lookAtPos;
        Vector3f lookAtUp;

        void InitUniforms() {

            // get the locations of the texture uniforms for the shader
            for (GLuint i = 0; i < textures.size(); i++) {
                auto loc = glGetUniformLocation(shader,
                    std::format("{}{}", shaderTexturePrefix, i).c_str());
                textureUniforms.push_back(loc);
            }

            glUseProgram(shader);
            GLint projMatLocation = glGetUniformLocation(shader, "projectionMatrix");
            if (projMatLocation >= 0) {
                glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, GLCameraProjectionMatrix());
            }
            GLint modMatLocation = glGetUniformLocation(shader, "modelMatrix");
            if (modMatLocation >= 0) {
                glUniformMatrix4fv(modMatLocation, 1, GL_FALSE, modelMatrix.GetArray());
            }
            GLint viewMatLocation = glGetUniformLocation(shader, "viewMatrix");
            if (viewMatLocation >= 0) {
                glUniformMatrix4fv(viewMatLocation, 1, GL_FALSE, viewMatrix.GetArray());
            }
        }

        void Render() {
            // TODO get rid of this hardwired thingy
            Vector3f RV = gProps.GetVector3f(ID("indexedMesh.section.rotation"));
            Matrix4f R;
            Matrix4f::SetFromEulers(RV.x, RV.y, RV.z, R);
            modelMatrix = R * modelMatrix;


            GLint modMatLocation = glGetUniformLocation(shader, "modelMatrix");
            if (modMatLocation >= 0) {
                glUniformMatrix4fv(modMatLocation, 1, GL_FALSE, modelMatrix.GetArray());
            }

            Matrix4f::LookAt(cameraPos, lookAtPos, lookAtUp, viewMatrix);
            GLint viewMatLocation = glGetUniformLocation(shader, "viewMatrix");
            if (viewMatLocation >= 0) {
                glUniformMatrix4fv(viewMatLocation, 1, GL_FALSE, viewMatrix.GetArray());
            }

            glUseProgram(shader);
            for (GLuint i = 0; i < textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, textures[i]);
                auto loc = textureUniforms[i];
                glUniform1i(loc, i);
            }

            const nugget::asset::ModelData& modelData = nugget::asset::GetModel(modelID);

            glDrawElements(GL_TRIANGLES, (GLsizei) modelData.indexBuffer.size(), GL_UNSIGNED_SHORT, 0);

            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                int a = 0;
            }

        }

        void UpdateFromPropertyTree(IDType nodeID, int sectionIndex) {
            static int count = 0;
            count++;


            std::vector<float> vertData;
            std::vector<float> uvData;
            std::vector<float> colsData;
            std::vector<float> normalData;

            struct Layer {
                int numFloats;
                IDType nodeID;
                std::function<void(IDType node, std::vector<float>& fill)> getFloats;

                std::vector<float> data;
            };

            static const GLsizei layerNumFloats[] = {
                    3,
                    2,
                    4,
                    3,
            };

            static const size_t bufferNumFloats = std::size(layerNumFloats);

            size_t stride = 0;
            for(auto &&x : layerNumFloats) {
                stride += x;
            }

            // Vertex buffer
            {
                // vbo allocation
                GLuint VBO;

                IDType modelNodeID = IDR(nodeID, ID("model"));
                modelID = gProps.GetID(modelNodeID);
                const nugget::asset::ModelData& modelData = nugget::asset::GetModel(modelID);

                size_t VBOsize = modelData.loadBuffer.size() * sizeof(float);

                if (sectionIndex >= VBOs.size()) {
                    check(sectionIndex == VBOs.size(), "Need to increment smoothly through section indices");
                    VBOs.push_back({});
                }
                VBOInfo& vboInfo = VBOs[sectionIndex];
                if (vboInfo.slot == 0 || vboInfo.size != VBOsize) {
                    if (vboInfo.slot != 0) {
                        check(VBOsize != 0, "Zero size?");
                        glDeleteBuffers(1, &vboInfo.slot);
                    }
                    glGenBuffers(1, &VBO);
                    vboInfo.size = VBOsize;
                    vboInfo.slot = VBO;
                    glBindBuffer(GL_ARRAY_BUFFER, vboInfo.slot);
                    glBufferData(GL_ARRAY_BUFFER, VBOsize, nullptr, GL_STATIC_DRAW);
                } else {
                    glBindBuffer(GL_ARRAY_BUFFER, vboInfo.slot);
                }

                glBufferSubData(GL_ARRAY_BUFFER, 0, modelData.loadBuffer.size() * sizeof(float), modelData.loadBuffer.data());

                for (size_t step = 0, i = 0; i < bufferNumFloats; step += layerNumFloats[i], ++i) {
                    // Set up vertex attributes
                    glVertexAttribPointer((GLuint)i, (GLuint)layerNumFloats[i], GL_FLOAT, GL_FALSE, (GLsizei)(stride * sizeof(float)), (void*)(step*sizeof(float)));
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
            {
                // texture allocation
                if (sectionIndex >= textures.size()) {
                    check(sectionIndex == textures.size(), "Need to increment smoothly through section indices");
                    textures.push_back({});
                }
                GLuint& texID = textures[sectionIndex];
                if (texID != 0) {
                    glDeleteTextures(1, &texID);
                }
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
                PROP(modelMatrix, Matrix4f, Matrix4f);
                PROP(viewMatrix, Matrix4f, Matrix4f);
                PROP(cameraPos, Vector3f, Vector3f);
                PROP(lookAtPos, Vector3f, Vector3f);
                PROP(lookAtUp, Vector3f, Vector3f);

                IDType primitiveID = IDR(nodeID, ID("primitive"));
                primitive = GLPrimitiveFromIdentifier(gProps.GetID(primitiveID));

                IDType shaderNodeHash = IDR(nodeID, "shader");
                IDType usedShaderID = gProps.GetID(shaderNodeHash);
                std::string usedShaderPath = IDToString(usedShaderID);
                IDType shaderProgramNoticeID = IDR({ usedShaderPath, "_internal", "_pglid" });
                shader = (GLuint)gProps.GetInt64(shaderProgramNoticeID);

                InitUniforms();
            }
        }

        void RegisterHotReloadHandlers(IDType nodeID, int sectionIndex) {
            gProps.RegisterHandlerOnChildren(Notice::Handler(nodeID,
                [this, nodeID, sectionIndex](IDType id) {
                    UpdateFromPropertyTree(nodeID, sectionIndex);
                }), regsteredHandlers);
            gProps.RegisterHandler(Notice::Handler(IDR(nodeID, "uvs"),
                [this, nodeID, sectionIndex](IDType uid) {
                    UpdateFromPropertyTree(nodeID, sectionIndex);
                }), regsteredHandlers);
            gProps.RegisterHandler(Notice::Handler(IDR(nodeID, "colors"),
                [this, nodeID, sectionIndex](IDType cid) {
                    UpdateFromPropertyTree(nodeID, sectionIndex);
                }), regsteredHandlers);

            IDType shaderNoideHash = IDR(nodeID, "shader");
            IDType shaderProgramNoticeID = IDR({ IDToString(gProps.GetID(shaderNoideHash)), "_internal", "_pglid" });
            gProps.RegisterHandler(Notice::Handler(shaderProgramNoticeID, [&, shaderProgramNoticeID](IDType id) {
                shader = (GLuint)gProps.GetInt64(shaderProgramNoticeID);
                }), regsteredHandlers);
        }

        void SetupFromPropertyTree(IDType nodeID, int sectionIndex) {
            UpdateFromPropertyTree(nodeID, sectionIndex);

            RegisterHotReloadHandlers(nodeID, sectionIndex);
        }

    };

    struct Renderable {
        APPLY_RULE_OF_MINUS_4(Renderable);

        Renderable() {
        }

        GLuint VAO = 0;
        StableVector<RenderableSection, 100> sections;

        void Init() {
            if (VAO == 0) {
                glGenVertexArrays(1, &VAO);
            }
        }

        bool AddSections(IDType id) {
            check(VAO, "Ya didn't init, innit?");
            // id is a section in propertytree to populate the instance
            check(gProps.KeyExists(id), "Node does not exist: {}\n", IDToString(id));
            std::vector<IDType> children;
            gProps.GetChildrenOfType(id, ValueAny::Type::parent_, children);
            glBindVertexArray(VAO);
            GLuint index = 0;
            for (auto&& x : children) {
                sections.emplace_back();
                auto& section = sections.back();
                section.SetupFromPropertyTree(x, index);
                index++;
            }
            glBindVertexArray(0);
            return true;
        }
    };

    static StableVector<Renderable, 100> renderables;

    static Renderable& NewRenderable() {
        renderables.emplace_back();
        return renderables.back();
    }

    static void InitRenderables() {
        // TODO pull the object from property tree with all sections
        Renderable& renderable = NewRenderable(); 
        renderable.Init();
        renderable.AddSections(ID("indexedMesh"));  // TODO get rid of hard wiring
    }

    void Init() {
        system::RegisterFunctionByID(IDR("CreateIndexedMesh"), [](IDType node) {
            int a = 0;
        });

        InitRenderables();
    }

    void Update() {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        glEnable(GL_FRAMEBUFFER_SRGB);

        for (auto&& x : renderables) {
            glBindVertexArray(x.VAO);
            for (auto&& y : x.sections) {
                y.Render();
            } 
        }

    }

    static size_t init_dummy[] =
    {
        {
            nugget::system::RegisterModule([]() {
            if (!Notice::gBoard.KeyExists(ID("commandLine.assignments.runHeadless"))) {
                Init();
            }
            return 0;
           }, 200)
        },
        {
            nugget::system::RegisterModule([]() {
            if (!Notice::gBoard.KeyExists(ID("commandLine.assignments.runHeadless"))) {
                Update();
            }
            return 0;
            }, 200, ID("update"))
        }
    };
}