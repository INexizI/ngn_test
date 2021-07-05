#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

#define internal static
#define local_presist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer
{
    // NOTE(D): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

// TODO(D): This is global for now.
global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

// NOTE(D): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(D): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter )
typedef DIRECT_SOUND_CREATE(direct_sound_create);



internal void
Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(XInputLibrary)
    {
        // TODO(D): Diagnostic
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }

    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}

        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if (!XInputSetState) {XInputSetState = XInputSetStateStub;}

        // TODO(D): Diagnostic
    }
    else
    {
        // TODO(D): Diagnostic
    }
}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    // NOTE(D): Load the Library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
        // NOTE(D): Get a DirectSound object! - cooperative
        direct_sound_create *DirectSoundCaptureCreate = (direct_sound_create*)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCaptureCreate && SUCCEEDED(DirectSoundCaptureCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                // NOTE(D): "Create" a primary buffer
                // TODO(D): DSBCAPS_GLOBALFOCUS?
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if(SUCCEEDED(Error))
                    {
                        // NOTE(D): We have finally set the format!
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                    else
                    {
                        // TODO(D): Diagnostic
                    }
                }
                else
                {
                    // TODO(D): Diagnostic
                }
            }
            else
            {
                // TODO(D): Diagnostic
            }

            // TODO(D): DSBCAPS_GETCURRENTPOSITION2
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;

            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if (SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created succefully.\n");
            }
        }
        else
        {
            // TODO(D): Diagnostic
        }
    }
    else
    {
        // TODO(D): Diagnostic
    }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    uint8 *Row = (uint8 *)Buffer->Memory;
    for (int Y = 0;
         Y < Buffer->Height;
         ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            uint8 Blue = (X + BlueOffset);
            uint8 Green = (Y + GreenOffset);
            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Buffer->Pitch;
    }
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    int BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    // NOTE(D): No More DC for us.
    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Buffer->Width * BytesPerPixel;

    // TODO(D): Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer,
                           HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    // TODO(D): Aspect ratio correction
    // TODO(D): Play with stretch modes
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
    Win32MainWindowCallback(HWND Window,
                            UINT Message,
                            WPARAM WParam,
                            LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP: 
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY: 
        {
            GlobalRunning = false;
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 VKCode = WParam;
            bool WasDown = ((LParam & (1 << 30)) != 0);
            bool IsDown = ((LParam & (1 << 31)) == 0);
            if(WasDown != IsDown)
            {
                if (VKCode == 'W'){}
                else if (VKCode == 'A'){}
                else if (VKCode == 'S'){}
                else if (VKCode == 'D'){}
                else if (VKCode == 'Q'){}
                else if (VKCode == 'E'){}
                else if (VKCode == VK_UP){}
                else if (VKCode == VK_LEFT){}
                else if (VKCode == VK_DOWN){}
                else if (VKCode == VK_RIGHT){}
                else if (VKCode == VK_ESCAPE)
                {
                    OutputDebugStringA("ESCAPE: ");
                    if(IsDown)
                    {
                        OutputDebugStringA("IsDown ");
                    }
                    if(WasDown)
                    {
                        OutputDebugStringA("WasDown ");
                    }
                    OutputDebugStringA("\n");
                }
                else if (VKCode == VK_SPACE)
                {
                }
            }

            bool32 AltKeyWasDown = (LParam & (1 << 29));
            if((VKCode == VK_F4) && AltKeyWasDown)
            {
                GlobalRunning = false;
            }
        } break;

        case WM_PAINT: 
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, 
                                       Dimension.Width, Dimension.Height);
            EndPaint(Window, &Paint);
        } break;

        default: 
        {
            // OutputDebugStringA("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return(Result);
}

int CALLBACK 
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    Win32LoadXInput();

    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon;
    WindowClass.lpszClassName = "EngineWindowClass";

    if(RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowEx(
            0,
            WindowClass.lpszClassName,
            "Engine",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0);
        if(Window)
        {
            HDC DeviceContext = GetDC(Window);

            // NOTE(D): Graphics test
            int XOffset = 0;
            int YOffset = 0;

            // NOTE(D): Sound test
            int SamplesPerSecond = 48000;
            int ToneHz = 256;
            uint32 RunningSampleIndex = 0;
            int SquareWavePeriod = SamplesPerSecond / ToneHz;
            int HalfSquareWavePeriod = SquareWavePeriod / 2;
            int BytesPerSample = sizeof(int16) * 2;
            int SecondaryBufferSize = SamplesPerSecond * BytesPerSample;

            Win32InitDSound(Window, SamplesPerSecond, SecondaryBufferSize);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            GlobalRunning = true;
            while(GlobalRunning)
            {
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                // TODO(D): Should we poll this frequently
                for (DWORD ControllerIndex = 0;
                     ControllerIndex < XUSER_MAX_COUNT;
                     ++ControllerIndex)
                {
                    XINPUT_STATE ControllerState;
                    if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                    {
                        // NOTE(D): This controller is plugged in
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                        bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;

                        XOffset += StickX >> 12;
                        YOffset += StickY >> 12;

                        XINPUT_VIBRATION Vibration;
                        if(AButton)
                        {
                            Vibration.wLeftMotorSpeed = 50000;
                            Vibration.wRightMotorSpeed = 50000;
                        }
                        else
                        {
                            Vibration.wLeftMotorSpeed = 0;
                            Vibration.wRightMotorSpeed = 0;
                        }
                        XInputSetState(0, &Vibration);
                    }
                    else
                    {
                        // NOTE(D): The controller is not available
                    }
                }

                RenderWeirdGradient(&GlobalBackbuffer, XOffset, YOffset);

                // NOTE(D): DirectSound output test
                DWORD PlayCursor;
                DWORD WriteCursor;
                if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                {
                    DWORD ByteToLock = RunningSampleIndex*BytesPerSample % SecondaryBufferSize;
                    DWORD BytesToWrite;
                    if(ByteToLock > PlayCursor)
                    {
                        BytesToWrite = SecondaryBufferSize - ByteToLock;
                        BytesToWrite += PlayCursor;
                    }
                    else
                    {
                        BytesToWrite = PlayCursor - ByteToLock;
                    }

                    VOID *Region1;
                    DWORD Region1Size;
                    VOID *Region2;
                    DWORD Region2Size;
                    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock,
                                                              BytesToWrite,
                                                              &Region1, &Region1Size,
                                                              &Region2, &Region2Size,
                                                              0)))
                    {                        
                        // TODO(D): assert that Region1Size/Region2Size is valid
                        DWORD Region1SampleCount = Region1Size/BytesPerSample;
                        int16 *SampleOut = (int16 *)Region1;
                        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
                        {
                            int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? 16000 : -16000;
                            *SampleOut++ = SampleValue;
                            *SampleOut++ = SampleValue;
                        }
                        DWORD Region2SampleCount = Region2Size/BytesPerSample;
                        SampleOut = (int16 *)Region2;
                        for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
                        {
                            int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? 16000 : -16000;
                            *SampleOut++ = SampleValue;
                            *SampleOut++ = SampleValue;
                        }
                    }
                }

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
                                           Dimension.Width, Dimension.Height);
            }
        }
        else
        {
            // TODO(D): Logging
        }
    }  
    else
    {
        // TODO(D): Logging
    }

    return(0);
};