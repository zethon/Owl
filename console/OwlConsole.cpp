#ifndef _WINDOWS
#include <sys/ioctl.h>
#endif

#include <QCoreApplication>
#include <QSysInfo>

#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <rang.hpp>

#include "../src/Parsers/BBCodeParser.h"
#include "../src/Parsers/ParserManager.h"
#include "../src/Utils/OwlUtils.h"
#include "../src/Utils/Moment.h"
#include "../src/Utils/OwlLogger.h"

#include "Core.h"
#include "OwlConsole.h"

namespace owl
{

void ConsoleApp::doHelp(const QString&)
{
    std::cout << "Owl Console Help\n";
    for (const auto& cmd : _commands)
    {
        if (!cmd.helpMsg.isEmpty())
        {
            const QString output = QString("\t%1\t- %2")
                .arg(cmd.commandNames.front())
                .arg(cmd.helpMsg);

            std::cout << output.toStdString();
        }
    }
}
    
ConsoleApp::ConsoleApp(QObject *parent)
    : QObject(parent),
      _prompt(_location)
{
    //struct winsize sz;
    //if (ioctl(0,TIOCGWINSZ, &sz) == 0 && sz.ws_col > 0 && sz.ws_row > 0)
    //{
    //    _appOptions.setOrAdd("wwidth", sz.ws_col);
    //    _appOptions.setOrAdd("wheight", sz.ws_row);
    //}
    //else
    //{
    //    // old DOS window default
    //    _appOptions.setOrAdd("wwidth", 80);
    //    _appOptions.setOrAdd("wheight", 25);
    //}

    auto logger = owl::rootLogger();
    logger->set_level(spdlog::level::off);
}

void ConsoleApp::setCommandfile(const QString &f)
{
    _commandFile = f;
}

void ConsoleApp::setColor(bool colorOn)
{
    if (colorOn)
    {
        rang::setControlMode(rang::control::Auto);
    }
    else
    {
        rang::setControlMode(rang::control::Off);
    }
}

void ConsoleApp::doLogin(const QString& options)
{
    QCommandLineParser p;
    p.addPositionalArgument("url", tr("The URL of the message board"), "url");
    p.addPositionalArgument("username", tr("The login name"), "username");
    p.addPositionalArgument("password", tr("The password"), "password");
    p.addOption(QCommandLineOption(QStringList() << "p" << "parser", "The parser name to use", "parser"));
    p.parse(QStringList() << "login" << options.trimmed().split(' '));

    const auto posArgs = p.positionalArguments();
    if (posArgs.size() < 1 || posArgs.at(0).isEmpty())
    {
        std::cout << p.helpText().toStdString() << std::endl;
        return;
    }

    QString boardUrl = posArgs.at(0);
    QString username;
    QString password;

    if (posArgs.size() < 2)
    {
        std::cout << "Username: " << std::flush;

        _terminal.setPrompt(true);
        _terminal.run();
        _terminal.setPrompt(false);
        username = _promptLine;
        _promptLine.clear();
    }
    else
    {
        username = posArgs.at(1);
    }

    if (posArgs.size() < 3)
    {
        _terminal.setEcho(false);
        _terminal.setPrompt(true);
        std::cout << "Password: " << std::flush;

        _terminal.run();
        _terminal.setEcho(true);
        _terminal.setPrompt(false);

        password = _promptLine;
        _promptLine.clear();
    }
    else
    {
        password = posArgs.at(2);
    }

    std::cout << "Connecting to " << boardUrl.toStdString() << std::endl;

    ParserBasePtr parser;

    if (p.isSet("parser"))
    {
        const QString name = p.value("parser");
        qDebug() << "Attempting to connect using parser " << name;

        parser = ParserManager::instance()->createParser(name, boardUrl, false);
        if (parser)
        {
            try
            {
                const auto bwInfo = parser->getBoardwareInfo();
                if (!bwInfo.has("success") || !bwInfo.getBool("success"))
                {
                    parser.reset();
                }
            }
            catch (const owl::Exception& ex)
            {
                qDebug() << "Parser " << name << " failed: " << ex.details();
                parser.reset();
            }
            catch (const std::exception& ex)
            {
                qDebug() << "Parser " << name << " failed: " << ex.what();
                parser.reset();
            }
            catch (...)
            {
                qDebug() << "Parser " << name << " failed: Unknown exception";
                parser.reset();
            }
        }
        else
        {
            std::cerr << "There was an error creating parser '" << name.toStdString() << "'" << std::endl;
        }
    }
    else
    {
        for (const auto& name : ParserManager::instance()->getParserNames())
        {
            qDebug() << "Attempting to connect using parser " << name;
            parser = ParserManager::instance()->createParser(name, boardUrl, false);
            if (!parser)
            {
                std::cerr << "There was an error creating parser '" << name.toStdString() << "'" << std::endl;
                continue;
            }

            try
            {
                const auto bwInfo = parser->getBoardwareInfo();
                if (!bwInfo.has("success") || !bwInfo.getBool("success"))
                {
                    parser.reset();
                }
                else
                {
                    // found a parser!
                    break;
                }
            }
            catch (const owl::Exception& ex)
            {
                qDebug() << "Parser " << name << " failed: " << ex.details();
                parser.reset();
            }
            catch (const std::exception& ex)
            {
                qDebug() << "Parser " << name << " failed: " << ex.what();
                parser.reset();
            }
            catch (...)
            {
                qDebug() << "Parser " << name << " failed: Unknown exception";
                parser.reset();
            }
        }
    }

    if (parser)
    {
        _parser = parser;
        std::cout << "Using parser '" << _parser->getName().toStdString() << "'" << std::endl;

        try
        {
            LoginInfo info(username, password);
            const auto results = _parser->login(info);

            if (results.getBool("success", false))
            {
                // reset our location object to the forum's root
                _location.reset();

                // update the prompt
                _prompt.setHost(boardUrl);

                std::cout
                    << fmt::format("Successfully signed in as '{}' to {}",
                            username.toStdString(), boardUrl.toStdString())
                    << '\n';
            }
            else
            {
                ConsoleApp::printError("Could not sign into {}", boardUrl.toStdString());
            }
        }
        catch (const owl::Exception& ex)
        {
            std::cerr << "Login failed: " << ex.details().toStdString() << std::endl;
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Login failed: " << ex.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Login failed: Unknown error" << std::endl;
        }
    }
    else
    {
        ConsoleApp::printError("Could not find a parser for board '{}'", boardUrl.toStdString());
    }
}

void ConsoleApp::doParsers(const QString&)
{
    std::cout << "Parsers folder:" << _luaFolder.toStdString() << std::endl << std::endl;
    for (const auto& p : ParserManager::instance()->getParsers())
    {
        std::cout << p.name.toStdString() << "\t\t(" << p.prettyName.toStdString() << ")" << std::endl;
    }

    std::cout << std::endl;
}

void ConsoleApp::doListForums(const QString& options)
{
    if (!verifyLoggedIn())
    {
        return;
    }

    QCommandLineParser p;
    p.addOption(QCommandLineOption(QStringList() << "i" << "ids" << "showids"));
    p.parse(QStringList() << "lf" << options.split(' '));

    ForumList forums;
    QString currentForum;

    // an empty stack implies the user is at the root forum
    if (_location.forums.size() == 0)
    {
        currentForum = _parser->getRootForumId();
    }
    else
    {
        currentForum = _location.forums.front().first;
    }

    forums = _parser->getForumList(currentForum);

    if (forums.size() > 0)
    {
        auto idx = 0u;
        _listItems.clear();

        for (const auto& f : forums)
        {
            if (f->getForumType() != Forum::LINK)
            {
                QString idstr;
                if (p.isSet("ids"))
                {
                    idstr = "(" + f->getId() + ")";
                }

                std::cout
                    << "["
                    << rang::style::bold
                    << rang::fg::magenta
                    << ++idx
                    << rang::fg::reset
                    << rang::style::reset
                    << "] "
                    << (f->hasUnread() ? "*" : "")
                    << f->getName().toStdString()
                    << idstr.toStdString()
                    << std::endl;

                _listItems.push_back(f);
            }
        }

        _lastListType = ListType::FORUMS;
    }
}

void ConsoleApp::doListThreads(const QString& options)
{
    if (!verifyLoggedIn())
    {
        return;
    }

    uint pagenumber = 1;
    uint perpage = 10;

    QCommandLineParser p;
    p.addPositionalArgument("pagenumber", "", "pagenumber");
    p.addPositionalArgument("perpage","", "perpage");
    p.addOption(QCommandLineOption(QStringList() << "i" << "ids" << "showids"));
    p.addOption(QCommandLineOption(QStringList() << "s" << "stickies"));
    p.addOption(QCommandLineOption(QStringList() << "t" << "times"));
    p.parse(QStringList() << "lt" << options.split(' '));

    if (p.positionalArguments().size() > 0)
    {
        bool bOk = false;
        uint temp = p.positionalArguments()[0].toUInt(&bOk);
        if (bOk)
        {
            pagenumber = temp;
        }

        if (p.positionalArguments().size() > 1)
        {
            bOk = false;
            temp = p.positionalArguments()[1].toUInt(&bOk);
            if (bOk && temp < 50) // hard perpage limit of 50
            {
                perpage = temp;
            }
        }
    }

    listThreads(pagenumber, perpage, p.isSet("ids"), p.isSet("stickies"), p.isSet("times"));
}

void ConsoleApp::listThreads(const uint pagenumber, const uint perpage, bool bShowIds, bool bStickies, bool bShowTimes)
{
    ThreadList threads;
    QString currentForum;

    if (_location.forums.empty())
    {
        currentForum = _parser->getRootForumId();
    }
    else
    {
        currentForum = _location.forums.front().first;
    }

    ForumPtr forum = std::make_shared<Forum>(currentForum);
    forum->setPageNumber(pagenumber);
    forum->setPerPage(perpage);

    threads = _parser->getThreadList(forum);

    if (threads.size() > 0)
    {
        _listItems.clear();

        // TODO: figure out how to handle stickies and the --nostickies option
//        const auto numstickies = std::count_if(threads.begin(), threads.end(),[](const owl::ThreadPtr t) { return t->isSticky(); });
//        // Say there are 3 stickies and we've requested to show 10 threads, then all we really have to show are 7
//        if (numstickies > 0 && !bShowStickies && numstickies < perpage)
//        {
//        }

        auto idx = 0u;
        owl::Moment moment;

        for (const owl::ThreadPtr& t : threads)
        {
            ++idx;

            const QString idText = bShowIds ? " [" + t->getId() + "]" : "";
            moment.setDateTime(t->getLastPost()->getDateTime());

            QString replyText;
            const auto replyCount = t->getReplyCount();
            if (replyCount== 1)
            {
                replyText = QString("1 reply");
            }
            else if (replyCount > 1)
            {
                replyText = QString("%1 replies").arg(replyCount);
            }

            std::cout
                << "["
                << rang::style::bold
                << rang::fg::magenta
                << idx
                << rang::fg::reset
                << "] "
                << (t->hasUnread() ? "*" : "")
                << (t->isSticky() ? rang::fg::yellow : rang::fg::reset)
                << t->getTitle().toStdString()
                << rang::fg::reset
                << rang::style::reset
                << idText.toStdString()
                << ", "
                << replyText.toStdString()
                << ", "
                << moment.toString().toLower().toStdString()
                << " by "
                << t->getLastPost()->getAuthor().toStdString()
                << rang::fg::reset
                << rang::bg::reset
                << rang::style::reset
                << std::endl;

            _listItems.push_back(t);
        }

        _lastListType = ListType::THREADS;
        _location.threadPage = pagenumber;
        _location.threadPP = perpage;
    }
    else
    {
        std::cout << "No threads returned\n";
    }
}

void ConsoleApp::doListPosts(const QString& options)
{
    if (!verifyLoggedIn())
    {
        return;
    }

    if (_location.thread.first.isEmpty())
    {
        ConsoleApp::printWarning("Cannot display thread because it has an invalid id");
    }

    uint pagenumber = 1;
    uint perpage = 10;

    QCommandLineParser p;
    p.addPositionalArgument("pagenumber", "", "pagenumber");
    p.addPositionalArgument("perpage","", "perpage");
    p.addOption(QCommandLineOption(QStringList() << "i" << "ids"));
    p.parse(QStringList() << "lp" << options.split(' '));

    if (p.positionalArguments().size() > 0)
    {
        bool bOk = false;
        uint temp = p.positionalArguments()[0].toUInt(&bOk);
        if (bOk)
        {
            pagenumber = temp;
        }

        if (p.positionalArguments().size() > 1)
        {
            bOk = false;
            temp = p.positionalArguments()[1].toUInt(&bOk);
            if (bOk) // hard perpage limit of 50
            {
                perpage = std::min(temp, static_cast<uint>(50));
            }
        }
    }

    listPosts(pagenumber, perpage, p.isSet("ids"));
}

void ConsoleApp::listPosts(const uint pagenumber, const uint perpage, bool bShowIds)
{
    const auto currentthread = _location.thread.first;
    Q_ASSERT(!currentthread.isEmpty());

    ThreadPtr thread = std::make_shared<Thread>(currentthread);
    thread->setPageNumber(pagenumber);
    thread->setPerPage(perpage);

    // allow 40 characters for other text with a min of 40
    const auto textwidth = std::max((uint)_appOptions.get<std::uint32_t>("width") - 40, 40u);
    PostList posts = _parser->getPosts(thread, ParserBase::PostListOptions::FIRST_POST);
    if (posts.size() > 0)
    {
        // prepare the item list
        _listItems.clear();

        owl::Moment moment;
        auto idx = ((thread->getPageNumber()-1) * thread->getPerPage()) + 1;

        for (const owl::PostPtr& p : posts)
        {
            moment.setDateTime(p->getDateTime());

            const QString idxtext = QString("[\033[1m\033[35m%1\033[0m]").arg(idx++);
            const QString postext = QString("\033[1m\033[37m%1\033[0m").arg(shortText(p->getText(), (uint)textwidth));

            const QString text = QString("%1 %2\"%3\" - %4 by %5")
                .arg(idxtext)
                .arg(p->hasUnread() ? "*" :"")
                .arg(postext)
                .arg(moment.toString().toLower())
                .arg(p->getAuthor());

            std::cout << text.toStdString() << '\n';
            _listItems.push_back(p);
        }

        _lastListType = ListType::POSTS;
        _location.postPage = pagenumber;
        _location.postPP = perpage;
        _location.postIds = bShowIds;
    }
    else
    {
        ConsoleApp::printWarning("No posts returned");
    }

}

void ConsoleApp::printPost(const PostPtr post, uint idx/*=0 */)
{
    BBRegExParser parser;
    owl::Moment moment(post->getDateTime());

//    ConsoleOutput output;
////    output << console.purple().bold().bg() << QString("Post #%1").arg(idx) << console.reset();
//    output << QString("hi%1").arg(idx);
////    _terminal.writelin(output);

//    OUTPUTLN(output);

    const QString idxtxt = (idx > 0) ? tr("\033[1m\033[35m#%1\033[0m ").arg(idx) : "";
    const QString authortxt = QString("\033[1m\033[34m%1\033[0m").arg(post->getAuthor());
    const QString timetxt = QString("\033[1m\033[37m%1\033[0m").arg(moment.toString());

    const QString firstline = QString("%1%2, %3")
        .arg(idxtxt)
        .arg(authortxt)
        .arg(timetxt);

    const QString posttxt = parser.toPlainText(post->getText());

    std::cout << firstline.toStdString() << '\n';
    std::cout << posttxt.toStdString() << '\n';

//    const QString text = QString("\n%1\n%2%3 by %4\n")
//        .arg(post->getText())
//        .arg(idxtxt)
//        .arg(timetxt)
//        .arg(authortxt);

//    OUTPUTLN(text);

    _lastListType = ListType::SINGLEPOST;
}

void ConsoleApp::printPost(uint idx)
{
    owl::ThreadPtr currentThread = std::make_shared<owl::Thread>(_location.thread.first);
    currentThread->setPageNumber(idx);
    currentThread->setPerPage(1);

    owl::PostList posts = _parser->getPosts(currentThread, ParserBase::PostListOptions::FIRST_POST);
    if (posts.size() > 0)
    {
        _location.postIdx = idx;
        printPost(posts.at(0), _location.postIdx);
    }
    else
    {
        ConsoleApp::printWarning("Invalid post index");
    }
}

bool ConsoleApp::verifyLoggedIn(bool bSupressMessage/*=false*/)
{
    bool bLoggedIn =_parser != nullptr;

    if (!bLoggedIn && !bSupressMessage)
    {
        std::cerr << "This action requires you to be logged into a board (see 'login' command)" << std::endl;
    }

    return bLoggedIn;
}

void ConsoleApp::gotoItemNumber(const size_t idx)
{
    if (_lastListType == ListType::POSTS)
    {
        printPost((uint)idx);
    }
    else if (idx > 0 && idx <= (size_t)_listItems.size())
    {
        const auto zeroIdx = idx - 1;
        BoardItemPtr item = _listItems.at((int)zeroIdx);

        if (_lastListType == ListType::FORUMS)
        {
            // we selected one of the listed forums, so "go" to that forum and list any sub-forums
            owl::ForumPtr newForum = item->upCast<ForumPtr>();
            _location.forums.push_front(std::make_pair(newForum->getId(), newForum->getName()));

            listForums();
        }
        else if (_lastListType == ListType::THREADS)
        {
            // we selected one of the listed threads, so we should "go" to that thread
            // and list the posts
            owl::ThreadPtr currentThread = item->upCast<ThreadPtr>();
            _location.thread = std::make_pair(currentThread->getId(), currentThread->getTitle());

            _location.postPage = 1;
            _location.postPP = 10;

            listPosts(_location.postPage, _location.postPP, _location.postIds);
        }
    }
    else
    {
        ConsoleApp::printError("Invalid index: {}", idx);
    }
}

void ConsoleApp::gotoNext(const QString &)
{
    if (_lastListType == ListType::THREADS)
    {
        listThreads(_location.threadPage+1, _location.threadPP, _location.threadIds, _location.threadStickies, _location.threadTimes);
    }
    else if (_lastListType == ListType::POSTS)
    {
        listPosts(_location.postPage+1, _location.postPP, _location.postIds);
    }
    else if (_lastListType == ListType::SINGLEPOST)
    {
        printPost(_location.postIdx+1);
    }
}

void ConsoleApp::gotoPrevious(const QString &)
{
    if (_lastListType == ListType::THREADS && _location.threadPage > 1)
    {
        listThreads(_location.threadPage-1, _location.threadPP, _location.threadIds, _location.threadStickies, _location.threadTimes);
    }
    else if (_lastListType == ListType::POSTS && _location.postPage > 1)
    {
        listPosts(_location.postPage-1, _location.postPP, _location.postIds);
    }
    else if (_lastListType == ListType::SINGLEPOST && _location.postIdx > 1)
    {
        printPost(_location.postIdx-1);

    }
}

void ConsoleApp::initCommands()
{
    _commands =
    {
        ConsoleCommand("help,?", tr("Display help message"), std::bind(&ConsoleApp::doHelp, this, std::placeholders::_1)),
        ConsoleCommand("sysinfo", "Show system info", std::bind(&ConsoleApp::doSysInfo, this, std::placeholders::_1)),
        ConsoleCommand("login", "Login to a remote board", std::bind(&ConsoleApp::doLogin, this, std::placeholders::_1)),
        ConsoleCommand("parsers", "List parsers",std::bind(&ConsoleApp::doParsers, this, std::placeholders::_1)),
        ConsoleCommand("quit,exit,q", "", [this](const QString&) { _bDoneApp = true; }),
        ConsoleCommand("version,about", tr("Display version information"),
            [](const QString&)
            {

                std::cout << "OwlConsol v" << OWLCONSOLE_VERSION << " \tBuilt: " OWLCONSOLE_BUILDTIMESTAMP << std::endl;
            }),

        ConsoleCommand("set", tr("Set application variable"),
            [&](const QString& options)
            {
                QCommandLineParser p;
                p.addPositionalArgument("key", "", "key");
                p.addPositionalArgument("value","", "value");
                p.parse(QStringList() << "lf" << options.split(' '));

                if (p.positionalArguments().size() == 1 && p.positionalArguments().at(0).isEmpty())
                {
                    for (const auto& p : _appOptions)
                    {
                        std::cout
                            << p.first.toStdString()
                            << '='
                            << p.second.toStdString()
                            << '\n';
                    }
                }
                else if (p.positionalArguments().size() == 2)
                {
                    const auto key = p.positionalArguments().at(0);
                    const auto val = p.positionalArguments().at(1);
                    _appOptions.setOrAdd(key, val);
                    std::cout
                        << fmt::format("The key '{}' has been set to '{}'\n",
                                key.toStdString(), val.toStdString());
                }
                else
                {
                   std::cout << "Usage: set {key} {value}\n";
                }
            })
    };

    _boardCommands =
    {
        ConsoleCommand("lf,forums", "List sub forums of the curent forum", std::bind(&ConsoleApp::doListForums, this, std::placeholders::_1)),
        ConsoleCommand("lt,threads", "List threads in the current forum", std::bind(&ConsoleApp::doListThreads, this, std::placeholders::_1)),
        ConsoleCommand("lp,posts", "List posts in the current thread", std::bind(&ConsoleApp::doListPosts, this, std::placeholders::_1)),
        ConsoleCommand("n,+", tr("Go to the next page of the last list"), std::bind(&ConsoleApp::gotoNext, this, std::placeholders::_1)),
        ConsoleCommand("p,-", tr("Go to the previous page of the last list"), std::bind(&ConsoleApp::gotoPrevious, this, std::placeholders::_1)),

        ConsoleCommand(".,..","Go to the parent forum",
            [&](const QString&)
            {
                if (_location.forums.size() > 0)
                {
                    _location.forums.pop_front();
                    listForums();
                }
            }),

        ConsoleCommand("/,root","Go to root folder",
            [&](const QString&)
            {
                _location.reset();
                listForums();
            }),

        ConsoleCommand("w,whereami", tr("Show the current forum and thread"),
            [&](const QString&)
            {
                if (_location.forums.size() > 0)
                {
                    const auto currentForum = _location.forums.front();
                    std::cout
                        << fmt::format("Current forum: {} ({})\n",
                            currentForum.second.toStdString(), currentForum.first.toStdString());
                }
                else
                {
                    std::cout
                        << fmt::format("Current forum: <root> ({})\n",
                            _parser->getRootForumId().toStdString());
                }

                if (!_location.thread.first.isEmpty())
                {
                    std::cout
                        << fmt::format("Current thread: {} ({})\n",
                            _location.thread.second.toStdString(),
                            _location.thread.first.toStdString());
                }
            }),

        ConsoleCommand("l", tr("Display the previous list"),
            [&](const QString&)
            {
                if (_lastListType == ListType::FORUMS)
                {
                    doListForums(QString());
                }
                else if (_lastListType == ListType::THREADS && _location.threadPage > 0)
                {
                    listThreads(_location.threadPage, _location.threadPP, _location.threadIds, _location.threadStickies, _location.threadTimes);
                }
                else if (_lastListType == ListType::POSTS && _location.postPage > 0)
                {
                    listPosts(_location.postPage, _location.postPP, _location.postIds);
                }
            })
    };
}

void ConsoleApp::parseCommand(const QString& cmdLn)
{
    // split the line by white space
    auto parts = cmdLn.trimmed().split(QRegExp("\\s"));
    
    if (parts.size() > 0)
    {
        ConsoleCommand* cmdPtr = nullptr;
        auto commandName = parts[0].toLower();

        // look for commands like 'help' and 'login'
        for (auto& c : _commands)
        {
            auto it = std::find(c.commandNames.begin(), c.commandNames.end(), commandName);
            if (it != c.commandNames.end())
            {
                cmdPtr = &c;
                break;
            }
        }
        
        // we didn't find a top-level command so look for a board-level command like 'lf' or 'lt'
        if (!cmdPtr)
        {
            for (auto& c : _boardCommands)
            {
                auto it = std::find(c.commandNames.begin(), c.commandNames.end(), commandName);
                if (it != c.commandNames.end())
                {
                    cmdPtr = &c;
                    break;
                }
            }
        }

        if (cmdPtr != nullptr)
        {
            parts.removeFirst();
            const auto cmdStr = parts.join(' ');
            cmdPtr->command(cmdStr);
        }
        else
        {
            // still nothing, so see if this is a number in which case we want to navigate to whatever
            // index of whatever thing the user last listed
            bool bOk = false;
            uint idx = cmdLn.toUInt(&bOk);
            if (bOk)
            {
                gotoItemNumber(idx);
            }
            else
            {
                std::cout
                    << fmt::format("Unknown command '{}'\n", commandName.toStdString());
            }
        }
    }
}
    
void ConsoleApp::doChar(QChar c)
{
    if (_terminal.isPrompt())
    {
        _promptLine.append(c);
    }
    else
    {
        _commandLine.append(c);
    }

    if (_terminal.getEcho())
    {
        std::cout << c.toLatin1() << std::flush;
    }
    else
    {
        std::cout << '*' << std::flush;
    }
}

void ConsoleApp::doBackspace()
{
    if (!_commandLine.isEmpty())
    {
        _commandLine.remove(_commandLine.size() - 1, 1);
        std::cout << "\b \b" << std::flush;
    }
}

bool ConsoleApp::doEnter()
{
    std::cout << std::endl;
    if (_terminal.isPrompt())
    {
        return true;
    }
    else if (!_commandLine.isEmpty())
    {
        parseCommand(_commandLine);
        _commandLine.clear();

        if (!_bDoneApp)
        {
            std::cout << _prompt.toStdString() << std::flush;
        }
    }

    return _bDoneApp;
}

void ConsoleApp::run()
{
    std::cout << "Owl Console " << OWLCONSOLE_VERSION << std::endl;
    std::cout << COPYRIGHT << std::endl;

    // initialize the ParserManager
    ParserManager::instance()->init(!_luaFolder.isEmpty(), _luaFolder);

    // set up all our terminal commands
    initCommands();

    // set up the Terminal signals
    QObject::connect(&_terminal, SIGNAL(onChar(QChar)), this, SLOT(doChar(QChar)), Qt::DirectConnection);
    QObject::connect(&_terminal, SIGNAL(onBackspace(void)), this, SLOT(doBackspace(void)), Qt::DirectConnection);
    QObject::connect(&_terminal, SIGNAL(onEnter(void)), this, SLOT(doEnter(void)), Qt::DirectConnection);

    try
    {
        std::cout << "Type \"help\" or \"quit\" to exit the program" << std::endl;

        if (!_commandFile.isEmpty())
        {
            QFile cmdf(_commandFile);
            if (cmdf.exists() && cmdf.open(QIODevice::ReadOnly))
            {
                QTextStream in(&cmdf);
                while (!in.atEnd())
                {
                    const QString cmd = in.readLine().trimmed();
                    if (!cmd.isEmpty())
                    {
                        std::cout << _prompt.toStdString() << cmd.toStdString() << std::endl;
                        parseCommand(cmd);
                    }
                }
            }
            else
            {
                std::cerr << "Command file '" << _commandFile.toStdString() << "' not found" << std::endl;
            }
        }
        else
        {
            for (const auto& cmd : _startCommands)
            {
                std::cout << _prompt.toStdString() << cmd.toStdString() << std::endl;
                parseCommand(cmd);
            }
        }

        if (!_bDoneApp)
        {
            std::cout << _prompt.toStdString() << std::flush;
            _terminal.run();
        }
    }
    catch (const owl::Exception& ex)
    {
        std::cerr << "Owl exception: " << ex.details().toStdString() << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Generic exception: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "There was an unknown exception" << std::endl;
    }

    Q_EMIT finished();
}

void ConsoleApp::doSysInfo(const QString&)
{
    std::cout << "version: " << OWLCONSOLE_VERSION << '\n';
    std::cout << fmt::format("build-date: ", OWLCONSOLE_BUILDTIMESTAMP) << '\n';
    std::cout << "os: " << QSysInfo::prettyProductName().toStdString() << '\n';
}

QString shortText(const QString& original, const uint maxwidth)
{
    static BBRegExParser bbparser;
    static QRegularExpression whitespace("[\\r\\n]");

    QString retval = bbparser.toPlainText(original);
    retval = retval.replace(whitespace, QString());

    if (retval.size() > static_cast<int>(maxwidth))
    {
        // TODO: would be nice if the '25' could be determined by the console's width
        retval = retval.left(maxwidth-3) + QStringLiteral("...");
    }

    return retval;
}

QString printableDateTime(const QDateTime &dt, bool bShowTime)
{
    QString retval { "Invalid date" };

    if (dt.isValid())
    {
        retval.clear();
        const auto now = QDateTime::currentDateTime();

        if (dt.date() == now.date())
        {
            retval += "Today" + (bShowTime ? (" at " + dt.time().toString("hh:mm")) : "");
        }
        else if (dt.daysTo(now) == 1)
        {
            retval += "Yesterday" + (bShowTime ? (" at " + dt.time().toString("hh:mm")) : "");
        }
        else
        {
            retval += dt.toString("yyyy-M-d") + (bShowTime ? (" " + dt.time().toString("hh:mm")) : "");
        }
    }

    return retval.toLower();
}

TextItem::TextItem(const QString &text)
    : _text(text)
{
    // do nothing
}

TextItem::TextItem(const TextItem &other)
{
    _text = other._text;
}

QString TextItem::operator()() const
{
    return _text;
}

} // namespace
