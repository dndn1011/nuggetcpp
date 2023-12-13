#include <iostream>
#include <Windows.h>
#include "debug.h"
#include "glinternal.h"

// Function pointer typedef for OpenGL functions
typedef void (APIENTRY* GLPROC)();

// Function to get the address of an OpenGL function
template<typename T>
T GetGLFunctionAddress(const char* functionName) {
    T result = reinterpret_cast<T>(wglGetProcAddress(functionName));
    if (!result) {
        check(0, "Failed to get address of {}\n", functionName);
    }
    return result;
}


namespace nugget::gl {

    // define type
    //example: typedef void(APIENTRY* GLFUNC_Clear)(GLbitfield);
#define GLDEF(a,b,c) typedef c(APIENTRY* GLFUNC_##a) b;
#include "gldefs.h"
#undef GLDEF

    // define pointers
    // example: GLFUNC_Clear Clear;
#define GLDEF(a,b,c) GLFUNC_##a a;
#include "gldefs.h"
#undef GLDEF

    // initialisation
    void InitFunctionPointers() {
        // example: Clear = GetGLFunctionAddress<GLFUNC_Clear>("gl" "Clear")
#define GLDEF(a,b,c) a = GetGLFunctionAddress<GLFUNC_##a>(#a);
#include "gldefs.h"
#undef GLDEF
    }

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

    GLuint shaderProgram;
    GLuint VAO;

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


        // Get device context
        HDC hdc = GetDC(hwnd);

        nugget::gl::Init(hdc);

        triangle_test();

        // Main loop
        MSG msg = {};
        while (true) {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT)
                    return 0;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // Initialize OpenGL state
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);


            // Use the shader program
            glUseProgram(shaderProgram);

            // Bind the VAO
            glBindVertexArray(VAO);

            // Draw the triangle
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Swap front and back buffers
            SwapBuffers(hdc);
        }

        return 0;
    }

    // Function to compile a shader and check for compilation errors
    GLuint compileShader(const std::string& shaderSource, GLenum shaderType) {
        GLuint shader = glCreateShader(shaderType);
        const GLchar* source = shaderSource.c_str();
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "Shader compilation failed: " << infoLog << std::endl;
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) {
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "Program linking failed: " << infoLog << std::endl;

            // Clean up resources (delete shaders and program when no longer needed)
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(program);

            return 0;
        }

        // Detach shaders after a successful link
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);

        return program;
    }

    GLuint linkProgram(GLuint geometryShader, GLuint vertexShader, GLuint fragmentShader) {
        GLuint program = glCreateProgram();
        if (geometryShader) {
            glAttachShader(program, geometryShader);
        }
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "Program linking failed: " << infoLog << std::endl;

            // Clean up resources (delete shaders and program when no longer needed)
            if (geometryShader) {
                glDeleteShader(geometryShader);
            }
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(program);

            return 0;
        }

        // Detach shaders after a successful link
        if (geometryShader) {
            glDetachShader(program, geometryShader);
        }
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);

        return program;
    }

    void triangle_test() {
        const char* vertexShaderSource = R"(
            #version 450 core
            out vec2 uv_g;
            out vec4 col_g;
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec2 aTexCoord;
            layout (location = 2) in vec4 aColour;
            void main()
            {
                uv_g = aTexCoord;
                col_g = aColour;
                gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
            }
        )";

        const char* geometryShaderSource = R"(
        #version 450 core

        layout (triangles) in;
        layout (triangle_strip, max_vertices = 3) out;

        in vec2 uv_g[3];
        out vec2 uv_f;
        in vec4 col_g[3];
        out vec4 col_f;

        void main() {
            for (int i = 0; i < 3; i++) {
                gl_Position = gl_in[i].gl_Position;
                col_f = col_g[i];
                uv_f = uv_g[i];
                EmitVertex();
            }
            EndPrimitive();
}
)";


        const char* fragmentShaderSource = R"(
            #version 450 core
            in vec2 uv_f;
            in vec4 col_f;
            out vec4 FragColor;
            void main()
            {
//                  FragColor = vec4(1,1,0,1);
                    FragColor = col_f;
//               FragColor = vec4(uv_f.x, uv_f.y, 0.0, 1.0);
//                  FragColor = vec4(uv_f.x, 1.0, 0.0, 1.0);
            }
        )";


        // Vertex Array Object (VAO)
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    
        // Vertex Buffer Object (VBO)
        float s = 0.5f;
        float vertices[] = {
            -s,  s, 0.0f,
             s,  -s, 0.0f,
             s,  s, 0.0f,

            -s,  s, 0.0f,
            -s, -s, 0.0f,
             s, -s, 0.0f,
        };
        float colours[] = {
             0.5,0,0,1,  // tl
             0,0.5,0,1,  // br
             0,0,0,1,  // tr

             0.5,0,0,1,  // tl
             1,1,1,1,  // bl
             0,0.5,0,1,  // br
        };
        float uvCoords[] = {
            1.0f, 0.0f,  // tl
            0.0f, 1.0f,  // br
            0.0f, 1.0f,  // tr

            1.0f, 0.0f,  // tl
            0.0f, 1.0f,  // br
            0.0f, 1.0f,   // bl
        };
        // Create Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
        GLuint VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // Bind VAO
        glBindVertexArray(VAO);

        // Bind VBO, set the vertices and UV coordinates data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(uvCoords)+sizeof(colours), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(uvCoords), uvCoords);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices)+ sizeof(uvCoords), sizeof(colours), colours);

        // Set up vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Set the attribute pointers for the UV coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)sizeof(vertices));
        glEnableVertexAttribArray(1);

        // Set the attribute pointers for the vertex colours
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(vertices) + sizeof(uvCoords)));
        glEnableVertexAttribArray(2);


        // Geometry Shader
        GLuint geometryShader = {};
        geometryShader = compileShader(geometryShaderSource, GL_GEOMETRY_SHADER);
        check(geometryShader != 0, ("vertex shader compilation failed\n"));

        // Vertex Shader
        GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
        check(vertexShader != 0, ("vertex shader compilation failed\n"));

        // Fragment Shader
        GLuint fragmentShader = compileShader(fragmentShaderSource,GL_FRAGMENT_SHADER);
        check(fragmentShader != 0, ("fragment shader compilation failed\n"));

        // Shader Program
        shaderProgram = linkProgram(geometryShader, vertexShader, fragmentShader);
        check(shaderProgram != 0, ("fragment shader compilation failed\n"));

        // Enable backface culling
//        glEnable(GL_CULL_FACE);

        // Specify which faces to cull
//        glCullFace(GL_BACK);


    }
}