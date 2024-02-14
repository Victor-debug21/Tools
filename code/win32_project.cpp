#include<windows.h>

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
};

struct win32_window_dimension
{
  int Width;
  int Height;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;

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
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
  if(Buffer->Memory)
    {
      VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

  Buffer->Width = Width;
  Buffer->Height = Height;

  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;

  int BytesPerPixel = 4;
  int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  Buffer->Pitch = Width*BytesPerPixel;

}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
  StretchDIBits(DeviceContext,
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory,
		&Buffer->Info,
		DIB_RGB_COLORS,
		SRCCOPY);

}

LRESULT CALLBACK
Win32MainWindowCallBack(HWND Window,
			UINT Message,
			WPARAM WParam,
			LPARAM LParam)
{
  LRESULT Result = 0;
  switch(Message)
    {
    case WM_CLOSE:
      {
	GlobalRunning = false;
      }break;
    case WM_ACTIVATEAPP:
      {
	OutputDebugStringA("WM_ACTIVATEAPP");
      }break;
    case WM_DESTROY:
      {
	GlobalRunning = false;
      }break;
    case WM_PAINT:
      {
	PAINTSTRUCT Paint;
	HDC DeviceContext = BeginPaint(Window, &Paint);
	win32_window_dimension Dimension = Win32GetWindowDimension(Window);
	Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
	EndPaint(Window, &Paint);
      }break;
    default:
      {
	Result = DefWindowProc(Window, Message, WParam, LParam);
      }
      
    }
  return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode)
{

  WNDCLASSA WindowClass = {};
  Win32ResizeDIBSection(&GlobalBackBuffer, 1920, 1080);
  WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
  WindowClass.lpfnWndProc = Win32MainWindowCallBack;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "??????";

  if(RegisterClassA(&WindowClass))
    {
      HWND Window = CreateWindowExA(0,
				    WindowClass.lpszClassName,
				    "?????",
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

	      win32_window_dimension Dimension = Win32GetWindowDimension(Window);
	      Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
	    }
	}
      else
	{
	}
    }
  else
    {
    }

  return(0);
}
