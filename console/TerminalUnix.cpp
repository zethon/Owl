#include <iostream>
#include <unistd.h>
#include <termios.h>
#include "Terminal.h"

namespace owl
{

Terminal::Terminal()
{
    termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

Terminal::~Terminal()
{
    termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

std::pair<bool, char> Terminal::getChar()
{
    using namespace std;

    bool bSuccess = false;
    char retchar;

    // get a keystroke
    while (true)
    {
        if (cin.fail())
        {
            break;
        }

        retchar = cin.get();
        bSuccess = true;
        break;
    }

    return std::make_pair(bSuccess, retchar);
}

void Terminal::run()
{
    bool bDone = false;

    while (!bDone)
    {
        char c1 = 0;
        char c2 = 0;

        const auto retpair = getChar();
        if (!retpair.first)
        {
            break;
        }

        c1 = retpair.second;

        if (c1 == 27)
        {
            std::tie(std::ignore, c2) = getChar();
        }

        if (c1 == 27 && c2 == '[')
        {
            char c3;
            std::tie(std::ignore, c3) = getChar();
        }
        else if (c1 == 0x08 || c1 == 0x7f)
        {
            onBackspace();
        }
        else if (c1 == 0x0A)
        {
            bDone = onEnter();
        }
        else
        {
            onChar(c1);
        }
    }
}

} // namespace

