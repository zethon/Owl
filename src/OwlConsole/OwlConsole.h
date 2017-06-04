// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <iostream>
#include <mutex>
#include <stack>
#include <QtCore>
#include <Parsers/Forum.h>
#include "Terminal.h"

namespace owl
{

class ParserBase;
using ParserBasePtr = std::shared_ptr<ParserBase>;

using namespace std;

void OUTPUT(const QString& text);
void OUTPUTLN(const QString& text);

QString printableDateTime(const QDateTime& dt, bool bShowTime);
QString shortText(const QString& text, const uint maxwidth);

class OutputItem : public QObject
{

public:
    virtual QString operator()() const = 0;
};

class ColorItem : public OutputItem
{

public:
    void reset()
    {

    }

    virtual QString operator()() const override
    {
        return "";
    }
};

class TextItem : public OutputItem
{
private:
    QString _text;

public:
    TextItem(const QString& text)
        : _text(text)
    {
        // do nothing
    }

    TextItem(const TextItem& other)
    {
        _text = other._text;
    }

    virtual ~TextItem() = default;

    virtual QString operator()() const override
    {
        return _text;
    }
};

class ConsoleOutput
{
    std::vector<const TextItem> _items;

public:
    using iterator = std::vector<const TextItem>::iterator;
    using const_iterator = std::vector<const TextItem>::const_iterator;

    iterator begin()
    {
        return _items.begin();
    }

    iterator end()
    {
        return _items.end();
    }

    const_iterator begin() const
    {
        return _items.cbegin();
    }

    const_iterator end() const
    {
        return _items.cend();
    }


    template<class T>
    ConsoleOutput& operator<<(const T& item)
    {
        _items.push_back(item);
        return *this;
    }

    ConsoleOutput& operator<<(const QString& item)
    {
        _items.emplace_back(TextItem(item));
        return *this;
    }
};

typedef std::function<void(const QString&)> Command;
    
struct ConsoleCommand
{
    QStringList commandNames;
    QString helpMsg;
    std::function<void(const QString&)> command;
    
    ConsoleCommand(const QString& list, const QString& help, Command c)
        : helpMsg(help), command(c)
    {
        for (const QString cmd : list.split(","))
        {
            commandNames << cmd.trimmed().toLower();
        }
    };
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

public:
    Prompt(const Location& loc)
        : _location(loc)
    {
        // nothing to do
    }

    void setHost(const QString& host)
    {
        if (host.isEmpty())
        {
            _hostBit.clear();
        }
        else
        {
            QUrl u  = QUrl::fromUserInput(host);
            _hostBit = "\033[1m\033[37m[\033[1m\033[36m" + u.host() + "\033[1m\033[37m]\033[0m ";
        }
    }

    const QString prompt() const
    {
        QString prompt;
        QString path;

        if (_hostBit.size() > 0)
        {
            path.clear();

            std::for_each(_location.forums.rbegin(), _location.forums.rend(),
                [&path, this](const Location::Info& info)
                {
                    path.append(QStringLiteral("/") + info.second);
                });

            if (path.isEmpty())
            {
                path.append(QStringLiteral("/"));
            }

            prompt = QString("%1\033[1m\033[32m%2\033[0m> ")
                .arg(_hostBit)
                .arg(path);
        }
        else
        {
            prompt = QString("\033[35m$\033[0m> ");
        }

        return prompt;
    }

    const std::string toStdString()
    {
        return prompt().toStdString();
    }

private:
    const Location&     _location;
    QString             _hostBit;
};

class ConsoleApp final : public QObject
{
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
    QString                     _promptLine;

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
    bool doEnter();
    void doChar(QChar c);
    void doBackspace();


    void run();
};

} // end namespace
