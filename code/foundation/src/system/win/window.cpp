#include "zeus/foundation/system/win/window.h"
#include "zeus/foundation/string/charset_utils.h"
#ifdef _WIN32
#include <Windows.h>
namespace
{
BOOL __stdcall EnumWindowsProc(HWND hwnd, LPARAM param)
{
    auto callback = reinterpret_cast<std::function<bool(HWND hwnd)>*>(param);
    return (*callback)(hwnd);
}
}
namespace zeus
{
std::string WinWindow::GetWindowClass(HWND hwnd)
{
    char classname[MAX_PATH + 1] = {0};
    auto classSize               = GetClassNameA(hwnd, classname, MAX_PATH);
    if (classSize)
    {
        return std::string(classname, classSize);
    }
    else
    {
        return "";
    }
}
std::string WinWindow::GetWindowTitle(HWND hwnd)
{
    if (!IsHungAppWindow(hwnd))
    {
        wchar_t title[MAX_PATH + 1] = {0};
        auto    titleSize           = GetWindowTextW(hwnd, title, MAX_PATH);
        if (titleSize)
        {
            return zeus::CharsetUtils::UnicodeToUTF8(title);
        }
    }
    return "";
}
DWORD WinWindow::GetWindowPid(HWND hwnd)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}
HWND WinWindow::GetWindowRoot(HWND hwnd)
{
    return GetAncestor(hwnd, GA_ROOT);
}
HWND WinWindow::GetWindowParent(HWND hwnd)
{
    return GetAncestor(hwnd, GA_PARENT);
}
RECT WinWindow::GetWindowPos(HWND hwnd)
{
    RECT rect = {};
    GetWindowRect(hwnd, &rect);
    return rect;
}
bool WinWindow::IsExist(HWND hwnd)
{
    return IsWindow(hwnd);
}
bool WinWindow::IsVisible(HWND hwnd)
{
    return IsWindowVisible(hwnd);
}
bool WinWindow::IsEnable(HWND hwnd)
{
    return IsWindowEnabled(hwnd);
}
bool WinWindow::IsMinimized(HWND hwnd)
{
    return IsIconic(hwnd);
}
bool WinWindow::IsMaximized(HWND hwnd)
{
    return IsZoomed(hwnd);
}
bool WinWindow::IsChild(HWND hwnd)
{
    return GetStyle(hwnd) & WS_CHILD;
}
bool WinWindow::IsHung(HWND hwnd)
{
    return IsHungAppWindow(hwnd);
}
DWORD WinWindow::GetStyle(HWND hwnd)
{
    return ::GetWindowLongW(hwnd, GWL_STYLE);
}
DWORD WinWindow::GetExtraStyle(HWND hwnd)
{
    return ::GetWindowLongW(hwnd, GWL_EXSTYLE);
}
bool WinWindow::EnumWindow(const std::function<bool(HWND hwnd)>& callback)
{
    return ::EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&callback));
}
bool WinWindow::EnumChildWindow(HWND parentWnd, const std::function<bool(HWND hwnd)>& callback)
{
    return ::EnumChildWindows(parentWnd, EnumWindowsProc, reinterpret_cast<LPARAM>(&callback));
}
} // namespace zeus
#endif
