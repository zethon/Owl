#include <QDesktopServices>

#include "Exception.h"

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
    QFileInfo info { pathIn };
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.isDir() ? pathIn : info.path()));
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

bool isWindowsHost()
{
#ifdef Q_OS_WIN32
    return true;
#else
    return false;
#endif
}

bool isMacHost()
{
#ifdef Q_OS_MACX
    return true;
#else
    return false;
#endif
}

bool isLinuxHost()
{
#ifdef Q_OS_LINUX
    return true;
#else
    return false;
#endif
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
        OWL_THROW_EXCEPTION(Exception(QString("Invalid Url")));
	}

	if (retUrl.at(retUrl.length() - 1) == '/')
	{
		retUrl.remove(retUrl.length() - 1, 1);
	}

	return retUrl;
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

std::string getRandomElement(const std::vector<std::string>& elements)
{
    return elements[std::rand() % elements.size()];
}

std::string generateRandomUserAgent()
{
    // Lists of browsers, operating systems, and versions
    std::vector<std::string> browsers =
    {
         "Mozilla/5.0", "Mozilla/4.0", "Opera/9.80", "Safari/537.36", 
        "Chrome/91.0.4472.124", "Firefox/89.0", "Edge/91.0.864.59", "Brave/1.25.68", 
        "Vivaldi/3.8", "SamsungBrowser/14.0"
    };
    
    std::vector<std::string> operatingSystems =
    {
        "(Windows NT 10.0; Win64; x64)", "(Macintosh; Intel Mac OS X 10_15_7)", 
        "(Linux; Android 10)", "(iPhone; CPU iPhone OS 14_4 like Mac OS X)", 
        "(iPad; CPU OS 13_3 like Mac OS X)", "(Windows NT 6.1; WOW64)", 
        "(X11; Ubuntu; Linux x86_64)", "(X11; Linux x86_64)", "(Windows Phone 10.0)", 
        "(Windows NT 6.3; Trident/7.0)"
    };
    
    std::vector<std::string> engines =
    {
        "AppleWebKit/537.36 (KHTML, like Gecko)", "Gecko/20100101 Firefox/89.0", 
        "Safari/605.1.15", "Edge/18.18363", "Presto/2.12.388 Version/12.18", 
        "Blink/537.36 Chrome/91.0.4472.124", "Trident/7.0", "Goanna/4.2", 
        "KHTML, like Gecko", "WebKit/534.30"
    };

    // Randomly select a browser, OS, and engine
    std::string browser = getRandomElement(browsers);
    std::string os = getRandomElement(operatingSystems);
    std::string engine = getRandomElement(engines);
    
    // Format the User-Agent string
    std::string userAgent = browser + " " + os + " " + engine;
    return userAgent;
}

} // owl namespace
