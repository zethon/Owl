#pragma once
#include <iostream>
#include <mutex>
#include <stack>

#include <QtCore>
#include <fmt/core.h>
#include <rang.hpp>

#include "../src/Parsers/Forum.h"
#include "Terminal.h"

using namespace std::string_literals;

namespace owl
{

class ParserBase;
using ParserBasePtr = std::shared_ptr<ParserBase>;

QString printableDateTime(const QDateTime& dt, bool bShowTime);
QString shortText(const QString& text, const uint maxwidth);

typedef std::function<void(const QString&)> Command;
    
struct ConsoleCommand
{
    QStringList commandNames;
    QString helpMsg;
    std::function<void(const QString&)> command;
    
    ConsoleCommand(const QString& list, const QString& help, Command c)
        : helpMsg(help), command(c)
    {
        for (const QString& cmd : list.split(","))
        {
            commandNames << cmd.trimmed().toLower();
        }
    }
};

struct Location
{
    Location() { reset(); }

    // a pair storing a forum/thread name and id
    using Info = std::pair<QString, QString>;

    // the stack of forums, with the parent forum pushed first and the current forum
    // on the top of the stack
    std::list<Info>    forums;

    // TODO: really should just hold a ThreadPtr

    // info about the current thread we're viewing, will eventually need the post-index
    // and page number?
    Info                thread;

    uint                threadPage = 0;
    uint                threadPP = 0;
    bool                threadIds = false;
    bool                threadStickies = false;
    bool                threadTimes = false;

    uint                postPage = 0;
    uint                postPP = 0;
    uint                postIdx = 0;
    bool                postIds = false;

    void reset()
    {
        forums.clear();
        thread.first.clear();
        thread.second.clear();
        threadPage = 0;
        threadPP = 0;
        threadIds = false;
        threadStickies = false;
        threadTimes = false;
        postIdx = 0;
        postPage = 0;
        postPP = 0;
        postIds = false;
    }
};

class Prompt
{
    const Location&     _location;
    std::string         _host;

    std::string stripWideCharacters(const std::string &text) const
    {
        std::string retval{ text };
        std::replace_if(retval.begin(), retval.end(), 
            [](auto c) { return !(c >= 0 && c < 256); }, '?');
        return retval;
    }

public:
    Prompt(const Location& loc)
        : _location(loc)
    {
        // nothing to do
    }

    void setHost(const QString& host)
    {
        _host = host.toStdString();
    }

    void print() const
    {
        if (_host.size() > 0)
        {
            std::string path;
            std::for_each(_location.forums.rbegin(), _location.forums.rend(),
                [&path](const Location::Info& info)
                {
                    path.append("/"s + info.second.toStdString());
                });

            if (path.empty())
            {
                path.append("/"s);
            }

            std::cout
                << rang::fg::cyan
                << _host
                << rang::fg::magenta
                << stripWideCharacters(path)
                << rang::fg::reset
                << rang::bg::reset
                << rang::style::reset
                << "> ";
        }
        else
        {
            std::cout
                << rang::style::bold
                << rang::fg::red
                << "$/"
                << rang::style::reset
                << rang::fg::reset
                << "> ";
        }
    }
};

class ConsoleApp final : public QObject
{
public:
    template<typename... Args>
    static void printError(Args&&... args)
    {
        std::cout
            << rang::fg::red
            << rang::style::bold
            << "-- "
            << "error: "
            << rang::fg::reset
            << rang::style::reset
            << fmt::format(std::forward<Args>(args)...)
            << '\n';
    }

    template<typename... Args>
    static void printWarning(Args&&... args)
    {
        std::cout
            << rang::fg::yellow
            << rang::style::bold
            << "-- "
            << "warning: "
            << rang::fg::reset
            << rang::style::reset
            << fmt::format(std::forward<Args>(args)...)
            << '\n';
    }

    template<typename... Args>
    static void printStatus(Args&&... args)
    {
        std::cout
            << rang::fg::magenta
            << rang::style::bold
            << "-- "
            << rang::fg::reset
            << rang::style::reset
            << fmt::format(std::forward<Args>(args)...)
            << '\n';
    }

public:
    ConsoleApp(QObject* parent = nullptr);
    virtual ~ConsoleApp() = default;

    void setLuaFolder(const QString& f) { _luaFolder = f; }
    void setCommandfile(const QString& f);

    QStringList& getStartCommands() { return _startCommands; }

    void setColor(bool colorOn);

Q_SIGNALS:
    void finished();

public Q_SLOTS:
    void run();

private:

    enum GotoTyoe
    {
        DIRECT,
        PARENT
    };

    enum ListType
    {
        FORUMS,     // listed the sub-furms of the current forum ('lf')
        THREADS,    // listed the threads in the current forum ('lt')
        POSTS,      // listed the posts in the current thread ('lt')
        SINGLEPOST  // displayed a single post (used by 'n' and 'p')
    };

    Q_OBJECT
    
    QString                     _commandFile;       // file of line-deliminted commands to execute on start
    QStringList                 _startCommands;     // list of commands passed on the command-line

    ParserBasePtr               _parser;            // parser object of active connection or null
    QString                     _luaFolder;         // folder used to load Lua parsers

    QList<BoardItemPtr>         _listItems;

    ListType                    _lastListType;
    Location                    _location;

    Terminal                    _terminal;
    QString                     _commandLine;

    QList<ConsoleCommand>       _commands;
    QList<ConsoleCommand>       _boardCommands;

    Prompt                      _prompt;

    bool                        _bDoneApp = false;

    owl::StringMap              _appOptions;
    
    void parseCommand(const QString& cmdLn);
    
    void doHelp(const QString& cmdLn);
    void doSysInfo(const QString& cmdLn);
    void doLogin(const QString&);
    void doParsers(const QString& cmdLn);

    void listForums() { doListForums(QString()); }
    void doListForums(const QString&);

    // called by 'lf'
    void doListThreads(const QString&);
    void listThreads(const uint pagenumber, const uint perpage, bool bIds, bool bStickies, bool bShowTimes);

    void doListPosts(const QString&);
    void listPosts(const uint pagenumber, const uint perpage, bool bShowIds);

    void printPost(const owl::PostPtr post, uint id);
    void printPost(uint postIdx);

    // makes sure there's an active connection and throws an error if not
    bool verifyLoggedIn(bool bSupressMessage = false);

    void gotoItemNumber(const size_t idx);
    void gotoNext(const QString&);
    void gotoPrevious(const QString&);

    void initCommands();

};

} // end namespace
