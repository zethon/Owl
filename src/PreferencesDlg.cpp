// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include <log4qt/ttcclayout.h>
#include <log4qt/rollingfileappender.h>
#include <Parsers/ParserManager.h>
#include <Utils/OwlUtils.h>
#include "Data/BoardManager.h"
#include "EditBoardDlg.h"
#include "PostTextEditor.h"
#include "Core.h"
#include "PreferencesDlg.h"


using namespace Log4Qt;

namespace owl
{

PreferencesDlg::PreferencesDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

    renderInterfaceSettings();
    renderBoardList();
    renderEditorSettings();
    renderProxySettings();
    renderParserSettings();
    renderAdvancedSettings();
    renderInternetSettings();

    connectInterfaceSettings();
    connectBoardList();
    connectEditorSettings();
    connectProxySettings();
    connectParserSettings();
    connectAdvancedSettings();
    connectInternetSettings();

    const auto count = listWidget->count();
    for (int i = 0; i < count; i++)
    {
        QListWidgetItem *item = listWidget->item(i);
        item->setSizeHint(QSize(item->sizeHint().width(), 48));
    }

    stackedWidget->setCurrentIndex(0);
    generalTabWidget->setCurrentIndex(0);

    QObject::connect(listWidget, &QListWidget::currentRowChanged,
    [this](int currentRow)
    {
        stackedWidget->setCurrentIndex(currentRow);
    });

    listWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    postListFontGB->setVisible(false);
    threadListOpeningGB->setVisible(false);
    showForumsOnlyCB->setVisible(false);
}

void PreferencesDlg::renderBoardList()
{
    boardListView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    boardListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    boardListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    boardListView->setSelectionMode(QAbstractItemView::SingleSelection);
    boardListView->setAlternatingRowColors(true);

    // create simple model for a tree view
    QStandardItemModel* model = new QStandardItemModel();
    QModelIndex parentItem;
    int iCount = 0;

    // set the columns
    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal, QVariant{});
    model->setHeaderData(1, Qt::Horizontal, tr("Name"));
    model->setHeaderData(2, Qt::Horizontal, tr("Username"));
    model->setHeaderData(3, Qt::Horizontal, tr("Url"));

    const auto boardlist = BoardManager::instance()->getBoardList();
    for (const auto& b : boardlist)
    {
        parentItem = model->index(iCount, 0, parentItem);

        model->insertRows(iCount, 1, parentItem);

        QModelIndex index = model->index(iCount, 0, parentItem);
        const QByteArray buffer(b->getFavIcon().toLatin1());
        auto image = QImage::fromData(QByteArray::fromBase64(buffer));

        if (image.width() != 32 || image.height() != 32)
        {
            // calculate the scaling factor based on wanting a 32x32 image
            qreal iXScale = (qreal)32 / (qreal)image.width();
            qreal iYScale = (qreal)32 / (qreal)image.height();

            QTransform transform;
            transform.scale(iXScale, iYScale);
            image = image.transformed(transform, Qt::SmoothTransformation);
        }

        const QPixmap boardPixmap = QPixmap::fromImage(image);
        model->setData(index, boardPixmap, Qt::DecorationRole);

        index = model->index(iCount, 1, parentItem);
        model->setData(index, b->getName());

        index = model->index(iCount, 2, parentItem);
        model->setData(index,b->getUsername());

        index = model->index(iCount, 3, parentItem);
        model->setData(index,b->getUrl());

        QString propName = QString("data_%1").arg(iCount);
        model->setProperty(propName.toLatin1(), QVariant::fromValue(b));

        auto item = model->itemFromIndex(model->index(iCount, 0, parentItem));
        item->setSizeHint(QSize(item->sizeHint().width(), 35));

        iCount++;
    }

    // set model and delegate to the treeview object
    boardListView->setModel(model);
    boardListView->header()->resizeSection(0, 42);
}
    
void PreferencesDlg::connectBoardList()
{
    QObject::connect(editBoardBtn, &QToolButton::clicked,
        [this](bool)
        {
            QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(boardListView->model());
            QModelIndex selected = boardListView->currentIndex();

            if (selected.isValid())
            {
                //QStandardItem* item = model->item(selected.row(), selected.column());
                QString propName = QString("data_%1").arg(selected.row());

                QVariant variant = model->property(propName.toLatin1());
                BoardPtr b = variant.value<BoardPtr>();

                EditBoardDlg* pDlg = new EditBoardDlg(b, this);
                connect(pDlg, SIGNAL(boardSavedEvent(BoardPtr)), this, SLOT(onBoardSavedEvent(BoardPtr)));
                pDlg->open();
            }
        });

    // delete needs a Qt::DirectConnection so we can remove the board from the UI
    // directly and then delete it from the BoardManager
    QObject::connect(deleteBoardBtn, &QToolButton::clicked, this,
        [this](bool)
        {
            QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(boardListView->model());
            QModelIndex selected = boardListView->currentIndex();

            if (selected.isValid())
            {
                QString propName = QString("data_%1").arg(selected.row());

                QVariant variant = model->property(propName.toLatin1());
                BoardPtr b = variant.value<BoardPtr>();

                QString strMsg =
                    QString(tr("Are you sure you want to delete the board \"%1\"?\n\n"
                    "This will permanently delete the set up of the account.")).arg(b->getName());

                QMessageBox* messageBox = new QMessageBox(
                        QMessageBox::Question,
                        tr("Delete Message Board"),
                        strMsg,
                        QMessageBox::Yes | QMessageBox::No,
                        this,
                        Qt::Sheet);

                messageBox->setWindowModality(Qt::WindowModal);
                if (messageBox->exec() == QMessageBox::Yes)
                {
                    Q_EMIT onBoardEdit(b, PreferencesDlg::BoardEditAction::Delete);
                    BoardManager::instance()->deleteBoard(b);
                    BoardManager::instance()->reload();
                    renderBoardList();
                }
            }
        }
    , Qt::DirectConnection);

    QObject::connect(moveUpBtn, &QToolButton::clicked,
        [this](bool)
        {
            QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(boardListView->model());
            QModelIndex selected = boardListView->currentIndex();

            if (selected.isValid() && selected.row() > 0)
            {
                auto selectedRow = selected.row();

                // change the displayOrder property of the selected board
                auto thisPropName = QString("data_%1").arg(selected.row());
                auto thisBoard = model->property(thisPropName.toLatin1()).value<BoardPtr>();
                auto sbdo = thisBoard->getOptions()->get<std::uint32_t>("displayOrder");
                thisBoard->getOptions()->setOrAdd("displayOrder",(uint)sbdo - 1);

                // change the displayOrder property of the board above it
                auto otherPropName = QString("data_%1").arg(selected.row() - 1);
                auto otherBoard = model->property(otherPropName.toLatin1()).value<BoardPtr>();
                sbdo = otherBoard->getOptions()->get<std::uint32_t>("displayOrder");
                otherBoard->getOptions()->setOrAdd("displayOrder",(uint)sbdo + 1);

                // update the model properties
                model->setProperty(thisPropName.toLatin1(), QVariant::fromValue(otherBoard));
                model->setProperty(otherPropName.toLatin1(), QVariant::fromValue(thisBoard));

                // remove the item from the data model
                auto rowList = model->takeRow(selectedRow);
                model->insertRow(selectedRow - 1, rowList);

                // change the view's selection
                QModelIndex newIdx = model->index(selectedRow - 1, selected.column());
                boardListView->setCurrentIndex(newIdx);

                // let the MainWindow know the displayOrder changed
                Q_EMIT onBoardEdit(thisBoard, PreferencesDlg::BoardEditAction::MoveUp);
            }
        });

    QObject::connect(moveDownBtn, &QToolButton::clicked,
        [this](bool)
        {
            QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(boardListView->model());
            QModelIndex selected = boardListView->currentIndex();

            if (selected.isValid() && selected.row() < (model->rowCount() - 1)) // zero based index
            {
                auto selectedRow = selected.row();

                // change the displayOrder property of the selected board
                auto thisPropName = QString("data_%1").arg(selected.row());
                auto thisBoard = model->property(thisPropName.toLatin1()).value<BoardPtr>();
                auto sbdo = thisBoard->getOptions()->get<std::uint32_t>("displayOrder");
                thisBoard->getOptions()->setOrAdd("displayOrder",(uint)sbdo + 1);

                // change the displayOrder property of the board above it
                auto otherPropName = QString("data_%1").arg(selected.row() + 1);
                auto otherBoard = model->property(otherPropName.toLatin1()).value<BoardPtr>();
                sbdo = otherBoard->getOptions()->get<std::uint32_t>("displayOrder");
                otherBoard->getOptions()->setOrAdd("displayOrder",(uint)sbdo - 1);

                // update the model properties
                model->setProperty(thisPropName.toLatin1(), QVariant::fromValue(otherBoard));
                model->setProperty(otherPropName.toLatin1(), QVariant::fromValue(thisBoard));

                // remove the item from the data model
                auto rowList = model->takeRow(selectedRow);
                model->insertRow(selectedRow + 1, rowList);

                // change the view's selection
                QModelIndex newIdx = model->index(selectedRow + 1, selected.column());
                boardListView->setCurrentIndex(newIdx);

                // make sure to notify the main window
//                _treeModified = true;

                // let the MainWindow know the displayOrder changed
                Q_EMIT onBoardEdit(thisBoard, PreferencesDlg::BoardEditAction::MoveDown);
            }
        });

    QObject::connect(showBoardIconsCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("boardlist.icons.visible", checked);
            Q_EMIT reloadBoardPanel();
        });
}

void PreferencesDlg::renderEditorSettings()
{
    auto idx = -1;
    Dictionary currentDict;
    auto dicts = SpellChecker::availableDictionaries();
    QMapIterator<QString, Dictionary> it(dicts);
    const QString languageName = SettingsObject().read("editor.spellcheck.language").toString();

    while (it.hasNext())
    {
        it.next();

        auto dictionary = it.value();
        QString menuItem = QString("%1 / %2")
            .arg(dictionary.languageName())
            .arg(dictionary.countryName());

        languageBox->addItem(menuItem, QVariant::fromValue(dictionary));

        if (it.key() == languageName)
        {
            idx = languageBox->count() - 1;
            currentDict = it.value();
        }
    }

    if (idx >= 0)
    {
        languageBox->setCurrentIndex(idx);
    }

    // set the check as typing setting
    checkAsTyping->setChecked(_settings.read("editor.spellcheck.enabled").toBool());

    for(int size : QFontDatabase::standardSizes())
    {
        sizeComboBox->addItem(QString().setNum(size));
    }

    const QFont editorFont(_settings.read("editor.font.family").toString());
    fontComboBox->setCurrentFont(editorFont);


    sizeComboBox->setCurrentText(QString::number(_settings.read("editor.font.size").toInt()));
    sizeComboBox->setValidator(new QIntValidator(1,1000));
}

void PreferencesDlg::connectEditorSettings()
{
    QObject::connect(resetBtn, &QPushButton::clicked,
        [this]()
        {
            if (!SpellChecker::instance()->resetWordlist())
            {
                QMessageBox::warning(this, APP_TITLE, tr("Could not delete the user dictionary. Please check your permissions and try again."), QMessageBox::Ok);
            }
    });

    QObject::connect(fontComboBox, &QFontComboBox::currentFontChanged,
        [this](const QFont& font)
        {
            _settings.write("editor.font.family", font.family());
        });

    QObject::connect(sizeComboBox, &QComboBox::currentTextChanged,
        [this](const QString& size)
        {
            _settings.write("editor.font.size", size.toInt());
        });

    QObject::connect(checkAsTyping, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("editor.spellcheck.enabled", checked);
        });

    QObject::connect(languageBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [this](int index)
        {
            QVariant currentData = languageBox->itemData(index);
            Dictionary dictionary = currentData.value<Dictionary>();

            QString language = dictionary.language().remove(".dic");
            language.truncate(5);

            _settings.write("editor.spellcheck.language", language);
        });
}

void PreferencesDlg::renderProxySettings()
{

}

void PreferencesDlg::connectProxySettings()
{

}

void PreferencesDlg::renderParserSettings()
{
    SettingsObject settings;

    const auto COLUMN_COUNT = 2;

    parserList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    parserList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    parserList->setSelectionBehavior(QAbstractItemView::SelectRows);
    parserList->setSelectionMode(QAbstractItemView::SingleSelection);
    parserList->setAlternatingRowColors(true);

    luaParsersEnabledCB->setChecked(settings.read("parsers.enabled").toBool());
    luaParsersPathTB->setText(settings.read("parsers.path").toString());

    QStandardItemModel* model = new QStandardItemModel(0, COLUMN_COUNT, parserList);

    model->setHorizontalHeaderItem(0, new QStandardItem(QString(tr("Parser Name"))));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString(tr("Location"))));

    QHash<QString, ParserInfo> parsers = PARSERMGR->getParsers();

    auto keys = PARSERMGR->getParsers().keys();
    qSort(keys.begin(), keys.end(),[] (QString k1, QString k2) { return k2 > k1; });

    for (auto& k : keys)
    {
        auto pInfo = parsers[k];

        auto row = model->rowCount();

        auto item = new QStandardItem(pInfo.prettyName);
        item->setSizeHint(QSize(item->sizeHint().width(), 35));

        model->appendRow(item);
        model->setItem(row, 1, new QStandardItem(pInfo.filename));
    }

    parserList->setModel(model);
}

void PreferencesDlg::connectParserSettings()
{
    QObject::connect(luaParsersPathBrowseBtn, &QPushButton::clicked,
        [this](bool)
        {
            QFileDialog dlg;
            dlg.setFileMode(QFileDialog::Directory);
            dlg.setOption(QFileDialog::ShowDirsOnly);
            dlg.setFilter(QDir::Writable);
            dlg.setDirectory(SettingsObject().read("parsers.path").toString());

            if (dlg.exec())
            {
                QDir dir(dlg.selectedFiles().at(0));
                if (dir.exists())
                {
                    luaParsersPathTB->setText(dlg.selectedFiles().at(0));
                }
            }
    });
}

void PreferencesDlg::renderInternetSettings()
{
    // add Owl's useragent tobrowseLogFilePB the list at the the first postiion
    const QString defaultAgent = QString("Mozilla/5.0 Firefox/3.5.6 %1 / %2").arg(APP_NAME).arg(OWL_VERSION);
    userAgentCB->insertItem(0, (QString)defaultAgent);

    const auto useragent = _settings.read("web.useragent").toString();
    const auto idx = userAgentCB->findText(useragent);
    if (idx != -1)
    {
        userAgentCB->setCurrentIndex(idx);
    }
    else
    {
        // the user probably set a custom user agent string, so add it to the list and then select it
        userAgentCB->insertItem(0, useragent);
        userAgentCB->setCurrentIndex(0);
    }

    userAgentCB->lineEdit()->setCursorPosition(0);

    enableProxyCB->setChecked(_settings.read("proxy.enabled").toBool());
    addressTB->setText(_settings.read("proxy.host").toString());
    addressTB->setEnabled(enableProxyCB->isChecked());
    portTB->setText(_settings.read("proxy.port").toString());
    portTB->setEnabled(enableProxyCB->isChecked());
    usernameTB->setText(_settings.read("proxy.authentication.username").toString());
    usernameTB->setEnabled(enableProxyCB->isChecked());
    passwordTB->setText(_settings.read("proxy.authentication.password").toString());
    passwordTB->setEnabled(enableProxyCB->isChecked());
    const auto index = proxyType->findText(_settings.read("proxy.type").toString(), Qt::MatchFixedString);
    proxyType->setCurrentIndex(index);
    proxyType->setEnabled(enableProxyCB->isChecked());

    // TODO: Issue #36, hiding the proxy settings for now since they're not fully implemented
    proxyGroupBox->setVisible(false);
}

void PreferencesDlg::connectInternetSettings()
{
    QObject::connect(userAgentCB, &QComboBox::currentTextChanged,
        [this](const QString& newText)
        {
            _settings.write("web.useragent", newText);
        });

    QObject::connect(enableProxyCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("proxy.enabled",checked);

            addressTB->setEnabled(checked);
            portTB->setEnabled(checked);
            usernameTB->setEnabled(checked);
            passwordTB->setEnabled(checked);
            proxyType->setEnabled(checked);
        });

    QObject::connect(addressTB, &QLineEdit::editingFinished,
        [this]()
        {
            _settings.write("proxy.host", addressTB->text());
        });

    QObject::connect(portTB, &QLineEdit::editingFinished,
        [this]()
        {
            _settings.write("proxy.port", portTB->text().toInt());
        });

    QObject::connect(usernameTB, &QLineEdit::editingFinished,
        [this]()
        {
            _settings.write("proxy.authentication.username", usernameTB->text());
        });

    QObject::connect(usernameTB, &QLineEdit::editingFinished,
        [this]()
        {
            _settings.write("proxy.authentication.password", usernameTB->text());
        });

    QObject::connect(proxyType, &QComboBox::currentTextChanged,
        [this](const QString& newText)
        {
            _settings.write("proxy.type", newText);
        });
}

void PreferencesDlg::renderAdvancedSettings()
{
    int iLogLevelIdx = loggingLevelLB->findText(_settings.read("logs.level").toString(), Qt::MatchFixedString);

    if (iLogLevelIdx >= 0)
    {
        this->loggingLevelLB->setCurrentIndex(iLogLevelIdx);
    }

    logToFileCB->setChecked(_settings.read("logs.file.enabled").toBool());
    logFileTB->setText(_settings.read("logs.file.path").toString());
    logFileTB->setCursorPosition(0);
}

void PreferencesDlg::connectAdvancedSettings()
{
    QObject::connect(browseLogFilePB, &QPushButton::clicked,
        [this](bool)
        {
            QFileDialog dlg;
            dlg.setFileMode(QFileDialog::Directory);
            dlg.setOption(QFileDialog::ShowDirsOnly);
            dlg.setFilter(QDir::Writable);
            dlg.setDirectory(_settings.read("logs.file.path").toString());

            if (dlg.exec())
            {
                QFileInfo info(dlg.selectedFiles().at(0));
                if (!info.isDir() || !info.isWritable())
                {
                    const QString msg = QString("The selected folder does not exist or is read only. Please select another folder.");
                    QMessageBox msgBox { QMessageBox::Warning, tr("Invalid Folder"), msg, QMessageBox::Ok, this, Qt::Sheet};
                    logger()->warn("The log file folder '%1' is invalid. Make sure it exists and is writable.", info.absolutePath());
                    msgBox.open();
                    return;
                }

                logFileTB->setText(dlg.selectedFiles().at(0));
                _settings.write("logs.file.path", logFileTB->text());
                resetLogFileAppender();
            }
        });

    QObject::connect(loggingLevelLB, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [this](int index)
        {
            bool bOk = false;
            const Log4Qt::Level level = Log4Qt::Level::fromString(this->loggingLevelLB->itemText(index), &bOk);
            if (bOk && level != Log4Qt::Level::NULL_INT)
            {
                _settings.write("logs.level", loggingLevelLB->currentText());
                const Level loglevel = Log4Qt::Level::fromString(loggingLevelLB->currentText());
                Log4Qt::Logger::rootLogger()->setLevel(loglevel);
            }
        });

    QObject::connect(logToFileCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("logs.file.enabled", checked);
            resetLogFileAppender();
    });
}

void PreferencesDlg::resetLogFileAppender()
{
    if (_settings.read("logs.file.enabled").toBool())
    {
        if (Log4Qt::Logger::rootLogger()->appender("static:app.rolling.appender") == nullptr)
        {
            QDir logDir(_settings.read("logs.file.path").toString());
            const QString logFilename { logDir.absoluteFilePath("owl.log") };

            TTCCLayout *p_layout = new TTCCLayout();
            p_layout->setDateFormat(Log4Qt::TTCCLayout::DateFormat::ABSOLUTEFMT);
            p_layout->activateOptions();

            RollingFileAppender* appender = new RollingFileAppender(p_layout, logFilename, true);
            appender->setName("static:app.rolling.appender");
            appender->setMaxFileSize("100KB");
            appender->setMaxBackupIndex(3);
            appender->activateOptions();
            Log4Qt::Logger::rootLogger()->addAppender(appender);
        }
    }
    else
    {
        if (Log4Qt::Logger::rootLogger()->appender("static:app.rolling.appender") != nullptr)
        {
           Log4Qt::Logger::rootLogger()->removeAppender("static:app.rolling.appender");
        }
    }
}

void PreferencesDlg::renderInterfaceSettings()
{
    renderGeneralSettings();
    renderBoardPaneSettings();
    renderThreadPaneSettings();
    renderPostPaneSettings();
}

void PreferencesDlg::renderGeneralSettings()
{
    const QString dtFormat = _settings.read("datetime.format").toString();

    if (dtFormat == "default")
    {
        useDefaultFormatRB->setChecked(true);
        dateFormatCB->setEnabled(true);
        timeFormatCB->setEnabled(true);
        usePrettyCB->setEnabled(true);
    }
    else if (dtFormat == "moment")
    {
        useMomentFormatRB->setChecked(true);
        dateFormatCB->setEnabled(false);
        timeFormatCB->setEnabled(false);
        usePrettyCB->setEnabled(false);
    }
    else
    {
        logger()->warn("Invalid datetime.format, value is '%1'. Using 'default' instead.", dtFormat);
        _settings.write("datetime.format", "default");
        useDefaultFormatRB->setChecked(true);
    }

    usePrettyCB->setChecked(_settings.read("datetime.date.pretty").toBool());
    
    QDateTime exampleDT { QDateTime::currentDateTime() };

    const auto timeSetting = _settings.read("datetime.time.format").toString();
    int itemCount = timeFormatCB->count();
    for (int idx = 0; idx < itemCount; idx++)
    {
        QString itemText = timeFormatCB->itemText(idx);
        timeFormatCB->setItemData(idx, itemText);
        if (timeSetting == itemText)
        {
            timeFormatCB->setCurrentIndex(idx);
        }
        timeFormatCB->setItemText(idx, exampleDT.toString(itemText));
    }

    const auto dateSetting = _settings.read("datetime.date.format").toString();
    itemCount = dateFormatCB->count();
    for (int idx = 0; idx < itemCount; idx++)
    {
        QString itemText = dateFormatCB->itemText(idx);
        dateFormatCB->setItemData(idx, itemText);

        if (itemText == "long")
        {
            itemText = exampleDT.date().toString(Qt::SystemLocaleLongDate);
            if (dateSetting == "long")
            {
                dateFormatCB->setCurrentIndex(idx);
            }
        }
        else if (itemText == "short")
        {
            itemText = exampleDT.date().toString(Qt::SystemLocaleShortDate);
            if (dateSetting == "short")
            {
                dateFormatCB->setCurrentIndex(idx);
            }
        }
        else if (itemText == "textdate")
        {
            itemText = exampleDT.date().toString(Qt::TextDate);
            if (dateSetting == "textdate")
            {
                dateFormatCB->setCurrentIndex(idx);
            }
        }

        dateFormatCB->setItemText(idx, itemText);
    }
}

void PreferencesDlg::renderBoardPaneSettings()
{
    showBoardIconsCB->setChecked(_settings.read("boardlist.icons.visible").toBool());

    boardBGColorLE->setText(_settings.read("boardlist.background.color").toString());
    colorSampleLBL2->setStyleSheet(QString("QLabel { border: 2px solid; background-color : %1; }")
        .arg(_settings.read("boardlist.background.color").toString()));

    boardTextColorLE->setText(_settings.read("boardlist.text.color").toString());
    boardTextColorSampleLBL->setStyleSheet(QString("QLabel { border: 2px solid; background-color : %1; }")
        .arg(_settings.read("boardlist.text.color").toString()));
}

void PreferencesDlg::renderThreadPaneSettings()
{
    threadListShowAvatarsCB->setChecked(_settings.read("threadlist.avatars.visible").toBool());
    threadListShowLastAuthorCB->setChecked(_settings.read("threadlist.lastauthor.visible").toBool());
    threadListShowPreviewCB->setChecked(_settings.read("threadlist.previewtext.visible").toBool());

    const QString newPostsSetting = _settings.read("navigation.thread.newposts").toString();
    const std::vector<std::pair<QString, QString>> newPosts = {
        { tr("Go to First New Post"), "new" },
        { tr("Go to First Post"), "first" },
        { tr("Go to Last Post") , "last" }
    };

    int idx = 0;
    for (const auto& item : newPosts)
    {
        newPostsCB->addItem(item.first);
        newPostsCB->setItemData(idx, item.second);
        if (item.second == newPostsSetting)
        {
            newPostsCB->setCurrentIndex(idx);
        }
        idx++;
    }

    const QString noNewPostsSetting = _settings.read("navigation.thread.nonewposts").toString();
    const std::vector<std::pair<QString, QString>> noNewPosts = {
        { tr("Go to First Post"), "first" },
        { tr("Go to Last Post") , "last" }
    };

    idx = 0;
    for (const auto& item : noNewPosts)
    {
        noNewPostsCB->addItem(item.first);
        noNewPostsCB->setItemData(idx, item.second);
        if (item.second == noNewPostsSetting)
        {
            noNewPostsCB->setCurrentIndex(idx);
        }
        idx++;
    }
}

void PreferencesDlg::renderPostPaneSettings()
{
    QList<int> fontSizes = QFontDatabase::standardSizes();

    // The default setting for standard size and fixed sized, which is grabbed from
    // QWebEngineSettings may not be in QFontDatabase::standardSizes(), so we check
    // and add it if necessary, which then requires us to sort the list.
    const int standardSize = _settings.read("postlist.font.standard.size").toInt();
    const int fixedSize = _settings.read("postlist.font.fixed.size").toInt();
    if (!fontSizes.contains(standardSize))
    {
        fontSizes.append(standardSize);
    }

    if (!fontSizes.contains(fixedSize))
    {
        fontSizes.append(fixedSize);
    }

    qSort(fontSizes);

    for (int size : fontSizes)
    {
        postStandardFontSize->addItem(QString::number(size));
        postFixedFontSize->addItem(QString::number(size));
    }

    postStandardFont->setCurrentFont(_settings.read("postlist.font.standard").toString());
    postStandardFontSize->setCurrentText(QString::number(standardSize));

    postSerifFont->setCurrentFont(_settings.read("postlist.font.serif").toString());
    postSansSerifFont->setCurrentFont(_settings.read("postlist.font.sansserif").toString());

    postFixedFont->setCurrentFont(_settings.read("postlist.font.fixed").toString());
    postFixedFontSize->setCurrentText(QString::number(_settings.read("postlist.font.fixed.size").toInt()));

    postListHoverCB->setChecked(_settings.read("postlist.highlight.enabled").toBool());
    pickColorBtn->setEnabled(_settings.read("postlist.highlight.enabled").toBool());
    postListColorLE->setText(_settings.read("postlist.highlight.color").toString());
    postListColorLE->setEnabled(_settings.read("postlist.highlight.enabled").toBool());
    colorSampleLBL->setStyleSheet(QString("QLabel { border: 2px solid; background-color : %1; }")
        .arg(_settings.read("postlist.highlight.color").toString()));

    postListShowAvatarsCB->setChecked(_settings.read("postlist.avatars.visible").toBool());
    collapseOldPostsCB->setChecked(_settings.read("navigation.posts.collapseold").toBool());
}

void PreferencesDlg::connectInterfaceSettings()
{
    connectGeneralSettings();
    connectBoardPaneSettings();
    connectThreadPaneSettings();
    connectPostPaneSettings();
}

void PreferencesDlg::connectGeneralSettings()
{
    QObject::connect(useMomentFormatRB, &QRadioButton::clicked,
        [this](bool)
        {
            _settings.write("datetime.format", "moment");
            dateFormatCB->setEnabled(false);
            timeFormatCB->setEnabled(false);
            usePrettyCB->setEnabled(false);
            Q_EMIT reloadThreadPanel();
            Q_EMIT reloadPostPanel();
        });

    QObject::connect(useDefaultFormatRB, &QRadioButton::clicked,
        [this](bool)
        {
            _settings.write("datetime.format", "default");
            dateFormatCB->setEnabled(true);
            timeFormatCB->setEnabled(true);
            usePrettyCB->setEnabled(true);
            Q_EMIT reloadThreadPanel();
            Q_EMIT reloadPostPanel();
        });

    QObject::connect(usePrettyCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("datetime.date.pretty", checked);
            Q_EMIT reloadThreadPanel();
            Q_EMIT reloadPostPanel();
        });

    QObject::connect(dateFormatCB, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [this](int index)
        {
            const QString dateSetting = dateFormatCB->itemData(index).toString();
            _settings.write("datetime.date.format", dateSetting);
            Q_EMIT reloadThreadPanel();
            Q_EMIT reloadPostPanel();
        });

    QObject::connect(timeFormatCB, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [this](int index)
        {
            const QString setting = timeFormatCB->itemData(index).toString();
            _settings.write("datetime.time.format", setting);
            Q_EMIT reloadThreadPanel();
            Q_EMIT reloadPostPanel();
        });
}

void PreferencesDlg::connectBoardPaneSettings()
{
    QObject::connect(pickColorBtn2, &QPushButton::clicked,
        [this](bool)
        {
            const QColor oldColor { _settings.read("boardlist.background.color").toString() };
            const QColor newColor = QColorDialog::getColor(oldColor, this);
            if (newColor.isValid())
            {
                boardBGColorLE->setText(newColor.name());
                colorSampleLBL2->setStyleSheet(QString("QLabel { border: 2px solid; background-color : %1; }").arg(newColor.name()));
                _settings.write("boardlist.background.color", newColor.name());
                Q_EMIT reloadBoardPanel();
            }
        });

    QObject::connect(boardBGColorLE, &QLineEdit::editingFinished,
        [this]()
        {
            const QString newColorTxt { boardBGColorLE->text() };

            if (QColor::isValidColor(newColorTxt))
            {
                colorSampleLBL2->setStyleSheet(QString("QLabel { border: 2px solid; background-color : %1; }").arg(newColorTxt));
                _settings.write("boardlist.background.color", newColorTxt);
                Q_EMIT reloadBoardPanel();
            }
        });

    QObject::connect(pickBoardTxtColorBTN, &QPushButton::clicked,
        [this](bool)
        {
            const QColor oldColor { _settings.read("boardlist.text.color").toString() };
            const QColor newColor = QColorDialog::getColor(oldColor, this);
            if (newColor.isValid())
            {
                boardTextColorLE->setText(newColor.name());
                boardTextColorSampleLBL->setStyleSheet(QString("QLabel { border: 2px solid; background-color : %1; }").arg(newColor.name()));
                _settings.write("boardlist.text.color", newColor.name());
                Q_EMIT reloadBoardPanel();
            }
        });


    QObject::connect(boardTextColorLE, &QLineEdit::editingFinished,
        [this]()
        {
            const QString newColorTxt { boardTextColorLE->text() };

            if (QColor::isValidColor(newColorTxt))
            {
                boardTextColorSampleLBL->setStyleSheet(QString("QLabel { border: 2px solid; background-color : %1; }").arg(newColorTxt));
                _settings.write("boardlist.text.color", newColorTxt);
                Q_EMIT reloadBoardPanel();
            }
        });
}

void PreferencesDlg::connectThreadPaneSettings()
{
    QObject::connect(threadListShowAvatarsCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("threadlist.avatars.visible", checked);
            Q_EMIT reloadThreadPanel();
            Q_EMIT reloadPostPanel();
        });

    QObject::connect(threadListShowLastAuthorCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("threadlist.lastauthor.visible", checked);
            Q_EMIT reloadThreadPanel();
            Q_EMIT reloadPostPanel();
        });

    QObject::connect(threadListShowPreviewCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("threadlist.previewtext.visible", checked);
            Q_EMIT reloadThreadPanel();
            Q_EMIT reloadPostPanel();
        });

    QObject::connect(newPostsCB, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [this](int index)
        {
            const QString data = newPostsCB->itemData(index).toString();
            _settings.write("navigation.thread.newposts", data);
        });

    QObject::connect(noNewPostsCB, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [this](int index)
        {
            const QString data = noNewPostsCB->itemData(index).toString();
            _settings.write("navigation.thread.nonewposts", data);
        });
}

void PreferencesDlg::connectPostPaneSettings()
{
    QObject::connect(postListHoverCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            postListColorLE->setEnabled(checked);
            pickColorBtn->setEnabled(checked);
            _settings.write("postlist.highlight.enabled", checked);
            Q_EMIT reloadPostPanel();
        });

    QObject::connect(pickColorBtn, &QPushButton::clicked,
        [this](bool)
        {
            const QColor oldColor { _settings.read("postlist.highlight.color").toString() };
            const QColor newColor = QColorDialog::getColor(oldColor, this);
            if (newColor.isValid())
            {
                postListColorLE->setText(newColor.name());
                colorSampleLBL->setStyleSheet(QString("QLabel { border: 2px solid; background-color : %1; }").arg(newColor.name()));
                Q_EMIT reloadPostPanel();
            }
    });

    QObject::connect(collapseOldPostsCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("navigation.posts.collapseold", checked);
        });

    QObject::connect(postListShowAvatarsCB, &QCheckBox::clicked,
        [this](bool checked)
        {
            _settings.write("postlist.avatars.visible", checked);
            Q_EMIT reloadPostPanel();
        });
}

} // end namespace
