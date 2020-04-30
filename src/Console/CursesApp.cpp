#include <iostream>
#include <string>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <fmt/format.h>

#include "Core.h"
#include "ColorScope.h"

#include "CursesApp.h"

namespace owl
{

namespace
{

void init_color_scheme()
{
    start_color();

    for (const auto& [code, rgb] : COLOR_CODES)
    {
        const auto& [r, g, b] = rgb;
        init_color(code, r, g, b);
    }

    for (const auto& element : boost::adaptors::index(DEFAULT_THEME)) 
    {
        if (element.index() == 0) continue;
        const auto& [name, fg, bg] = element.value();

        init_pair(static_cast<int>(element.index()), fg, bg);
    }
}

void print_color_settings()
{
    clear();

    constexpr int startX = 2;
    int startY = 2;

    for (const auto& element : boost::adaptors::index(DEFAULT_THEME))
    {
        if (element.index() == 0) continue;

        move(startY + static_cast<int>(element.index()), startX);
        const ColorPairInfo& info = element.value();
        attron(COLOR_PAIR(element.index()));
        addstr(fmt::format("{:^30}", info.name, 10).c_str());
        attroff(COLOR_PAIR(element.index()));
    }

    startY += static_cast<int>(std::size(DEFAULT_THEME) + 2);
    for (const auto& element : boost::adaptors::index(DEFAULT_THEME))
    {
        if (element.index() == 0) continue;

        move(startY + static_cast<int>(element.index()), startX);
        const ColorPairInfo& info = element.value();
        attron(COLOR_PAIR(element.index()));
        attron(A_BOLD);
        addstr(fmt::format("{:^30}", info.name, 10).c_str());
        attroff(COLOR_PAIR(element.index()));
        attroff(A_BOLD);
    }

    refresh();
    getch();
}

}

CursesApp::CursesApp()
{
    _window = initscr();
    curs_set(0); // hide cursor
    keypad(stdscr, TRUE);
    cbreak();
    noecho();

    if (!has_colors())
    {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    init_color_scheme();
}

CursesApp::~CursesApp()
{
    endwin();
}

void CursesApp::run()
{
    bool done = false;

    while (!done)
    {
        printHome();

        auto ch = getch();
        switch (ch)
        {
            case KEY_END:
            {
                print_color_settings();
                clear();
                break;
            }

            case 'q':
            case 'Q':
            {
                done = true;
                break;
            }
        }
    }
}

std::tuple<int, int> CursesApp::getScreenSize() const
{
    int x = 0;
    int y = 0;
    getmaxyx(_window, y, x);
    return { x, y };
}

void CursesApp::printBottomMenu()
{
    auto xy = getScreenSize();
    auto y = std::get<1>(xy);

    move(y-1, 0);

    attron(COLOR_PAIR(color_pair_num("MenuBar")));
    const auto menu =
        fmt::format("{:<{}}", "[?]Help [q]Quit [/]Prompt [u]Login", std::get<0>(xy));
    addstr(menu.c_str());
    attroff(COLOR_PAIR(color_pair_num("MenuBar")));
}

void CursesApp::printHome()
{
    auto [x,y] = getScreenSize();

    ColorScope cs{ _window, color_pair_num("WelcomeText"), true };
    const std::string message = fmt::format(" :: {} :: ", APP_TITLE);
    cs.printXY(x - static_cast<int>(message.size() + 1), 0, message);

    const std::string debugInfo =
        fmt::format("X: {}, Y: {}, COLORS: {}, COLOR_PAIRS: {}", x, y, COLORS, COLOR_PAIRS);
    cs.reset(color_pair_num("DebugInfo"));
    cs.printXY(0, 0, debugInfo);
//    cs.reset();

    cs.reset(color_pair_num("Seperator"));
    {
//        auto smallwin = newwin(2, 2, 5, 5);
        auto win = newwin(2, 2, 5, 5);
        ColorScope smallcs{ win, color_pair_num("BoxOutline") };
        box(win, 0, 0);
//        smallcs.printXY(2, 2, "Hi there!");
    }
//    move(10,20);
//    box(_window, 0, 0);


    cs.reset(color_pair_num("BoxOutline"));
    cs.drawHorizontalLine(10, 20, 30);
//    cs.reset();

    printBottomMenu();
    move(y - 1, x - 1);
    refresh();
}

} // namespace
