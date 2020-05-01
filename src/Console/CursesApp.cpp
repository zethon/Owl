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
    doMainMenu();
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

    attron(COLOR_PAIR(owl::color_pair("MenuBar")));
    const auto menu =
        fmt::format("{:<{}}", "[?]Help [q]Quit [+]New Board", std::get<0>(xy));
    addstr(menu.c_str());
    attroff(COLOR_PAIR(owl::color_pair("MenuBar")));
}

void CursesApp::printHome()
{
    auto [x,y] = getScreenSize();

    ColorScope cs{ _window, owl::color_pair("WelcomeText") };
    const std::string message = fmt::format(" :: {} :: ", APP_TITLE);
    cs.printXY(x - static_cast<int>(message.size() + 1), 0, message);

    printBottomMenu();

    if (_showdebuginfo)
    {
        const std::string debugInfo =
            fmt::format("X: {}, Y: {}, COLORS: {}, COLOR_PAIRS: {}", x, y, COLORS, COLOR_PAIRS);
        cs.reset(owl::color_pair("DebugInfo"));
        cs.printXY(0, 0, debugInfo);
    }
}

// Board Name, URL, username, last connected
using MockData = std::tuple<std::string, std::string, std::string, std::string>;
const MockData mockData[] =
{
    { "AMB", "anothermessageboard.com", "Max Power", "2020-04-30" },
    { "AMB", "anothermessageboard.com", "AltUser", "2020-04-03" },
    { "JUOT", "juot.net", "zethon", "2020-04-03" },
    { "O&A Forums", "onaforums.net", "Hermano Joe", "2020-04-12" },
    { "Reddit", "reddit.com", "qizxo", "2020-04-30" },
    { "ShuttyBoard", "shittyboard.com/forums", "Wolosocu", "2020-03-24" }
};

void printMainMenu(const CursesApp& app, std::uint8_t selection)
{
    int width = 0;
    int height = 0;
    getmaxyx(app.window(), height, width);

    const std::string newBoard = "Add new board";
    const std::string header =
        fmt::format("{:<15} {:<30} {:<15} {:20}",
            "Board", "URL", "Username", "Last Login");

    int startX = static_cast<int>((width / 2) - header.size() / 2);
    int startY = 5;

    ColorScope ui { app.window(), "MenuItem" };

    if (0 == selection)
    {
        ui.reset("MenuItemSelected");
    }

    ui.printXY(startX, startY++, newBoard);
    startY++;

    ui.reset("MenuHeader");
    ui.printXY(startX, startY++, header);

    for (const auto& element : boost::adaptors::index(mockData, 1))
    {
        const auto& [board, url, username, lastdate] = element.value();
        std::string selectionStyle{ "MenuItem" };
        if (element.index() == selection) selectionStyle = "MenuItemSelected";
        ui.reset(selectionStyle);

        auto line = fmt::format("{:<15} {:<30} {:<15} {:<20}", board, url, username, lastdate);
        boost::algorithm::trim(line);
        ui.printXY(startX, startY++, line.c_str());
    }
}

void doInput()
{
    int width = 0;
    int height = 0;
    getmaxyx(stdscr, height, width);

    echo();
    cbreak();
    curs_set(1);
    move(height - 2, 0);

    getch();
}

void CursesApp::doMainMenu()
{
    std::uint8_t selection = 0;

    bool done = false;
    while (!done)
    {
        printHome();
        printMainMenu(*this, selection);
        auto ch = getch();
        switch (ch)
        {
            case KEY_DOWN:
            {
                if (selection < std::size(mockData)) selection++;
                break;
            }

            case KEY_UP:
            {
                if (selection > 0) selection--;
                break;
            }

            case KEY_END:
            {
                print_color_settings();
                clear();
                break;
            }

            case 'Q':
            case 'q':
            {
                done = true;
                break;
            }

            case 'D':
            case 'd':
            {
                _showdebuginfo = !_showdebuginfo;
                clear();
                break;
            }

            case KEY_HOME:
            {
                doInput();
                break;
            }
        }
    }
}

} // namespace
