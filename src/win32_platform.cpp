#include "defines.h"
#include "my_math.h"
#include "input.h"
#include "platform.h"
#include "logger.h"
#include "config.h"

// Game Layer
#include "game.cpp"

// Asset Layer
#include "assets.cpp"

// Renderer Layer
#include <windows.h>
#include "gl_renderer.cpp"

global_variable char* fileIOBuffer = 0;
global_variable uint32_t transientBytesUsed = 0;
global_variable char* transientBuffer = 0;
global_variable bool running = true;
global_variable HWND window;
global_variable KeyCode keyCodeLookupTable[KEY_COUNT] = {};

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
      
      input.screenSize.x = r.right - r.left;
      input.screenSize.y = r.bottom - r.top;
      
      renderer_resize();
      
      break;
    }
    
    default:
    {
      result = DefWindowProcA(hwnd, msg, wParam, lParam);
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



int main()
{ 
  // KeyCode lookup Table
  {
    keyCodeLookupTable[VK_LBUTTON] = KEY_LEFT_MOUSE;
    keyCodeLookupTable[VK_MBUTTON] = KEY_MIDDLE_MOUSE;
    keyCodeLookupTable[VK_RBUTTON] = KEY_RIGHT_MOUSE;
    
    keyCodeLookupTable['A'] = KEY_A;
    keyCodeLookupTable['B'] = KEY_B;
    keyCodeLookupTable['C'] = KEY_C;
    keyCodeLookupTable['D'] = KEY_D;
    keyCodeLookupTable['E'] = KEY_E;
    keyCodeLookupTable['F'] = KEY_F;
    keyCodeLookupTable['G'] = KEY_G;
    keyCodeLookupTable['H'] = KEY_H;
    keyCodeLookupTable['I'] = KEY_I;
    keyCodeLookupTable['J'] = KEY_J;
    keyCodeLookupTable['K'] = KEY_K;
    keyCodeLookupTable['L'] = KEY_L;
    keyCodeLookupTable['M'] = KEY_M;
    keyCodeLookupTable['N'] = KEY_N;
    keyCodeLookupTable['O'] = KEY_O;
    keyCodeLookupTable['P'] = KEY_P;
    keyCodeLookupTable['Q'] = KEY_Q;
    keyCodeLookupTable['R'] = KEY_R;
    keyCodeLookupTable['S'] = KEY_S;
    keyCodeLookupTable['T'] = KEY_T;
    keyCodeLookupTable['U'] = KEY_U;
    keyCodeLookupTable['V'] = KEY_V;
    keyCodeLookupTable['W'] = KEY_W;
    keyCodeLookupTable['X'] = KEY_X;
    keyCodeLookupTable['Y'] = KEY_Y;
    keyCodeLookupTable['Z'] = KEY_Z;
    keyCodeLookupTable['0'] = KEY_0;
    keyCodeLookupTable['1'] = KEY_1;
    keyCodeLookupTable['2'] = KEY_2;
    keyCodeLookupTable['3'] = KEY_3;
    keyCodeLookupTable['4'] = KEY_4;
    keyCodeLookupTable['5'] = KEY_5;
    keyCodeLookupTable['6'] = KEY_6;
    keyCodeLookupTable['7'] = KEY_7;
    keyCodeLookupTable['8'] = KEY_8;
    keyCodeLookupTable['9'] = KEY_9;
    
    keyCodeLookupTable[VK_SPACE] = KEY_SPACE,
    keyCodeLookupTable[VK_OEM_3] = KEY_TICK,
    keyCodeLookupTable[VK_OEM_MINUS] = KEY_MINUS,
    // TODO ???
    keyCodeLookupTable[VK_OEM_PLUS] = KEY_EQUAL,
    keyCodeLookupTable[VK_OEM_4] = KEY_LEFT_BRACKET,
    keyCodeLookupTable[VK_OEM_6] = KEY_RIGHT_BRACKET,
    keyCodeLookupTable[VK_OEM_1] = KEY_SEMICOLON,
    keyCodeLookupTable[VK_OEM_7] = KEY_QUOTE,
    keyCodeLookupTable[VK_OEM_COMMA] = KEY_COMMA,
    keyCodeLookupTable[VK_OEM_PERIOD] = KEY_PERIOD,
    keyCodeLookupTable[VK_OEM_2] = KEY_FORWARD_SLASH,
    keyCodeLookupTable[VK_OEM_5] = KEY_BACKWARD_SLASH,
    keyCodeLookupTable[VK_TAB] = KEY_TAB,
    keyCodeLookupTable[VK_ESCAPE] = KEY_ESCAPE,
    keyCodeLookupTable[VK_PAUSE] = KEY_PAUSE,
    keyCodeLookupTable[VK_UP] = KEY_UP,
    keyCodeLookupTable[VK_DOWN] = KEY_DOWN,
    keyCodeLookupTable[VK_LEFT] = KEY_LEFT,
    keyCodeLookupTable[VK_RIGHT] = KEY_RIGHT,
    keyCodeLookupTable[VK_BACK] = KEY_BACKSPACE,
    keyCodeLookupTable[VK_RETURN] = KEY_RETURN,
    keyCodeLookupTable[VK_DELETE] = KEY_DELETE,
    keyCodeLookupTable[VK_INSERT] = KEY_INSERT,
    keyCodeLookupTable[VK_HOME] = KEY_HOME,
    keyCodeLookupTable[VK_END] = KEY_END,
    keyCodeLookupTable[VK_PRIOR] = KEY_PAGE_UP,
    keyCodeLookupTable[VK_NEXT] = KEY_PAGE_DOWN,
    keyCodeLookupTable[VK_CAPITAL] = KEY_CAPS_LOCK,
    keyCodeLookupTable[VK_NUMLOCK] = KEY_NUM_LOCK,
    keyCodeLookupTable[VK_SCROLL] = KEY_SCROLL_LOCK,
    keyCodeLookupTable[VK_APPS] = KEY_MENU,
    
    keyCodeLookupTable[VK_SHIFT] = KEY_SHIFT,
    keyCodeLookupTable[VK_LSHIFT] = KEY_SHIFT,
    keyCodeLookupTable[VK_RSHIFT] = KEY_SHIFT,
    
    keyCodeLookupTable[VK_CONTROL] = KEY_CONTROL,
    keyCodeLookupTable[VK_LCONTROL] = KEY_CONTROL,
    keyCodeLookupTable[VK_RCONTROL] = KEY_CONTROL,
    
    keyCodeLookupTable[VK_MENU] = KEY_ALT,
    keyCodeLookupTable[VK_LMENU] = KEY_ALT,
    keyCodeLookupTable[VK_RMENU] = KEY_ALT,
    
    keyCodeLookupTable[VK_F1] = KEY_F1;
    keyCodeLookupTable[VK_F2] = KEY_F2;
    keyCodeLookupTable[VK_F3] = KEY_F3;
    keyCodeLookupTable[VK_F4] = KEY_F4;
    keyCodeLookupTable[VK_F5] = KEY_F5;
    keyCodeLookupTable[VK_F6] = KEY_F6;
    keyCodeLookupTable[VK_F7] = KEY_F7;
    keyCodeLookupTable[VK_F8] = KEY_F8;
    keyCodeLookupTable[VK_F9] = KEY_F9;
    keyCodeLookupTable[VK_F10] = KEY_F10;
    keyCodeLookupTable[VK_F11] = KEY_F11;
    keyCodeLookupTable[VK_F12] = KEY_F12;
    
    keyCodeLookupTable[VK_NUMPAD0] = KEY_NUMPAD_0;
    keyCodeLookupTable[VK_NUMPAD1] = KEY_NUMPAD_1;
    keyCodeLookupTable[VK_NUMPAD2] = KEY_NUMPAD_2;
    keyCodeLookupTable[VK_NUMPAD3] = KEY_NUMPAD_3;
    keyCodeLookupTable[VK_NUMPAD4] = KEY_NUMPAD_4;
    keyCodeLookupTable[VK_NUMPAD5] = KEY_NUMPAD_5;
    keyCodeLookupTable[VK_NUMPAD6] = KEY_NUMPAD_6;
    keyCodeLookupTable[VK_NUMPAD7] = KEY_NUMPAD_7;
    keyCodeLookupTable[VK_NUMPAD8] = KEY_NUMPAD_8;
    keyCodeLookupTable[VK_NUMPAD9] = KEY_NUMPAD_9;
  }
  
  char* gameMemory = (char*)malloc(FILE_IO_BUFFER_SIZE + TRANSIENT_BUFFER_SIZE);
  fileIOBuffer = gameMemory;
  transientBuffer = gameMemory + FILE_IO_BUFFER_SIZE;
  
  // Delta Time Stuff
  global_variable LARGE_INTEGER ticksPerSecond;
  LARGE_INTEGER lastTickCount, currentTickCount;
  QueryPerformanceFrequency(&ticksPerSecond);
  QueryPerformanceCounter(&lastTickCount);
  float dt = 0.0f;
  
  platform_create_window(500, 500, "Simple TD");
  
  init_open_gl(window);
  
  init_game();
  
  while(running)
  {
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
    
    update_game(dt);
    
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
    if (fileIOBuffer)
    {
      HANDLE file = CreateFile(
                               path,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               0,
                               OPEN_EXISTING,
                               0, 0);
      
      if (file != INVALID_HANDLE_VALUE)
      {
        LARGE_INTEGER fSize;
        if (GetFileSizeEx(file, &fSize))
        {
          *fileSize = (uint32_t)fSize.QuadPart;
          
          if (*fileSize < FILE_IO_BUFFER_SIZE)
          {
            // Use File IO Buffer
            memset(fileIOBuffer, 0, FILE_IO_BUFFER_SIZE);
            buffer = fileIOBuffer;
            
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
            CAKEZ_ASSERT(0, "File size: %d, too large for File IO Buffer: %d", 
                         *fileSize, FILE_IO_BUFFER_SIZE);
            CAKEZ_WARN("File size: %d, too large for File IO Buffer: %d", 
                       *fileSize, FILE_IO_BUFFER_SIZE);
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
      CAKEZ_ASSERT(0, "No File IO Buffer");
      CAKEZ_WARN("No File IO Buffer");
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
    transientBytesUsed += sizeInBytes;
  }
  else
  {
    CAKEZ_ASSERT(0, "Exausted Transient Storage!");
  }
  
  return buffer;
}