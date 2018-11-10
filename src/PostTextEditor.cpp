// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include <QtCore>
#include <QtGui>
#include <QMenu>
#include "Data/Board.h"
#include <Parsers/ParserBase.h>
#include <Utils/Settings.h>
#include "PostTextEditor.h"

#include <hunspell/hunspell.hxx>

namespace owl
{

/////////////////////////////////////////////////////////////////////////
// SpellChecker
/////////////////////////////////////////////////////////////////////////

bool SpellChecker::isCorrect(const QString& word)
{
    if (!_spellcheck || !_textCodec)
    {
        return true;
    }

    return _spellcheck->spell(word.toStdString()) != 0;
}

QStringList SpellChecker::suggestions(const QString& word)
{
    QStringList retval;

    if (_spellcheck && _textCodec)
    {
        const auto wordList = _spellcheck->suggest(word.toStdString());
        for (const auto& sword : wordList)
        {
            retval << QString::fromStdString(sword);
        }
    }

    return retval;
}

void SpellChecker::loadDictionary(const QString &dictFilePath)
{
    _spellcheck.reset();

//    _logger->debug("Loading dictionary from: %1", dictFilePath);

    QString affixFilePath(dictFilePath);
    affixFilePath.replace(".dic", ".aff");

    _spellcheck = std::make_shared<Hunspell>(affixFilePath.toLocal8Bit(), dictFilePath.toLocal8Bit());
    _textCodec = QTextCodec::codecForName(_spellcheck->get_dic_encoding());

    if (!_textCodec) 
    {
        _textCodec = QTextCodec::codecForName("UTF-8");
    }

    // also load user word list
    loadUserWordlist();
}

void SpellChecker::loadUserWordlist()
{
    QFile userWordlistFile { GetCustomDictionaryName() };
    if (userWordlistFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&userWordlistFile);
        for (QString word = stream.readLine(); !word.isEmpty(); word = stream.readLine()) 
        {
            _spellcheck->add(_textCodec->fromUnicode(word).constData());
        }
    }
}

void SpellChecker::addToUserWordlist(const QString &word)
{
    const QString customDictionary { GetCustomDictionaryName() };
    _spellcheck->add(_textCodec->fromUnicode(word).constData());

    QFile file { GetCustomDictionaryName() };
    if (file.open(QIODevice::Append))
    {
        QTextStream stream(&file);
        stream << word << "\n";
        file.close();
    }
}

bool SpellChecker::resetWordlist()
{
    bool bReset = true;
    QFile file { GetCustomDictionaryName() };

    if (file.exists() && file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        for (QString word = stream.readLine(); !word.isEmpty(); word = stream.readLine())
        {
            _spellcheck->remove(_textCodec->fromUnicode(word).constData());
        }

        file.close();

        bReset = file.remove();
    }

    return bReset;
}

QMap<QString, Dictionary> SpellChecker::availableDictionaries()
{
    QMap<QString, Dictionary> dictionaries;
    QStringList paths;

#ifdef Q_OS_WIN
    // TODO: Where will the dictionaries live on Windows? Where should we look for them?
    paths << QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    paths << qApp->applicationDirPath();
#elif defined (Q_OS_OSX)
    // On OSX the dictionaries are in the bundle: Owl.app/Content/Resources/dictionaries
    QFileInfo temp(QCoreApplication::applicationDirPath() + "/../Resources");
    paths << temp.absoluteFilePath();
#endif

    for (auto& path : paths) 
    {
        QDir dictPath(path + QDir::separator() + "dictionaries");
        dictPath.setFilter(QDir::Files);
        dictPath.setNameFilters(QStringList() << "*.dic");
        
        if (dictPath.exists()) 
        {
            // loop over all dictionaries in directory
            QDirIterator it(dictPath);
            while (it.hasNext()) 
            {
                it.next();

                QString language = it.fileName().remove(".dic");
                language.truncate(5); // just language and country code

                Dictionary dict(it.fileName(), it.filePath());
                dictionaries.insert(language, dict);
            }
        }
    }

    return dictionaries;
}

owl::SpellCheckerPtr SpellChecker::instance()
{
    if (!_instance)
    {
        _instance.reset(new SpellChecker());
    }

    return _instance;
}

const QString SpellChecker::GetCustomDictionaryName()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).absoluteFilePath("owl.dict");
}

void SpellChecker::init()
{
    bool bFound = false;
    const QString languageStr = SettingsObject().read("editor.spellcheck.language").toString();

    const auto dictionaries = availableDictionaries().toStdMap();
    for (auto& item : dictionaries)
    {
        if (languageStr == item.first)
        {
            auto dict = item.second;
            loadDictionary(dict.filePath());
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        _spellcheck.reset();
        _textCodec = nullptr;
    }
}

SpellCheckerPtr SpellChecker::_instance = nullptr;

/////////////////////////////////////////////////////////////////////////
// MarkdownHighlighter
/////////////////////////////////////////////////////////////////////////

EditorHighlighter::EditorHighlighter(QTextDocument* document, SpellCheckerPtr spellChecker)
    : QSyntaxHighlighter(document),
      _spellChecker(spellChecker),
      _charsOnly("[A-Za-z]*")
{
    // QTextCharFormat::SpellCheckUnderline has issues with Qt 5.
    _spellFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    _spellFormat.setUnderlineColor(Qt::red);

    QObject::connect(this->document(), &QTextDocument::cursorPositionChanged,
        [this](const QTextCursor & cursor)
        {
            this->_currentCursor = cursor;
        });
}

void EditorHighlighter::highlightBlock(const QString &textBlock)
{
    if (!_spellChecker || !document()->isEmpty())
    {
        // get the position of the cursor in the current block
        auto positionInBlock = _currentCursor.positionInBlock();
        QStringList wordList = textBlock.split(QRegExp("\\W+"), QString::SkipEmptyParts);
        int index = 0;

        for (QString word : wordList)
        {
            const auto wordLength = word.length();

            index = textBlock.indexOf(word, index);

            // test positionInBlock, if this word contains the cursor then we don't want to bother
            // checking it since the user could still be typing`
            if ((positionInBlock < index || positionInBlock > (index + wordLength))
                    && _charsOnly.exactMatch(word)
                    && !_spellChecker->isCorrect(word))
            {
                setFormat(index, wordLength, _spellFormat);
            }

            index += word.length();
        }
    }
}

/////////////////////////////////////////////////////////////////////////
// SpellCheckEdit
/////////////////////////////////////////////////////////////////////////

SpellCheckEdit::SpellCheckEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    if (SettingsObject().read("editor.spellcheck.enabled").toBool())
    {
        _highlighter = EditorHighlighterPtr(
            new EditorHighlighter(this->document(), SpellChecker::instance()));
    }

    setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    QObject::connect(this, &QPlainTextEdit::textChanged,
        [this]()
        {
            // TODO: The MAXLENGTH check should be improved. Perhaps there should also be a label counter in the
            // dialog telling the user how many characters they have left
            if (this->toPlainText().length() > _maxLength)
            {
                auto diff = this->toPlainText().length() - _maxLength;
                QString newStr = this->toPlainText();
                newStr.chop(diff);
                this->document()->setPlainText(newStr);

                QTextCursor cursor(this->textCursor());
                cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
                this->setTextCursor(cursor);
            }
        });
}

void SpellCheckEdit::focusInEvent(QFocusEvent *e)
{
    QPlainTextEdit::focusInEvent(e);
    Q_EMIT focusChangedEvent(true);
}

void SpellCheckEdit::focusOutEvent(QFocusEvent *e)
{
    QPlainTextEdit::focusInEvent(e);
    Q_EMIT focusChangedEvent(false);
}

void SpellCheckEdit::showContextMenu(const QPoint& pos)
{
    auto contextMenu = this->createStandardContextMenu();
    auto spellChecker = SpellChecker::instance();

    if (spellChecker)
    {
        const QRegExp charsOnly("[A-Za-z]*");
        QTextCursor cursor = cursorForPosition(pos);
        cursor.select(QTextCursor::WordUnderCursor);
        const QString selectedText { cursor.selectedText() };

        if (cursor.hasSelection()
            && !spellChecker->isCorrect(selectedText)
            && charsOnly.exactMatch(selectedText))
        {
            int cursorPosition = cursor.position();
            auto suggestionList = spellChecker->suggestions(selectedText);
            auto newMenu = new QMenu(this);

            // add the suggestions
            for (auto i = 0; i < std::min(suggestionList.size(), 5); i++)
            {
                QAction* action = newMenu->addAction(suggestionList.at(i));

                QObject::connect(action, &QAction::triggered,
                    [cursor, cursorPosition, action](bool) mutable
                    {
                        cursor.beginEditBlock();

                        // replace wrong spelled word with suggestion
                        cursor.setPosition(cursorPosition);
                        cursor.select(QTextCursor::WordUnderCursor);
                        cursor.insertText(action->text());

                        cursor.endEditBlock();
                    });
            }

            const auto addToDict = newMenu->addAction(tr("Add to Dictionary"));
            QObject::connect(addToDict, &QAction::triggered,
                [this, selectedText, cursor, cursorPosition]() mutable
                {
                    SpellChecker::instance()->addToUserWordlist(selectedText);

                    // insert the selected block of text, this doesn't change the text but it does
                    // cause the highlighter to redraw itself with the newly added word
                    cursor.beginEditBlock();
                    cursor.setPosition(cursorPosition);
                    cursor.select(QTextCursor::WordUnderCursor);
                    cursor.insertText(selectedText);
                    cursor.endEditBlock();
                });

            newMenu->addSeparator();

            // get the default menu and add the QActions to our menu
            auto defaultMenu = this->createStandardContextMenu();
            for (auto a : defaultMenu->actions())
            {
                newMenu->addAction(a);
                if (a->parent() == contextMenu)
                {
                    a->setParent(newMenu);
                }
            }

            delete contextMenu;
            contextMenu = newMenu;
        }
    }

    // for some reason the default context menu puts white text on a white background for the selected
    // item, so manually set it here
    contextMenu->setStyleSheet("QMenu:item:selected { background: rgb(102, 153, 255); color: white; }");

    // show the contextMenu and cleanup when we're done
    contextMenu->exec(mapToGlobal(pos));
    delete contextMenu;
}

} // namespace

