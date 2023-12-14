#include "glinternal.h"
#include "debug.h"
#include "shader.h"
#include "Notice.h"
#include "identifier.h"

using namespace nugget::identifier;
namespace nugget::gl {

    // Function to compile a shader and check for compilation errors
    GLuint CompileShader(const std::string& shaderSource, GLenum shaderType) {
        GLuint shader = glCreateShader(shaderType);
        const GLchar* source = shaderSource.c_str();
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
            outputAlways("Shader compilation failed: {}\n", infoLog);
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
            outputAlways("Program linking failed: {}", infoLog);

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

    GLuint LinkProgram(GLuint geometryShader, GLuint vertexShader, GLuint fragmentShader) {
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
            outputAlways("Program linking failed: {}\n",infoLog);

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

    void CompileShaderFromProperties(IDType node) {
        IDType vertexNode = IDR(node, ID("vertex"));
        IDType geometryNode = IDR(node, ID("geometry"));
        IDType fragmentNode = IDR(node, ID("fragment"));
        IDType programNode = IDR(node, IDR("program"));

        IDType vertexIdNode = IDR(vertexNode, IDR("glid"));
        IDType geometryIdNode = IDR(geometryNode, IDR("glid"));
        IDType fragmentIdNode = IDR(fragmentNode, IDR("glid"));
        IDType programIdNode = IDR(programNode, IDR("glid"));
        // e.g. properties.shaders.foo.program.glid

        if (Notice::KeyExists(programIdNode)) {
            if (auto id = Notice::GetInt64(programIdNode) != 0) {
                glDeleteProgram(id);
            }
        }
        if (Notice::KeyExists(vertexIdNode)) {
            if (auto id = Notice::GetInt64(vertexIdNode) != 0) {
                glDeleteShader(id);
            }
        }
        if (Notice::KeyExists(geometryIdNode)) {
            if (auto id = Notice::GetInt64(geometryIdNode) != 0) {
                glDeleteShader(id);
            }

        }
        if (Notice::KeyExists(fragmentIdNode)) {
            if (auto id = Notice::GetInt64(fragmentIdNode) != 0) {
                glDeleteShader(id);
            }
        }

        std::string vertexShaderSource = Notice::GetString(vertexNode);
        std::string geometryShaderSource = Notice::GetString(geometryNode);
        std::string fragmentShaderSource = Notice::GetString(fragmentNode);

        // Geometry Shader
        GLuint geometryShader = {};
        geometryShader = CompileShader(geometryShaderSource, GL_GEOMETRY_SHADER);
        testAlways(geometryShader != 0, ("vertex shader compilation failed\n"));

        // Vertex Shader
        GLuint vertexShader = CompileShader(vertexShaderSource, GL_VERTEX_SHADER);
        testAlways(vertexShader != 0, ("vertex shader compilation failed\n"));

        // Fragment Shader
        GLuint fragmentShader = CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
        testAlways(fragmentShader != 0, ("fragment shader compilation failed\n"));

        // Shader Program
        GLuint shaderProgram = LinkProgram(geometryShader, vertexShader, fragmentShader);
        testAlways(shaderProgram != 0, ("fragment shader compilation failed\n"));

        Notice::Set(vertexIdNode, (int64_t)vertexShader);
        Notice::Set(geometryIdNode, (int64_t)geometryShader);
        Notice::Set(fragmentIdNode, (int64_t)fragmentShader);
        Notice::Set(programIdNode, (int64_t)shaderProgram);

    }
     
}