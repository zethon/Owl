#pragma once

#include <string>

#include <boost/circular_buffer.hpp>

namespace owl
{

class CommandHistory
{
    boost::circular_buffer<std::string> _buffer { 100 };
    std::size_t                         _currentPos = 0;
    std::string                         _historyFile;

public:

    CommandHistory() = default;

    void setHistoryFile(const std::string& val) { _historyFile = val; }
    void loadHistory(bool throwOnError=true);
    void saveHistory();

    void setBufferSize(std::size_t size) { _buffer.set_capacity(size); }

    void commit(const std::string& command);
    void clear() { _buffer.clear(); }
    void reset() { _currentPos = _buffer.size(); }

    bool up();
    bool down();

    std::size_t size() const { return _buffer.size(); }
    std::string getCurrent() const;
    std::string at(std::size_t index) const;

    decltype(_buffer.begin()) begin() { return _buffer.begin(); }
    decltype(_buffer.end()) end() { return _buffer.end(); }

};

} // namespace arcc
