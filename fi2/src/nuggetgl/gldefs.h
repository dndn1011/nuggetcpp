
GLDEF(glGenerateMipmap, (GLenum target), void)
GLDEF(glGenVertexArrays, (GLsizei n, GLuint* arrays), void)
GLDEF(glBindVertexArray, (GLuint array), void)
GLDEF(glGenBuffers, (GLsizei n, GLuint* buffers), void)
GLDEF(glBindBuffer, (GLenum target, GLuint buffer), void)
GLDEF(glBufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage), void)
GLDEF(glCreateShader, (GLenum type), GLuint)
GLDEF(glShaderSource, (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length), void)
GLDEF(glCompileShader, (GLuint shader), void)
GLDEF(glCreateProgram, (void), GLuint)
GLDEF(glAttachShader, (GLuint program, GLuint shader), void)
GLDEF(glLinkProgram, (GLuint program), void)
GLDEF(glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer), void)
GLDEF(glEnableVertexAttribArray, (GLuint index), void)
GLDEF(glUseProgram, (GLuint program), void)
GLDEF(glGetShaderiv, (GLuint shader, GLenum pname, GLint* params), void)
GLDEF(glGetShaderInfoLog, (GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog), void)
GLDEF(glDeleteShader, (GLuint shader), void)
GLDEF(glGetProgramiv, (GLuint program, GLenum pname, GLint* params), void)
GLDEF(glGetProgramInfoLog, (GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog), void)
GLDEF(glDeleteProgram, (GLuint program), void)
GLDEF(glDetachShader, (GLuint program, GLuint shader), void)
GLDEF(glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void* data), void)



