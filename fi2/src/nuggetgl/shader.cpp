#include "glinternal.h"
#include "debug.h"
#include "shader.h"
#include "Notice.h"
#include "identifier.h"
#include <format>
#include "propertytree.h"

namespace nugget::gl {
    using namespace nugget::identifier;
    using namespace properties;

    struct ShaderProgramInfo {
        IDType node = IDType::null;
        GLuint vertexShader = 0;
        GLuint geometryShader = 0;
        GLuint fragmentShader = 0;
        GLuint shaderProgram = 0;
    };
    std::unordered_map<IDType, ShaderProgramInfo> shaderMap;

    GLuint GetShaderHandle(IDType node) {
        return shaderMap[node].shaderProgram;
    }

    void FreeShader(IDType node) {
        ShaderProgramInfo info = shaderMap.at(node);
        glDeleteShader(info.vertexShader);
        glDeleteShader(info.geometryShader);
        glDeleteShader(info.fragmentShader);
        glDeleteShader(info.shaderProgram);
    }

    void SetAllUniforms(const char* name, const float* matrix) {
        for (auto&& x : shaderMap) {
            GLint projMatLocation = glGetUniformLocation(x.second.shaderProgram, "projectionMatrix");
            if (projMatLocation >= 0) {
                glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, matrix);
            }
        }
    }

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

        if (shaderMap.contains(node)) {
            FreeShader(node);
        }

        IDType vertexNode = IDR(node, ID("vertex"));
        IDType geometryNode = IDR(node, ID("geometry"));
        IDType fragmentNode = IDR(node, ID("fragment"));
        IDType programNode = IDR(node, IDR("program"));

        std::string vertexShaderSource = gProps.GetString(vertexNode);
        std::string geometryShaderSource = gProps.GetString(geometryNode);
        std::string fragmentShaderSource = gProps.GetString(fragmentNode);
            
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

        ShaderProgramInfo &info = shaderMap[node];

        check(info.node == IDType::null, "What are we supposed to do here?");

        info.vertexShader = vertexShader;
        info.geometryShader = geometryShader;
        info.fragmentShader = fragmentShader;
        info.shaderProgram = shaderProgram;

        IDType notice = IDR("nugget.gl.shaders", node);
        if (Notice::gBoard.KeyExists(notice)) {
            Notice::gBoard.Set(notice, Notice::gBoard.GetInt32(notice) + 1);
        } else {
            Notice::gBoard.Set(notice, 0);
        }

    }
     
}