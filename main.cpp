#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

// typedef unsigned char uint8;

global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;

global_variable int BitmapWidth;
global_variable int BitmapHeight;

global_variable int BytePerPixel = 4;

internal void DrawGradient(int XOffset, int YOffset)
{
    int Pitch = BitmapInfo.bmiHeader.biWidth * BytePerPixel;
    uint8_t *Row = (uint8_t *) BitmapMemory;

    for(int Y = 0; Y < BitmapHeight; ++Y)
    {
        uint8_t *Pixel = (uint8_t *) Row;
        for(int X = 0; X < BitmapWidth; ++X)
        {
            *Pixel = (uint8_t) (XOffset + X);
            ++Pixel;

            *Pixel = (uint8_t) (YOffset + Y);
            ++Pixel;

            *Pixel = 0;
            ++Pixel;

            *Pixel = 0;
            ++Pixel;
        }
        Row += Pitch;
    }
}

internal void Win32ResizeDIBSection(int Width, int Height)
{

    if(BitmapMemory)
    {
        VirtualFree(0, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = BitmapWidth * BitmapHeight * BytePerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = WindowRect->right - WindowRect->left;
    int WindowHeight = WindowRect->bottom - WindowRect->top;

    StretchDIBits(DeviceContext,
                    0, 0, BitmapWidth, BitmapHeight,
                    0, 0, WindowWidth, WindowHeight,
                    BitmapMemory,
                    &BitmapInfo,
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
            RECT ClientRect;
            GetClientRect(hwnd, &ClientRect);
            int Height = ClientRect.bottom - ClientRect.top;
            int Width = ClientRect.right - ClientRect.left;
            Win32ResizeDIBSection(Width, Height);
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
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            RECT ClientRect;
            GetClientRect(hwnd, &ClientRect);
            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
            EndPaint(hwnd, &Paint);
        } break;

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

                DrawGradient(XOffset, YOffset);
                HDC DeviceContext = GetDC(WindowHandle);
                RECT ClientRect;
                GetClientRect(WindowHandle, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
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