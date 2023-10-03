#pragma once

#include <QApplication>
#include <QtCore>
#include <QMainWindow>
#include "Data/BoardManager.h"

namespace owl
{

const QString GetDatabaseName();

class SettingsFile;
using SettingsFilePtr = std::shared_ptr<SettingsFile>;

class OwlApplication : public QApplication
{
	Q_OBJECT

public:
    OwlApplication(int& argc, char **argv[]);
    virtual ~OwlApplication();

	void init();

private:
    void initCommandLine();
    void initializeLogger();

    QString                 _jsonConfig;
    QString                 _parserFolder;
    QString                 _dbFileName;

    QSqlDatabase            _db;
    SettingsFilePtr         _settingsFile;
    QLockFile*              _settingsLock = nullptr;

    bool                    _resetdb = false;
    bool                    _resetcfg = false;
    bool                    _resetui = false;
};

} //namespace owl
