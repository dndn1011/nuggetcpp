#include "glinternal.h"
#include "debug.h"
#include "shader.h"
#include "Notice.h"
#include "identifier.h"
#include <format>
#include "propertytree.h"

using namespace nugget::identifier;
namespace nugget::gl {
    using namespace properties;
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

        std::string nodeStr = IDToString(node);
        gProps.SetAsParent(IDR({ nodeStr, "_internal" }));
        IDType vertexIdNode = IDR({ nodeStr, "_internal","_vglid" });
        IDType geometryIdNode = IDR({ nodeStr, "_internal","_gglid" });
        IDType fragmentIdNode = IDR({ nodeStr, "_internal","_fglid" });
        IDType programIdNode = IDR({ nodeStr, "_internal","_pglid" });

        // e.g. shaders.foo.program.glid

        if (gProps.KeyExists(programIdNode)) {
            auto id = gProps.GetUint64(programIdNode);
            if (id) {
                glDeleteProgram((GLuint)id);
            }
        }
        if (gProps.KeyExists(vertexIdNode)) {
            auto id = gProps.GetUint64(vertexIdNode);
            if (id) {
                glDeleteShader((GLuint)id);
            }
        }
        if (gProps.KeyExists(geometryIdNode)) {
            auto id = gProps.GetUint64(geometryIdNode);
            if (id) {
                glDeleteShader((GLuint)id);
            }

        }
        if (gProps.KeyExists(fragmentIdNode)) {
            auto id = gProps.GetUint64(fragmentIdNode);
            if (id) {
                glDeleteShader((GLuint)id);
            }
        }

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

        gProps.Set(vertexIdNode, (int64_t)vertexShader);
        gProps.Set(geometryIdNode, (int64_t)geometryShader);
        gProps.Set(fragmentIdNode, (int64_t)fragmentShader);
        gProps.Set(programIdNode, (int64_t)shaderProgram);

    }
     
}