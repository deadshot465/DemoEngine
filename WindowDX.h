#pragma once
#include <chrono>
#include <memory>
#include <string_view>
#include <Windows.h>
#include "IWindow.h"

namespace DX
{
	namespace DX11
	{
		class GraphicsDevice;
	}

	namespace DX12
	{
		class GraphicsDevice;
	}

	class Window
		: public IWindow
	{
	public:
		Window(int width, int height, std::string_view title, bool fullScreen = false);
		~Window();
		
		bool Initialize(bool dx12);
		void Broadcast();

	private:
		ATOM Register();
		void Create();
		void SetHwnd(HWND hwnd);
		void OnCreate();
		void OnUpdate();
		void OnDestroy();

		RECT GetClientRect() const noexcept;

		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		HWND mHwnd = nullptr;
		DEVMODEW mDevMode = {};
		std::unique_ptr<DX11::GraphicsDevice> mGraphicsDevice = nullptr;
		std::unique_ptr<DX12::GraphicsDevice> mGraphicsDeviceDX12 = nullptr;
		std::chrono::time_point<std::chrono::steady_clock> mLastTime;
		float mElapsedTime = 0.0f;
		bool mDx12 = false;
	};
}

