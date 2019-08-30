#include <iostream>
#include <conio.h>

#include "Terminal.h"

namespace owl
{

Terminal::Terminal()
{
}

Terminal::~Terminal()
{
}

std::pair<bool, char> Terminal::getChar()
{
    return std::make_pair(true, 'c');
}

void Terminal::run()
{
    bool done = false;

    while (!done)
    {
        const int c = _getch();

        switch (c)
        {
            default:
            {
                onChar(c);
            }
            break;

            case '\b':
            {
                Q_SIGNAL onBackspace();
            }
            break;

            case '\r':
            {
                done = onEnter();
            }
        }
    }
}

} // namespace
