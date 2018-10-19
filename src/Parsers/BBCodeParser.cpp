// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include <iterator>
#include <list>
#include "BBCodeParser.h"

namespace owl
{

BBRegExParser::BBRegExParser(const BBRegExParser::QuoteStyle &style, QObject *parent)
    : QObject(parent),
      _simpleQuoteRegEx("\\[quote\\]", QRegularExpression::CaseInsensitiveOption),
      _simpleQuoteFullRegEx("\\[quote\\](((?R)|.*?)*)\\[\\/quote\\]", _ro),
      _quotePrefix("\\[quote", QRegularExpression::CaseInsensitiveOption),
      _vbQuoteFixRegEx("^\\[url\\=.+?\\]Originally Posted by (.+?)\\[\\/url\\]", _ro),
      _vbulletinQuote("\\[quote=(.*?)\\](.*)\\[/quote\\]", _ro),
      _quoteWithValues("\\[quote\\s+(uid\\=(?<uid>\\d+)|name\\=\"(?<username>.+?)\"|post\\=(?<postid>\\d+)|\\s)+\\](?<quotetext>.*?)\\[/quote\\]", _ro),
      _quotestyle(style)
{
    _regexpairs.emplace_back(QRegularExpression("\\[(b)\\](.+?)\\[\\/\\1\\]", _ro), "<\\1>\\2</\\1>");
    _regexpairs.emplace_back(QRegularExpression("\\[(i)\\](.+?)\\[\\/\\1\\]", _ro), "<\\1>\\2</\\1>");
    _regexpairs.emplace_back(QRegularExpression("\\[(u)\\](.+?)\\[\\/\\1\\]", _ro), "<\\1>\\2</\\1>");
    _regexpairs.emplace_back(QRegularExpression("\\[(s)\\](.+?)\\[\\/\\1\\]", _ro), "<\\1>\\2</\\1>");
    _regexpairs.emplace_back(QRegularExpression("\\[url\\=(.+?)\\](.+?)\\[\\/url\\]", _ro), "<a href=\"\\1\">\\2</a>");
    _regexpairs.emplace_back(QRegularExpression("\\[url\\](.+?)\\[\\/url\\]", _ro), "<a href=\"\\1\">\\1</a>");
    _regexpairs.emplace_back(QRegularExpression("\\[img\\](.+?)\\[\\/img\\]", _ro), "<img src=\"\\1\" onload=\"NcodeImageResizer.createOn(this);\" />");
}

BBRegExParser::BBRegExParser(QObject *parent)
    : BBRegExParser(QuoteStyle::UNKNOWN, parent)
{
    // do nothing
}

QString BBRegExParser::toHtml(const QString &bbcode)
{
    QString retval(bbcode);
    std::shared_ptr<QuoteFormatter> qf = std::make_shared<HTMLQuoteFormatter>();

// Bug #181: not sure why this replacement was added to begin with but perhaps such replacements need
//           to be done in the Parser if needed
//    retval.replace("<", "&lt;");
//    retval.replace(">", "&gt;");

    // look for the string "[QUOTE". If it's not found, then we don't have to worry
    // about trying to handle BBCode quotes
    if (retval.contains(_quotePrefix))
    {
        // check for [QUOTE] tags
        if (retval.contains(_simpleQuoteRegEx))
        {
            retval = handleSimpleQuote(retval, qf);
        }

        // see if we should try to determine the quoting style used if we don't know it
        if (_quotestyle == QuoteStyle::UNKNOWN)
        {
            if (retval.contains(_vbulletinQuote))
            {
                _quotestyle = QuoteStyle::VBULLETIN;
            }
            else if (retval.contains(_quoteWithValues))
            {
                _quotestyle = QuoteStyle::KEYVALUE;
            }
        }

        switch (_quotestyle)
        {
            default:
            break;

            case QuoteStyle::VBULLETIN:
                retval = handleVBQuote(retval, qf);
            break;

            case QuoteStyle::KEYVALUE:
                retval = handleQuoteWithValues(retval);
            break;
        }
    }

    for (const auto& regpair : _regexpairs)
    {
        retval.replace(regpair.first, regpair.second);
    }

    retval.replace("\n", "<br/>");

    return retval;
}

struct SquareBracketStripper
{
  enum
  {
      open_bracket = '[',
      close_bracket = ']'
  };

  size_t count = 0;

  bool operator()(char c)
  {
    bool skip = (count > 0) || c == open_bracket;
    if (c == open_bracket)
    {
      ++count;
    }
    else if (c== close_bracket && count > 0)
    {
      --count;
    }

    return skip;
  }
};

QString BBRegExParser::toPlainText(const QString &bbcode)
{
    QString retval { bbcode };
    std::shared_ptr<QuoteFormatter> qf = std::make_shared<TextQuoteFormatter>();

    if (retval.contains(_quotePrefix))
    {
        // check for [QUOTE] tags
        if (retval.contains(_simpleQuoteRegEx))
        {
            retval = handleSimpleQuote(retval, qf);
        }

        // see if we should try to determine the quoting style used if we don't know it
        if (_quotestyle == QuoteStyle::UNKNOWN)
        {
            if (retval.contains(_vbulletinQuote))
            {
                _quotestyle = QuoteStyle::VBULLETIN;
            }
            else if (retval.contains(_quoteWithValues))
            {
                _quotestyle = QuoteStyle::KEYVALUE;
            }
        }

        switch (_quotestyle)
        {
            default:
            break;

            case QuoteStyle::VBULLETIN:
                retval = handleVBQuote(retval, qf);
            break;

            case QuoteStyle::KEYVALUE:
                retval = handleQuoteWithValues(retval);
            break;
        }
    }

    for (const auto& rp : _regexpairs)
    {
        retval.replace(rp.first, "\\2");
    }

    // remove any stray bbcode elements, like if you have: "this [b]is[i]a test[/b]!!!"
    // what will be left is: "this is[i]a test!!!", so we want to remove the [i]
    retval.replace(QRegularExpression("\\[/*[a-zA-Z]{1}\\]"), QString());

    return retval;
}

QString BBRegExParser::handleSimpleQuote(const QString &original, QuoteFormatterPtr qf)
{
    auto offset = 0u;
    QString retval(original);

    auto it = _simpleQuoteFullRegEx.globalMatch(retval);
    if (it.hasNext())
    {
        while (it.hasNext())
        {
            auto match = it.next();

            const auto mstart = match.capturedStart(0) + offset;
            const auto mend = match.capturedEnd(0) + offset;

            QString innerText = handleSimpleQuote(match.captured(1), qf);

            // Tapatalk will take vbulletin's quoting style [QUOTE=Username;12345] and turn it into
            // [QUOTE][url=http://linktopost]Originally Posted by Username[/url], so in an effort to
            // be more consistent, we look for that and undo it if necessary
            if (innerText.contains(_vbQuoteFixRegEx))
            {               
                innerText = innerText.replace(_vbQuoteFixRegEx, qf->getPreQuoteReplacer()).trimmed();
            }

            const QString middle = qf->getQuoteBody(innerText.trimmed());

            offset += middle.length() - (mend - mstart);
            retval = retval.mid(0, mstart) + middle + retval.mid(mend);
        }
    }

    return retval;
}

QString BBRegExParser::handleVBQuote(const QString &original, QuoteFormatterPtr qf)
{
    auto offset = 0u;
    QString retval(original);

    auto it = _vbulletinQuote.globalMatch(retval);
    if (it.hasNext())
    {
        // Example: [QUOTE=User;1234]Text![/QUOTE]
        // \0 = the entire capture
        // \1 = User;1234
        // \2 Text!
        while (it.hasNext())
        {
            auto match = it.next();

            const auto mstart = match.capturedStart(0) + offset;
            const auto mend = match.capturedEnd(0) + offset;

            // we're expecting captured(1) to be something like: Max Power;12345
            // with the username and the userid being quoted, so the return
            // value of handleVBUserString is .first=username and .second=userid
            //
            // TODO: Tapatalk (or someone) turns the user info into a link that says "originally posted by"
            // need to handle this and make the quotes look consistent, but for now let's just let it be
            const auto userinfo = handleVBUserString(match.captured(1));
            const auto innerText = handleVBQuote(match.captured(2), qf);

            QString middle;
            if (!userinfo.first.isEmpty())
            {
                middle = qf->getQuoteBody(userinfo.first, innerText);
            }
            else
            {
                middle = qf->getQuoteBody(innerText);
            }

            offset += middle.length() - (mend - mstart);
            retval = retval.mid(0, mstart) + middle + retval.mid(mend);
        }
    }

    return retval;
}

std::pair<QString, quint32> BBRegExParser::handleVBUserString(const QString &original)
{
    QString username;
    quint32 userid = 0;

    if (original.contains(';'))
    {
        const QStringList list = original.split(';');
        username = list.at(0);

        bool ok = false;
        userid = list.at(1).toInt(&ok);
        if (!ok)
        {
            userid = 0;
        }
    }
    else
    {
        username = original;
    }

    return std::make_pair(username, userid);
}

QString BBRegExParser::handleQuoteWithValues(const QString &original)
{
    auto offset = 0u;
    QString retval(original);

    auto it = _quoteWithValues.globalMatch(retval);
    if (it.hasNext())
    {
        // Example: [QUOTE uid="123" username="Max Power" post="321"]Text![/QUOTE]
        while (it.hasNext())
        {
            auto match = it.next();

            const auto& username = match.captured("username");
            const auto& innertext = match.captured("quotetext");

            const auto mstart = match.capturedStart(0) + offset;
            const auto mend = match.capturedEnd(0) + offset;

            const QString middle = QString(tr("<blockquote><b>%1</b> wrote:</br></br>%2</blockquote>"))
                .arg(username)
                .arg(innertext);

            offset += middle.length() - (mend - mstart);
            retval = retval.mid(0, mstart) + middle + retval.mid(mend);
        }
    }

    return retval;
}


//} // namespace

//namespace owl
//{
//
//using BBToken::TokenType::TT_OPENBRACKET;
//using BBToken::TokenType::TT_UNKNOWN;
//using BBToken::TokenType::TT_TEXT;
//using BBToken::TokenType::TT_OPENBRACKET;
//using BBToken::TokenType::TT_CLOSEBRACKET;
//using BBToken::TokenType::TT_EQUAL;
//using BBToken::TokenType::TT_FWDSLASH;
//using BBToken::TokenType::TT_NEWLINE;
//using BBToken::TokenType::TT_DOUBLEQUOTE;
//using BBToken::TokenType::TT_SINGLEQUOTE;
//using BBToken::TokenType::TT_ASTERISK;
//
//bool BBParser::parse(bool bThrowOnError /*= false*/)
//{
//    bool retval = false;
//
//    if (privateParse(bThrowOnError))
//    {
//        retval = matchedParse(bThrowOnError);
//    }
//
//    return retval;
//}
//
//const bool BBParser::isValidTagName(const QString &tagname) const
//{
//    return std::find(_validTags.cbegin(), _validTags.cend(), tagname.toUpper()) != _validTags.cend();
//}
//
//bool BBParser::privateParse(bool bThrowOnError)
//{
//    bool bHandled = false;
//
//    while (!_tokens.empty())
//    {
//        bHandled = false;
//        auto curToken = _tokens.front();
//
//        if (curToken.getType() == TT_OPENBRACKET)
//        {
//            auto nextIt = std::next(_tokens.begin());
//            if (nextIt != _tokens.end())
//            {
//                if ((*nextIt).getType() == TT_TEXT)
//                {
//                    bHandled = parseOpenTag();
//                }
//                else if ((*nextIt).getType() == TT_FWDSLASH)
//                {
//                    bHandled = parseCloseTag();
//                }
//            }
//        }
//
//        if (!bHandled)
//        {
//            assert(_tokens.size() > 0);
//            curToken = _tokens.front();
//
//            if (_nodelist.size() > 0
//                    && _nodelist.back()->getNodeType() == BBNode::NodeType::TEXT)
//            {
//                QString newtext(_nodelist.back()->getText() + curToken.getText());
//                _nodelist.back()->setText(newtext);
//            }
//            else
//            {
//                _nodelist.emplace_back(new BBText(curToken.getText()));
//            }
//
//            _tokens.pop_front();
//        }
//    }
//
//    return true;
//}
//
//bool BBParser::parseCloseTag()
//{
//    bool bHandled = false;
//
//    // pop the [
//    QString buffer(_tokens.front().getText());
//    _tokens.pop_front();
//
//    if (!_tokens.empty())
//    {
//        // pop the /
//        buffer.append(_tokens.front().getText());
//        _tokens.pop_front();
//
//        if (!_tokens.empty() && _tokens.front().getType() == TT_TEXT)
//        {
//            // store off what we expect to be the tag name
//            QString tagName(_tokens.front().getText());
//
//            // now pop the TEXT (as in [/TEXT])
//            buffer.append(_tokens.front().getText());
//            _tokens.pop_front();
//
//            if (!_tokens.empty() && _tokens.front().getType() == TT_CLOSEBRACKET)
//            {
//                // the token after the text is a ], so let's see if this is a valid tag name
//                QRegExp regex("^[A-Za-z][A-Za-z0-9]*$");
//                if (regex.exactMatch(tagName))
//                {
//                    // pop the ]
//                    _tokens.pop_front();
//
//                    // signal that this is handled
//                    bHandled = true;
//
//                    // add this closing tag
//                    _nodelist.emplace_back(new BBTag(tagName, true));
//                }
//                else
//                {
//                    // pop the ] and add it to the buffer
//                    buffer.append(_tokens.front().getText());
//                    _tokens.pop_front();
//                }
//            }
//        }
//    }
//
//    if (!bHandled)
//    {
//        // we didn't get a valid closing tag, so treat everything as text
//        _tokens.push_front(BBToken(BBToken::TokenType::TT_TEXT, buffer));
//    }
//
//    return bHandled;
//}
//
//bool BBParser::parseOpenTag()
//{
//    // pop off the [ into the buffer
//    QString buffer(_tokens.front().getText());
//    _tokens.pop_front();
//
//    // NOTE: the calling function *NEEDS* to ensure that the token after the [
//    // is in fact a TT_TEXT
//    assert(!_tokens.empty() && _tokens.front().getType() == TT_TEXT);
//
//    // save off the tag name
//    QString tagName(_tokens.front().getText());
//
//    // pop off the TEXT into the buffer
//    buffer.append(_tokens.front().getText());
//    _tokens.pop_front();
//
//    // before we move on, make sure this is a valid tag name
//    if (!isValidTagName(tagName))
//    {
//        // This text is something like: [123
//        // OR an Invalid Tag Name
//        _tokens.push_front(BBToken(BBToken::TokenType::TT_TEXT, buffer));
//        return false;
//    }
//
//    if (!_tokens.empty() && _tokens.front().getType() == TT_CLOSEBRACKET)
//    {
//        // we got a simple tag, first pop the close bracket
//        _tokens.pop_front();
//
//        // add the open tag to the list
//        _nodelist.emplace_back(new BBTag(tagName, false));
//
//        // and return true to indicate we handled it
//        return true;
//    }
//    else if (!_tokens.empty() && _tokens.front().getType() == TT_EQUAL)
//    {
//        // so far we have: [TEXT=
//        // which looks like a value tag, but only if the next two symbols work out
//
//        // pop the = sign
//        buffer.append(popFrontText());
//
//        if (!_tokens.empty() && _tokens.front().getType() == TT_TEXT)
//        {
//            // so now we have [TEXT=OTHERTEXT
//            QString value = popFrontText();
//            buffer.append(value);
//
//            if (!_tokens.empty() && _tokens.front().getType() == TT_CLOSEBRACKET)
//            {
//                // looks like a valid value tag! so pop the ]
//                _tokens.pop_front();
//
//                _nodelist.emplace_back(new BBValueTag(tagName, value));
//
//                // return true to indicate we handles everything
//                return true;
//            }
//        }
//    }
//
//    // all else failed so create a text token from what we pop'd, push it to the front
//    // and signal back to the parser to treat it as text
//    _tokens.push_front(BBToken(BBToken::TokenType::TT_TEXT, buffer));
//    return false;
//}
//
//bool BBParser::matchedParse(bool bThrowOnError)
//{
//    // now we have a list of BBNodes but we need to check that every closing
//    // tag has a corresponding open tag
//    using TagStack = std::stack<BBTagPtr>;
//    std::map<QString, TagStack> tagmap;
//
//    for (auto node : _nodelist)
//    {
//        // if this is a tag
//        if (node->isTag())
//        {
//            BBTagPtr tag = std::dynamic_pointer_cast<BBTag>(node);
//
//            // if this is a closing tag then we want to see if there's a corresponding
//            // opening tag on the stack
//            if (tag->isClosingTag())
//            {
//                if (tagmap.find(tag->getTagName()) != tagmap.end())
//                {
//                    auto& tagstack = tagmap.at(tag->getTagName());
//                    tag->setMatchedTag(tagstack.top());
//                    tagstack.top()->setMatchedTag(tag);
//                    tagstack.pop();
//
//                    if (tagstack.empty())
//                    {
//                        tagmap.erase(tagmap.find(tag->getTagName()));
//                    }
//                }
//            }
//            else
//            {
//                // this is an open tag, so we need to put things on a tag stack
//                if (tagmap.find(tag->getTagName()) == tagmap.end())
//                {
//                    TagStack tagstack;
//                    tagstack.push(tag);
//                    tagmap.insert(std::make_pair(tag->getTagName(), std::move(tagstack)));
//                }
//                else
//                {
//                    tagmap.at(tag->getTagName()).push(tag);
//                }
//            }
//        }
//    }
//
//    return true;
//}
//
//auto BBScanner::Scan(QString* text) -> owl::BBTokenList
//{
//    QTextStream stream(text);
//    return Scan(stream);
//}
//
//auto BBScanner::Scan(QTextStream& buffer) -> owl::BBTokenList
//{
//	bool bInsideBracket = false;
//	int iSpaceCount = 0;
//
//	_tokens.clear();
//
//	QString bufferTxt(buffer.readAll());
//
//	QString strAccum;
//	QTextStream accum(&strAccum, QIODevice::ReadWrite);
//
//	for (auto itr(bufferTxt.begin()); itr != bufferTxt.end(); ++itr)
//	{
//		QChar ch = *itr;
//
//		switch (ch.toLatin1())
//		{
//			case QChar::SpecialCharacter::CarriageReturn:
//				{
//					if (!strAccum.isEmpty())
//					{
//                        _tokens.push_back(BBToken(BBToken::TokenType::TT_TEXT, strAccum));
//						strAccum.clear();
//					}
//
//					if (*(itr+1) == QChar::SpecialCharacter::LineFeed)
//					{
//						++itr;
//					}
//
//                    _tokens.push_back(BBToken(BBToken::TokenType::TT_NEWLINE));
//				}
//                break;
//
//			case '[':
//				{
//					if (!strAccum.isEmpty())
//					{
//                        _tokens.push_back(BBToken(BBToken::TokenType::TT_TEXT, strAccum));
//						strAccum.clear();
//					}
//
//                    _tokens.push_back(BBToken(BBToken::TokenType::TT_OPENBRACKET,ch));
//				}
//				break;
//
//			case ']':
//				{
//					if (!strAccum.isEmpty())
//					{
//                        _tokens.push_back(BBToken(BBToken::TokenType::TT_TEXT, strAccum));
//						strAccum.clear();
//					}
//
//                    _tokens.push_back(BBToken(BBToken::TokenType::TT_CLOSEBRACKET,ch));
//				}
//				break;
//
//			case '=':
//				{
//					if (!strAccum.isEmpty())
//					{
//                        _tokens.push_back(BBToken(BBToken::TokenType::TT_TEXT, strAccum));
//						strAccum.clear();
//					}
//
//                    _tokens.push_back(BBToken(BBToken::TokenType::TT_EQUAL,ch));
//				}
//				break;
//
//			case '"':
//				{
//					if (!strAccum.isEmpty())
//					{
//                        _tokens.push_back(BBToken(BBToken::TokenType::TT_TEXT, strAccum));
//						strAccum.clear();
//					}
//
//                    _tokens.push_back(BBToken(BBToken::TokenType::TT_DOUBLEQUOTE,ch));
//				}
//				break;
//
//			case '\'':
//				{
//					if (!strAccum.isEmpty())
//					{
//                        _tokens.push_back(BBToken(BBToken::TokenType::TT_TEXT, strAccum));
//						strAccum.clear();
//					}
//
//                    _tokens.push_back(BBToken(BBToken::TokenType::TT_SINGLEQUOTE,ch));
//				}
//				break;
//
//			case '/':
//				{
//					if (!strAccum.isEmpty())
//					{
//                        _tokens.push_back(BBToken(BBToken::TokenType::TT_TEXT, strAccum));
//						strAccum.clear();
//					}
//
//                    _tokens.push_back(BBToken(BBToken::TokenType::TT_FWDSLASH,ch));
//				}
//				break;
//
//			case '*':
//				{
//					if (!strAccum.isEmpty())
//					{
//                        _tokens.push_back(BBToken(BBToken::TokenType::TT_TEXT, strAccum));
//						strAccum.clear();
//					}
//
//                    _tokens.push_back(BBToken(BBToken::TokenType::TT_ASTERISK,ch));
//				}
//				break;
//
//			default:
//				accum << ch;
//			break;
//		}
//	}
//
//	if (!strAccum.isEmpty())
//	{
//        _tokens.push_back(BBToken(BBToken::TokenType::TT_TEXT, strAccum));
//		strAccum.clear();
//	}
//
//	return _tokens;
//}
//
//QString BBCToHTmlGenerator::doGenerate(const QString& bbcode)
//{
//	QString strHtml;
////	QString bbcTemp(bbcode);
////	QTextStream stream(&bbcTemp, QIODevice::ReadOnly);
//
////	BBCScanner scanner;
////	BBCTokenList tokens = scanner.Scan(stream);
//
////	if (tokens.size() > 0)
////	{
////		QString output;
////        BBCParser parser(std::move(tokens));
////		if (parser.parse())
////		{
////			QTextStream out(&output);
//
////            const auto objects = parser.getObjects();
////			for (auto obj : objects)
////			{
////				strHtml += obj->toHtml();
////				//out <<
////				//output.append(genStatement(obj));
////			}
////		}
////	}
//
//	return strHtml;
//}
//
//QString TextGenerator::toHtml(const BBNodeList& nodelist)
//{
//    QString retval;
//
//    for (const auto& node : nodelist)
//    {
//        retval.append(node->getHtml());
//    }
//
//    return retval;
//}
//
//QString TextGenerator::toBBCode(const BBNodeList& nodelist)
//{
//    QString retval;
//
//    for (const auto& node : nodelist)
//    {
//        retval.append(node->getBBCode());
//    }
//
//    return retval;
//}
//
//
////QString BBCToHTmlGenerator::genStatement(BBCObjectPtr stmt)
////{
////	QString strHtml;
//
////	//
////	// This design feels poor. We have to start by upcasting stmt to
////	// the "highest level" classes working our way down to see which
////	// is the first one that works since (1) different classes get
////	// generated differently and it's the generator class that might
////	// need to do some funky stuff
////	//
//
////	return strHtml;
////}

} // namespace
