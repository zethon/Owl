#include "CommandHistory.h"
#include "../src/Utils/Exception.h"
#include <fmt/core.h>
#include <QFile>

namespace owl
{

void CommandHistory::setHistoryFile(const QString& filename)
{
    _historyFile = filename;
}

void CommandHistory::loadHistory(bool throwOnError)
{
    if (_historyFile.isEmpty()) return;

    QFile file(_historyFile);
    if (bool opened = file.open(QIODevice::ReadOnly); !opened && throwOnError)
    {
        const auto msg = fmt::format("Cannot load file '{}'", _historyFile.toStdString());
        OWL_THROW_EXCEPTION(owl::Exception(QString::fromStdString(msg)));
    }
    else if (opened)
    {
        QTextStream in(&file);

        while (!in.atEnd())
        {
            const QString line = in.readLine();
            if (!line.isEmpty()) commit(line);
        }

        file.close();
    }
}

void CommandHistory::saveHistory()
{
    if (_historyFile.isEmpty()) return;

    QFile file(_historyFile);

    file.dir

    if (bool opened = file.open(QIODevice::WriteOnly | QIODevice::Text); !opened)
    {
        const auto msg = fmt::format("Cannot load file '{}'", _historyFile.toStdString());
        OWL_THROW_EXCEPTION(owl::Exception(QString::fromStdString(msg)));
    }

    QTextStream out(&file);
    for (const auto& line : _buffer)
    {
        out << line << endl;
    }

    file.close();
}

void CommandHistory::commit(const QString& command)
{
    _buffer.push_back(command);
    _currentPos = _buffer.size();

    if (!_historyFile.isEmpty())
    {
        QFile file(_historyFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            QTextStream stream(&file);
            stream << command << endl;
            file.close();
        }
    }
}

void CommandHistory::up()
{
    if (_currentPos > 0) _currentPos--;
}

void CommandHistory::down()
{
    if (_currentPos < _buffer.size()) _currentPos++;
}

QString CommandHistory::getCurrent() const
{
    if (_buffer.size() > 0)
    {
        return _buffer[_currentPos];
    }

    return QString{};
}

}