#include "SimpleArgs.h"

#include <iostream>
#include <regex>
#include <vector>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

namespace owl
{

// https://stackoverflow.com/questions/18378798/use-boost-program-options-to-parse-an-arbitrary-string
/// @brief Tokenize a string.  The tokens will be separated by each non-quoted
///        space or equal character.  Empty tokens are removed.
///
/// @param input The string to tokenize.
///
/// @return Vector of tokens.
std::vector<std::string> tokenizeArgs(const std::string& input)
{
  typedef boost::escaped_list_separator<char> separator_type;
  separator_type separator("",    // The escape characters.
                           " ",    // The separator characters.
                           "\"\'"); // The quote characters.

  // Tokenize the intput.
  boost::tokenizer<separator_type> tokens(input, separator);

  // Copy non-empty tokens from the tokenizer into the result.
  std::vector<std::string> result;
  std::copy_if(tokens.begin(), tokens.end(), std::back_inserter(result), 
          [](const std::string& s) { return !s.empty(); });
  return result;
}

bool validParamName(const std::string_view& name)
{
    static const char* validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvqxyz0123456789";
    return name.find_first_not_of(validChars) == std::string_view::npos;
}

void SimpleArgs::parse(const std::string& original)
{
    if (original.size() == 0) return;
    _original = original;

    _tokenVector = tokenizeArgs(_original);

    for (auto i = 0u; i < _tokenVector.size(); i++)
    {
        std::string_view token = _tokenVector.at(i);

        if (boost::starts_with(token, "--"))
        {
            token.remove_prefix(2);
            if (const auto equals = token.find_first_of("=");
                equals != std::string_view::npos)
            {
                std::string_view key(token.data(), equals);
                token.remove_prefix(equals + 1);
                if (!validParamName(key)) continue;
                if (token.size() > 0)
                {
                    _named.insert_or_assign(std::string{key}, token);
                }
                else
                {
                    _named.insert_or_assign(std::string{key}, std::string_view{});
                }
            }
            else
            {
                if (!validParamName(token)) continue;

                // allow something like: myprogram --option=true --option2
                // so that `--option2` doesn't require a value but is still a 
                // named argument
                _named.insert_or_assign(std::string{token}, std::string_view{});
            }
        }
        else if (boost::starts_with(token, "-"))
        {
            token.remove_prefix(1);
            if (!validParamName(token)) continue;

            if (i < _tokenVector.size() - 1
                && !boost::starts_with(_tokenVector.at(i+1), "-"))
            {
                i++;
                _named.insert_or_assign(std::string{token}, _tokenVector.at(i));
            }
            else
            {
                // allow something like: myprogram --option=true --option2
                // so that `--option2` doesn't require a value but is still a 
                // named argument
                _named.insert_or_assign(std::string{token}, std::string_view{});
            }
        }
        else
        {
            _positionals.push_back(i);
        }
    }
}

std::size_t SimpleArgs::getPositionalCount() const 
{
    return _positionals.size();
}

std::string SimpleArgs::getPositional(unsigned int index) const 
{
    return std::string { _tokenVector.at(_positionals.at(index)) };
}

std::size_t SimpleArgs::getNamedCount() const 
{ 
    return _named.size(); 
}

std::string SimpleArgs::getNamedArgument(const std::string& name) const 
{ 
    if (const auto& temp = _named.find(name);
        temp != _named.end())
    {
        return std::string { _named.at(name) };
    }

    return std::string{};
}

bool SimpleArgs::hasArgument(const std::string& name) const 
{ 
    return _named.find(name) != _named.end(); 
}

std::size_t SimpleArgs::getTokenCount() const
{
    return _tokenVector.size();
}

std::string SimpleArgs::getToken(unsigned int index) const
{
    return std::string { _tokenVector.at(index) };
}

} // namespace