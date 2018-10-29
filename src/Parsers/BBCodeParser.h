#pragma once
#include <stack>
#include <vector>
#include <memory>
#include <QtCore>
#include <log4qt/logger.h>

namespace owl
{

class QuoteFormatter
{
public:
    virtual ~QuoteFormatter() = default;

    virtual QString getPreQuoteReplacer() = 0;
    virtual QString getQuoteBody(const QString& text) = 0;
    virtual QString getUsernameReplacer(const QString& username) = 0;
    virtual QString getQuoteBody(const QString& username, const QString& body) = 0;
};

using QuoteFormatterPtr = std::shared_ptr<QuoteFormatter>;

class HTMLQuoteFormatter final : public QuoteFormatter
{
public:
    QString getPreQuoteReplacer() override
    {
        return QString("<b>\\1</b> wrote:<br/>");
    }

    QString getUsernameReplacer(const QString& username) override
    {
        return QString("<b>%1</b> wrote:<br/>").arg(username);
    }

    QString getQuoteBody(const QString& body) override
    {
        return QString("<blockquote>%1</blockquote>").arg(body);
    }

    QString getQuoteBody(const QString& username, const QString& body) override
    {
        return QString("<blockquote><b>%1</b> wrote:<br/><br/>%2</blockquote>")
                .arg(username)
                .arg(body);
    }
};

class TextQuoteFormatter final : public QuoteFormatter
{
public:
    QString getPreQuoteReplacer() override
    {
        return QString("\\1 wrote:\n");
    }

    QString getUsernameReplacer(const QString& username) override
    {
        return QString("%1 wrote:\n").arg(username);
    }

    QString getQuoteBody(const QString& body) override
    {
        const size_t width = 50;
        QString retval;

        for (size_t i = 0; i < static_cast<size_t>(body.size()); i += width)
        {
            retval += QString("> %1\n").arg(body.mid(static_cast<int>(i),width));
        }

        return retval + "\n";
    }

    QString getQuoteBody(const QString& username, const QString& body) override
    {
        return QString("%1%2")
                .arg(getUsernameReplacer(username))
                .arg(getQuoteBody(body));
    }
};

class StripQuoteFormatter final : public QuoteFormatter
{
public:
    QString getPreQuoteReplacer() override
    {
        return QString("\\1 wrote:\n");
    }

    QString getUsernameReplacer(const QString& username) override
    {
        return QString("%1 wrote:\n").arg(username);
    }

    QString getQuoteBody(const QString& body) override
    {
        const auto width = 50;
        QString retval;

        for (size_t i = 0; i < static_cast<size_t>(body.size()); i += width)
        {
            retval += QString("> %1\n").arg(body.mid(static_cast<int>(i),width));
        }

        return retval + "\n";
    }

    QString getQuoteBody(const QString& username, const QString& body) override
    {
        return QString("%1%2")
                .arg(getUsernameReplacer(username))
                .arg(getQuoteBody(body));
    }
};


class BBRegExParser : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER

public:

    enum class QuoteStyle
    {
        UNKNOWN,
        VBULLETIN,      // [QUOTE=Max Power;12345]
        KEYVALUE,       // [QUOTE uid=26297 name="TheRover" post=1120490]
        COLONDELIM      // [QUOTE="Max Power, post: 12345, member: 1"]
    };

    BBRegExParser(const QuoteStyle& style, QObject* parent);
    BBRegExParser(QObject* parent = nullptr);

    virtual ~BBRegExParser() = default;

    const QuoteStyle getQuoteStyle() const { return _quotestyle; }
    void setQuoteStyle(const QuoteStyle& style) { _quotestyle = style; }
    void resetQuoteStyle() { _quotestyle = QuoteStyle::UNKNOWN; }

    QString toHtml(const QString& bbcode);
    QString toPlainText(const QString& bbcode);

private:
    using RegExReplacePair = std::pair<QRegularExpression, QString>;

    // default pattern options
    const QRegularExpression::PatternOptions _ro =
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption;

    // basic patterns that are "easily" handled
	std::vector<RegExReplacePair> _regexpairs;

    const QRegularExpression _simpleQuoteRegEx;
    const QRegularExpression _simpleQuoteFullRegEx;
    const QRegularExpression _quotePrefix;

    const QRegularExpression _vbQuoteFixRegEx;

    // will look for something like: [QUOTE=Max Power;12345]
    const QRegularExpression _vbulletinQuote;

    // will look for something like: [QUOTE uid=26297 name="TheRover" post=1120490]
    const QRegularExpression _quoteWithValues;

    QuoteStyle  _quotestyle = QuoteStyle::UNKNOWN;

    QString handleSimpleQuote(const QString& original, QuoteFormatterPtr qf);

    QString handleVBQuote(const QString& original, QuoteFormatterPtr qf);
    std::pair<QString, quint32> handleVBUserString(const QString& original);

    QString handleQuoteWithValues(const QString& original);
};

//} // namespace


//// Simple Tags [tag]
//// Value Tags [name=value]
//// Parametrized Tags [tag param=value]
//
//// [img]http://this.is.an.image/link.img[/img]
//// [url=http://www.google.com]This is a link[/url]
//
//namespace owl
//{
//
//using TagFormat = std::pair<QString, QString>;
//const std::vector<TagFormat> tagConfig =
//{
//    { "IMG", "<%1 src=\"%2\" />" }
//};
//
//class BBToken;
//using BBTokenPtr = std::shared_ptr<BBToken>;
//using BBTokenList = std::list<BBToken>;
//
//class BBNode;
//class BBTag;
//class BBText;
//
//using BBNodePtr = std::shared_ptr<BBNode>;
//using BBNodeWeakPtr = std::weak_ptr<BBNode>;
//using BBNodeList = std::vector<BBNodePtr>;
//using BBTagPtr = std::shared_ptr<BBTag>;
//using BBParameters = std::map<QString, QString>;
//
//class BBNode : public std::enable_shared_from_this<BBNode>
//{
//public:
//    enum class NodeType
//    {
//        SIMPLE,
//        VALUE,
//        PARAMETER,
//        TEXT
//    };
//
//   BBNode(const QString& text, const NodeType tagType)
//        : _text(text),
//          _type(tagType)
//    {
//        // do nothing
//    }
//
//    virtual ~BBNode() = default;
//
//    void setText(const QString& text) { _text = text; }
//    const QString getText() const { return _text; }
//
//    virtual bool isTag() { return false; }
//
//    const NodeType getNodeType() const { return _type; }
//
//    virtual const QString getBBCode() const = 0;
//    virtual const QString getHtml() const = 0;
//
//private:
//
//    QString         _text;
//    NodeType         _type;
//};
//
//class BBTag : public BBNode
//{
//    bool            _bClosingTag = false;
//    BBTagPtr        _matchTag = nullptr;
//    QString         _htmlFormat;
//
//public:
//    BBTag(const QString& name, bool closingTag = false)
//        : BBTag(name, closingTag, BBNode::NodeType::SIMPLE)
//    {
//        // nothing
//    }
//
//    virtual ~BBTag() = default;
//
//    virtual const QString getBBCode() const override
//    {
//        return "[" + QString(isClosingTag() ? "/" : "") + getText() + "]";
//    }
//
//    virtual const QString getHtml() const override
//    {
//        return "<" + QString(isClosingTag() ? "/" : "") + getTagName() + ">";
//    }
//
//    const QString getTagName() const { return getText(); }
//
//    const BBTagPtr getMatchedTag() const { return _matchTag; }
//    void setMatchedTag(BBTagPtr match)
//    {
//        _matchTag = match;
//    }
//
//    void setClosingTag(bool bClosing) { _bClosingTag = bClosing; }
//    virtual bool isClosingTag() const { return _bClosingTag; }
//
//    virtual bool isTag() override { return true; }
//
//protected:
//    BBTag(const QString& name, bool closingTag, NodeType type)
//        : BBNode(name, type),
//          _bClosingTag(closingTag)
//    {
////        if (_bClosingTag)
////        {
////            _htmlFormat = "</" + getTagName() + ">";
////        }
////        else if (tagConfig.find(name.toUpper()))
////        {
////            _htmlFormat = tagConfig[name.toUpper()];
////        }
////        else
////        {
////            _htmlFormat = "<%1>"
////        }
//    }
//};
//
//class BBValueTag : public BBTag
//{
//    QString     _value;
//
//public:
//    BBValueTag(const QString& name = QString(), const QString& value = QString())
//        : BBTag(name, false, BBNode::NodeType::VALUE),
//          _value(value)
//    {
//        // do nothing
//    }
//
//    virtual ~BBValueTag() = default;
//
//    const QString getValue() const { return _value; }
//
//    virtual const QString getBBCode() const override
//    {
//        return "[" + getText() + "=" + getValue() + "]";
//    }
//
////    virtual const QString getHtml() const override
////    {
////        if (isClosingTag())
////        {
////            return BBTag::getHtml();
////        }
////        else
////        {
//////            const auto& tagname = getTagName();
//////            if (tagname.toUpper() == QString("IMG"))
//////            {
//////                return "<img src=\"" + getValue() + "\" />";
//////            }
//////            else if (tagname.toUpper() == QString("URL"))
//////            {
//////                return "<a href="
//////            }
//////            {
//////                return "<"
//////            }
////        }
////    }
//};
//
//class BBParameterTag : public BBTag
//{
//    BBParameters _params;
//
//public:
//    BBParameterTag(const QString& name, const BBParameters& params)
//        :  BBTag(name, false, BBNode::NodeType::PARAMETER),
//          _params(params)
//    {
//        // nothing to do
//    }
//
//    virtual ~BBParameterTag() = default;
//
//    virtual const QString getBBCode() const override
//    {
//        return "[" + getText() + "!!PARAMETERS!!]";
//    }
//
//};
//
//class BBText : public BBNode
//{
//public:
//    BBText(const QString& text = QString())
//        : BBNode(text, BBNode::NodeType::TEXT)
//    {
//        // do nothing
//    }
//
//    virtual ~BBText() = default;
//
//    virtual const QString getBBCode() const override
//    {
//        return getText();
//    }
//
//    virtual const QString getHtml() const override
//    {
//        return getText();
//    }
//};
//
//class BBToken : public QObject
//{
//	Q_OBJECT
//	LOG4QT_DECLARE_QCLASS_LOGGER
//
//public:
//    enum class TokenType
//	{
//		TT_UNKNOWN,
//		TT_TEXT,
//		TT_OPENBRACKET,
//		TT_CLOSEBRACKET,
//		TT_EQUAL,
//		TT_FWDSLASH,
//		TT_NEWLINE,
//		TT_DOUBLEQUOTE,
//		TT_SINGLEQUOTE,
//        TT_ASTERISK,
//        TT_SPACE
//	};
//
//    BBToken(TokenType tokenType, const QString& var)
//		: _value(var),
//		  _type(tokenType)
//	{
//		// do nothing
//	}		  
//
//    BBToken(TokenType tokenType)
//        : BBToken(tokenType, QString())
//	{
//		// do nothing
//	}
//
//    BBToken(const BBToken& other)
//        : BBToken(other._type, other._value)
//	{
//		// do nothing!
//	}
//
//    BBToken()
//	{
//		// do nothing;
//	}
//
//    virtual ~BBToken()
//	{
//		// do nothing
//	}
//
//    BBToken& operator=(const BBToken& other)
//    {
//        _type = other._type;
//        _value = other._value;
//
//        return *this;
//    }
//
//    virtual const TokenType getType() const { return _type; }
//    virtual const QString getText() const { return _value; }
//
//private:
//	QString	_value;
//    TokenType	_type = TokenType::TT_UNKNOWN;
//};
//
///**
//* \brief Scanner class breaks raw BBCode text into logical tokens
//*
//* Scans the text stream and returns a vector of BBCTokens for
//* processing by the BBCParser
//* 
//* \note All the functions declared in this class are thread-safe.
//*
//* \see BBCParser
//*
//*/
//class BBScanner : public QObject
//{
//	Q_OBJECT
//	LOG4QT_DECLARE_QCLASS_LOGGER
//
//public: 
//
//    BBScanner() = default;
//    virtual ~BBScanner() = default;
//
//    virtual BBTokenList Scan(QString* text);
//    virtual BBTokenList Scan(QTextStream& buffer);
//
//    const BBTokenList Tokens() const { return _tokens; }
//
//private:
//
//    BBTokenList	_tokens;
//};
//
//class BBParser : public QObject
//{
//	Q_OBJECT
//	LOG4QT_DECLARE_QCLASS_LOGGER
//
//    const std::vector<QString> _validTags =
//    {
//        "B", "I", "U", "S"
//    };
//
//public: 
//	
//    BBParser(const BBTokenList& tokens)
//        : _tokens(tokens)
//    {
//        // do nothing
//    }
//
//	bool parse(bool bThrowOnError = false);
//
//    const BBNodeList getNodeList() const { return _nodelist; }
//
//    const bool isValidTagName(const QString& tagname) const;
//
//private:
//
//    bool privateParse(bool bThrowOnError);
//    bool matchedParse(bool bThrowOnError);
//
//    bool parseOpenTag();
//    bool parseCloseTag();
//
//    QString popFrontText()
//    {
//        const auto retval = _tokens.front().getText();
//        _tokens.pop_front();
//
//        return retval;
//    }
//
//	// The List of tokens returned from BBCScanner
//    BBTokenList				_tokens;
//
//    // The resulting BBNodeList from parsing
//    BBNodeList              _nodelist;
//};
//
//class TextGenerator : public QObject
//{
//    Q_OBJECT
//    LOG4QT_DECLARE_QCLASS_LOGGER
//
//    using TagFunc = std::function<void(BBTagPtr, QTextStream*)>;
//
//public:
//
//
//    TextGenerator() = default;
//    virtual ~TextGenerator() = default;
//
//    QString toHtml(const BBNodeList& nodelist);
//    QString toBBCode(const BBNodeList& nodelist);
//};
//
//class BBCToHTmlGenerator : public QObject
//{
//	Q_OBJECT
//	LOG4QT_DECLARE_QCLASS_LOGGER
//
//public:
//
//	BBCToHTmlGenerator()
//	{
//		// default constructor
//	}
//
//	virtual ~BBCToHTmlGenerator()
//	{
//		// default destructor
//	}
//
//	QString doGenerate(const QString& bbcode);
//
//private:
////	QString genStatement(BBCObjectPtr stmt);
//};

} // namespace
