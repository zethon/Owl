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
