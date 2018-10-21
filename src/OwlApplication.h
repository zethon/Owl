#pragma once

#include <QApplication>
#include <QtCore>
#include <QMainWindow>
#include <log4qt/consoleappender.h>
#include <log4qt/dailyrollingfileappender.h>
#include <log4qt/rollingfileappender.h>
#include <log4qt/logger.h>
#include <log4qt/ttcclayout.h>
#include <log4qt/logmanager.h>
#include "Data/BoardManager.h"

using namespace Log4Qt;

namespace owl
{

const QString GetDatabaseName();

class SettingsFile;
using SettingsFilePtr = std::shared_ptr<SettingsFile>;

class OwlApplication : public QApplication
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER

public:
    OwlApplication(int& argc, char **argv[]);
    virtual ~OwlApplication();

	void init();

private:
    void initCommandLine();
    void initConsoleAppender();
    void initializeDatabase();
    void initializeLogger();

    void logStartupInfo();

    void registerMetaTypes();

    QString                 _jsonConfig;
    QString                 _parserFolder;
    QString                 _dbFileName;

    QSqlDatabase            _db;
    SettingsFilePtr         _settingsFile;
    QLockFile*              _settingsLock = nullptr;
};

} //namespace owl
