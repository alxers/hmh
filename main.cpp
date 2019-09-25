#include <windows.h>
#include <stdint.h>
#include <dsound.h>

#define internal static
#define local_persist static
#define global_variable static

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytePerPixel;

};

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackbuffer;

struct win32_window_dimension
{
    int Width;
    int Height;
};

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Height = ClientRect.bottom - ClientRect.top;
    Result.Width = ClientRect.right - ClientRect.left;

    return(Result);
}

typedef HRESULT Direct_Sound_Create(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter);
internal void win32InitSound(HWND Window, int bufferSize, int samplesPerSec)
{
    // Load library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if(DSoundLibrary)
    {    
        Direct_Sound_Create *direct_sound_create = (Direct_Sound_Create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        LPDIRECTSOUND directSound;
        if(SUCCEEDED(direct_sound_create(0, &directSound, 0)))
        {

            WAVEFORMATEX pcfxFormat = {};
            pcfxFormat.wFormatTag = WAVE_FORMAT_PCM;
            pcfxFormat.nChannels = 2;
            pcfxFormat.nSamplesPerSec = samplesPerSec;
            pcfxFormat.wBitsPerSample = 16;
            pcfxFormat.nBlockAlign = (pcfxFormat.nChannels * pcfxFormat.wBitsPerSample) / 8;
            pcfxFormat.nAvgBytesPerSec = pcfxFormat.nSamplesPerSec * pcfxFormat.nBlockAlign;
            pcfxFormat.cbSize = 0;

            if(SUCCEEDED(directSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC bufferDescription = {};
                bufferDescription.dwSize = sizeof(bufferDescription);
                bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER primaryBuffer;
                if(SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
                {
                    if(SUCCEEDED(primaryBuffer->SetFormat(&pcfxFormat)))
                    {
                        //
                    }
                }
            }

            DSBUFFERDESC bufferDescription = {};
            bufferDescription.dwSize = sizeof(bufferDescription);
            bufferDescription.dwFlags = 0;
            bufferDescription.dwBufferBytes = bufferSize;
            bufferDescription.lpwfxFormat = &pcfxFormat;

            LPDIRECTSOUNDBUFFER secondBuffer;
            if(SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &secondBuffer, 0)))
            {
            }
        }
    }
}

internal void DrawGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{
    uint8_t *Row = (uint8_t *) Buffer.Memory;

    for(int Y = 0; Y < Buffer.Height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *) Row;
        for(int X = 0; X < Buffer.Width; ++X)
        {
            uint8_t Blue = (X + XOffset);
            uint8_t Green = (Y + YOffset);
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer.Pitch;
    }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{

    if(Buffer->Memory)
    {
        VirtualFree(0, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytePerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = Buffer->Width * Buffer->Height * Buffer->BytePerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Buffer->Info.bmiHeader.biWidth * Buffer->BytePerPixel;
}

internal void Win32DisplayBufferInWindow(win32_offscreen_buffer Buffer,
                                            HDC DeviceContext,
                                            int WindowWidth,
                                            int WindowHeight,
                                            int X,
                                            int Y
                                        )
{
    StretchDIBits(DeviceContext,
                    0, 0, WindowWidth, WindowHeight,
                    0, 0, Buffer.Width, Buffer.Height,
                    Buffer.Memory,
                    &Buffer.Info,
                    DIB_RGB_COLORS,
                    SRCCOPY);
}

LRESULT CALLBACK MainWindowCallback(
    HWND   hwnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    LRESULT result = 0;
    switch(uMsg)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_DESTROY:
        {
            Running = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_CLOSE:
        {
            Running = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(hwnd, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;

            win32_window_dimension Dim = Win32GetWindowDimension(hwnd);
            Win32DisplayBufferInWindow(GlobalBackbuffer, DeviceContext, Dim.Width, Dim.Height, X, Y);
            EndPaint(hwnd, &Paint);
        } break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32_t VKCode = wParam;
            if(VKCode == 'W')
            {
                OutputDebugStringA("W pressed");
            }
        }

        default:
        {
            OutputDebugStringA("default\n");
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }

    return(result);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{

    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = "HMHWindowClass";

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    if(RegisterClass(&WindowClass))
    {
        // create window here
        HWND WindowHandle = CreateWindowExA( 
            0,
            WindowClass.lpszClassName,
            "HMHWindowName",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            hInstance,
            0
        );

        if(WindowHandle)
        {
            Running = true;

            int XOffset = 0;
            int YOffset = 0;

            win32InitSound(WindowHandle, 48000, 48000);

            while(Running)
            {
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                DrawGradient(GlobalBackbuffer, XOffset, YOffset);
                HDC DeviceContext = GetDC(WindowHandle);

                win32_window_dimension Dim = Win32GetWindowDimension(WindowHandle);
                Win32DisplayBufferInWindow(GlobalBackbuffer, DeviceContext, Dim.Width, Dim.Height, 0, 0);
                ReleaseDC(WindowHandle, DeviceContext);

                ++XOffset;
            }
        }
        else
        {
            //
        }
    }
    else
    {
        //
    }

    return 0;
}