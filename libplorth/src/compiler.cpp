/*
 * Copyright (c) 2017-2018, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <plorth/context.hpp>
#include <plorth/parser.hpp>

namespace plorth
{
  static std::shared_ptr<value> compile_token(
    const std::shared_ptr<runtime>&,
    const std::shared_ptr<token>&
  );

  std::shared_ptr<quote> context::compile(const std::u32string& source,
                                          const std::u32string& filename,
                                          int line,
                                          int column)
  {
    class parser parser(source, filename, line, column);
    std::vector<std::shared_ptr<token>> result;
    std::vector<std::shared_ptr<value>> values;

    if (!parser.parse(result))
    {
      auto error_message = parser.error();

      if (error_message.empty())
      {
        error_message = U"Unknown error.";
      }
      error(error::code::syntax, error_message, &parser.position());

      return std::shared_ptr<quote>();
    }
    values.reserve(result.size());
    for (const auto& token : result)
    {
      values.push_back(compile_token(m_runtime, token));
    }

    return m_runtime->compiled_quote(values);
  }

  static std::shared_ptr<array> compile_array_token(
    const std::shared_ptr<class runtime>& runtime,
    const std::shared_ptr<token::array>& token
  )
  {
    const auto& elements = token->elements();
    const auto size = elements.size();
    std::shared_ptr<value> result[size];

    for (std::size_t i = 0; i < size; ++i)
    {
      result[i] = compile_token(runtime, elements[i]);
    }

    return runtime->array(result, size);
  }

  static std::shared_ptr<object> compile_object_token(
    const std::shared_ptr<class runtime>& runtime,
    const std::shared_ptr<token::object>& token
  )
  {
    const auto& properties = token->properties();
    const auto size = properties.size();
    std::vector<object::value_type> result;

    result.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      const auto& property = properties[i];

      result.push_back(object::value_type(
        property.first,
        compile_token(runtime, property.second)
      ));
    }

    return runtime->object(result);
  }

  static std::shared_ptr<quote> compile_quote_token(
    const std::shared_ptr<class runtime>& runtime,
    const std::shared_ptr<token::quote>& token
  )
  {
    const auto& children = token->children();
    const auto size = children.size();
    std::vector<std::shared_ptr<value>> result;

    result.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      result.push_back(compile_token(runtime, children[i]));
    }

    return runtime->compiled_quote(result);
  }

  static std::shared_ptr<string> compile_string_token(
    const std::shared_ptr<class runtime>& runtime,
    const std::shared_ptr<token::string>& token
  )
  {
    return runtime->string(token->value());
  }

  static std::shared_ptr<symbol> compile_symbol_token(
    const std::shared_ptr<class runtime>& runtime,
    const std::shared_ptr<token::symbol>& token
  )
  {
    return runtime->symbol(token->id(), &token->position());
  }

  static std::shared_ptr<word> compile_word_token(
    const std::shared_ptr<class runtime>& runtime,
    const std::shared_ptr<token::word>& token
  )
  {
    return runtime->word(
      compile_symbol_token(runtime, token->symbol()),
      compile_quote_token(runtime, token->quote())
    );
  }

  static std::shared_ptr<value> compile_token(
    const std::shared_ptr<class runtime>& runtime,
    const std::shared_ptr<token>& token
  )
  {
    if (!token)
    {
      return std::shared_ptr<value>();
    }
    switch (token->type())
    {
      case token::type::array:
        return compile_array_token(
          runtime,
          std::static_pointer_cast<token::array>(token)
        );

      case token::type::object:
        return compile_object_token(
          runtime,
          std::static_pointer_cast<token::object>(token)
        );

      case token::type::quote:
        return compile_quote_token(
          runtime,
          std::static_pointer_cast<token::quote>(token)
        );

      case token::type::string:
        return compile_string_token(
          runtime,
          std::static_pointer_cast<token::string>(token)
        );

      case token::type::symbol:
        return compile_symbol_token(
          runtime,
          std::static_pointer_cast<token::symbol>(token)
        );

      case token::type::word:
        return compile_word_token(
          runtime,
          std::static_pointer_cast<token::word>(token)
        );
    }

    return std::shared_ptr<value>();
  }
}
