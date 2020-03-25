#include "WindowDX.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cassert>
#include "GraphicsDeviceDX11.h"
#include "GraphicsDeviceDX12.h"

DX::Window::Window(int width, int height, std::string_view title, bool fullScreen)
	: IWindow(width, height, title, fullScreen)
{
}

DX::Window::~Window()
{
	mGraphicsDevice.reset();
	mGraphicsDeviceDX12.reset();

	DestroyWindow(mHwnd);

	UnregisterClassW(L"MyWindowClass", nullptr);
}

bool DX::Window::Initialize(bool dx12)
{
	mDx12 = dx12;

	try
	{
		if (Register())
			Create();
		else
			throw std::runtime_error("Failed to register the window.\n");
	}
	catch (const std::exception&)
	{
		throw;
	}

	return mIsInitialized;
}

void DX::Window::Broadcast()
{
	MSG msg = {};
	
	while (PeekMessageW(&msg, mHwnd, NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	Sleep(1);
}

ATOM DX::Window::Register()
{
	WNDCLASSEXW wc = {};
	ZeroMemory(&wc, sizeof(WNDCLASSEXW));
	wc.cbClsExtra = NULL;
	wc.cbSize = static_cast<UINT>(sizeof(WNDCLASSEXW));
	wc.cbWndExtra = NULL;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	wc.hCursor = nullptr;
	wc.hIcon = nullptr;
	wc.hIconSm = nullptr;
	wc.hInstance = nullptr;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"MyWindowClass";
	wc.lpszMenuName = L"";
	wc.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;

	return RegisterClassExW(&wc);
}

void DX::Window::Create()
{
	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	int x = 0;
	int y = 0;

	if (mIsFullScreen)
	{
		ZeroMemory(&mDevMode, sizeof(DEVMODEW));
		mDevMode.dmSize = static_cast<WORD>(sizeof(DEVMODEW));
		mDevMode.dmBitsPerPel = 32ul;
		mDevMode.dmPelsHeight = static_cast<DWORD>(screen_height);
		mDevMode.dmPelsWidth = static_cast<DWORD>(screen_width);
		mDevMode.dmFields = DM_BITSPERPEL | DM_PELSHEIGHT | DM_PELSWIDTH;
		ChangeDisplaySettingsW(&mDevMode, CDS_FULLSCREEN);
	}
	else
	{
		screen_width = mWidth;
		screen_height = mHeight;
		x = (GetSystemMetrics(SM_CXSCREEN) - screen_width) / 2;
		y = (GetSystemMetrics(SM_CYSCREEN) - screen_height) / 2;
	}

	auto title = std::wstringstream();
	for (int i = 0; i < mTitle.size(); ++i)
	{
		title << mTitle[i];
	}

	mHwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, L"MyWindowClass", title.str().c_str(), WS_OVERLAPPEDWINDOW, x, y, screen_width, screen_height, nullptr, nullptr, nullptr, this);

	if (!mHwnd)
		throw std::runtime_error("Failed to create the window.\n");

	SetWindowLongPtrW(mHwnd, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(LoadCursorW(nullptr, IDC_ARROW)));
	SetWindowLongPtrW(mHwnd, GCLP_HICON, reinterpret_cast<LONG_PTR>(LoadIcon(nullptr, IDI_APPLICATION)));
	SetWindowLongPtrW(mHwnd, GCLP_HICONSM, reinterpret_cast<LONG_PTR>(LoadIcon(nullptr, IDI_APPLICATION)));

	ShowWindow(mHwnd, SW_SHOW);
	SetForegroundWindow(mHwnd);
	SetFocus(mHwnd);

	UpdateWindow(mHwnd);

	mIsInitialized = true;
}

void DX::Window::SetHwnd(HWND hwnd)
{
	mHwnd = hwnd;
}

void DX::Window::OnCreate()
{
	auto rect = GetClientRect();
	auto width = rect.right - rect.left;
	auto height = rect.bottom - rect.top;
	
	if (mDx12)
		mGraphicsDeviceDX12 = std::make_unique<DX12::GraphicsDevice>(mHwnd, static_cast<int32_t>(width), static_cast<int32_t>(height));
	else
		mGraphicsDevice = std::make_unique<DX11::GraphicsDevice>(mHwnd, static_cast<int>(width), static_cast<int>(height));

	mLastTime = std::chrono::high_resolution_clock::now();
}

void DX::Window::OnUpdate()
{
	assert(mGraphicsDevice || mGraphicsDeviceDX12);

	auto current_time = std::chrono::high_resolution_clock::now();
	auto elapsed_time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - mLastTime).count();

	if (mDx12)
	{
		mGraphicsDeviceDX12->Render(elapsed_time);
	}
	else
	{
		mGraphicsDevice->Render(elapsed_time);
	}

	mLastTime = current_time;
}

void DX::Window::OnDestroy()
{
	mIsInitialized = false;
}

RECT DX::Window::GetClientRect() const noexcept
{
	auto rect = RECT();
	::GetClientRect(mHwnd, &rect);
	return rect;
}

LRESULT DX::Window::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		auto window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		window->OnUpdate();
		break;
	}
	case WM_CREATE:
	{
		auto window = reinterpret_cast<Window*>(reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
		window->SetHwnd(hwnd);
		window->OnCreate();
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		break;
	}
	case WM_DESTROY:
	{
		auto window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		window->OnDestroy();
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProcW(hwnd, msg, wparam, lparam);
	}

	return NULL;
}
