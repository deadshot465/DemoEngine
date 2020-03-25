#pragma once
#include <string>
#include <string_view>

class IWindow
{
public:
	IWindow(int width, int height, std::string_view title, bool fullScreen);
	virtual ~IWindow() = default;

protected:
	int mWidth = 0;
	int mHeight = 0;
	std::string mTitle = "";
	bool mIsInitialized = false;
	bool mIsFullScreen = false;
};
