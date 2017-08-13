#include <plorth/unicode.hpp>

namespace plorth
{
  unistring json_stringify(const unistring& input)
  {
    unistring result;

    result.reserve(input.length() + 2);
    result.append(1, '"');

    for (const auto& c : input)
    {
      switch (c)
      {
        case 010:
          result.append(1, '\\');
          result.append(1, 'b');
          break;

        case 011:
          result.append(1, '\\');
          result.append(1, 't');
          break;

        case 012:
          result.append(1, '\\');
          result.append(1, 'n');
          break;

        case 014:
          result.append(1, '\\');
          result.append(1, 'f');
          break;

        case 015:
          result.append(1, '\\');
          result.append(1, 'r');
          break;

        case '"':
        case '\\':
        case '/':
          result.append(1, '\\');
          result.append(1, c);
          break;

        default:
          if (unichar_iscntrl(c))
          {
            char buffer[7];

            std::snprintf(buffer, 7, "\\u%04x", c);
            for (const char* p = buffer; *p; ++p)
            {
              result.append(1, static_cast<unichar>(*p));
            }
          } else {
            result.append(1, c);
          }
      }
    }

    result.append(1, '"');

    return result;
  }

  bool is_number(const unistring& input)
  {
    const auto length = input.length();
    unistring::size_type start;
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

    for (auto i = start; i < length; ++i)
    {
      const auto c = input[i];

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
}
