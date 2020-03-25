#include "IWindow.h"

IWindow::IWindow(int width, int height, std::string_view title, bool fullScreen)
	: mWidth(width), mHeight(height), mTitle(title), mIsFullScreen(fullScreen)
{
}
