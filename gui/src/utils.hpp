#ifndef PLORTH_GUI_UTILS_HPP_GUARD
#define PLORTH_GUI_UTILS_HPP_GUARD

#include <plorth/plorth.hpp>
#include <glibmm.h>

namespace plorth
{
  namespace gui
  {
    namespace utils
    {
      /**
       * Simple utility function for converting between two different string
       * types.
       */
      template<class A, class B>
      inline A string_convert(const B& original)
      {
        A result;

        result.reserve(original.length());
        for (const auto& c : original)
        {
          result.append(1, c);
        }

        return result;
      }
    }
  }
}

#endif /* !PLORTH_GUI_UTILS_HPP_GUARD */
