#ifndef PLORTH_UTILS_HPP_GUARD
#define PLORTH_UTILS_HPP_GUARD

#include <string>

namespace plorth
{
  bool is_word_part(char);
  bool str_is_number(const std::string&);
  std::string to_json_string(const std::string&);
}

#endif /* !PLORTH_UTILS_HPP_GUARD */
