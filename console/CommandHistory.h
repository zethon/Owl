#include <vector>
#include <boost/circular_buffer.hpp>
#include <QString>

namespace owl
{

class CommandHistory
{
    boost::circular_buffer<QString> _buffer;
    std::size_t _currentPos = 0;
    QString _historyFile;

public:
    CommandHistory()
        : _buffer(100)
    {}

    void setHistoryFile(const QString& filename);
    void loadHistory(bool throwOnError = false);
    void saveHistory();

    void setBufferSize(std::size_t size) { _buffer.resize(size); }

    void commit(const QString& command);
    void clear() { _buffer.clear(); }
    void reset() { _currentPos = _buffer.size(); }

    void up();
    void down();

    std::size_t size() const { return _buffer.size(); }
    QString getCurrent() const;
};

}