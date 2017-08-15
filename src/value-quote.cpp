#include <plorth/context.hpp>
#include <plorth/token.hpp>
#include <plorth/value-number.hpp>
#include <plorth/value-string.hpp>

#include "./utils.hpp"

#include <sstream>

namespace plorth
{
  static ref<value> parse_value(
    const ref<context>&,
    std::vector<token>::const_iterator&,
    const std::vector<token>::const_iterator&
  );
  static bool parse_declaration(
    const ref<context>&,
    std::vector<token>::const_iterator&,
    const std::vector<token>::const_iterator&
  );

  namespace
  {
    class compiled_quote : public quote
    {
    public:
      explicit compiled_quote(const std::vector<token>& tokens)
        : m_tokens(tokens) {}

      inline enum quote_type quote_type() const
      {
        return quote_type_compiled;
      }

      bool call(const ref<context>& ctx) const
      {
        auto current = std::begin(m_tokens);
        const auto end = std::end(m_tokens);

        while (current != end)
        {
          const class token& token = *current;

          switch (token.type())
          {
            case token::type_string:
            case token::type_lparen:
            case token::type_lbrack:
            case token::type_lbrace:
              {
                const ref<value> val = parse_value(ctx, current, end);

                if (!val)
                {
                  return false;
                }
                ctx->push(val);
              }
              break;

            case token::type_colon:
              if (!parse_declaration(ctx, current, end))
              {
                return false;
              }
              break;

            case token::type_word:
              if (!ctx->call(current++->text()))
              {
                return false;
              }
              break;

            case token::type_rparen:
            case token::type_rbrack:
            case token::type_rbrace:
            case token::type_comma:
            case token::type_semicolon:
              {
                std::stringstream ss;

                ss << "Unexpected " << *current;
                ctx->error(error::code_syntax, ss.str().c_str());

                return false;
              }
          }
        }

        return true;
      }

      unistring to_string() const
      {
        unistring result;
        bool first = true;

        for (const auto& token : m_tokens)
        {
          if (first)
          {
            first = false;
          } else {
            result += ' ';
          }
          result += token.to_source();
        }

        return result;
      }

      bool equals(const ref<value>& that) const
      {
        const compiled_quote* q;

        if (!that->is(type_quote) || that.cast<quote>()->is(quote_type_compiled))
        {
          return false;
        }
        q = that.cast<compiled_quote>();
        if (m_tokens.size() != q->m_tokens.size())
        {
          return false;
        }
        for (std::size_t i = 0; i < m_tokens.size(); ++i)
        {
          if (m_tokens[i] != q->m_tokens[i])
          {
            return false;
          }
        }

        return true;
      }

    private:
      const std::vector<token> m_tokens;
    };

    class native_quote : public quote
    {
    public:
      explicit native_quote(callback cb)
        : m_callback(cb) {}

      inline enum quote_type quote_type() const
      {
        return quote_type_native;
      }

      bool call(const ref<context>& ctx) const
      {
        m_callback(ctx);

        return !ctx->error();
      }

      unistring to_string() const
      {
        return utf8_decode("[native quote]");
      }

      bool equals(const ref<value>& that) const
      {
        // Currently there is no way to compare two std::function instances
        // against each other, even when they are the same type.
        return this == that.get();
      }

    private:
      const callback m_callback;
    };

    class curried_quote : public quote
    {
    public:
      explicit curried_quote(const ref<value>& argument, const ref<class quote>& quote)
        : m_argument(argument)
        , m_quote(quote) {}

      inline enum quote_type quote_type() const
      {
        return quote_type_curried;
      }

      bool call(const ref<context>& ctx) const
      {
        ctx->push(m_argument);

        return m_quote->call(ctx);
      }

      bool equals(const ref<value>& that) const
      {
        const curried_quote* q;

        if (!that->is(type_quote) || !that.cast<quote>()->is(quote_type_curried))
        {
          return false;
        }
        q = that.cast<curried_quote>();

        return m_argument->equals(q->m_argument) && m_quote->equals(q->m_quote);
      }

      unistring to_string() const
      {
        unistring result;

        result += m_argument->to_source();
        result += ' ';
        result += m_quote->to_source();
        result += utf8_decode(" call");

        return result;
      }

    private:
      const ref<value> m_argument;
      const ref<quote> m_quote;
    };

    class composed_quote : public quote
    {
    public:
      explicit composed_quote(const ref<quote>& left, const ref<quote>& right)
        : m_left(left)
        , m_right(right) {}

      inline enum quote_type quote_type() const
      {
        return quote_type_composed;
      }

      bool call(const ref<context>& ctx) const
      {
        return m_left->call(ctx) && m_right->call(ctx);
      }

      bool equals(const ref<value>& that) const
      {
        const composed_quote* q;

        if (!that->is(type_quote) || !that.cast<quote>()->is(quote_type_composed))
        {
          return false;
        }
        q = that.cast<composed_quote>();

        return m_left->equals(q->m_left) && m_right->equals(q->m_right);
      }

      unistring to_string() const
      {
        unistring result;

        result += m_left->to_source();
        result += utf8_decode(" call ");
        result += m_right->to_source();
        result += utf8_decode(" call");

        return result;
      }

    private:
      const ref<quote> m_left;
      const ref<quote> m_right;
    };

    class negated_quote : public quote
    {
    public:
      explicit negated_quote(const ref<class quote>& quote)
        : m_quote(quote) {}

      inline enum quote_type quote_type() const
      {
        return quote_type_negated;
      }

      bool call(const ref<context>& ctx) const
      {
        bool result;

        if (!m_quote->call(ctx) || !ctx->pop_boolean(result))
        {
          return false;
        }
        ctx->push_boolean(!result);

        return true;
      }

      bool equals(const ref<value>& that) const
      {
        const negated_quote* q;

        if (!that->is(type_quote) || !that.cast<quote>()->is(quote_type_negated))
        {
          return false;
        }
        q = that.cast<negated_quote>();

        return m_quote->equals(q->m_quote);
      }

      unistring to_string() const
      {
        return m_quote->to_source() + " call not";
      }

    private:
      const ref<quote> m_quote;
    };

    class constant_quote : public quote
    {
    public:
      explicit constant_quote(const ref<value>& val)
        : m_value(val) {}

      bool call(const ref<context>& ctx) const
      {
        ctx->push(m_value);

        return true;
      }

      inline enum quote_type quote_type() const
      {
        return quote_type_constant;
      }

      bool equals(const ref<value>& that) const
      {
        const constant_quote* q;

        if (!that->is(type_quote) || !that.cast<quote>()->is(quote_type_negated))
        {
          return false;
        }
        q = that.cast<constant_quote>();

        return m_value->equals(q->m_value);
      }

      unistring to_string() const
      {
        return m_value->to_source();
      }

    private:
      const ref<value> m_value;
    };
  }

  ref<quote> runtime::quote(const std::vector<token>& tokens)
  {
    return new (*m_memory_manager) compiled_quote(tokens);
  }

  ref<quote> runtime::quote(quote::callback callback)
  {
    return new (*m_memory_manager) native_quote(callback);
  }

  ref<quote> runtime::curry(const ref<class value>& argument, const ref<class quote>& quote)
  {
    return new (*m_memory_manager) curried_quote(argument, quote);
  }

  ref<quote> runtime::compose(const ref<class quote>& left, const ref<class quote>& right)
  {
    return new (*m_memory_manager) composed_quote(left, right);
  }

  ref<quote> runtime::constant(const ref<class value>& value)
  {
    return new (*m_memory_manager) constant_quote(value);
  }

  unistring quote::to_source() const
  {
    return "(" + to_string() + ")";
  }

  static ref<value> parse_quote(const ref<context>& ctx,
                                std::vector<token>::const_iterator& current,
                                const std::vector<token>::const_iterator& end)
  {
    std::vector<token> result;
    int counter = 1;

    while (current != end)
    {
      const class token& token = *current++;

      if (token.is(token::type_lparen))
      {
        ++counter;
      }
      else if (token.is(token::type_rparen) && !--counter)
      {
        break;
      }
      result.push_back(token);
    }
    if (counter > 0)
    {
      ctx->error(error::code_syntax, "Unterminated quote.");

      return ref<value>();
    }

    return ctx->runtime()->value<compiled_quote>(result);
  }

  static ref<value> parse_array(const ref<context>& ctx,
                                std::vector<token>::const_iterator& current,
                                const std::vector<token>::const_iterator& end)
  {
    array::container_type elements;
    ref<value> val;

    for (;;)
    {
      if (current >= end)
      {
        ctx->error(error::code_syntax, "Unterminated array literal.");

        return ref<value>();
      }
      else if (current->is(token::type_rbrack))
      {
        ++current;
        break;
      }
      if (!(val = parse_value(ctx, current, end)))
      {
        return ref<value>();
      }
      elements.push_back(val);
      if (current >= end)
      {
        ctx->error(error::code_syntax, "Unterminated array literal.");

        return ref<value>();
      }
      else if (current->is(token::type_comma))
      {
        ++current;
      }
      else if (!current->is(token::type_rbrack))
      {
        std::stringstream ss;

        ss << "Unexpected " << *current << "; Missing `]'";
        ctx->error(error::code_syntax, ss.str().c_str());

        return ref<value>();
      }
    }

    return ctx->runtime()->value<array>(elements);
  }

  static ref<value> parse_object(const ref<context>& ctx,
                                 std::vector<token>::const_iterator& current,
                                 const std::vector<token>::const_iterator& end)
  {
    object::container_type properties;
    unistring id;
    ref<value> val;

    for (;;)
    {
      if (current >= end)
      {
        ctx->error(error::code_syntax, "Unterminated object literal.");

        return ref<value>();
      }
      else if (current->is(token::type_rbrace))
      {
        ++current;
        break;
      }
      else if (!current->is(token::type_string))
      {
        ctx->error(error::code_syntax, "Missing key for object literal.");

        return ref<value>();
      }
      id = current++->text();
      if (current >= end || !current->is(token::type_colon))
      {
        ctx->error(error::code_syntax, "Missing `:' after key of an object.");

        return ref<value>();
      }
      else if (!(val = parse_value(ctx, ++current, end)))
      {
        return ref<value>();
      }
      properties[id] = val;
      if (current >= end)
      {
        ctx->error(error::code_syntax, "Unterminated object literal.");

        return ref<value>();
      }
      else if (current->is(token::type_comma))
      {
        ++current;
      }
      else if (!current->is(token::type_rbrace))
      {
        std::stringstream ss;

        ss << "Unexpected " << *current << "; Missing `]'";
        ctx->error(error::code_syntax, ss.str().c_str());

        return ref<value>();
      }
    }

    return ctx->runtime()->value<object>(properties);
  }

  static ref<value> parse_value(const ref<context>& ctx,
                                std::vector<token>::const_iterator& current,
                                const std::vector<token>::const_iterator& end)
  {
    const class token& token = *current++;

    switch (token.type())
    {
      case token::type_string:
        return ctx->runtime()->value<string>(token.text());

      case token::type_lparen:
        return parse_quote(ctx, current, end);

      case token::type_lbrack:
        return parse_array(ctx, current, end);

      case token::type_lbrace:
        return parse_object(ctx, current, end);

      case token::type_word:
        {
          const unistring& text = token.text();

          if (!text.compare(utf8_decode("null")))
          {
            return ctx->runtime()->null();
          }
          else if (!text.compare(utf8_decode("true")))
          {
            return ctx->runtime()->true_value();
          }
          else if (!text.compare(utf8_decode("false")))
          {
            return ctx->runtime()->false_value();
          }
          else if (is_number(text))
          {
            return ctx->runtime()->value<number>(std::stod(utf8_encode(text)));
          }
        }

      default:
        {
          std::stringstream ss;

          ss << "Unexpected " << token << ", expected value.";
          ctx->error(error::code_syntax, ss.str().c_str());

          return ref<value>();
        }
    }
  }

  static bool parse_declaration(const ref<context>& ctx,
                                std::vector<token>::const_iterator& current,
                                const std::vector<token>::const_iterator& end)
  {
    unistring name;
    std::vector<token> tokens;
    int counter = 1;

    if (++current >= end || !current->is(token::type_word))
    {
      ctx->error(error::code_syntax, "Missing name after word declaration.");

      return false;
    }
    name = current++->text();
    while (current != end)
    {
      const class token& token = *current++;

      if (token.is(token::type_colon))
      {
        ++counter;
      }
      else if (token.is(token::type_semicolon) && !--counter)
      {
        break;
      }
      tokens.push_back(token);
    }
    if (counter > 0)
    {
      ctx->error(error::code_syntax, "Unterminated declaration.");

      return false;
    }
    ctx->declare(name, ctx->runtime()->value<compiled_quote>(tokens));

    return true;
  }

  /**
   * call ( quote -- )
   *
   * Executes quote taken from top of the stack.
   */
  static void w_call(const ref<context>& ctx)
  {
    ref<quote> q;

    if (ctx->pop_quote(q))
    {
      q->call(ctx);
    }
  }

  /**
   * compose ( quote quote -- quote )
   *
   * Constructs new quote which will call the two given quotes in sequence.
   */
  static void w_compose(const ref<context>& ctx)
  {
    ref<quote> left;
    ref<quote> right;

    if (ctx->pop_quote(right) && ctx->pop_quote(left))
    {
      ctx->push(ctx->runtime()->compose(left, right));
    }
  }

  /**
   * curry ( any quote -- quote )
   *
   * Constructs curried quote where given value will be pushed into the stack
   * before calling the original quote.
   */
  static void w_curry(const ref<context>& ctx)
  {
    ref<value> argument;
    ref<quote> q;

    if (ctx->pop_quote(q) && ctx->pop(argument))
    {
      ctx->push(ctx->runtime()->curry(argument, q));
    }
  }

  /**
   * negate ( quote -- quote )
   *
   * Constructs negated version of given quote which negates boolean result
   * returned by the original quote.
   */
  static void w_negate(const ref<context>& ctx)
  {
    ref<quote> quo;

    if (ctx->pop_quote(quo))
    {
      ctx->push(ctx->runtime()->value<negated_quote>(quo));
    }
  }

  namespace api
  {
    runtime::prototype_definition quote_prototype()
    {
      return
      {
        { "call", w_call },
        { "compose", w_compose },
        { "curry", w_curry },
        { "negate", w_negate }
      };
    }
  }
}
