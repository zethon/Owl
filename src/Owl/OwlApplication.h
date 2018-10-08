// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <QApplication>
#include <QtCore>
#include <QMainWindow>
#include <consoleappender.h>
#include <dailyrollingfileappender.h>
#include <rollingfileappender.h>
#include <logger.h>
#include <ttcclayout.h>
#include <logmanager.h>
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
