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

std::pair<bool, char> getChar2()
{
    using namespace std;

    bool bSuccess = false;
    char retchar = '\0';

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

std::string Terminal::getLine()
{
    bool done = false;
    _commandline.clear();

    while (!done)
    {
        const auto retpair = getChar2();
        if (!retpair.first) break;

        switch (retpair.second)
        {
            default:
                if (auto op = onChar2(retpair.second);
                    op == boost::none || !*op)
                {
                    _commandline += retpair.second;
                    if (_echo)
                    {
                        std::cout << retpair.second << std::flush;
                    }
                    else
                    {
                        std::cout << '*' << std::flush;
                    }
                }
            break;

            case 0x0a: // enter
                if (auto op = onEnter2();
                    op == boost::none || !*op)
                {
                    done = true;
                }
            break;

            case 0x7f:
            case 0x08:
                if (auto op = onBackspace2();
                    (op == boost::none || !*op) && _commandline.size() > 0)
                {
                    _commandline.resize(_commandline.size () - 1);
                    backspace();
                }
            break;

            case 72:
                this->onHome();
            break;

            case 70:
                this->onEnd();
            break;

            case 21: // CTRL-U
                if (auto op = onClearLine();
                    op == boost::none || !*op)
                {
                    backspaces(_commandline.size());
                    _commandline.clear();
                }
            break;

            case 23: // CTRL-W
                this->onDeleteWord();
            break;

            case 0x1b: // ESC
            {
                if (std::cin.get() == '[')
                {
                    switch (std::cin.get())
                    {
                        default:
                        break;

                        case 'A':
                            this->onUpArrow();
                        break;

                        case 'B':
                            this->onDownArrow();
                        break;

                        case 'C':
                            this->onRightArrow();
                        break;

                        case 'D':
                            this->onLeftArrow();
                        break;

                    }
                }
            }
            break;
        }
    }

    return _commandline;
}

void Terminal::backspace()
{
    std::cout << "\b \b" << std::flush;
}


// START OLD CODE
std::pair<bool, char> Terminal::getChar()
{
    using namespace std;

    bool bSuccess = false;
    char retchar = '\0';

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
// END OLD CODE


} // namespace

