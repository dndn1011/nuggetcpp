#include <iostream>
#include <Windows.h>
#include "debug.h"
#include "glinternal.h"
#include <functional>
#include "shader.h"
#include "notice.h"
#include "identifier.h"
#include "asset/asset.h"

namespace nugget::gl {
    using namespace nugget::identifier;

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

    int OpenWindow() {
        // Create a WinAPI window

        // Register the window class
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"MyOpenGLProject";

        RegisterClass(&wc);

        // Create the window
        HWND hwnd = CreateWindowEx(0, L"MyOpenGLProject", L"My OpenGL Project", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            (int)Notice::GetInt64(IDR("properties.app.window.w")),
            (int)Notice::GetInt64(IDR("properties.app.window.h")),
            0, 0, GetModuleHandle(NULL), 0);

        if (!hwnd) {
            std::cerr << "Failed to create window" << std::endl;
            return -1;
        }

        // Show the window
        ShowWindow(hwnd, SW_SHOW);

        SetWindowPos(hwnd, NULL,
            (int)Notice::GetInt64(ID("properties.app.window.x")),
            (int)Notice::GetInt64(IDR("properties.app.window.y")),
            0, 0, SWP_NOZORDER | SWP_NOSIZE);

        hdc = GetDC(hwnd);

        nugget::gl::Init(hdc);

        triangle_test();

        return 0;
    }

    struct Renderable {
        GLenum primitive;
        GLuint buffer;
        GLint start;
        GLsizei length;
        GLuint shader;
    };

    Renderable renderable;

    GLuint textureID;

    void Update() {
        // Initialize OpenGL state
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the shader program
        glUseProgram(renderable.shader);


        // Bind the VAO
        glBindVertexArray(renderable.buffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        auto loc = glGetUniformLocation(renderable.shader, "quadTexture");
        glUniform1i(loc , 0);



        // Draw the triangle
        glDrawArrays(GL_TRIANGLES_ADJACENCY,
            renderable.start,
            renderable.length
        );

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            int a = 0;
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

    // Vertex Buffer Object (VBO)
    float s = 1.0f;
#if 0
    float vertices[] = {
         s,  s, 0.0f,     // TR
         s,  -s, 0.0f,    // BR
        -s,  s, 0.0f,     // TL
        -s, -s, 0.0f,     // BL

         0,  0, 0.0f,
         0,  0, 0.0f,
    };
#endif
    float colours[] = {
        //             1,0,0,1,  // tr
        //             0.5f,0,0,1,  // br
        //             0.3f,0,0,1,  // tl
        //             0.7f,0,0,1,  // bl
                     1,0,0,1,
                     0,1,0,1,
                     0,0,1,1,
                     1,1,0,1,

                     0.5,0,0,1,  // tl
                     0,0.5,0,1,  // br
    };
    float uvCoords[] = {
        0.0f, 1.0f,  // tr
        1.0f, 1.0f,  // br
        0.0f, 0.0f,  // tl
        1.0f, 0.0f,   // bl

        1.0f, 0.0f,  // tl
        0.0f, 1.0f,  // br

        0.0f, 1.0f,  // tr
        1.0f, 1.0f,  // br
        0.0f, 0.0f,  // tl
        1.0f, 0.0f,   // bl

        1.0f, 0.0f,  // tl
        0.0f, 1.0f,  // br

        0.0f, 1.0f,  // tr
        1.0f, 1.0f,  // br
        0.0f, 0.0f,  // tl
        1.0f, 0.0f,   // bl

        1.0f, 0.0f,  // tl
        0.0f, 1.0f,  // br
    };

    void ApplyRenderingData() {
        std::vector<IDType> children;
        Notice::GetChildrenOfType(IDR("properties.render.scene"), ValueAny::Type::IDType, children);
        std::vector<float> vertData;
        std::vector<float> uvData;
        std::vector<float> colsData;
        for (auto&& x : children) {         // each object
            IDType id = Notice::GetID(x);   // points to the data
            std::vector<IDType> childSections;
            Notice::GetChildrenWithNodeExisting(id, ID("verts"), childSections);
            for (auto&& y : childSections) {   // each section
                IDType verts = IDR(y, ID("verts"));
                Vector3fList vertices;
                if (Notice::GetVector3fList(verts, vertices)) {
                    for (auto&& z : vertices.data) {
                        vertData.push_back(z.x);
                        vertData.push_back(z.y);
                        vertData.push_back(z.z);
                    }
                } else {
                    assert(0);
                }
                IDType uvsid = IDR(y, ID("uvs"));
                Vector2fList uvs;
                if (Notice::GetVector2fList(uvsid, uvs)) {
                    for (auto&& z : uvs.data) {
                        uvData.push_back(z.x);
                        uvData.push_back(z.y);
                    }
                } else {
                    assert(0);
                }
                IDType colsid = IDR(y, ID("colors"));
                ColorList cols;
                if (Notice::GetColorList(colsid, cols)) {
                    for (auto&& z : cols.data) {
                        colsData.push_back(z.r);
                        colsData.push_back(z.g);
                        colsData.push_back(z.b);
                        colsData.push_back(z.a);
                    }
                } else {
                    assert(0);
                }
            }
        }

        auto vsize = vertData.size();
        auto usize = uvData.size();
        auto csize = colsData.size();

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vsize, vertData.data());
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * vsize, sizeof(float) * usize, uvData.data());
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * vsize + sizeof(float) * usize, sizeof(float)*csize, colsData.data());

        // Set up vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Set the attribute pointers for the UV coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(float) * vsize));
        glEnableVertexAttribArray(1);

        // Set the attribute pointers for the vertex colours
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)((sizeof(float) * vsize) + sizeof(float)*usize));
        glEnableVertexAttribArray(2);

    }

    void triangle_test() {
        GLuint VAO;

        IDType asset = Notice::GetID(ID("properties.testobj.section.texture"));
        const nugget::asset::TextureData &texture = nugget::asset::GetTexture(asset);
            


        // Vertex Array Object (VAO)
//        glGenVertexArrays(1, &VAO);
//        glBindVertexArray(VAO);
    

        // Create Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
        GLuint VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // Bind VAO
        glBindVertexArray(VAO);

        {
            // Bind VBO, set the vertices and UV coordinates data
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 1200 + sizeof(uvCoords) + sizeof(colours), nullptr, GL_STATIC_DRAW);
            ApplyRenderingData();
        }

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.data);
        glBindTexture(GL_TEXTURE_2D, 0);

        std::unordered_map<IDType, GLenum> primitiveMap = {
            { ID("GL_TRIANGLES_ADJACENCY"),GL_TRIANGLES_ADJACENCY }
        };

        std::vector<Notice::Handler> handlers;
        Notice::RegisterHandlerOnChildren(Notice::Handler(IDR("properties.shaders.biquad"), [](IDType id) {
            CompileShaderFromProperties(IDR("properties.shaders.biquad"));
            }), handlers);

        IDType vid = IDR("properties.testobj.section.verts");
        IDType uid = IDR("properties.testobj.section.uvs");
        IDType cid = IDR("properties.testobj.section.colors");
        IDType startid = IDR("properties.testobj.section.start");
        IDType lengthid = IDR("properties.testobj.section.length");
        IDType primid = IDR("properties.testobj.section.primitive");
        IDType shaderid = IDR("properties.testobj.section.shader");

        Notice::RegisterHandler(Notice::Handler(vid, [](IDType vid) {
            ApplyRenderingData();
            }));
        Notice::RegisterHandler(Notice::Handler(uid, [](IDType uid) {
            ApplyRenderingData();
            }));
        Notice::RegisterHandler(Notice::Handler(cid, [](IDType cid) {
            ApplyRenderingData();
            }));
        Notice::RegisterHandler(Notice::Handler(startid, [](IDType startid) {
            renderable.start = (GLint)Notice::GetInt64(startid);
            }));
        Notice::RegisterHandler(Notice::Handler(lengthid, [](IDType lengthid) {
            renderable.length = (GLint)Notice::GetInt64(lengthid);
            }));
        Notice::RegisterHandler(Notice::Handler(primid, [&](IDType primid) {
            renderable.primitive = primitiveMap.at(Notice::GetID(primid));
            }));

        CompileShaderFromProperties(IDR("properties.shaders.biquad"));

        renderable.buffer = VAO;
        renderable.length = (GLsizei)Notice::GetInt64(lengthid);
        renderable.start = (GLint)Notice::GetInt64(startid);
        renderable.primitive = primitiveMap.at(Notice::GetID(primid));
        renderable.shader = (GLuint) Notice::GetInt64(IDR(Notice::GetID(shaderid), IDR("program", IDR("glid"))));

        // Enable backface culling
//        glEnable(GL_CULL_FACE);

        // Specify which faces to cull
//        glCullFace(GL_BACK);


    }
}