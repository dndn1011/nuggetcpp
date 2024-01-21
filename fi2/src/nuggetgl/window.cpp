#include <iostream>
#include <Windows.h>
#include "debug.h"
#include "glinternal.h"
#include <functional>
#include "shader.h"
#include "notice.h"
#include "identifier.h"
#include "asset/asset.h"
#include <format>
#include "utils/StableVector.h"
#include "propertytree.h"

namespace nugget::gl {
    using namespace nugget::identifier;
    using namespace nugget::properties;

#if 0
    struct GPUTexturePool {
        std::vector<GLuint> freePool;
        GPUTexturePool() {
            GLint maxGPUTextures;
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxGPUTextures);
            for (GLuint i = 0; i < maxGPUTextures; i++) {
                freePool.push_back(i);
            }
        }
        GLuint NewUnit() {
            GLuint v = freePool.back();
            freePool.pop_back();
            return v;
        }
        void FreeUnit(GLuint v) {
            freePool.push_back(v);
        }
    };

    static GPUTexturePool texturePool;
#endif

#define PRIMDEF(a) { ID(#a),a },
    static std::unordered_map<IDType, GLenum> primitiveMap = {
        PRIMDEF(GL_TRIANGLES_ADJACENCY)
    };


    void Init(HDC hdc) {
        // Set pixel format (add this if not present in your code)
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;

        int pixelFormat = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, pixelFormat, &pfd);

        // Create and make the rendering context current
        HGLRC hglrc = wglCreateContext(hdc);
        wglMakeCurrent(hdc, hglrc);

        InitFunctionPointers();


        CompileShaderFromProperties(IDR("shaders.biquad"));

    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

namespace nugget::gl {
    using namespace identifier;

    HDC hdc;

    void triangle_test();

    Vector4f screenGeom;

    int OpenWindow() {
        // Create a WinAPI window

        // Register the window class
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"MyOpenGLProject";

        RegisterClass(&wc);

        screenGeom =
        {
            (float)gNotice.GetInt64(IDR("app.window.x")),
            (float)gNotice.GetInt64(IDR("app.window.y")),
            (float)gNotice.GetInt64(IDR("app.window.w")),
            (float)gNotice.GetInt64(IDR("app.window.h")),
        };


        // Create the window
        HWND hwnd = CreateWindowEx(0, L"MyOpenGLProject", L"My OpenGL Project", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            (int)screenGeom.z,
            (int)screenGeom.w,
            0, 0, GetModuleHandle(NULL), 0);

        if (!hwnd) {
            std::cerr << "Failed to create window" << std::endl;
            return -1;
        }

        // Show the window
        ShowWindow(hwnd, SW_SHOW);

        SetWindowPos(hwnd, NULL,
            (int)gNotice.GetInt64(ID("app.window.x")),
            (int)gNotice.GetInt64(IDR("app.window.y")),
            0, 0, SWP_NOZORDER | SWP_NOSIZE);

        hdc = GetDC(hwnd);

        nugget::gl::Init(hdc);

        triangle_test();

        std::vector<Notice::Handler> handlers;
        gNotice.RegisterHandlerOnChildren(Notice::Handler(IDR("shaders.biquad"), [](IDType id) {
            CompileShaderFromProperties(IDR("shaders.biquad"));
            }), handlers);

        return 0;
    }

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
        Matrix4f projectionMatrix;
        Matrix4f modelMatrix;
        Matrix4f viewMatrix;
        float fov;
        float nearClip;
        float farClip;
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
                glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, projectionMatrix.GetArray());
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

            Vector3f RV = gNotice.GetVector3f(ID("testobj.section.rotation"));
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

        void UpdateFromPropertyTree(IDType nodeID,int sectionIndex) {
            static int count = 0;
            count++;

            std::vector<float> vertData;
            std::vector<float> uvData;
            std::vector<float> colsData;

            IDType verts = IDR(nodeID, ID("verts"));
            Vector3fList vertices;
            if (gNotice.GetVector3fList(verts, vertices)) {
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
            if (gNotice.GetVector2fList(uvsid, uvs)) {
                for (auto&& z : uvs.data) {
                    uvData.push_back(z.x);
                    uvData.push_back(z.y);
                }
            } else {
                assert(0);
            }
            IDType colsid = IDR(nodeID, ID("colors"));
            ColorList cols;
            if (gNotice.GetColorList(colsid, cols)) {
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
                VBOInfo &vboInfo = VBOs[sectionIndex];
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
                GLuint &texID = textures[sectionIndex];
                if (texID != 0) {
                    glDeleteTextures(1, &texID);
                }
                glGenTextures(1, &texID);

                // texture load
                IDType textureNode = IDR(nodeID, "texture");
                IDType textureName = gNotice.GetID(textureNode);
                const nugget::asset::TextureData& texture = nugget::asset::GetTexture(textureName);
                glBindTexture(GL_TEXTURE_2D, texID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.data);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

#define PROP(a,t1,t2) IDType a##ID = IDR(nodeID,#a);a = (t2)gNotice.Get##t1(a##ID)
            // put it together
            {
                PROP(start, Int64, GLuint);
                PROP(length, Int64, GLsizei);
                PROP(modelMatrix, Matrix4f, Matrix4f);
                PROP(viewMatrix, Matrix4f, Matrix4f);
                PROP(fov, Float, float);
                PROP(nearClip, Float, float);
                PROP(farClip, Float, float);
                PROP(cameraPos, Vector3f, Vector3f);
                PROP(lookAtPos, Vector3f, Vector3f);
                PROP(lookAtUp, Vector3f, Vector3f);

                PROP(projectionMatrix, Matrix4f, Matrix4f);

                Matrix4f::CreateProjectionMatrix(
                    screenGeom.z, 
                    screenGeom.w, 
                    fov, nearClip, farClip, projectionMatrix);

                IDType primitiveID = IDR(nodeID, "primitive");
                primitive = primitiveMap.at(gNotice.GetID(primitiveID));

                IDType shaderNodeHash = IDR(nodeID, "shader");
                IDType usedShaderID = gNotice.GetID(shaderNodeHash);
                std::string usedShaderPath = IDToString(usedShaderID);
                IDType shaderProgramNoticeID = IDR({ usedShaderPath, "_internal", "_pglid" });
                shader = (GLuint)gNotice.GetInt64(shaderProgramNoticeID);

                Init();

#if 0

                gNotice.RegisterHandler(gNotice.Handler(startNoticeID, [this](IDType startid) {
                    renderable.start = (GLint)gNotice.GetInt64(startid);
                    }));
                gNotice.RegisterHandler(gNotice.Handler(lengthNoticeID, [this](IDType lengthid) {
                    renderable.length = (GLint)gNotice.GetInt64(lengthid);
                    }));
                gNotice.RegisterHandler(gNotice.Handler(primNoticeID, [this](IDType primid) {
                    renderable.primitive = primitiveMap.at(gNotice.GetID(primid));
                    }));
#endif
            }
        }

        std::vector<Notice::Handler> regsteredHandlers;

        void SetupFromPropertyTree(IDType nodeID, int sectionIndex) {
            output("-------@@@@@@@------------> {}\n", IDToString(nodeID));
            IDType shaderNoideHash = IDR(nodeID, "shader");
            IDType shaderProgramNoticeID = IDR({ IDToString(gNotice.GetID(shaderNoideHash)), "_internal", "_pglid" });
            UpdateFromPropertyTree(nodeID, sectionIndex);
            UpdateFromPropertyTree(nodeID, sectionIndex);

            gNotice.RegisterHandlerOnChildren(Notice::Handler(nodeID,
                [this, nodeID, sectionIndex](IDType id) {
                UpdateFromPropertyTree(nodeID, sectionIndex);
                }), regsteredHandlers);


            gNotice.RegisterHandler(Notice::Handler(IDR(nodeID, "uvs"),
                [this, nodeID, sectionIndex](IDType uid) {
                UpdateFromPropertyTree(nodeID, sectionIndex);
                }), regsteredHandlers);
            gNotice.RegisterHandler(Notice::Handler(IDR(nodeID, "colors"),
                [this, nodeID, sectionIndex](IDType cid) {
                UpdateFromPropertyTree(nodeID, sectionIndex);
                }), regsteredHandlers);

            gNotice.RegisterHandler(Notice::Handler(shaderProgramNoticeID, [&, shaderProgramNoticeID](IDType id) {
                shader = (GLuint)gNotice.GetInt64(shaderProgramNoticeID);
                }), regsteredHandlers);
                    
        }
    };

    struct Renderable {
        GLuint VAO = 0;
        StableVector<RenderableSection,100> sections;
        
        void Init() {
            if (VAO == 0) {
                glGenVertexArrays(1, &VAO);
            }
        }

        bool AddSections(IDType id) {
            check(VAO, "Ya didn't init, innit?");
            // id is a section in propertytree to populate the instance
            check(gNotice.KeyExists(id), "Node does not exist: {}\n", IDToString(id));
            std::vector<IDType> children;
            gNotice.GetChildrenOfType(id, ValueAny::Type::parent_, children);
            glBindVertexArray(VAO);
            GLuint index = 0;
            for (auto&& x : children) {
                sections.emplace_back();
                auto& section = sections.back();
                section.SetupFromPropertyTree(x,index);
                index++;
            }
            glBindVertexArray(0);
            return true;
        }
    };

    static Renderable renderable;

    void Update() {
        // Initialize OpenGL state
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);

        glBindVertexArray(renderable.VAO);

        for (auto&& x : renderable.sections) {
            x.Render();
        }

        // Swap front and back buffers
        SwapBuffers(hdc);
    }


    void MainLoop(const std::function<void()>& updateCallback) {
        // Main loop
        MSG msg = {};
        while (true) {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT)
                    return;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            Update();
            updateCallback();
        }
    }

    void triangle_test() {
        renderable.Init();
        renderable.AddSections(ID("testobj"));
    }

}

