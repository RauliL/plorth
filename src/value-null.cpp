#include <plorth/value-null.hpp>

namespace plorth
{
  null::null() {}

  bool null::equals(const ref<value>& that) const
  {
    return that->is(type_null);
  }

  unistring null::to_string() const
  {
    return unistring();
  }

  unistring null::to_source() const
  {
    return utf8_decode("null");
  }
}
