#include "plorth.hpp"

namespace plorth
{
  static inline std::size_t utf8_size(unsigned char);

  std::size_t unicode_size(unsigned int cp)
  {
    if (cp > 0x10ffff
        || (cp & 0xfffe) == 0xfffe
        || (cp >= 0xd800 && cp <= 0xdfff)
        || (cp >= 0xfdd0 && cp <= 0xfdef))
    {
      return 0;
    }
    else if (cp <= 0x7f)
    {
      return 1;
    }
    else if (cp <= 0x07ff)
    {
      return 2;
    }
    else if (cp <= 0xffff)
    {
      return 3;
    } else {
      return 4;
    }
  }

  bool unicode_encode(unsigned int cp, char* buffer)
  {
    if (cp > 0x10ffff
        || (cp & 0xfffe) == 0xfffe
        || (cp >= 0xd800 && cp <= 0xdfff)
        || (cp >= 0xfdd0 && cp <= 0xfdef))
    {
      return false;
    }
    else if (cp <= 0x7f)
    {
      *buffer++ = static_cast<char>(cp);
    }
    else if (cp <= 0x07ff)
    {
      *buffer++ = static_cast<char>(0xc0 | ((cp & 0x7c0) >> 6));
      *buffer++ = static_cast<char>(0x80 | (cp & 0x3f));
    }
    else if (cp <= 0xffff)
    {
      *buffer++ = static_cast<char>(0xe0 | ((cp & 0xf000)) >> 12);
      *buffer++ = static_cast<char>(0x80 | ((cp & 0xfc0)) >> 6);
      *buffer++ = static_cast<char>(0x80 | (cp & 0x3f));
    } else {
      *buffer++ = static_cast<char>(0xf0 | ((cp & 0x1c0000) >> 18));
      *buffer++ = static_cast<char>(0x80 | ((cp & 0x3f000) >> 12));
      *buffer++ = static_cast<char>(0x80 | ((cp & 0xfc0) >> 6));
      *buffer++ = static_cast<char>(0x80 | (cp & 0x3f));
    }
    *buffer = 0;

    return true;
  }

  bool unicode_decode(const char* input, unsigned int& slot)
  {
    const std::size_t size = utf8_size(input[0]);
    unsigned int result;

    switch (size)
    {
      case 1:
        result = static_cast<unsigned int>(input[0]);
        break;

      case 2:
        result = static_cast<unsigned int>(input[0] & 0x1f);
        break;

      case 3:
        result = static_cast<unsigned int>(input[0] & 0x0f);
        break;

      case 4:
        result = static_cast<unsigned int>(input[0] & 0x07);
        break;

      case 5:
        result = static_cast<unsigned int>(input[0] & 0x03);
        break;

      case 6:
        result = static_cast<unsigned int>(input[0] & 0x01);
        break;

      default:
        return false;
    }
    for (std::size_t i = 1; i < size; ++i)
    {
      if ((input[i] & 0xc0) == 0x80)
      {
        result = (result << 6) | (input[i] & 0x3f);
      } else {
        return false;
      }
    }
    slot = result;

    return true;
  }

  bool unicode_is_cntrl(unsigned int cp)
  {
    static const unsigned int cntrl_table[19][2] =
    {
      { 0x0000, 0x001f }, { 0x007f, 0x009f }, { 0x00ad, 0x00ad },
      { 0x0600, 0x0603 }, { 0x06dd, 0x06dd }, { 0x070f, 0x070f },
      { 0x17b4, 0x17b5 }, { 0x200b, 0x200f }, { 0x202a, 0x202e },
      { 0x2060, 0x2063 }, { 0x206a, 0x206f }, { 0xd800, 0xf8ff },
      { 0xfeff, 0xfeff }, { 0xfff9, 0xfffb }, { 0x1d173, 0x1d17a },
      { 0xe0001, 0xe0001 }, { 0xe0020, 0xe007f }, { 0xf0000, 0xffffd },
      { 0x100000, 0x10fffd }
    };

    for (int i = 0; i < 19; ++i)
    {
      if (cp >= cntrl_table[i][0] && cp <= cntrl_table[i][1])
      {
        return true;
      }
    }

    return false;
  }

  unsigned int unicode_tolower(unsigned int cp)
  {
    if (cp >= 'A' && cp <= 'Z')
    {
      return cp + 32;
    }
    if (cp >= 0x00C0)
    {
      if ((cp >= 0x00C0 && cp <= 0x00D6) || (cp >= 0x00D8 && cp <= 0x00DE))
      {
        return cp + 32;
      }
      else if ((cp >= 0x0100 && cp < 0x0138) || (cp > 0x0149 && cp < 0x0178))
      {
        if (cp == 0x0130)
        {
          return 0x0069;
        }
        else if ((cp & 1) == 0)
        {
          return cp + 1;
        }
      }
      else if (cp == 0x0178)
      {
        return 0x00FF;
      }
      else if ((cp >= 0x0139 && cp < 0x0149) || (cp > 0x0178 && cp < 0x017F))
      {
        if (cp & 1)
        {
          return cp + 1;
        }
      }
      else if (cp >= 0x0200 && cp <= 0x0217)
      {
        if ((cp & 1) == 0)
        {
          return cp + 1;
        }
      }
      else if ((cp >= 0x0401 && cp <= 0x040C) || (cp >= 0x040E && cp <= 0x040F))
      {
        return cp + 80;
      }
      else if (cp >= 0x410 && cp <= 0x042F)
      {
        return cp + 32;
      }
      else if (cp >= 0x0460 && cp <= 0x047F)
      {
        if ((cp & 1) == 0)
        {
          return cp + 1;
        }
      }
      else if (cp >= 0x0531 && cp <= 0x0556)
      {
        return cp + 48;
      }
      else if (cp >= 0x10A0 && cp <= 0x10C5)
      {
        return cp + 48;
      }
      else if (cp >= 0xFF21 && cp <= 0xFF3A)
      {
        return cp + 32;
      }
    }

    return cp;
  }

  unsigned int unicode_toupper(unsigned int cp)
  {
    if (cp >= 'a' && cp <= 'z')
    {
      return cp - 32;
    }
    if (cp >= 0x00E0)
    {
      if ((cp >= 0x00E0 && cp <= 0x00F6) || (cp >= 0x00F8 && cp <= 0x00FE))
      {
        return cp - 32;
      }
      else if (cp == 0x00FF)
      {
        return 0x0178;
      }
      else if ((cp >= 0x0100 && cp < 0x0138) || (cp > 0x0149 && cp < 0x0178))
      {
        if (cp == 0x0131)
        {
          return 0x0049;
        }
        else if (cp & 1)
        {
          return cp - 1;
        }
      }
      else if ((cp >= 0x0139 && cp < 0x0149) || (cp > 0x0178 && cp < 0x017F))
      {
        if ((cp & 1) == 0)
        {
          return cp - 1;
        }
      }
      else if (cp == 0x017F)
      {
        return 0x0053;
      }
      else if (cp >= 0x0200 && cp <= 0x0217)
      {
        if (cp & 1)
        {
          return cp - 1;
        }
      }
      else if (cp >= 0x0430 && cp <= 0x044F)
      {
        return cp - 32;
      }
      else if ((cp >= 0x0451 && cp <= 0x045C) || (cp >= 0x045E && cp <= 0x045F))
      {
        return cp - 80;
      }
      else if (cp >= 0x0460 && cp <= 0x047F)
      {
        if (cp & 1)
        {
          return cp - 1;
        }
      }
      else if (cp >= 0x0561 && cp < 0x0587)
      {
        return cp - 48;
      }
      else if (cp >= 0xFF41 && cp <= 0xFF5A)
      {
        return cp - 32;
      }
    }

    return cp;
  }

  std::size_t utf8_strlen(const char* input)
  {
    std::size_t length = 0;

    if (input)
    {
      while (*input)
      {
        input += utf8_size(*input);
        ++length;
      }
    }

    return length;
  }

  static inline std::size_t utf8_size(unsigned char input)
  {
    if ((input & 0x80) == 0x00)
    {
      return 1;
    }
    else if ((input & 0xc0) == 0x80)
    {
      return 0;
    }
    else if ((input & 0xe0) == 0xc0)
    {
      return 2;
    }
    else if ((input & 0xf0) == 0xe0)
    {
      return 3;
    }
    else if ((input & 0xf8) == 0xf0)
    {
      return 4;
    }
    else if ((input & 0xfc) == 0xf8)
    {
      return 5;
    }
    else if ((input & 0xfe) == 0xfc)
    {
      return 6;
    } else {
      return 0;
    }
  }
}
