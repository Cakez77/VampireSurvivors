#include "render_interface.h"
#include "shader_header.h"
#include "custom_gl.h"

//#############################################################
//                  Internal Structures
//#############################################################
struct Texture
{
  uint32_t ID;
  long long lastEditTimestamp;
};

struct GLContext
{
  bool initialized = false;
  bool vSync = true;
  
  HDC dc;
  uint32_t programID;
  uint32_t screenSizeID;
  uint32_t materialSBOID;
  uint32_t transformSBOID;
  Texture textureAtlas01;
  
  uint32_t materialCount;
  Material materials[MAX_MATERIALS];
  
  uint32_t transformCount;
  Transform transforms[MAX_TRANSFORMS];
};


//#############################################################
//                  Global Variables
//#############################################################
global_variable GLContext glContext;
global_variable PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
global_variable PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;


//#############################################################
//                  Internal Functions
//#############################################################
#ifdef DEBUG
internal void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                         GLsizei length, const GLchar* message, const void* user)
{
  if(severity == GL_DEBUG_SEVERITY_MEDIUM ||
     severity == GL_DEBUG_SEVERITY_HIGH)
  {
    CAKEZ_ASSERT(0, "OpenGL Error: %s", message);
  }
}
#endif // DEBUG

internal void init_open_gl_functions()
{
  init_gl_func(glCreateShader);
  init_gl_func(glGetUniformLocation);
  init_gl_func(glUniform1f);
  init_gl_func(glUniform2fv);
  init_gl_func(glUniform3fv);
  init_gl_func(glUniform1i);
  init_gl_func(glUniformMatrix4fv);
  init_gl_func(glVertexAttribDivisor);
  init_gl_func(glActiveTexture);
  init_gl_func(glBufferSubData);
  init_gl_func(glDrawArraysInstanced);
  init_gl_func(glBindFramebuffer);
  init_gl_func(glCheckFramebufferStatus);
  init_gl_func(glGenFramebuffers);
  init_gl_func(glFramebufferTexture2D);
  init_gl_func(glDrawBuffers);
  init_gl_func(glDeleteFramebuffers);
  init_gl_func(glBlendFunci);
  init_gl_func(glBlendEquation);
  init_gl_func(glClearBufferfv);
  init_gl_func(glShaderSource);
  init_gl_func(glCompileShader);
  init_gl_func(glGetShaderiv);
  init_gl_func(glGetShaderInfoLog);
  init_gl_func(glCreateProgram);
  init_gl_func(glAttachShader);
  init_gl_func(glLinkProgram);
  init_gl_func(glValidateProgram);
  init_gl_func(glGetProgramiv);
  init_gl_func(glGenBuffers);
  init_gl_func(glGenVertexArrays);
  init_gl_func(glGetAttribLocation);
  init_gl_func(glBindVertexArray);
  init_gl_func(glEnableVertexAttribArray);
  init_gl_func(glVertexAttribPointer);
  init_gl_func(glBindBuffer);
  init_gl_func(glBindBufferBase);
  init_gl_func(glBufferData);
  init_gl_func(glGetVertexAttribPointerv);
  init_gl_func(glUseProgram);
  init_gl_func(glDeleteVertexArrays);
  init_gl_func(glDeleteBuffers);
  init_gl_func(glDeleteProgram);
  init_gl_func(glDeleteShader);
  init_gl_func(wglSwapIntervalEXT);
  init_gl_func(glDrawElementsInstanced);
  init_gl_func(glGenerateMipmap);
  init_gl_func(glGetProgramInfoLog);
}

internal bool init_open_gl(void* window)
{
  // Fake Window Bullshit
  {
    HWND fake_window = CreateWindowEx(0,"cakez_window_class", "window name", WS_OVERLAPPEDWINDOW, 
                                      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                      CW_USEDEFAULT, 0, 0, GetModuleHandleA(0), 0);
    
    if(!fake_window)
    {
      platform_print_error();
      return false;
    }
    
    
    HDC fake_dc = GetDC(fake_window);
    if(!fake_dc)
    {
      platform_print_error();
      return false;
    }
    
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    int pixel_format = ChoosePixelFormat(fake_dc, &pfd);
    
    if(!pixel_format)
    {
      platform_print_error();
      return false;
    }
    
    if(!SetPixelFormat(fake_dc, pixel_format, &pfd))
    {
      platform_print_error();
      return false;
    }
    
    HGLRC fake_glrc = wglCreateContext(fake_dc);
    
    if(!fake_glrc)
    {
      platform_print_error();
      return false;
    }
    
    if(!wglMakeCurrent(fake_dc, fake_glrc))
    {
      platform_print_error();
      return false;
    }
    
    wglChoosePixelFormatARB = 
    (PFNWGLCHOOSEPIXELFORMATARBPROC)get_gl_proc("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = 
    (PFNWGLCREATECONTEXTATTRIBSARBPROC)get_gl_proc("wglCreateContextAttribsARB");
    
    wglMakeCurrent(fake_dc, 0);
    wglDeleteContext(fake_glrc);
    ReleaseDC(fake_window, fake_dc);
    DestroyWindow(fake_window);
  }
  
  
  
  // Actual OpenGL Initialization
  {
    HGLRC gldc = 0;
    glContext.dc = GetDC((HWND)window);
    
    if(!glContext.dc)
    {
      platform_print_error();
      return false;
    }
    
    int pixelFormat[32];
    UINT numFormats;
    const int pixelAttribs[] = {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      /* WGL_SWAP_EXCHANGE_ARB causes problems with window menu in fullscreen */
      WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB,
      WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
      WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
      WGL_COLOR_BITS_ARB, 32,
      WGL_ALPHA_BITS_ARB, 8,
      WGL_DEPTH_BITS_ARB, 24,
      // WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
      // WGL_SAMPLES_ARB, 4,
      0
    };
    
    int  contextAttributes[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
      WGL_CONTEXT_MINOR_VERSION_ARB, 3,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      
#ifdef DEBUG
      WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
      0
    };
    
    if(!wglChoosePixelFormatARB(glContext.dc, pixelAttribs, 0, 1, pixelFormat, &numFormats))
    {
      CAKEZ_ERROR("Failed to choose Pixel Format for OpenGL!");
      return false;
    }
    CAKEZ_ASSERT(numFormats,"Should never happen?");
    
    PIXELFORMATDESCRIPTOR pfd = {0};
    DescribePixelFormat(glContext.dc, pixelFormat[0], sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    
    if(!SetPixelFormat(glContext.dc, pixelFormat[0], &pfd))
    {
      platform_print_error();
      return false;
    }
    
    gldc = wglCreateContextAttribsARB(glContext.dc, 0, contextAttributes);
    if(!gldc)
    {
      platform_print_error();
      return false;
    }
    
    if(!wglMakeCurrent(glContext.dc, gldc))
    {
      platform_print_error();
      return false;
    }
  }
  
  // Setup Debugging for OpenGL
  {
#ifdef DEBUG
    init_gl_func(glDebugMessageCallback);
    glDebugMessageCallback(&gl_debug_callback, 0);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    // glEnable(GL_DEBUG_OUTPUT);
#endif
  }
  
  init_open_gl_functions();
  
  // Create Programs
  {
    uint32_t fileSize = 0;
    char* tmpHeaderBuffer = platform_read_file("src/shader_header.h", &fileSize);
    
    // Scuffed malloc because file reading is done through a "fileIOBuffer" that 
    // is being overwritten by multiple file reads
    char* shaderHeader = (char*)malloc(fileSize);
    CAKEZ_ASSERT(shaderHeader, "Failed to allocate Space for the Shader Header");
    memcpy(shaderHeader, tmpHeaderBuffer, fileSize);
    
    char* vertexShader = platform_read_file("assets/shaders/quad.vert", &fileSize);
    uint32_t vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    char* vertexSources[] = 
    {
      "#version 430 core\n",
      shaderHeader,
      vertexShader
    };
    
    glShaderSource(vertexShaderID, ArraySize(vertexSources), vertexSources, 0);
    glCompileShader(vertexShaderID);
    
    fileSize = 0;
    char* fragShader = platform_read_file("assets/shaders/quad.frag", &fileSize);
    uint32_t fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShaderID, 1, &fragShader, 0);
    glCompileShader(fragShaderID);
    
    // Validate if Shaders work
    {
      int programSuccess;
      char programInfoLog[512];
      glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &programSuccess);
      
      if(!programSuccess)
      {
        glGetShaderInfoLog(fragShaderID, 512, 0, programInfoLog);
        
        CAKEZ_ASSERT(0, "Failed to compile shader program: %s", programInfoLog);
        
        return 0;
      }
    }
    
    glContext.programID = glCreateProgram();
    glAttachShader(glContext.programID, vertexShaderID);
    glAttachShader(glContext.programID, fragShaderID);
    glLinkProgram(glContext.programID);
    
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragShaderID);
    
    // Validate if Shaders work
    {
      int programSuccess;
      char programInfoLog[512];
      glGetProgramiv(glContext.programID, GL_LINK_STATUS, &programSuccess);
      
      if(!programSuccess)
      {
        glGetProgramInfoLog(glContext.programID, 512, 0, programInfoLog);
        
        CAKEZ_ASSERT(0, "Failed to compile shader program: %s", programInfoLog);
        
        return 0;
      }
    }
    free(shaderHeader);
    
    
    uint32_t VAO = 0;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // Material Storage Buffer
    {
      glGenBuffers(1, &glContext.materialSBOID);
      
      // Binds the SSBO to a binding Idx, only because we have one buffer
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, glContext.materialSBOID);
    }
    
    // Transform Stroage Buffer 
    {
      glGenBuffers(1, &glContext.transformSBOID);
      
      // Binds the SSBO to a binding Idx, only because we have one buffer
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);
    }
    
    // 
    {
      glGenTextures(1, &glContext.textureAtlas01.ID);
      glBindTexture(GL_TEXTURE_2D, glContext.textureAtlas01.ID);
      
      // set the texture wrapping/filtering options (on the currently bound texture object)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      
      // load and generate the texture
      int width = 0, height = 0;
      char* data = get_asset(TEXTURE_ATLAS_01, &width, &height);
      if (data)
      {
        glContext.textureAtlas01.lastEditTimestamp = get_last_edit_timestamp(TEXTURE_ATLAS_01);
        CAKEZ_ASSERT(glContext.textureAtlas01.lastEditTimestamp > 0,
                     "Failed getting edit Timestamp from TextureID: %d", TEXTURE_ATLAS_01);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
      }
      else
      {
        CAKEZ_ASSERT(0, "Failed to get data for TextureID: %d", TEXTURE_ATLAS_01);
      }
    }
    
    
    // TODO: Think, about what?
    glUseProgram(glContext.programID);
    
    // Supply Screen Size of the Shader
    {
      glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screenSize");
      glUniform2fv(glContext.screenSizeID, 1, (float*)&input.screenSize);
    }
  }
  
  glContext.initialized = true;
  
  return true;
}

internal void add_transform(Transform t = {})
{
  if(glContext.transformCount < MAX_TRANSFORMS)
  {
    glContext.transforms[glContext.transformCount++] = t;
  }
  else
  {
    CAKEZ_ASSERT(0, "Reached maximum amount of transforms");
  }
}

internal int get_material_idx(Vec4 color)
{
  int idx = 0;
  bool foundMaterial = false;
  
  for(int materialIdx = 0; materialIdx < glContext.materialCount; materialIdx++)
  {
    if(glContext.materials[materialIdx].color == color)
    {
      idx = materialIdx;
      foundMaterial = true;
      break;
    }
  }
  
  if(!foundMaterial)
  {
    Material m = {color};
    
    if(glContext.materialCount < MAX_MATERIALS)
    {
      glContext.materials[glContext.materialCount++] = m;
    }
    else
    {
      CAKEZ_ASSERT(0, "Reached maximum amount of materials");
    }
  }
  
  return idx;
}

internal void hot_reload_textures()
{
  long long lastEditTimestamp = get_last_edit_timestamp(TEXTURE_ATLAS_01);
  
  if(lastEditTimestamp > glContext.textureAtlas01.lastEditTimestamp)
  {
    // load and generate the texture
    int width = 0, height = 0;
    char* data = get_asset(TEXTURE_ATLAS_01, &width, &height);
    if (data)
    {
      glContext.textureAtlas01.lastEditTimestamp = get_last_edit_timestamp(TEXTURE_ATLAS_01);
      CAKEZ_ASSERT(glContext.textureAtlas01.lastEditTimestamp > 0,
                   "Failed getting edit Timestamp from TextureID: %d", TEXTURE_ATLAS_01);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
      CAKEZ_ASSERT(0, "Failed to get data for TextureID: %d", TEXTURE_ATLAS_01);
    }
  }
}

internal bool gl_render()
{
  // Render Loop
  {
    glClearColor(0.2f, 0.05f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
#ifdef DEBUG
    hot_reload_textures();
#endif
    
    glContext.materials[glContext.materialCount++] = {1.0f, 0.0f, 0.0f, 1.0f};
    glContext.materials[glContext.materialCount++] = {0.0f, 0.0f, 1.0f, 1.0f};
    
    // Copy Materials to GPU
    {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.materialSBOID);
      glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * glContext.materialCount,
                   glContext.materials, GL_STATIC_DRAW);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
      
      glContext.materialCount = 0;
    }
    
    // Copy Transforms to GPU
    {
      // Use the Buffer, (active)
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.transformSBOID);
      glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * glContext.transformCount,
                   glContext.transforms, GL_STATIC_DRAW);
      
      //Undinds the buffer after usage (inactive)
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
      
      glDrawArraysInstanced(GL_TRIANGLES, 0, 6, glContext.transformCount);
      glContext.transformCount = 0;
    }
    
    
    SwapBuffers(glContext.dc);
  }
  
  
  return true;
}

//#############################################################
//           Implementations from render_interface.h
//#############################################################

// @Note(tkap, 22/11/2022): Not the best name
void draw_transform(DrawData drawData)
{
  Sprite s = get_sprite(drawData.spriteID);
  Transform t = {};
  t.pos = drawData.pos;
  t.size = drawData.size;
  t.atlasOffset = s.atlasOffset;
  t.spriteSize = s.subSize;
  t.renderOptions = drawData.renderOptions;
  t.materialIdx = get_material_idx(drawData.color);
  add_transform(t);
}

void draw_quad(Vec2 pos, Vec2 size, Vec4 color, RenderOptions renderOptions)
{
  draw_transform({.pos = pos, .size = size, .color = color, .renderOptions = renderOptions});
}

void draw_sprite(SpriteID spriteID, Vec2 pos, Vec2 scale, Vec4 color, RenderOptions renderOptions)
{
  Sprite s = get_sprite(spriteID);
  Vec2 subSize = vec_2(s.subSize.x, s.subSize.y);
  draw_transform({ .spriteID = spriteID, .pos = pos, .size = subSize * scale, 
                   .color = color, .renderOptions = renderOptions});
}

void draw_line(Vec2 a, Vec2 b, Vec4 color)
{
  // TODO: Optimize this, use angle, rotation in shader
  float lineLength = length(b - a);
  Vec2 direction = normalize(b - a);
  
  draw_quad(a + direction * 0.0f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 0.1f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 0.2f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 0.3f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 0.4f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 0.5f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 0.6f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 0.7f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 0.8f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 0.9f * lineLength, {2.0f, 2.0f}, color);
  draw_quad(a + direction * 1.0f * lineLength, {2.0f, 2.0f}, color);
}

void draw_circle(Circle c, Vec4 color)
{
  float angle = 0.32f;
  for(uint32_t i = 0; i < 20; i++)
  {
    Vec2 a = Vec2{c.radius * sinf(i * angle), c.radius * cosf(i * angle)};
    draw_quad(c.pos + a, {2.0f, 2.0f}, color);
  }
}

bool renderer_get_vertical_sync()
{
  return glContext.vSync;
}

void renderer_set_vertical_sync(bool vSync)
{
  glContext.vSync = vSync;
  wglSwapIntervalEXT(vSync);
}

void renderer_resize()
{
  if(glContext.initialized)
  {
    glViewport(0, 0, input.screenSize.x, input.screenSize.y);
    glUniform2fv(glContext.screenSizeID, 1, (float*)&input.screenSize);
  }
}
