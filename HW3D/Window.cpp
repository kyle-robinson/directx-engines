#include "Window.h"
#include "resource.h"
#include <sstream>
#include "imgui/imgui_impl_win32.h"

Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept : hInst( GetModuleHandle( nullptr ) )
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof( wc );
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = static_cast<HICON>(LoadImage( hInst, MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 32, 32, 0 ));
	wc.hCursor = LoadCursor( Window::WindowClass::GetInstance(), (LPCWSTR)IDR_ANICURSOR1 );
	wc.hbrBackground = nullptr; // CreateSolidBrush(RGB(255, 0, 0))
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = static_cast<HICON>(LoadImage( hInst, MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 16, 16, 0 ));
	RegisterClassEx( &wc );
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass( wndClassName, GetInstance() );
}

const wchar_t* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}

Window::Window( int width, int height, const char* name ) : width( width ), height( height )
{
	// calculate window size
	RECT wr = { 0 };
	wr.right = width + wr.left;
	wr.bottom = height + wr.bottom;
	if ( AdjustWindowRect( &wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE ) == 0 )
		throw WND_LAST_EXCEPT();

	// convert 'const char*' to 'const wchar_t*'
	std::wstring w;
	std::copy(name, name + strlen(name), back_inserter(w));
	const WCHAR* wChar = w.c_str();

	// create window and get handle
	hWnd = CreateWindow(
		WindowClass::GetName(), wChar,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, WindowClass::GetInstance(), this
	);

	if ( hWnd == nullptr )
		throw WND_LAST_EXCEPT();

	ShowWindow( hWnd, SW_SHOWDEFAULT );

	ImGui_ImplWin32_Init( hWnd );

	// create graphics object
	pGfx = std::make_unique<Graphics>( hWnd, width, height );

	// register mouse raw input device
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = 0u;
	rid.hwndTarget = nullptr;

	if ( RegisterRawInputDevices( &rid, 1, sizeof( rid ) ) == FALSE )
		throw WND_LAST_EXCEPT();
}

Window::~Window()
{
	ImGui_ImplWin32_Shutdown();
	DestroyWindow( hWnd );
	PostQuitMessage( 0 );
}

void Window::EndWindow() const noexcept
{
	ImGui_ImplWin32_Shutdown();
	DestroyWindow( hWnd );
	PostQuitMessage( 0 );
}

void Window::SetTitle( const std::wstring& title )
{
	if ( SetWindowText( hWnd, title.c_str() ) == 0 )
		throw WND_LAST_EXCEPT();
}

void Window::EnableCursor() noexcept
{
	cursorEnabled = true;
	ShowCursor();
	EnableImGuiMouse();
	FreeCursor();
}

void Window::DisableCursor() noexcept
{
	cursorEnabled = false;
	HideCursor();
	DisableImGuiMouse();
	ConfineCursor();
}

bool Window::CursorEnabled() const noexcept
{
	return cursorEnabled;
}

std::optional<int> Window::ProcessMessages() noexcept
{
	MSG msg;
	
	// while the queue has messages - remove and dispatch them
	while ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
	{
		if ( msg.message == WM_QUIT )
		{
			// return optional wrapping int
			return ( int )msg.wParam;
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	// return empty optional when not quitting app
	return { };
}

Graphics& Window::Gfx()
{
	if ( !pGfx )
		throw WND_NOGFX_EXCEPT();
	return *pGfx;
}

void Window::ConfineCursor() noexcept
{
	RECT rect;
	GetClientRect( hWnd, &rect );
	MapWindowPoints( hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2 );
	ClipCursor( &rect );
}

void Window::FreeCursor() noexcept
{
	ClipCursor( nullptr );
}

void Window::ShowCursor() noexcept
{
	while ( ::ShowCursor( TRUE ) < 0 );
}

void Window::HideCursor() noexcept
{
	while ( ::ShowCursor( FALSE ) >= 0 );
}

void Window::EnableImGuiMouse() noexcept
{
	ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void Window::DisableImGuiMouse() noexcept
{
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

LRESULT WINAPI Window::HandleMsgSetup( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
{
	if ( msg == WM_NCCREATE )
	{
		// extract ptr to window class
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		// set WinAPI-managed user data to store ptr to window class
		SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd) );
		// set message proc to normal
		SetWindowLongPtr( hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk) );
		// forward message to the window class handler
		return pWnd->HandleMsg( hWnd, msg, wParam, lParam );
	}
	// if message is received before the WM_NCCREATE message, handle with default handler
	return DefWindowProc( hWnd, msg, wParam, lParam );

}

LRESULT WINAPI Window::HandleMsgThunk( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
{
	// retrieve ptr to window class
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr( hWnd, GWLP_USERDATA ));
	// forward message to window instance handler
	return pWnd->HandleMsg( hWnd, msg, wParam, lParam );
}

LRESULT Window::HandleMsg( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
{
	if ( ImGui_ImplWin32_WndProcHandler( hWnd, msg, wParam, lParam ) )
		return true;

	switch ( msg )
	{
	case WM_CLOSE:
		EndWindow();
		return 0;

	case WM_KILLFOCUS:
		kbd.ClearState();
		break;

	case WM_ACTIVATE:
		if ( !cursorEnabled )
		{
			if ( wParam & WA_ACTIVE || wParam & WA_CLICKACTIVE )
			{
				ConfineCursor();
				HideCursor();
			}
			else
			{
				FreeCursor();
				ShowCursor();
			}
		}
		break;

	/* Keyboard Messages */
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if ( ImGui::GetIO().WantCaptureKeyboard )
			break;
		if ( !( lParam & 0x40000000 ) || kbd.AutorepeatIsEnabled() )
			kbd.OnKeyPressed( static_cast<unsigned char>(wParam) );
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		if ( ImGui::GetIO().WantCaptureKeyboard )
			break;
		kbd.OnKeyReleased( static_cast<unsigned char>(wParam) );
		break;

	case WM_CHAR:
		if ( ImGui::GetIO().WantCaptureKeyboard )
			break;
		kbd.OnChar( static_cast<unsigned char>(wParam) );
		break;
	/* End Keyboard Messages */

	/* Mouse Messages */
	case WM_MOUSEMOVE:
	{
		if ( !cursorEnabled )
		{
			if ( !mouse.IsInWindow() )
			{
				SetCapture( hWnd );
				mouse.OnMouseEnter();
				HideCursor();
			}
		}

		// stifle mouse message if imgui wants to capture mouse
		if ( ImGui::GetIO().WantCaptureMouse )
			break;

		const POINTS pt = MAKEPOINTS( lParam );
		// in client region
		if ( pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height )
		{
			mouse.OnMouseMove( pt.x, pt.y );
			if ( !mouse.IsInWindow() )
			{
				SetCapture( hWnd );
				mouse.OnMouseEnter();
			}
		}
		// not in client
		else
		{
			if ( wParam & ( MK_LBUTTON | MK_RBUTTON ) )
			{
				mouse.OnMouseMove( pt.x, pt.y );
			}
			else
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
		}
		break;
	}

	case WM_MOUSEHOVER:
	{
		SetCursor( LoadCursor( Window::WindowClass::GetInstance(), (LPCWSTR)IDR_ANICURSOR1 ) );
		break;
	}

	case WM_LBUTTONDOWN:
	{
		SetForegroundWindow( hWnd );
		if ( !cursorEnabled )
		{
			ConfineCursor();
			HideCursor();
		}

		SetCursor( LoadCursor( Window::WindowClass::GetInstance(), (LPCWSTR)IDR_ANICURSOR2 ));

		if ( ImGui::GetIO().WantCaptureMouse )
			break;
		const POINTS pt = MAKEPOINTS( lParam );
		mouse.OnLeftPressed( pt.x, pt.y );
		break;
	}

	case WM_RBUTTONDOWN:
	{
		if ( ImGui::GetIO().WantCaptureMouse )
			break;
		const POINTS pt = MAKEPOINTS( lParam );
		mouse.OnRightPressed( pt.x, pt.y );
		break;
	}

	case WM_LBUTTONUP:
	{
		SetCursor(LoadCursor(Window::WindowClass::GetInstance(), (LPCWSTR)IDR_ANICURSOR1));
		
		if ( ImGui::GetIO().WantCaptureMouse )
			break;
		const POINTS pt = MAKEPOINTS( lParam );
		mouse.OnLeftReleased( pt.x, pt.y );
		break;
	}

	case WM_RBUTTONUP:
	{
		if ( ImGui::GetIO().WantCaptureMouse )
			break;
		const POINTS pt = MAKEPOINTS( lParam );
		mouse.OnRightReleased( pt.x, pt.y );
		break;
	}

	case WM_MOUSEWHEEL:
	{
		if ( ImGui::GetIO().WantCaptureMouse )
			break;
		const POINTS pt = MAKEPOINTS( lParam );
		const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
		mouse.OnWheelDelta( pt.x, pt.y, delta );
		/*if ( GET_WHEEL_DELTA_WPARAM( wParam ) > 0 )
		{
			mouse.OnWheelUp( pt.x, pt.y );
		}
		else if ( GET_WHEEL_DELTA_WPARAM( wParam ) < 0 )
		{
			mouse.OnWheelDown( pt.x, pt.y );
		}*/
		break;
	}
	/* End of Mouse Messages */

	/* Raw Mouse Messages */
	case WM_INPUT:
	{
		UINT size;
		// get the size of the input data
		if ( GetRawInputData(
			reinterpret_cast<HRAWINPUT>(lParam),
			RID_INPUT,
			nullptr,
			&size,
			sizeof( RAWINPUTHEADER ) ) == -1 )
		{
			break;
		}
		rawBuffer.resize( size );

		if ( GetRawInputData(
			reinterpret_cast<HRAWINPUT>(lParam),
			RID_INPUT,
			rawBuffer.data(),
			&size,
			sizeof( RAWINPUTHEADER ) ) != size )
		{
			break;
		}

		auto& ri = reinterpret_cast<const RAWINPUT&>( *rawBuffer.data() );
		if ( ri.header.dwType == RIM_TYPEMOUSE && ( ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0 ) )
		{
			mouse.OnRawDelta( ri.data.mouse.lLastX, ri.data.mouse.lLastY );
		}

		break;
	}
	/* End of Raw Mouse Messages */
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

Window::HrException::HrException( int line, const char* file, HRESULT hr ) noexcept : WindowException( line, file ), hr( hr ) { }

const char* Window::HrException::what() const noexcept
{	
	std::stringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept
{
	return "Window Exception";
}

std::string Window::WindowException::TranslateErrorCode( HRESULT hr ) noexcept
{
	char* pMsgBuf = nullptr;
	DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		reinterpret_cast<LPWSTR>(&pMsgBuf), 0, nullptr
	);
	
	if ( nMsgLen == 0 )
		return "Unidentified Error Code";

	std::string errorString = pMsgBuf;
	LocalFree( pMsgBuf );
	return errorString;
}

HRESULT Window::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::HrException::GetErrorDescription() const noexcept
{
	return WindowException::TranslateErrorCode( hr );
}

const char* Window::NoGfxException::GetType() const noexcept
{
	return "Window Exception [No Graphics]";
}