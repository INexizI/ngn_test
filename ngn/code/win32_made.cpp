#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#define internal static
#define local_presist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

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

win32_window_dimension
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
RenderWeirdGradient(win32_offscreen_buffer Buffer, int BlueOffset, int GreenOffset)
{
    uint8 *Row = (uint8 *)Buffer.Memory;
    for (int Y = 0;
         Y < Buffer.Height;
         ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer.Width; ++X)
        {
            uint8 Blue = (X + BlueOffset);
            uint8 Green = (Y + GreenOffset);
            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Buffer.Pitch;
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
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Buffer->Width * BytesPerPixel;

    // TODO(D): Clear this to black
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, 
                           int WindowWidth, int WindowHeight,
                           win32_offscreen_buffer Buffer,
                           int X, int Y, int Width, int Height)
{
    // TODO(D): Aspect ratio correction
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer.Width, Buffer.Height,
                  Buffer.Memory,
                  &Buffer.Info,
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

        case WM_PAINT: 
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                                       GlobalBackbuffer, X, Y, Width, Height);
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
    WNDCLASS WindowClass = {};

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

            int XOffset = 0;
            int YOffset = 0;
            
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
                for ()
                {
                }
                

                RenderWeirdGradient(GlobalBackbuffer, XOffset, YOffset);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                                  GlobalBackbuffer,
                                  0, 0, Dimension.Width, Dimension.Height);

                ++XOffset;
                YOffset += 2;
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