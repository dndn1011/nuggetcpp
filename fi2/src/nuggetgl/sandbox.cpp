#include "system.h"
#include "notice.h"
#include "identifier.h"
#include "glinternal.h"
#include "utils/StableVector.h"
#include "propertytree.h"
#include "asset/asset.h"
#include "render.h"

namespace nugget::gl::sandbox {
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


        const std::string texturePrefix = "texture";
        GLenum primitive;
        GLint start;
        GLsizei length;
        Matrix4f modelMatrix;
        Matrix4f viewMatrix;
        Vector3f cameraPos;
        Vector3f lookAtPos;
        Vector3f lookAtUp;

        void Init() {
            for (GLuint i = 0; i < textures.size(); i++) {
                auto loc = glGetUniformLocation(shader,
                    std::format("{}{}", texturePrefix, i).c_str());
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

            Vector3f RV = gProps.GetVector3f(ID("testobj.section.rotation"));
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

            // Draw the triangle
            glDrawArrays(primitive,
                start,
                length
            );

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

            IDType verts = IDR(nodeID, ID("verts"));
            Vector3fList vertices;
            if (gProps.GetVector3fList(verts, vertices)) {
                for (auto&& z : vertices.data) {
                    vertData.push_back(z.x);
                    vertData.push_back(z.y);
                    vertData.push_back(z.z);
                }
            } else {
                assert(0);
            }
            IDType uvsid = IDR(nodeID, ID("uvs"));
            Vector2fList uvs;
            if (gProps.GetVector2fList(uvsid, uvs)) {
                for (auto&& z : uvs.data) {
                    uvData.push_back(z.x);
                    uvData.push_back(z.y);
                }
            } else {
                assert(0);
            }
            IDType colsid = IDR(nodeID, ID("colors"));
            ColorList cols;
            if (gProps.GetColorList(colsid, cols)) {
                for (auto&& z : cols.data) {
                    colsData.push_back(z.r);
                    colsData.push_back(z.g);
                    colsData.push_back(z.b);
                    colsData.push_back(z.a);
                }
            } else {
                assert(0);
            }
            auto vsize = vertData.size() * sizeof(float);
            auto usize = uvData.size() * sizeof(float);
            auto csize = colsData.size() * sizeof(float);

            // Vertex buffer
            {
                // vbo allocation
                GLuint VBO;
                size_t VBOsize = vsize + usize + csize;
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
                glBufferSubData(GL_ARRAY_BUFFER, 0, vsize, vertData.data());
                glBufferSubData(GL_ARRAY_BUFFER, vsize, usize, uvData.data());
                glBufferSubData(GL_ARRAY_BUFFER, vsize + usize, csize, colsData.data());

                // Set up vertex attributes
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);

                // Set the attribute pointers for the UV coordinates
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)vsize);
                glEnableVertexAttribArray(1);

                // Set the attribute pointers for the vertex colours
                glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(vsize + usize));
                glEnableVertexAttribArray(2);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    
    
    void triangle_test() {
        renderable.Init();
        renderable.AddSections(ID("testobj"));
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
    
    void Init() {
        triangle_test();
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