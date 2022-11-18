#include "defines.h"
#include "my_math.h"
#include "input.h"
#include "platform.h"
#include "logger.h"
#include "config.h"

#include <windows.h>

// Renderer Layer
#include "gl_renderer.cpp"

global_variable char* fileIOBuffer = 0;
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
			
			input.screenSize.x = r.right - r.left;
			input.screenSize.y = r.bottom - r.top;
			
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
	fileIOBuffer = (char*)malloc(FILE_IO_BUFFER_SIZE);
	
	platform_create_window(500, 500, "Simple TD");
	
	init_open_gl(window);
	
	while(running)
	{
		platform_update_window();
		
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
