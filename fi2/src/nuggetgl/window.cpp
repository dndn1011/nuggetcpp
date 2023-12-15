#include <iostream>
#include <Windows.h>
#include "debug.h"
#include "glinternal.h"
#include <functional>
#include "shader.h"
#include "notice.h"
#include "identifier.h"

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

    GLuint VAO;
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
            CW_USEDEFAULT, CW_USEDEFAULT, 850, 520, 0, 0, GetModuleHandle(NULL), 0);

        if (!hwnd) {
            std::cerr << "Failed to create window" << std::endl;
            return -1;
        }

        // Show the window
        ShowWindow(hwnd, SW_SHOW);

        SetWindowPos(hwnd, NULL, -1649, -211, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        hdc = GetDC(hwnd);

        nugget::gl::Init(hdc);

        triangle_test();

        return 0;
    }

    void Update() {
        // Initialize OpenGL state
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        // Use the shader program
        glUseProgram((GLuint)Notice::GetInt64(IDR("properties.shaders.biquad.program.glid")));

        // Bind the VAO
        glBindVertexArray(VAO);

        // Draw the triangle
        glDrawArrays(GL_TRIANGLES_ADJACENCY,
            (GLsizei)Notice::GetInt64(IDR("properties.test.A")),
            (GLsizei)Notice::GetInt64(IDR("properties.test.B")));

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
    };

    void ApplyRenderingData() {
        auto vertices = Notice::GetVector3fList(IDR("properties.render.object.section.verts"));
        size_t vsize = vertices.data.size()*3;
        std::vector<float> verts;
        for (auto&& x : vertices.data) {
            verts.push_back(x.x);
            verts.push_back(x.y);
            verts.push_back(x.z);
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*vsize, verts.data());
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * vsize, sizeof(uvCoords), uvCoords);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * vsize + sizeof(uvCoords), sizeof(colours), colours);

        // Set up vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Set the attribute pointers for the UV coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(float) * vsize));
        glEnableVertexAttribArray(1);

        // Set the attribute pointers for the vertex colours
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)((sizeof(float) * vsize) + sizeof(uvCoords)));
        glEnableVertexAttribArray(2);

    }

    void triangle_test() {

        // Vertex Array Object (VAO)
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    

        // Create Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
        GLuint VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // Bind VAO
        glBindVertexArray(VAO);

        // Bind VBO, set the vertices and UV coordinates data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*1200 + sizeof(uvCoords)+sizeof(colours), nullptr, GL_STATIC_DRAW);
        ApplyRenderingData();

        std::vector<Notice::Handler> handlers;
        Notice::RegisterHandlerOnChildren(Notice::Handler(IDR("properties.shaders.biquad"), [](IDType id) {
            CompileShaderFromProperties(IDR("properties.shaders.biquad"));
            }), handlers);

        Notice::RegisterHandler(Notice::Handler(IDR("properties.render.object.section.verts"), [](IDType id) {
            ApplyRenderingData();
            }));

        CompileShaderFromProperties(IDR("properties.shaders.biquad"));


        // Enable backface culling
//        glEnable(GL_CULL_FACE);

        // Specify which faces to cull
//        glCullFace(GL_BACK);


    }
}