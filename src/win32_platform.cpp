#include "defines.h"
#include "my_math.h"
#include "input.h"
#include "platform.h"
#include "logger.h"
#include "common.h"
#include "config.h"
#include "shared.h"

// Asset Layer
#include "assets.cpp"

// Renderer Layer
#include <windows.h>
#include <windowsx.h>
#include "gl_renderer.cpp"

// Memory
global_variable int transientBytesUsed = 0;
global_variable char* transientBuffer = 0;

global_variable bool running = true;
global_variable HWND window;

LRESULT CALLBACK window_callback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  LRESULT result = 0;
  
  switch (msg)
  {
    case WM_CLOSE:
    {
      running = false;
      
      break;
    }
    
    case WM_DESTROY:
    {
      PostQuitMessage(0);
      
      break;
    }
    
    case WM_SIZE:
    {
      // Set the Screen Size
      RECT r;
      GetClientRect(window, &r);
      
      input->screenSize.x = r.right - r.left;
      input->screenSize.y = r.bottom - r.top;
      
      renderer_resize();
      
      break;
    }
    
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
      if(wParam < KEY_COUNT)
      {
        bool isDown = !(HIWORD(lParam) & KF_UP);
        int isEcho = isDown && ((lParam >> 30) & 1);
        if(!isEcho)
        {
          input->keys[wParam].halfTransitionCount += 1;
          input->keys[wParam].isDown = isDown;
        }
      }
    } break;
    
    default:
    {
      result = DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    
    case WM_MOUSEMOVE:
    {
      input->oldMousePos = input->mousePosScreen;
      
      // Mouse Position
      input->mousePosScreen.x = (float)GET_X_LPARAM(lParam);
      input->mousePosScreen.y = (float)GET_Y_LPARAM(lParam);
      
      // Relative Movement
      input->relMouseScreen = input->mousePosScreen - input->oldMousePos;
      
      break;
    }
    
    
  }
  
  return result;
}

internal bool platform_create_window(int width, int height, const char *title)
{
  HINSTANCE instance = GetModuleHandleA(0);
  
  // Setup and register window class
  HICON icon = LoadIcon(instance, IDI_APPLICATION);
  WNDCLASS wc = {};
  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = window_callback;
  wc.hInstance = instance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW); // NULL; => Manage the cursor manually
  wc.lpszClassName = "cakez_window_class";
  
  if (!RegisterClassA(&wc))
  {
    MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }
  
  // Create window
  uint32_t client_x = 100;
  uint32_t client_y = 100;
  uint32_t client_width = width;
  uint32_t client_height = height;
  
  uint32_t window_x = client_x;
  uint32_t window_y = client_y;
  uint32_t window_width = client_width;
  uint32_t window_height = client_height;
  
  uint32_t window_style =
    WS_OVERLAPPED |
    WS_SYSMENU |
    WS_CAPTION |
    WS_THICKFRAME |
    WS_MINIMIZEBOX |
    WS_MAXIMIZEBOX;
  
  // topmost | WS_EX_TOPMOST;
  uint32_t window_ex_style = WS_EX_APPWINDOW | WS_EX_TOPMOST;
  window_ex_style = WS_EX_APPWINDOW;
  
  // Obtain the size of the border
  RECT border_rect = {};
  AdjustWindowRectEx(&border_rect,
                     (DWORD)window_style,
                     0,
                     (DWORD)window_ex_style);
  
  window_x += border_rect.left;
  window_y += border_rect.top;
  
  window_width += border_rect.right - border_rect.left;
  window_height += border_rect.bottom - border_rect.top;
  
  window = CreateWindowExA((DWORD)window_ex_style, "cakez_window_class", title,
                           (DWORD)window_style, window_x, window_y, window_width, window_height,
                           0, 0, instance, 0);
  
  if (window == 0)
  {
    MessageBoxA(NULL, "Window creation failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }
  
  // Show the window
  ShowWindow(window, SW_SHOW);
  
  return true;
}

internal void platform_update_window()
{
  MSG msg;
  
  while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}


typedef void(init_game_type)(GameState*, Input*, Dunno*);
typedef void(update_game_type)(GameState*, float);


int main()
{ 
  // TODO Allocate input later
  Input i = {};
  input = &i;
  
  Dunno* dunno = (Dunno*)malloc(sizeof(Dunno));
  *dunno = {};
  
  // game dll stuff
  init_game_type* init_game = 0;
  update_game_type* update_game = 0;
  
  long long lastEditDLLTimestamp = 0;
  long long DLLTimestamp = platform_last_edit_timestamp("game.dll");
  HMODULE gameDLL = LoadLibrary("game_load.dll");
  
  init_game = (init_game_type*)GetProcAddress(gameDLL, "init_game");
  update_game = (update_game_type*)GetProcAddress(gameDLL, "update_game");
  
  
  transientBuffer = (char*)malloc(TRANSIENT_BUFFER_SIZE);
  
  // Delta Time Stuff
  global_variable LARGE_INTEGER ticksPerSecond;
  LARGE_INTEGER lastTickCount, currentTickCount;
  QueryPerformanceFrequency(&ticksPerSecond);
  QueryPerformanceCounter(&lastTickCount);
  float dt = 0.0f;
  
  platform_create_window(WORLD_SIZE.x, WORLD_SIZE.y, "VSClone");
  
  gl_init(window, dunno);
  // @Note(tkap, 21/11/2022): To not blow up my pc
  renderer_set_vertical_sync(true);
  
  // Seed for random numbers
  srand((uint32_t)__rdtsc());
  
  GameState gameState = {};
  
  // TODO ENABLE LATER
  init_game(&gameState, input, dunno);
  
  while(running)
  {
    long long DLLTimestamp = platform_last_edit_timestamp("game.dll");
    if(DLLTimestamp > lastEditDLLTimestamp)
    {
      FreeLibrary(gameDLL);
      if(CopyFile("game.dll", "game_load.dll", false))
      {
        lastEditDLLTimestamp = DLLTimestamp;
      }
      
      // Load Lib
      {
        gameDLL = LoadLibrary("game_load.dll");
        
        init_game = (init_game_type*)GetProcAddress(gameDLL, "init_game");
        CAKEZ_ASSERT(init_game, "DUH!");
        update_game = (update_game_type*)GetProcAddress(gameDLL, "update_game");
        CAKEZ_ASSERT(update_game, "DUH!");
        
        init_game(&gameState, input, dunno);
      }
    }
    
    // Reset temporary memory
    transientBytesUsed = 0;
    
    // Evaludate Delta Time
    {
      QueryPerformanceCounter(&currentTickCount);
      
      uint64_t elapsedTicks = currentTickCount.QuadPart - lastTickCount.QuadPart;
      
      // Convert to Microseconds to not loose precision, by deviding a small numbner by a large one
      uint64_t elapsedTimeInMicroseconds = (elapsedTicks * 1000000) / ticksPerSecond.QuadPart;
      
      lastTickCount = currentTickCount;
      
      // Time in milliseconds
      dt = (float)elapsedTimeInMicroseconds / 1000.0f;
      
      // Lock dt to 50ms
      if (dt > 50.0f)
      {
        dt = 50.0f;
      }
      
      // Time in seconds
      dt /= 1000.0f;
    }
    
    platform_update_window();
    
    update_game(&gameState, dt);
    for(int key_i = 0; key_i < KEY_COUNT; key_i++)
    {
      input->keys[key_i].halfTransitionCount = 0;
    }
    
    gl_render();
  }
  
  return 0;
}


//#############################################################
//          Implementations from platform.h
//#############################################################
void platform_log(char *msg, TextColor color)
{
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  
  uint32_t colorBits = 0;
  
  switch (color)
  {
    case TEXT_COLOR_WHITE:
    colorBits = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    break;
    
    case TEXT_COLOR_GREEN:
    colorBits = FOREGROUND_GREEN;
    break;
    
    case TEXT_COLOR_YELLOW:
    colorBits = FOREGROUND_GREEN | FOREGROUND_RED;
    break;
    
    case TEXT_COLOR_RED:
    colorBits = FOREGROUND_RED;
    break;
    
    case TEXT_COLOR_LIGHT_RED:
    colorBits = FOREGROUND_RED | FOREGROUND_INTENSITY;
    break;
  }
  
  SetConsoleTextAttribute(consoleHandle, (WORD)colorBits);
  
#ifdef DEBUG
  OutputDebugStringA(msg);
#endif
  
  WriteConsoleA(consoleHandle, msg, strlen(msg), 0, 0);
}

void platform_print_error()
{
  int error_ = GetLastError();
  LPTSTR error_text = 0;
  
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                0,
                error_,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&error_text,
                0,
                0);
  
  if(error_text)
  {
    CAKEZ_ASSERT(0, "%s", error_text);
    LocalFree(error_text);
  }
}

char *platform_read_file(char *path, uint32_t *fileSize)
{
  char *buffer = 0;
  
  if (fileSize)
  {
    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ,
                             0, OPEN_EXISTING, 0, 0);
    
    if (file != INVALID_HANDLE_VALUE)
    {
      LARGE_INTEGER fSize;
      if (GetFileSizeEx(file, &fSize))
      {
        *fileSize = (uint32_t)fSize.QuadPart;
        buffer = platform_allocate_transient(*fileSize + 1);
        
        if (buffer)
        {
          DWORD bytesRead;
          if (ReadFile(file, buffer, *fileSize, &bytesRead, 0) &&
              *fileSize == bytesRead)
          {
          }
          else
          {
            CAKEZ_WARN("Failed reading file %s", path);
            buffer = 0;
          }
        }
        else
        {          
          CAKEZ_ASSERT(0, "Could not allocate: %d bytes, to Load file %s",
                       *fileSize, path);
          CAKEZ_WARN("Could not allocate: %d bytes, to Load file %s",
                     *fileSize, path);
        }
      }
      else
      {
        CAKEZ_WARN("Failed getting size of file %s", path);
      }
      
      CloseHandle(file);
    }
    else
    {
      CAKEZ_WARN("Failed opening file %s", path);
    }
  }
  else
  {
    CAKEZ_ASSERT(0, "No Length supplied!");
    CAKEZ_WARN("No Length supplied!");
  }
  
  return buffer;
}

long long platform_last_edit_timestamp(char* path)
{
  long long time = 0;
  
  HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_WRITE,
                           0, OPEN_EXISTING, 0, 0);
  
  if (file != INVALID_HANDLE_VALUE)
  {
    FILETIME writeTime;
    if (GetFileTime(file, 0, 0, &writeTime))
    {
      ULARGE_INTEGER tmp = {writeTime.dwLowDateTime, writeTime.dwHighDateTime};
      time = tmp.QuadPart;
    }
    else
    {
      CAKEZ_WARN("Failed getting file time of file %s ", path);
    }
    
    CloseHandle(file);
  }
  else
  {
    CAKEZ_WARN("Failed opening file %s", path);
  }
  
  return time;
}

char* platform_allocate_transient(uint32_t sizeInBytes)
{
  char* buffer = 0;
  
  if(transientBytesUsed + sizeInBytes < TRANSIENT_BUFFER_SIZE)
  {
    buffer = transientBuffer + transientBytesUsed;
    memset(buffer, 0, sizeInBytes);
    transientBytesUsed += sizeInBytes;
  }
  else
  {
    CAKEZ_ASSERT(0, "Exausted Transient Storage!");
  }
  
  return buffer;
}