#pragma once
#include <tuple>
#include <curses.h>

namespace owl
{

class CursesApp
{
    WINDOW* _window = nullptr;
    bool _showdebuginfo = false;

public:
    CursesApp();
    ~CursesApp();

    void run();

    WINDOW* window() const { return _window; }

private:
    std::tuple<int, int> getScreenSize() const;

    void printHome();
    void printBottomMenu();
    void doMainMenu();
};

} // namespace
