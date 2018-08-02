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
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE* POSSIBILITY OF SUCH DAMAGE.
 */
#include <plorth/context.hpp>
#include <plorth/value-word.hpp>
#include "./utils.hpp"

namespace plorth
{
  static bool eval_val(const std::shared_ptr<context>&,
                       const std::shared_ptr<value>&,
                       std::shared_ptr<value>&);
  static bool eval_ary(const std::shared_ptr<context>&,
                       const std::shared_ptr<array>&,
                       std::shared_ptr<value>&);
  static bool eval_obj(const std::shared_ptr<context>&,
                       const std::shared_ptr<object>&,
                       std::shared_ptr<value>&);
  static bool eval_sym(const std::shared_ptr<context>&,
                       const std::shared_ptr<symbol>&,
                       std::shared_ptr<value>&);
  static bool eval_wrd(const std::shared_ptr<context>&,
                       const std::shared_ptr<word>&,
                       std::shared_ptr<value>&);

  bool value::eval(const std::shared_ptr<context>& ctx,
                   const std::shared_ptr<value>& val,
                   std::shared_ptr<value>& slot)
  {
    if (!val)
    {
      slot.reset();

      return true;
    }
    switch (val->type())
    {
      case value::type::array:
        return eval_ary(ctx, std::static_pointer_cast<array>(val), slot);

      case value::type::object:
        return eval_obj(ctx, std::static_pointer_cast<object>(val), slot);

      case value::type::symbol:
        return eval_sym(ctx, std::static_pointer_cast<symbol>(val), slot);

      case value::type::word:
        return eval_wrd(ctx, std::static_pointer_cast<word>(val), slot);

      default:
        return eval_val(ctx, val, slot);
    }
  }

  static bool eval_val(const std::shared_ptr<context>& ctx,
                       const std::shared_ptr<value>& val,
                       std::shared_ptr<value>& slot)
  {
    slot = val;

    return true;
  }

  static bool eval_ary(const std::shared_ptr<context>& ctx,
                       const std::shared_ptr<array>& ary,
                       std::shared_ptr<value>& slot)
  {
    const auto size = ary->size();
    std::shared_ptr<value> elements[size];

    for (array::size_type i = 0; i < size; ++i)
    {
      const auto& element = ary->at(i);
      std::shared_ptr<value> element_slot;

      if (element && !value::eval(ctx, element, element_slot))
      {
        return false;
      }
      elements[i] = element_slot;
    }
    slot = ctx->runtime()->array(elements, size);

    return true;
  }

  static bool eval_obj(const std::shared_ptr<context>& ctx,
                       const std::shared_ptr<object>& obj,
                       std::shared_ptr<value>& slot)
  {
    std::vector<object::value_type> properties;

    properties.reserve(obj->size());
    for (const auto& property : obj->entries())
    {
      std::shared_ptr<value> value_slot;

      if (property.second && !value::eval(ctx, property.second, value_slot))
      {
        return false;
      }
      properties.push_back({ property.first, value_slot });
    }
    slot = ctx->runtime()->object(properties);

    return true;
  }

  static bool eval_sym(const std::shared_ptr<context>& ctx,
                       const std::shared_ptr<symbol>& sym,
                       std::shared_ptr<value>& slot)
  {
    const auto id = sym->id();

    if (!id.compare(U"null"))
    {
      slot.reset();
    }
    else if (!id.compare(U"true"))
    {
      slot = ctx->runtime()->true_value();
    }
    else if (!id.compare(U"false"))
    {
      slot = ctx->runtime()->false_value();
    }
    else if (!id.compare(U"drop"))
    {
      return ctx->pop(slot);
    }
    else if (is_number(id))
    {
      slot = ctx->runtime()->number(id);
    } else {
      ctx->error(
        error::code::syntax,
        U"Unexpected `" + id + U"'; Missing value."
      );

      return false;
    }

    return true;
  }

  static bool eval_wrd(const std::shared_ptr<context>& ctx,
                       const std::shared_ptr<word>& wrd,
                       std::shared_ptr<value>& slot)
  {
    ctx->error(
      error::code::syntax,
      U"Unexpected word declaration; Missing value."
    );

    return false;
  }
}
