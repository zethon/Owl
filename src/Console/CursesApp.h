#pragma once
#include <tuple>
#include <curses.h>

namespace owl
{

class CursesApp
{
    WINDOW* _window = nullptr;

public:
    CursesApp();
    ~CursesApp();

    void run();

private:
    std::tuple<int, int> getScreenSize() const;

    void printHome();
    void printBottomMenu();
};

} // namespace
