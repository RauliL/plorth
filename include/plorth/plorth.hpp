#ifndef PLORTH_PLORTH_HPP_GUARD
#define PLORTH_PLORTH_HPP_GUARD

#include <plorth/config.hpp>

namespace plorth
{
  class context;
  class runtime;
  class token;

  // Different types of values.
  class array;
  class boolean;
  class error;
  class null;
  class number;
  class object;
  class quote;
  class string;

  namespace memory
  {
    struct pool;
    struct slot;

    class managed;
    class manager;
  }
}

#endif /* !PLORTH_PLORTH_HPP_GUARD */
