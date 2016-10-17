#include <iostream>

#include "unicode.hpp"
#include "utils.hpp"

namespace plorth
{
  bool is_word_part(char c)
  {
    return c != '('
      && c != ')'
      && c != '['
      && c != ']'
      && c != '{'
      && c != '}'
      && c != ':'
      && c != ','
      && !std::isspace(c);
  }

  bool str_is_number(const std::string& input)
  {
    const std::string::size_type length = input.length();
    std::string::size_type start;
    bool dot_seen = false;

    if (!length)
    {
      return false;
    }
    if (input[0] == '+' || input[0] == '-')
    {
      start = 1;
      if (length < 2)
      {
        return false;
      }
    } else {
      start = 0;
    }
    for (std::string::size_type i = start; i < length; ++i)
    {
      const char c = input[i];

      if (c == '.')
      {
        if (dot_seen || i == start || i + 1 > length)
        {
          return false;
        }
        dot_seen = true;
      }
      else if (!std::isdigit(c))
      {
        return false;
      }
    }

    return true;
  }

  std::string to_json_string(const std::string& input)
  {
    std::string result;
    const char* pointer = input.c_str();

    result += "\"";
    while (*pointer)
    {
      unsigned int cp;

      if (!unicode_decode(pointer, cp))
      {
        break;
      }
      pointer += unicode_size(cp);
      switch (cp)
      {
        case 010:
          result += "\\b";
          break;

        case 011:
          result += "\\t";
          break;

        case 012:
          result += "\\n";
          break;

        case 014:
          result += "\\f";
          break;

        case 015:
          result += "\\r";
          break;

        case '"':
        case '\\':
        case '/':
          result += "\\";
          result += static_cast<char>(cp);
          break;

        default:
          {
            char buffer[7];

            if (unicode_is_cntrl(cp))
            {
              std::snprintf(buffer, 7, "\\u%04x", cp);
              result += buffer;
            }
            else if (unicode_encode(cp, buffer))
            {
              result += buffer;
            }
          }
          break;
      }
    }
    result += "\"";

    return result;
  }
}
