#ifndef PLORTH_UNICODE_HPP_GUARD
#define PLORTH_UNICODE_HPP_GUARD

namespace plorth
{
  std::size_t unicode_size(unsigned int);
  bool unicode_encode(unsigned int, char*);
  bool unicode_decode(const char*, unsigned int&);
  bool unicode_is_cntrl(unsigned int);
  unsigned int unicode_tolower(unsigned int);
  unsigned int unicode_toupper(unsigned int);

  std::size_t utf8_strlen(const char*);
}

#endif /* !PLORTH_UNICODE_HPP_GUARD */
