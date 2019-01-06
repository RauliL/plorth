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
#ifndef PLORTH_MODULE_HPP_GUARD
#define PLORTH_MODULE_HPP_GUARD

#include <plorth/value-object.hpp>

namespace plorth
{
  namespace module
  {
    /**
     * Class which implements loading modules from some kind of source into
     * execution context.
     */
    class manager : public memory::managed
    {
    public:
      /** File extension used for external modules by default. */
      static const std::u32string default_module_file_extension;

      /**
       * Constructs module manager which loads modules from file system, if
       * file system modules have been enabled.
       *
       * \param lookup_paths          List of directories in the local file
       *                              system where modules will be searched
       *                              from.
       * \param module_file_extension File extension used to recognize modules
       *                              from other files.
       */
      static std::shared_ptr<manager> file_system(
        memory::manager& memory_manager,
        const std::vector<std::u32string>& lookup_paths
          = std::vector<std::u32string>(),
        const std::u32string& module_file_extension
          = default_module_file_extension
      );

      /**
       * Constructs new module manager which is unable to import modules.
       */
      static std::shared_ptr<manager> dummy(memory::manager& memory_manager);

      /**
       * Attempts to import an module from given path and if successful,
       * returns it's dictionary as an object.
       *
       * \param ctx  Execution context where the module will be imported into.
       *             Also used for error reporting.
       * \param path Path to import the module from.
       * \return     Reference to the imported module as an object, or null
       *             reference if the import failed for some reason.
       */
      virtual std::shared_ptr<object> import_module(
        const std::shared_ptr<context>& ctx,
        const std::u32string& path
      ) = 0;
    };
  }
}

#endif /* !PLORTH_MODULE_HPP_GUARD */
