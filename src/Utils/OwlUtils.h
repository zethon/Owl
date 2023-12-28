#pragma once
#include <limits>
#include <QtCore>

namespace owl
{

inline const QString GetConnectionsFilename()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
        .absoluteFilePath(QStringLiteral("connections.json"));
}

bool isWindowsHost();
bool isMacHost();
bool isLinuxHost();

void openFolder(const QString& pathIn);

QString getResourceHtmlFile(const QString& file);

const QString sanitizeUrl(const QString& url);

QString previewText(const QString &original);
QString previewText(const QString& original, uint maxLen);

template <typename T>
bool numericEquals(T x, T y)
{
    return fabs(x-y) < std::numeric_limits<T>::epsilon();
}

} // owl namespace
