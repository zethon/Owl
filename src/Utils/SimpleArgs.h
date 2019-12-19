#pragma once

#include <string>
#include <vector>
#include <map>

#include <boost/utility/string_view.hpp>

namespace owl
{

///
/// A simple parser that seperates positional and named arguments
///
class SimpleArgs
{
    std::string                                     _original;
    std::vector<std::string>                        _tokenVector;

    std::vector<unsigned int>                       _positionals;
    std::map<std::string, std::string_view>         _named;
    
public:
    SimpleArgs() {}
    SimpleArgs(const std::string& val)
        : _original(val)
    {
        parse(_original);
    }

    void parse(const std::string& = std::string());
    void clear()
    {
        _original.clear();
        _tokenVector.clear();
        _positionals.clear();
        _named.clear();
    }

    std::string original() const { return _original; }

    std::size_t getPositionalCount() const;
    std::string getPositional(unsigned int index) const;
    
    std::size_t getNamedCount() const;
    std::string getNamedArgument(const std::string& name) const;
    bool hasArgument(const std::string& name) const;

    std::size_t getTokenCount() const;
    std::string getToken(unsigned int index) const;
};

} // namespace
