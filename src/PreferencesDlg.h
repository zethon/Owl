// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <memory>
#include <QtGui>
#include <Utils/Settings.h>
#include "ui_PreferencesDlg.h"

namespace Ui
{  
	class PreferencesDlg;
}

namespace owl
{

class Board;
using BoardPtr = std::shared_ptr<Board>;
	
class PreferencesDlg : public QDialog, public Ui::PreferencesDlg
{
	Q_OBJECT
	
public:
    enum class BoardEditAction
    {
        Edit,
        Delete,
        MoveUp,
        MoveDown
    };

	PreferencesDlg(QWidget *parent = nullptr);
    virtual ~PreferencesDlg() = default;

Q_SIGNALS:
    void onBoardEdit(BoardPtr, BoardEditAction);
    void reloadBoardPanel();
    void reloadThreadPanel();
    void reloadPostPanel();

private Q_SLOTS:

private:
    void renderInterfaceSettings();
    void renderGeneralSettings();
    void renderBoardPaneSettings();
    void renderThreadPaneSettings();
    void renderPostPaneSettings();

    void connectInterfaceSettings();
    void connectGeneralSettings();
    void connectBoardPaneSettings();
    void connectThreadPaneSettings();
    void connectPostPaneSettings();

    void renderBoardList();
    void connectBoardList();
    void renderEditorSettings();
    void connectEditorSettings();
    void renderProxySettings();
    void connectProxySettings();
    void renderParserSettings();
    void connectParserSettings();

    void renderInternetSettings();
    void connectInternetSettings();

    void renderAdvancedSettings();
    void connectAdvancedSettings();

    void resetLogFileAppender();

    SettingsObject _settings;
};

} //namespace owl
