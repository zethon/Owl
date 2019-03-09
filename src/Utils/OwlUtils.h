#pragma once
#include <QtCore>

namespace owl
{

const std::string getOSString();
bool isWindowsHost();
bool isMacHost();
bool isLinuxHost();

void openFolder(const QString& pathIn);

QString getResourceHtmlFile(const QString& file);

const QString sanitizeUrl(const QString& url);

int randomInteger(int low, int high);

QString previewText(const QString &original);
QString previewText(const QString& original, uint maxLen);

} // owl namespace
