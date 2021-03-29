#pragma once
#include <memory>
#include <QtCore>
#include <QtGui>
#include <QPlainTextEdit>
#include <QtCore5Compat/QTextCodec>

class Hunspell;
typedef std::shared_ptr<Hunspell> HunspellPtr;

namespace owl
{

class SpellChecker;
typedef std::shared_ptr<SpellChecker> SpellCheckerPtr;

class EditorHighlighter;
typedef std::shared_ptr<EditorHighlighter> EditorHighlighterPtr;

/////////////////////////////////////////////////////////////////////////
// Dictionary
/////////////////////////////////////////////////////////////////////////

class Dictionary
{

public:
	Dictionary() = default;
	Dictionary(const QString& language, 
				const QString& filePath) :
		_language(language),
		_filePath(filePath)
	{
	}

	Dictionary(const Dictionary& other)
	{
		_language = other._language;
		_filePath = other._filePath;
	}

	virtual ~Dictionary() 
	{ 
	}

	QString language() const { return _language; }
	QString filePath() const { return _filePath; }

	QString languageName() const 
	{
		return QLocale(_language).nativeLanguageName();
	}

	QString countryName() const
	{
		return QLocale(_language).nativeCountryName();
	}

private:
	QString _language;
	QString _filePath;

};

/////////////////////////////////////////////////////////////////////////
// SpellChecker
/////////////////////////////////////////////////////////////////////////

#define SPELLCHECKER	SpellChecker::instance()

class SpellChecker : public QObject
{
	Q_OBJECT

	HunspellPtr	_spellcheck;
	QTextCodec*	_textCodec = nullptr;

	static SpellCheckerPtr	_instance;

public:
	static SpellCheckerPtr instance();
    static const QString GetCustomDictionaryName();
	static QMap<QString, Dictionary> availableDictionaries();

	virtual ~SpellChecker() = default;

	void init();

	void loadDictionary(const QString &dictFilePath);

    void loadUserWordlist();
    void addToUserWordlist(const QString&);
    bool resetWordlist();

    bool isCorrect(const QString& word);
    QStringList suggestions(const QString&);

private: 
	explicit SpellChecker() = default;
};

/////////////////////////////////////////////////////////////////////////
// MarkdownHighlighter
/////////////////////////////////////////////////////////////////////////

class EditorHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	EditorHighlighter(QTextDocument *document, SpellCheckerPtr spellChecker);
    ~EditorHighlighter() = default;

	void setSpellingCheckEnabled(bool enabled);

protected:
	void highlightBlock(const QString &textBlock) override;

private:
    SpellCheckerPtr _spellChecker;

    const QRegularExpression   _charsOnly;
    QTextCharFormat _spellFormat;
    QTextCursor     _currentCursor;
};

/////////////////////////////////////////////////////////////////////////
// SpellCheckEditor
/////////////////////////////////////////////////////////////////////////

class SpellCheckEdit : public QPlainTextEdit
{
    Q_OBJECT

    EditorHighlighterPtr		_highlighter;
    SpellCheckerPtr				_spellChecker;
    int                        _maxLength = 10000;

public:
    SpellCheckEdit(QWidget* parent = 0);
    virtual ~SpellCheckEdit() = default;

    const uint maxLength() const { return _maxLength; }
    void setMaxLength(const uint val) { _maxLength = val; }

Q_SIGNALS:
    void focusChangedEvent(bool bHasFocus);

protected:
    virtual void focusInEvent(QFocusEvent *e) override;
    virtual void focusOutEvent(QFocusEvent *e) override;

private Q_SLOTS:
    void showContextMenu(const QPoint&);
};

} // namespace

Q_DECLARE_METATYPE(owl::Dictionary);
