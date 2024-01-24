#include "system.h"
#include "propertytree.h"
#include "glinternal.h"
#include "debug.h"
#include "render.h"
#include "asset/asset.h"
#include "utils/StableVector.h"

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

        void Init() {

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

//            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

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

            std::vector<Layer> layers = {
                {
                    3,
                    ID("verts"),
                    [&](IDType node, std::vector<float>& fill) { gProps.GetVector3fList(node).ToFloats(fill); },
                },
                {
                    2,
                    ID("uvs"),
                    [&](IDType node, std::vector<float>& fill) { gProps.GetVector2fList(node).ToFloats(fill); },
                },
                {
                    4,
                    ID("colors"),
                    [&](IDType node, std::vector<float>& fill) { gProps.GetColorList(node).ToFloats(fill); },
                },
                {
                    3,
                    ID("normals"),
                    [&](IDType node, std::vector<float>& fill) { gProps.GetVector3fList(node).ToFloats(fill); },
                },
            };

            size_t bufferNumFloats = 0;
            size_t stride = 0;

            for (auto&& x : layers) {
                x.getFloats(IDR(nodeID, x.nodeID),x.data);
                bufferNumFloats += x.data.size();
                stride += x.numFloats;
            }

            size_t numVerts = bufferNumFloats / stride;

            check(numVerts* stride == bufferNumFloats, "incomplete data");

            std::vector<GLfloat> buffer(bufferNumFloats);

            for (size_t offset=0, i = 0; i < layers.size(); offset+= layers[i].numFloats, ++i) {
                for (size_t j = 0; j < numVerts; ++j) {
                    for (size_t k = 0; k < layers[i].numFloats; ++k) {
                        buffer[offset + j * stride + k] = layers[i].data[j * layers[i].numFloats + k];
            //            output("index {} = {} of layer {}\n", offset + j * stride + k, j* layers[i].numFloats + k,i);
                    }
                }
            }

            Int64List indexData;
            gProps.GetInt64List(IDR(nodeID, ID("indices")), indexData);
            std::vector<uint16_t> indices;
            indices.reserve(indexData.data.size());
            for (int i = 0; i < indexData.data.size(); ++i) {
                indices.push_back((GLushort)indexData.data[i]);
            }            

            // Vertex buffer
            {
                // vbo allocation
                GLuint VBO;
                size_t VBOsize = bufferNumFloats * sizeof(float);
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

                // vbo data
                glBufferSubData(GL_ARRAY_BUFFER, 0, bufferNumFloats*sizeof(float), buffer.data());

                for (size_t step = 0, i = 0; i < layers.size(); step += layers[i].numFloats, ++i) {
                    // Set up vertex attributes
                    glVertexAttribPointer((GLuint)i, (GLuint)layers[i].numFloats, GL_FLOAT, GL_FALSE, (GLsizei)(stride * sizeof(float)), (void*)(step*sizeof(float)));
                    glEnableVertexAttribArray((GLuint)i);
                }

                glGenBuffers(1, &IBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

//                glBindBuffer(GL_ARRAY_BUFFER, 0);
//                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.data);
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

                Init();

#if 0

                gProps.RegisterHandler(gProps.Handler(startNoticeID, [this](IDType startid) {
                    renderable.start = (GLint)gProps.GetInt64(startid);
                    }));
                gProps.RegisterHandler(gProps.Handler(lengthNoticeID, [this](IDType lengthid) {
                    renderable.length = (GLint)gProps.GetInt64(lengthid);
                    }));
                gProps.RegisterHandler(gProps.Handler(primNoticeID, [this](IDType primid) {
                    renderable.primitive = primitiveMap.at(gProps.GetID(primid));
                    }));
#endif
            }
        }

        void SetupFromPropertyTree(IDType nodeID, int sectionIndex) {
            output("-------@@@@@@@------------> {}\n", IDToString(nodeID));
            IDType shaderNoideHash = IDR(nodeID, "shader");
            IDType shaderProgramNoticeID = IDR({ IDToString(gProps.GetID(shaderNoideHash)), "_internal", "_pglid" });
            UpdateFromPropertyTree(nodeID, sectionIndex);

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

            gProps.RegisterHandler(Notice::Handler(shaderProgramNoticeID, [&, shaderProgramNoticeID](IDType id) {
                shader = (GLuint)gProps.GetInt64(shaderProgramNoticeID);
                }), regsteredHandlers);

        }

    };

    struct Renderable {
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

    static Renderable renderable;

    void Init() {
        renderable.Init();
        renderable.AddSections(ID("indexedCube"));
    }

    void Update() {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);

        glBindVertexArray(renderable.VAO);

        for (auto&& x : renderable.sections) {
            x.Render();
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