// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include "Data/Board.h"
#include "ui_ConfiguringBoardDlg.h"

namespace Ui
{
	class ConfiguringBoardDlg;
}

namespace spdlog
{
    class logger;
}

namespace owl
{
	
class ConfiguringBoardDlg : public QDialog, public Ui::ConfiguringBoardDlg
{
	Q_OBJECT
	
public:
	ConfiguringBoardDlg(QWidget *parent = 0);
    virtual ~ConfiguringBoardDlg() = default;

	void setInfo(const QString& url,
		const QString& username,
		const QString& password,
		const QString& parser,
		const bool asGuest);

    // should be called after the window is shown
	void start();

	QString getErrorString() const { return _errorStr; }
	bool getSuccess() const { return _bSuccess; }

Q_SIGNALS:
    void completeEvent(StringMap params);

protected Q_SLOTS:
	void onClicked();
	void onConfigurationError();
    void onCompleted(StringMap params);

Q_SIGNALS:
	void newBoardAddedEvent(BoardPtr);

private:
	void doConfigure();

	StringMap autoConfigure();
	StringMap singleConfigure();

    StringMap manualTapatalkConfigure();
    QString resolveFinalUrl(const QString& originalUrl, const QString& foundUrl);

	StringMap createBoard(const QString& parserName, const QString& urlText);

	QString _urlString;
	QString _username;
	QString _password;
	QString _parser;
	bool _asGuest;
	bool _bAutoDetect;
	BoardPtr _newBoard;

	QUrl _targetUrl;
	QMovie* _workingMovie;
	bool _bCompleted;
	bool _bSuccess;
	QString _errorStr;

	QFuture<void>* _future;
	QFutureWatcher<void>* _watcher;

    // The subpaths that will be searched look for a board. The order these paths appear in the list
    // is the order in which they will be searched.
	const QStringList FORUMPATHS;

    // The icon files that will be searched to find an icon to use for the board. The order in which
    // these filenames appear is the order in which they will be searched.
	const QStringList ICONFILES;

    std::shared_ptr<spdlog::logger>  _logger;
};

} //namespace owl
