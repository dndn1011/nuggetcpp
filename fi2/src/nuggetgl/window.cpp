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
#include "system.h"
#include "render.h"

namespace nugget::gl {
    using namespace nugget::identifier;
    using namespace nugget::properties;

    Vector4f screenGeom;


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

        int gl33_attribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0,
        };
         
        
        HGLRC hglrcTemp = wglCreateContext(hdc);
        wglMakeCurrent(hdc, hglrcTemp);
        InitFunctionPointers();


        HGLRC hglrc = wglCreateContextAttribsARB(hdc, nullptr, gl33_attribs);
        wglMakeCurrent(hdc, hglrc);

        wglDeleteContext(hglrcTemp);


        CompileShaderFromProperties(IDR("shaders.simple"));

        GLCameraSetProjectionFromProperties(ID("testobj.section"));
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
            (float)gProps.GetInt64(IDR("app.window.x")),
            (float)gProps.GetInt64(IDR("app.window.y")),
            (float)gProps.GetInt64(IDR("app.window.w")),
            (float)gProps.GetInt64(IDR("app.window.h")),
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
            (int)gProps.GetInt64(ID("app.window.x")),
            (int)gProps.GetInt64(IDR("app.window.y")),
            0, 0, SWP_NOZORDER | SWP_NOSIZE);

        hdc = GetDC(hwnd);

        nugget::gl::Init(hdc);

        std::vector<Notice::Handler> handlers;
        gProps.RegisterHandlerOnChildren(Notice::Handler(IDR("shaders.simple"), [](IDType id) {
            CompileShaderFromProperties(IDR("shaders.simple"));
            }), handlers);

        return 0;
    }
    
    static int count = 0;
    void UpdateBegin() {
        // Initialize OpenGL state
#if 0
        if (++count % 100 < 50) {
            glClearColor(0.2f, 0.4f, 0.0f, 1.0f);
        } else {
            glClearColor(0.0f, 0.4f, 0.0f, 1.0f);
        }
#else
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#endif
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SetAllUniforms("projectionMatrix", GLCameraProjectionMatrix());

    }

    void UpdateEnd() {
        // Swap front and back buffers
        SwapBuffers(hdc);
    }

#if 0
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
#endif

    static size_t init_dummy[] =
    {
        {
            nugget::system::RegisterModule([]() {
            if (!Notice::gBoard.KeyExists(ID("commandLine.assignments.runHeadless"))) {
                OpenWindow();
            }
            return 0;
           }, 199,identifier::ID("init"),__FILE__)
        },
        {
            // frame begin
            nugget::system::RegisterModule([]() {
            if (!Notice::gBoard.KeyExists(ID("commandLine.assignments.runHeadless"))) {
                UpdateBegin();
            }
            return 0;
            }, 199, ID("update"))
        },
        {
            // frame end
            nugget::system::RegisterModule([]() {
            if (!Notice::gBoard.KeyExists(ID("commandLine.assignments.runHeadless"))) {
                UpdateEnd();
            }
            return 0;
            }, 220, ID("update"))
        }
    };
}

