// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include "Exception.h"

#ifdef Q_OS_LINUX
    #include <sys/utsname.h>
#endif

namespace owl
{

void openFolder(const QString& pathIn)
{
// Mac, Windows support folder or file.
#if defined(Q_OS_WIN)
    QString param;
    if (!QFileInfo(pathIn).isDir())
    {
        param = QLatin1String("/select,");
    }

    param += QDir::toNativeSeparators(pathIn);
    QProcess::startDetached(QStringLiteral("explorer.exe"), QStringList(param));
#elif defined(Q_OS_MAC)
    //Q_UNUSED(parent)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e") << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"").arg(pathIn);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e") << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
    throw NotImplementedException("owl::openFolder() on Linux not yet supported");
#endif
}

// Loads the given filename from the client's HTML resources and
// returns and empty string if the file is not found
// TODO: This should be implemented in a class which stores the
// previously loaded files to save time when reloading
QString getResourceHtmlFile(const QString& file)
{
    QString tempName = QString(":/html/%1").arg(file);
    QFile resFile(tempName);

    if (resFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&resFile);
        return in.readAll();
    }

    return QString();
}

const QString getOSString()
{
    QString strOS("Unknown OS");

#ifdef Q_OS_WIN32
    switch(QSysInfo::windowsVersion())
    {
        case QSysInfo::WV_XP: strOS = "Windows XP"; break;
        case QSysInfo::WV_VISTA: strOS = "Windows Vista"; break;
        case QSysInfo::WV_WINDOWS7: strOS = "Windows 7"; break;
        case QSysInfo::WV_WINDOWS8: strOS = "Windows 8"; break;
        case QSysInfo::WV_WINDOWS8_1: strOS = "Windows 8.1"; break;
        case QSysInfo::WV_WINDOWS10: strOS = "Windows 10"; break;

        default: strOS = "Windows (unknown version)"; break;
    }
#elif defined Q_OS_MACX
    // hack since we're using an older version of qt
    const int osint = QSysInfo::macVersion();

    switch(osint)
    {
        case QSysInfo::MV_10_7: strOS = "Mac OS X 10.7"; break;
        case QSysInfo::MV_10_8: strOS = "Mac OS X 10.8"; break;
        case QSysInfo::MV_10_9:  strOS = "Mac OS X 10.9"; break;
        case QSysInfo::MV_10_10:  strOS = "Mac OS X 10.10"; break;
        case QSysInfo::MV_10_11:  strOS = "Mac OS X 10.11"; break;
        case QSysInfo::MV_10_12:  strOS = "Mac OS X 10.12"; break;

        default: strOS = "Mac OS X (unknown version)"; break;
    }
#elif defined Q_OS_LINUX
    utsname info{};
    if (uname(&info) == 0)
    {
        strOS = QString("%1 %2").arg(info.sysname).arg(info.release);
    }
    else
    {
        strOS = "Linux (unknown version)";
    }
#endif

    return strOS;
}

const QString sanitizeUrl(const QString& urlStr)
{
	QString retUrl(urlStr);
	QUrl urlObj = QUrl::fromUserInput(retUrl);

	if (urlObj.isValid())
	{
		// TODO (Issue #42): eventually to suppose HTTP Authentication

		// looks like Qt4 will put an @ in the url even in the UserInfo is blank
		//urlObj.setUserInfo("");

        if (urlObj.scheme() != "http" && urlObj.scheme() != "https")
		{
			urlObj.setScheme("http");
		}

		QString path(urlObj.path());

		if (!path.isEmpty())
		{
			QRegExp file("[^/]+\\.[^\\.]*$");
			path = path.replace(file, "");

			urlObj.setPath(path);
		}

		retUrl = urlObj.toString();
	}
	else
	{
        OWL_THROW_EXCEPTION(OwlException(QString("Invalid Url")));
	}

	if (retUrl.at(retUrl.length() - 1) == '/')
	{
		retUrl.remove(retUrl.length() - 1, 1);
	}

	return retUrl;
}

int randomInteger(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

QString previewText(const QString& original, uint maxLen)
{
    QString retval { original };

    // don't bother doing the work if the string is less than 10 characters
    if (maxLen != 0 && (uint)original.size() > maxLen && original.size() > 10)
    {
        int startFrom = ((uint)original.size() - maxLen) * -1;
        int lastSpace = retval.lastIndexOf(QRegularExpression(R"(\s+)"), startFrom);

        // no last space was found
        if (lastSpace == -1)
        {
            // go back 3 spaces for the '...'
            lastSpace = maxLen - 3;
        }

        retval.remove(lastSpace, original.size() - lastSpace);
        retval.append("...");
    }

    return retval;
}

QString previewText(const QString &original)
{
    return previewText(original, 128);
}

} // owl namespace
