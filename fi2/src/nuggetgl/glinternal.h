#pragma once
#include <windows.h>
#include <gl/GL.h>
#include "debug.h"

typedef ptrdiff_t GLsizeiptr;
typedef intptr_t GLintptr;
typedef char GLchar;

#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872

#define GL_TEXTURE0 0x84C0

#define GL_GEOMETRY_SHADER 0x8DD9

#define GL_TRIANGLES_ADJACENCY 0x000C
#define GL_ELEMENT_ARRAY_BUFFER 0x8893

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001


namespace nugget::gl {
	// Function pointer typedef for OpenGL functions
	typedef void (APIENTRY* GLPROC)();

	// Function to get the address of an OpenGL function
	template<typename T>
	T GetGLFunctionAddress(const char* functionName) {
		auto addr = wglGetProcAddress(functionName);
		T result = reinterpret_cast<T>(addr);
		if (!result) {
			check(0, "Failed to get address of {}\n", functionName);
		}
		return result;
	}


	// define type
	//example: typedef void(APIENTRY* GLFUNC_Clear)(GLbitfield);
#define GLDEF(a,b,c) typedef c(APIENTRY* GLFUNC_##a) b;
#include "gldefs.h"
#undef GLDEF

	// define pointers
	// example: GLFUNC_Clear Clear;
#define GLDEF(a,b,c) extern GLFUNC_##a a;
#include "gldefs.h"
#undef GLDEF

	void InitFunctionPointers();

}
