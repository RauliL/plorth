#include <plorth/context.hpp>
#include <plorth/value-error.hpp>
#include <plorth/value-quote.hpp>

#include <cassert>

namespace plorth
{
  namespace api
  {
    runtime::prototype_definition global_dictionary();
    runtime::prototype_definition array_prototype();
    runtime::prototype_definition boolean_prototype();
    runtime::prototype_definition error_prototype();
    runtime::prototype_definition number_prototype();
    runtime::prototype_definition object_prototype();
    runtime::prototype_definition quote_prototype();
    runtime::prototype_definition string_prototype();
  }

  static inline ref<object> make_prototype(runtime*, const char*, const runtime::prototype_definition&);

  runtime::runtime(memory::manager* memory_manager)
    : m_memory_manager(memory_manager)
  {
    assert(memory_manager);

    m_null_value = new (*m_memory_manager) class null();
    m_true_value = new (*m_memory_manager) class boolean(true);
    m_false_value = new (*m_memory_manager) class boolean(false);

    for (auto& entry : api::global_dictionary())
    {
      m_dictionary[utf8_decode(entry.first)] = quote(entry.second);
    }

    m_array_prototype = make_prototype(this, "array", api::array_prototype());
    m_boolean_prototype = make_prototype(this, "boolean", api::boolean_prototype());
    m_error_prototype = make_prototype(this, "error", api::error_prototype());
    m_number_prototype = make_prototype(this, "number", api::number_prototype());
    m_object_prototype = make_prototype(this, "object", api::object_prototype());
    m_quote_prototype = make_prototype(this, "quote", api::quote_prototype());
    m_string_prototype = make_prototype(this, "string", api::string_prototype());
  }

  ref<context> runtime::new_context()
  {
    return new (*m_memory_manager) context(this);
  }

  static inline ref<object> make_prototype(class runtime* runtime,
                                           const char* name,
                                           const runtime::prototype_definition& definition)
  {
    object::container_type properties;
    ref<object> prototype;

    for (auto& entry : definition)
    {
      properties[utf8_decode(entry.first)] = runtime->quote(entry.second);
    }

    prototype = runtime->value<object>(properties);
    runtime->dictionary()[utf8_decode(name)] = runtime->value<object>(object::container_type({
      { utf8_decode("prototype"), prototype }
    }));

    return prototype;
  }
}
