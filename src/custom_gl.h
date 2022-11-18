#pragma once

extern "C"
{
#define APIENTRYP APIENTRY *
#define GLAPI extern
	
#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_SWAP_METHOD_ARB               0x2007
#define WGL_SWAP_COPY_ARB                 0x2029
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_ACCELERATION_ARB              0x2003
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_ALPHA_BITS_ARB                0x201B
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_SAMPLES_ARB                   0x2042
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x00000001
#define WGL_SAMPLE_BUFFERS_ARB            0x2041
	
#define GL_TRUE                           1
#define GL_ARRAY_BUFFER                   0x8892
#define GL_SHADER_STORAGE_BUFFER          0x90D2
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_FALSE                          0
#define GL_FLOAT                          0x1406
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_LINK_STATUS                    0x8B82
#define GL_TRIANGLES                      0x0004
#define GL_STATIC_DRAW                    0x88E4
#define GL_FRAMEBUFFER                    0x8D40
#define GL_R8                             0x8229
#define GL_TEXTURE_2D                     0x0DE1
#define GL_RGBA16F                        0x881A
#define GL_RGBA                           0x1908
#define GL_HALF_FLOAT                     0x140B
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_REPEAT                         0x2901
#define GL_LINEAR                         0x2601
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_DEPTH_COMPONENT                0x1902
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_RED                            0x1903
#define GL_CLAMP_TO_BORDER                0x812D
#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_NONE                           0
#define GL_DEPTH_TEST                     0x0B71
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_SCISSOR_TEST                   0x0C11
#define GL_RGBA8                          0x8058
#define GL_LESS                           0x0201
#define GL_BLEND                          0x0BE2
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_FUNC_ADD                       0x8006
#define GL_COLOR                          0x1800
#define GL_ALWAYS                         0x0207
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_OUTPUT                   0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_UNSIGNED_INT                   0x1405
#define GL_MULTISAMPLE                    0x809D
#define GL_CULL_FACE                      0x0B44
#define GL_CW                             0x0900
#define GL_CCW                            0x0901
#define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
#define GL_DEBUG_TYPE_ERROR               0x824C
	
	typedef          float         khronos_float_t;
	typedef signed   long long int khronos_ssize_t;
	typedef unsigned long long int khronos_usize_t;
	typedef signed   long long int khronos_intptr_t;
	typedef unsigned long long int khronos_uintptr_t;
	
	typedef unsigned int GLuint;
	typedef void GLvoid;
	typedef unsigned int GLenum;
	typedef int GLint;
	typedef int GLsizei;
	typedef char GLchar;
	typedef khronos_float_t GLfloat;
	typedef unsigned char GLboolean;
	typedef khronos_ssize_t GLsizeiptr;
	typedef khronos_intptr_t GLintptr;
	typedef unsigned int GLbitfield;
	
	typedef BOOL (WINAPI* PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
	typedef HGLRC (WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
	typedef void (APIENTRY* GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
	
	GLAPI void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures);
	GLAPI void APIENTRY glGenTextures (GLsizei n, GLuint *textures);
	GLAPI void APIENTRY glBindTexture (GLenum target, GLuint texture);
	GLAPI void APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
	GLAPI void APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param);
	GLAPI void APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
	GLAPI void APIENTRY glDrawBuffer (GLenum buf);
	GLAPI void APIENTRY glClear (GLbitfield mask);
	GLAPI void APIENTRY glClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	GLAPI void APIENTRY glReadBuffer (GLenum src);
	GLAPI void APIENTRY glDepthMask (GLboolean flag);
	GLAPI void APIENTRY glDisable (GLenum cap);
	GLAPI void APIENTRY glEnable (GLenum cap);
	GLAPI void APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
	GLAPI void APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height);
	GLAPI void APIENTRY glDepthFunc (GLenum func);
	GLAPI void APIENTRY glCullFace (GLenum mode);
	GLAPI void APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor);
	GLAPI void APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count);
	GLAPI void APIENTRY glFrontFace (GLenum mode);
	//GLAPI void APIENTRY glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
	
}

internal PROC get_gl_proc(char* proc_name)
{
	PROC proc = wglGetProcAddress(proc_name);
	if(!proc)
	{
		platform_print_error();
		ExitProcess(1);
	}
	return proc;
}
#define make_gl_func(func_name, return_type, ...) \
typedef return_type(APIENTRYP func_name##_proc)(__VA_ARGS__); \
global_variable func_name##_proc func_name = 0;

#define init_gl_func(func_name) \
func_name = (func_name##_proc)get_gl_proc(#func_name); \
CAKEZ_ASSERT(func_name, "Failed to get OpenGL Function")

// Grab all the functions needed to render in OpenGL
#ifdef DEBUG
make_gl_func(glDebugMessageCallback, void, GLDEBUGPROC callback, void* userParam);
#endif
make_gl_func(glCreateShader, GLuint, GLenum type)
make_gl_func(glGetUniformLocation, GLint, GLuint program, const GLchar* name)
make_gl_func(glUniform1f, void, GLint location, GLfloat v0)
make_gl_func(glUniform2fv, void, GLint location, GLsizei count, const GLfloat *value);
make_gl_func(glUniform3fv, void, GLint location, GLsizei count, const GLfloat *value);
make_gl_func(glUniform1i, void, GLint location, GLint v0);
make_gl_func(glUniformMatrix4fv, void, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
make_gl_func(glVertexAttribDivisor, void, GLuint index, GLuint divisor)
make_gl_func(glActiveTexture, void, GLenum texture)
make_gl_func(glBufferSubData, void, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data)
make_gl_func(glDrawArraysInstanced, void, GLenum mode, GLint first, GLsizei count, GLsizei primcount)
make_gl_func(glBindFramebuffer, void, GLenum target, GLuint framebuffer)
make_gl_func(glCheckFramebufferStatus, GLenum, GLenum target)
make_gl_func(glGenFramebuffers, void, GLsizei n, GLuint *ids)
make_gl_func(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
make_gl_func(glDrawBuffers, void, GLsizei n, const GLenum *bufs)
make_gl_func(glDeleteFramebuffers, void, 	GLsizei n, GLuint *framebuffers)
make_gl_func(glBlendFunci, void,	GLuint buf, GLenum sfactor, GLenum dfactor)
make_gl_func(glBlendEquation, void, GLenum mode)
make_gl_func(glClearBufferfv, void, GLenum buffer, GLint drawbuffer, const GLfloat* value)
make_gl_func(glShaderSource, void, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
make_gl_func(glCompileShader, void, GLuint shader);
make_gl_func(glGetShaderiv, void, GLuint shader, GLenum pname, GLint *params);
make_gl_func(glGetShaderInfoLog, void, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
make_gl_func(glCreateProgram, GLuint, void);
make_gl_func(glAttachShader, void, GLuint program, GLuint shader);
make_gl_func(glLinkProgram, void, GLuint program);
make_gl_func(glValidateProgram, void, GLuint program);
make_gl_func(glGetProgramiv, void, GLuint program, GLenum pname, GLint *params);
make_gl_func(glGetProgramInfoLog, void, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
make_gl_func(glGenBuffers, void, GLsizei n, GLuint *buffers);
make_gl_func(glGenVertexArrays, void, GLsizei n, GLuint *arrays);
make_gl_func(glGetAttribLocation, GLint, GLuint program, const GLchar *name);
make_gl_func(glBindVertexArray, void, GLuint array);
make_gl_func(glEnableVertexAttribArray, void, GLuint index);
make_gl_func(glVertexAttribPointer, void, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
make_gl_func(glBindBuffer, void, GLenum target, GLuint buffer);
make_gl_func(glBindBufferBase, void, GLenum target, GLuint index, GLuint buffer);
make_gl_func(glBufferData, void, GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
make_gl_func(glGetVertexAttribPointerv, void, GLuint index, GLenum pname, GLvoid **pointer);
make_gl_func(glUseProgram, void, GLuint program);
make_gl_func(glDeleteVertexArrays, void, GLsizei n, const GLuint *arrays);
make_gl_func(glDeleteBuffers, void, GLsizei n, const GLuint *buffers);
make_gl_func(glDeleteProgram, void, GLuint program);
make_gl_func(glDeleteShader, void, GLuint shader);
make_gl_func(wglSwapIntervalEXT, BOOL, int interval);
make_gl_func(glDrawElementsInstanced, void, GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei primcount);
make_gl_func(glGenerateMipmap, void, GLenum target);
