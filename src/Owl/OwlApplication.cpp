#include <QMessageBox>
#include <QtSql>
#include <QWebEngineSettings>
#include <QtQml>
#include <QSysInfo>
#include <Parsers/ParserManager.h>
#include <Utils/Settings.h>
#include <Utils/OwlUtils.h>
#include "ErrorReportDlg.h"
#include "Core.h"
#include "OwlApplication.h"

#include <boost/optional/optional.hpp>

#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <Utils/OwlLogger.h>

namespace owl
{

const QString GetDatabaseName()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
        .absoluteFilePath(QStringLiteral("owl.sqlite"));
}

static void loadDefaultSettings(SettingsFilePtr settings)
{
    auto root = settings->root();
    root->write("version", OWL_VERSION);

    root->write("logs.level", "off");
    root->write("logs.file.enabled", false);
    root->write("logs.file.path",
                QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    root->write("parsers.enabled", true);
#ifdef Q_OS_MAC
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("Resources");
    dir.cd("parsers");
    root->write("parsers.path", dir.absolutePath());
#else
    root->write("parsers.path", QDir(QDir::currentPath()).filePath("parsers"));
#endif

    root->write("editor.font.family", "Helvetica");
    root->write("editor.font.size", 14);
    root->write("editor.spellcheck.enabled", true);
    root->write("editor.spellcheck.language", "en_US"); // TODO: default?

    root->write("view.threads.action", 0);

    root->write("proxy.enabled", false);
    root->write("proxy.type", "http");
    root->write("proxy.host", "");
    root->write("proxy.port", 0);
    root->write("proxy.authentication.username", "");
    root->write("proxy.authentication.password", "");

    const QString defaultAgent = QString("Mozilla/5.0 Firefox/3.5.6 %1 / %2").arg(APP_NAME).arg(OWL_VERSION);
    root->write("web.useragent", defaultAgent);

    root->write("boardlist.icons.visible", true);
    root->write("boardlist.background.color", "#444444");
    root->write("boardlist.text.color", "#FFFFFF");
    root->write("boardlist.highlight.color", "#FFFFFF");

    root->write("threadlist.avatars.visible", true);
    root->write("threadlist.lastauthor.visible", true);
    root->write("threadlist.previewtext.visible", true);

    const auto* websettings = QWebEngineSettings::globalSettings();
    root->write("postlist.font.standard", websettings->fontFamily(QWebEngineSettings::FontFamily::StandardFont));
    root->write("postlist.font.standard.size", websettings->fontSize(QWebEngineSettings::DefaultFontSize));
    root->write("postlist.font.serif", websettings->fontFamily(QWebEngineSettings::FontFamily::SerifFont));
    root->write("postlist.font.sansserif", websettings->fontFamily(QWebEngineSettings::FontFamily::SansSerifFont));
    root->write("postlist.font.fixed", websettings->fontFamily(QWebEngineSettings::FontFamily::FixedFont));
    root->write("postlist.font.fixed.size", websettings->fontSize(QWebEngineSettings::DefaultFixedFontSize));
    root->write("postlist.highlight.enabled", true);
    root->write("postlist.highlight.color", "#e4ebf1");
    root->write("postlist.avatars.visible", true);

    root->write("datetime.format", "default"); // "default", "moment"
    root->write("datetime.date.pretty", true); // use of "Today" and "Yesterday"
    root->write("datetime.date.format", "textdate"); // "long", "short", "textdate"
    root->write("datetime.time.format", "h:mm ap");

    root->write("navigation.thread.newposts", "new"); // "new", "first", "last"
    root->write("navigation.thread.nonewposts", "last"); // "first", "last"
    root->write("navigation.posts.collapseold", true);
}

static std::pair<bool, QString>
    initSettings(const QString& filename, SettingsFilePtr settings, bool resetcfg)
{
    QFileInfo info(filename);

    QDir dir { info.absoluteDir() };
    if (!dir.exists() && !dir.mkpath(QStringLiteral(".")))
    {
        const QString errorMessage = QStringLiteral("Cannot create directory: %1").arg(dir.path());
        return std::make_pair(false, errorMessage);
    }

    if (info.exists() && resetcfg)
    {
        if (!QFile::remove(filename))
        {
            return { false, QStringLiteral("Could not delete file: %1").arg(filename) };
        }
    }

    settings->setFilePath(filename);
    if (settings->hasError())
    {
        const QString errorMessage = settings->errorMessage();
        return std::make_pair(false, errorMessage);
    }

    if (settings->root()->data().isEmpty())
    {
        loadDefaultSettings(settings);
    }

    SettingsObject::setDefaultFile(settings.get());
    return std::make_pair(true, QString());
}

// NOTE: This is called before the logger is initialized
void registerMetaTypes()
{
    qRegisterMetaType<owl::BoardPtr>("BoardPtr");
    qRegisterMetaType<owl::BoardWeakPtr>("BoardWeakPtr");
    qRegisterMetaType<owl::BoardList>("BoardList");

    qRegisterMetaType<owl::ForumPtr>("ForumPtr");
    qRegisterMetaType<owl::ForumList>("ForumList");

    qRegisterMetaType<owl::ThreadPtr>("ThreadPtr");
    qRegisterMetaType<owl::ThreadList>("ThreadList");

    qRegisterMetaType<owl::PostPtr>("PostPtr");
    qRegisterMetaType<owl::PostList>("PostList");

    qRegisterMetaType<owl::ParserBasePtr>("ParserBasePtr");

    qRegisterMetaType<owl::StringMap>("StringMap");
    qRegisterMetaType<owl::StringMapPtr>("StringMapPtr");

    qmlRegisterType<SettingsObject>("reader.owl", 1, 0, "Settings");
}

OwlApplication::OwlApplication(int& argc, char **argv[])
    : QApplication(argc,*argv),
      _settingsFile(new SettingsFile)
{
    setApplicationName(QStringLiteral(APP_NAME));
    setOrganizationName(QStringLiteral(ORGANIZATION_NAME));
    setOrganizationDomain(QStringLiteral(ORGANZATION_DOMAIN));
    setApplicationVersion(OWL_VERSION);

    // register Owl-specific meta types
    registerMetaTypes();

    // process any command line options
    initCommandLine();
}

OwlApplication::~OwlApplication()
{
    owl::SpdLogPtr logger = owl::rootLogger();

    try
    {
        if (_db.isOpen())
        {
            logger->debug("Closing Owl database");
            _db.close();
        }
    }
    catch (const Exception& ex)
    {
        logger->error("Error destructing application object: {}", ex.message().toStdString());
    }
    catch (const std::exception& ex)
    {
        logger->error("There was an error shutting down the application: {}", ex.what());
    }
    catch (...)
    {
        logger->error("There was an unknown error shutting down the application");
    }

    logger->info("Exiting Owl");
}

void OwlApplication::init()
{
    // initialize the JSON settings file
    const auto retval = initSettings(_jsonConfig,_settingsFile, _resetcfg);
    if (!retval.first)
    {
        OWL_THROW_EXCEPTION(Exception(retval.second));
    }

    // initialize the logger and write the "Starting Owl..." message to the log
    initializeLogger();

    // initialize the application's db
    _db = BoardManager::instance()->initializeDatabase(_dbFileName);
    if (!_db.isValid() || !_db.isOpen())
    {
        const QString msg = QString("Could not initialize database at %1").arg(_dbFileName);
        OWL_THROW_EXCEPTION(owl::Exception(msg));
    }

    // load the native and Lua parsers
    SettingsObject object;
    const bool parsersEnabled = object.read("parsers.enabled").toBool();
    if (parsersEnabled)
    {
        const auto parsersPath = !_parserFolder.isEmpty() ? _parserFolder : object.read("parsers.path").toString();
        ParserManager::instance()->init(true, parsersPath);
    }
    else
    {
       ParserManager::instance()->init(false);
    }

    // create the board objects from the db
    BoardManager::instance()->loadBoards(_resetdb);

    if (_resetui)
    {
        const QString writePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        const QString filename { QDir(writePath).absoluteFilePath("owl.ini") };
        if (QFileInfo(filename).exists())
        {
            if (!QFile::remove(filename))
            {
                owl::SpdLogPtr logger = owl::rootLogger();
                logger->warn("could not remove ui config file '{}'", filename.toStdString());
            }
        }
    }
}

// TODO: finish this when I'm sober
void OwlApplication::initCommandLine()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Owl command line options"));

    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption({{"c", "config"}, QStringLiteral("Use the specified config value instead of the default"), "config"});
    parser.addOption({{"p", "parser"},QStringLiteral("Specify a parser folder"), "parser"});
    parser.addOption({{"b", "boards"}, QStringLiteral("Specify a boards database file"), "boards"});
    parser.addOption({"resetdb", QStringLiteral("Reset the default database")});
    parser.addOption({"resetcfg", QStringLiteral("Reset the default configuration file")});
    parser.addOption({"resetui", QStringLiteral("Reset the UI settings")});
    parser.addOption({"resetall", QStringLiteral("Reset all settings")});

    parser.process(*this);

    _resetdb = parser.isSet("resetdb");
    _resetcfg = parser.isSet("resetcfg");
    _resetui = parser.isSet("resetui");

    if (parser.isSet("resetall"))
    {
        _resetdb = true;
        _resetcfg = true;
        _resetui = true;
    }

    if (parser.isSet("config"))
    {
        _jsonConfig = parser.value("config");
    }
    else
    {
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
        _jsonConfig = dir.absoluteFilePath("settings.json");
    }

    if (parser.isSet("boards"))
    {
        _dbFileName = parser.value("boards");
    }
    else
    {
        _dbFileName = GetDatabaseName();
    }

    if (parser.isSet("parser"))
    {
        _parserFolder = parser.value("parser");
    }
    // else, leave it empty to signal we'll use the folder in the config file
}
    
void OwlApplication::initializeLogger()
{
    SettingsObject settings;

    const QString levelString = settings.read("logs.level").toString().toLower();
    const auto configLevel = spdlog::level::from_str(levelString.toStdString());

    auto logger = owl::rootLogger();
    logger->set_level(configLevel);

    boost::optional<std::string> errorMessage;

    if (settings.read("logs.file.enabled").toBool())
    {
        const QString logPath { settings.read("logs.file.path").toString() };
        QFileInfo info(logPath);
        if (!info.isDir() || !info.isWritable())
        {
            errorMessage = fmt::format("The log file folder '{}' is invalid. Make sure it exists and is writable.", logPath.toStdString());
            return;
        }

        QDir logDir(logPath);
        const QString logFilename { logDir.absoluteFilePath("owl.log") };

        auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt> (
            logFilename.toStdString(), 1024 * 1024 * 5, 3);

        logger->sinks().push_back(rotating);
    }

    // log startup info
    logger->info("Starting {} version {} built {}", APP_NAME, OWL_VERSION, OWL_VERSION_DATE_TIME);
    logger->info("Logging initialized to level '{}'", spdlog::level::to_string_view(configLevel));

    if (settings.read("logs.file.enabled").toBool())
    {
        if (errorMessage)
        {
            logger->warn(errorMessage.value());
        }
        else
        {
            logger->info("Logging files will be stored in {}",
                settings.read("logs.file.path").toString().toStdString());
        }
    }

    logger->debug("Operating System: {}", QSysInfo::prettyProductName().toStdString());
    logger->debug("Current working directory: {}", QDir::currentPath().toStdString());
    logger->info("Settings file '{}'", _settingsFile->filePath().toStdString());
}

} // namespace owl
