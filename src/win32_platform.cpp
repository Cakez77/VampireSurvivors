#include "defines.h"
#include "my_math.h"
#include "input.h"
#include "platform.h"
#include "logger.h"
#include "common.h"
#include "config.h"
#include "shared.h"
#include "sound.h"

// Asset Layer
#include "assets.cpp"

// Renderer Layer
#include <windows.h>
#include <windowsx.h>
#include "gl_renderer.cpp"

// Audio
#include <xaudio2.h>
#include <tlhelp32.h>

struct XAudioVoice : IXAudio2VoiceCallback
{
  bool playing;
  IXAudio2SourceVoice* voice;
  
  // Unused methods are stubs
  void OnStreamEnd() {playing = false;}
#pragma warning(disable : 4100)
  void OnBufferStart(void* pBufferContext) {playing = true;}
  
  // Unused methods are stubs
  void OnVoiceProcessingPassEnd() {}
  void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
  void OnBufferEnd(void* pBufferContext) {}
  void OnLoopEnd(void* pBufferContext) {}
  void OnVoiceError(void* pBufferContext, HRESULT Error) {}
#pragma warning(default : 4100)
};

struct XAudioState
{
  IXAudio2* device;
  IXAudio2MasteringVoice* masteringVoice;
  
  XAudioVoice voices[MAX_PLAYING_SOUNDS];
};

global_variable XAudioState xAudioState;

// Memory
global_variable int persistentBytesUsed = 0;
global_variable int transientBytesUsed = 0;
global_variable char* persistentBuffer = 0;
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
      
      input->screenSize.x = (r.right - r.left);
      input->screenSize.y = (r.bottom - r.top);
      
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
      
      break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    {
      bool isDown = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
        ? true: false;
      
      input->keys[wParam].isDown = isDown;
      input->keys[wParam].halfTransitionCount++;
      
      break;
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
    
    default:
    {
      result = DefWindowProcA(hwnd, msg, wParam, lParam);
    }
  }
  
  return result;
}

internal bool platform_create_window(int width, int height, char* title)
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
  
  window = CreateWindowExA((DWORD)window_ex_style, 
                           "cakez_window_class", title,
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

//#############################################################
//                  XAudio 2
//#############################################################
internal bool init_audio()
{
  // Allocate Memory
  soundState->buffer = platform_allocate_persistent(MAX_BYTES_SOUND_BUFFER);
  if(!soundState->buffer)
  {
    CAKEZ_ASSERT(0, "Failed to allocate Sound Buffer");
    return false;
  }
  
  
  // See: https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--initialize-xaudio2
  if (CoInitializeEx(0, COINIT_MULTITHREADED) != 0)
  {
    CAKEZ_ASSERT(0, "Failed to initialize xAudio2");
    return false;
  }
  
  if (XAudio2Create(&xAudioState.device, 0, XAUDIO2_USE_DEFAULT_PROCESSOR) != 0)
  {
    CAKEZ_ASSERT(0, "Failed to Create xAudio2 Device");
    return false;
  }
  
  // This is like the primary buffer in DirectSound
  if (xAudioState.device->CreateMasteringVoice(&xAudioState.masteringVoice) != 0)
  {
    CAKEZ_ASSERT(0, "Failed to Create xAudio2 Mastering Voice");
    return false;
  }
  
  // Adjust these to your specific WAV format used in your game
  // You can refactor this to be a different format or auto-detect it if you want to support
  // different exported formats. But I usually stick to just one format for my games.
  WAVEFORMATEX waveFormat = {0};
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = 2; // Stereo
  waveFormat.wBitsPerSample = 16; // 16 bits
  waveFormat.nSamplesPerSec = 48000; // 48 Khz
  waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
  waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
  waveFormat.cbSize = 0;
  
  // TODO: Submixes can be used to act as "Sound Groups" allowing us to control Volume 
  // only for a certain Type of Sound, skipped here because it's easier without
  
  for(int voiceIdx = 0; voiceIdx < MAX_PLAYING_SOUNDS; voiceIdx++)
  {
    XAudioVoice* xAudioVoice = &xAudioState.voices[voiceIdx];
    
    // These are like the secondary buffers in DirectSound, 
    int result = xAudioState.device->
      CreateSourceVoice(&xAudioVoice->voice, 
                        &waveFormat, XAUDIO2_VOICE_NOPITCH, 
                        XAUDIO2_DEFAULT_FREQ_RATIO, 
                        (IXAudio2VoiceCallback*)xAudioVoice,
                        0 /* Send audio to submix here, 0 = mastering voice */);
    
    // Reduce the Volume by 95%?????
    xAudioVoice->voice->SetVolume(0.05f);
    
    if (result)
    {
      CAKEZ_ASSERT(0, "Failed to Create xAudio2 Voice");
      return false;
    }
  }
  
  return true;
}

typedef void(init_game_type)(GameState*, Input*, RenderData*);
typedef void(update_game_type)(GameState*, Input*, RenderData*, float);

int main()
{ 
  transientBuffer = (char*)malloc(TRANSIENT_BUFFER_SIZE);
  persistentBuffer = (char*)malloc(PERSISTENT_BUFFER_SIZE);
  
  input = (Input*)platform_allocate_persistent(sizeof(Input));
  if(!input)
  {
    CAKEZ_FATAL("Failed to allocate Memory for Input!");
    return -1;
  }
  
  soundState = (SoundState*)platform_allocate_persistent(sizeof(SoundState));
  if(!input)
  {
    CAKEZ_FATAL("Failed to allocate Memory for Sounds!");
    return -1;
  }
  
  RenderData* renderData = (RenderData*)platform_allocate_persistent(sizeof(RenderData));
  if(!renderData)
  {
    CAKEZ_FATAL("Failed to allocate Memory for RenderData!");
    return -1;
  }
  
  GameState* gameState = (GameState*)platform_allocate_persistent(sizeof(GameState));
  if(!gameState)
  {
    CAKEZ_FATAL("Failed to allocate Memory for the GameState!");
    return -1;
  }
  
  // game dll stuff
  init_game_type* init_game = 0;
  update_game_type* update_game = 0;
  
  long long lastEditDLLTimestamp = 0;
  HMODULE gameDLL = NULL;
  
  // Delta Time Stuff
  global_variable LARGE_INTEGER ticksPerSecond;
  LARGE_INTEGER lastTickCount, currentTickCount;
  QueryPerformanceFrequency(&ticksPerSecond);
  QueryPerformanceCounter(&lastTickCount);
  float dt = 0.0f;
  
  platform_create_window(SCREEN_SIZE.x, SCREEN_SIZE.y, "VSClone");
  
  gl_init(window, renderData);
  // @Note(tkap, 21/11/2022): To not blow up my pc
  renderer_set_vertical_sync(false);
  
  // Audio
  init_audio();
  platform_play_sound(SOUND_BACKGROUND, true);
  
  bool isGameInitialized = false;
  
  // Seed for random numbers
  srand((uint32_t)__rdtsc());
  
  while(running)
  {
    running = !gameState->quitApp;
    
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		DLL STUFF START		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    {
      long long DLLTimestamp = platform_last_edit_timestamp("game.dll");
      if(DLLTimestamp > lastEditDLLTimestamp)
      {
        lastEditDLLTimestamp = DLLTimestamp;
        printf("New DLL found\n");
        
        if(gameDLL)
        {
          BOOL freeResult = FreeLibrary(gameDLL);
          assert(freeResult);
          gameDLL = NULL;
          printf("Freed library\n");
        }
        
        while(!CopyFile("game.dll", "game_load.dll", false)) { Sleep(10); }
        printf("Copied DLL\n");
        while(true)
        {
          gameDLL = LoadLibrary("game_load.dll");
          if(gameDLL) { break; }
          Sleep(10);
        }
        printf("Loaded DLL\n");
        
        init_game = (init_game_type*)GetProcAddress(gameDLL, "init_game");
        CAKEZ_ASSERT(init_game, "Failed to load init_game function from game DLL");
        update_game = (update_game_type*)GetProcAddress(gameDLL, "update_game");
        CAKEZ_ASSERT(update_game, "Failed to load update_game function from game DLL");
      }
    }
    
    if(!isGameInitialized)
    {
      isGameInitialized = true;
      init_game(gameState, input, renderData);
    }    
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		DLL STUFF END		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    
    
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
    
    if(is_key_pressed(KEY_K))
    {
      if(platform_file_exists("gameState.bin"))
      {
        platform_delete_file("gameState.bin");
      }
      platform_write_file("gameState.bin", (char*)gameState, sizeof(GameState), true);
    }
    
    if(is_key_pressed(KEY_L))
    {
      uint32_t fileSize;
      char* buffer = platform_read_file("gameState.bin", &fileSize);
      
      if(buffer)
      {
        if(fileSize == sizeof(GameState))
        {
          memcpy(gameState, buffer, fileSize);
        }
        else
        {
          CAKEZ_ASSERT(0, "GameState Size changed!")
        }
      }
      else
      {
        CAKEZ_ASSERT(0, "Failed to read GameState");
      }
    }
    
    update_game(gameState, input, renderData, dt);
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
  
  WriteConsoleA(consoleHandle, msg, (int)strlen(msg), 0, 0);
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

unsigned long platform_write_file(char *path,
                                  char *buffer,
                                  uint32_t size,
                                  bool overwrite)
{
  DWORD bytesWritten = 0;
  
  HANDLE file = CreateFile(path,
                           overwrite ? GENERIC_WRITE : FILE_APPEND_DATA,
                           FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
  
  if (file != INVALID_HANDLE_VALUE)
  {
    if (!overwrite)
    {
      DWORD result = SetFilePointer(file, 0, 0, FILE_END);
      if (result == INVALID_SET_FILE_POINTER)
      {
        CAKEZ_WARN("Failed to set file pointer to the end");
      }
    }
    
    BOOL result = WriteFile(file, buffer, size, &bytesWritten, 0);
    if (result && size == bytesWritten)
    {
      // Success
    }
    else
    {
      CAKEZ_WARN("Failed writing file %s", path);
    }
    CloseHandle(file);
  }
  else
  {
    CAKEZ_WARN("Failed opening file %s", path);
  }
  
  return bytesWritten;
}

bool platform_file_exists(char *path)
{
  DWORD attributes = GetFileAttributes(path);
  
  return (attributes != INVALID_FILE_ATTRIBUTES &&
          !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

void platform_delete_file(char *path)
{
  CAKEZ_ASSERT(DeleteFileA(path) != 0, "Failed deleting file: %s", path);
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

void platform_play_sound(SoundID soundID, bool loop)
{
  Sound* sound = 0;
  for(int soundIdx = 0; soundIdx < soundState->allocatedSoundsCount; soundIdx++)
  {
    Sound* s = &soundState->allocatedSounds[soundIdx];
    
    if(s->ID == soundID)
    {
      sound = s;
      break;
    }
  }
  
  if(!sound)
  {
    if(soundState->allocatedSoundsCount < MAX_ALLOCATED_SOUNDS)
    {
      sound = &soundState->allocatedSounds[soundState->allocatedSoundsCount++];
    }
    
    // Load a new file from disk and copy that into the sound state
    uint32_t fileSize = 0;
    char* soundFile = platform_read_file(SoundFiles[soundID], &fileSize);
    
    if(soundFile)
    {
      // Find the Data of the WAV file
      WaveDataChunk *dataChunk = (WaveDataChunk*)(soundFile + sizeof(WaveFileHeader));
      while(*(uint32_t*)&dataChunk->dataChunkId != FOURCC("data"))
      {
        dataChunk = (WaveDataChunk*)((char*)(dataChunk) + sizeof(WaveDataChunk) + dataChunk->dataSize);
      }
      
      if((int)dataChunk->dataSize < MAX_BYTES_SOUND_BUFFER - soundState->bytesUsed)
      {
        sound->ID = soundID;
        sound->data = &soundState->buffer[soundState->bytesUsed];
        sound->sizeInBytes = dataChunk->dataSize;
        
        char* soundData = (char*)dataChunk + sizeof(WaveDataChunk);
        
        memcpy(sound->data, soundData, dataChunk->dataSize);
        soundState->bytesUsed += dataChunk->dataSize;
      }
      else
      {
        CAKEZ_ASSERT(0, "Exausted Sound Buffer");
      }
    }
    else
    {
      CAKEZ_ASSERT(0, "Failed Loading Sound");
    }
  }
  
  // Load the sound into xaudio
  {
    for(int voiceIdx = 0; voiceIdx < MAX_PLAYING_SOUNDS; voiceIdx++)
    {
      XAudioVoice xAudioVoice = xAudioState.voices[voiceIdx];
      
      if(xAudioVoice.playing)
      {
        continue;
      }
      
      XAUDIO2_BUFFER stupidBuffer = {0};
      stupidBuffer.AudioBytes = sound->sizeInBytes;
      stupidBuffer.pAudioData = (BYTE *)sound->data;
      stupidBuffer.Flags = XAUDIO2_END_OF_STREAM;
      stupidBuffer.LoopCount = loop? XAUDIO2_MAX_LOOP_COUNT: 0;
      
      if (int result = xAudioVoice.voice->SubmitSourceBuffer(&stupidBuffer))
      {
        CAKEZ_ASSERT(0, "Failed to play Sound: %d", sound->ID);
      }
      xAudioVoice.voice->Start(0, 0);
    }
  }
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

char* platform_allocate_persistent(uint32_t sizeInBytes)
{
  char* buffer = 0;
  
  if(persistentBytesUsed + sizeInBytes < PERSISTENT_BUFFER_SIZE)
  {
    buffer = persistentBuffer + persistentBytesUsed;
    memset(buffer, 0, sizeInBytes);
    persistentBytesUsed += sizeInBytes;
  }
  else
  {
    CAKEZ_ASSERT(0, "Exausted Persitent Storage!");
  }
  
  return buffer;
  
}


































