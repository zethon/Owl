#include <QMainWindow>
#include <QMenuBar>
#include <QDebug>
#include "windows.h"

#define IDM_SHOWMENU 0x010

extern "C" void setupTitleBar(WId winId)
{
    if (winId == 0) return;

    HMENU hMenu = ::GetSystemMenu((HWND)(winId), FALSE);
    if (hMenu == NULL) return;

    ::AppendMenuA(hMenu, MF_SEPARATOR, 0, 0);
    ::AppendMenuA(hMenu, MF_STRING, IDM_SHOWMENU, "Show menu");
}

extern "C" void setShowMenuText(WId winId, const char* text)
{
    if (winId == 0) return;
    
    HMENU hMenu = ::GetSystemMenu((HWND)(winId), FALSE);
    if (hMenu == NULL) return;

    auto count = ::GetMenuItemCount(hMenu);
    if (count > 0)
    {
        ::ModifyMenuA(hMenu, count - 1, MF_BYPOSITION, IDM_SHOWMENU, text);
    }
}

extern "C" bool handleWindowsEvent(const QMainWindow& window, void* payload, long* result)
{
    MSG* m = (MSG*)payload;

    if (m->message == WM_SYSCOMMAND)
    {
        if ((m->wParam & 0xfff0) == IDM_SHOWMENU)
        {
            window.menuBar()->setVisible(true);
            *result = 0;
            return true;
        }
    }
    return false;
}
