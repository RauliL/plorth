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
#include <plorth/module.hpp>
#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
# include <climits>
# include <fstream>
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# if HAVE_SYS_STAT_H
#  include <sys/stat.h>
# endif
# if HAVE_UNISTD_H
#  include <unistd.h>
# endif
#endif

#include <algorithm>
#include <cstring>

#include "./utils.hpp"

namespace plorth
{
  bool runtime::import(const std::shared_ptr<class context>& context,
                       const std::u32string& path)
  {
    std::shared_ptr<class object> module;

    // Do not allow importing anything if the runtime does not have a module
    // manager.
    if (!m_module_manager)
    {
      context->error(error::code::import, U"Modules have been disabled.");

      return false;
    }

    // Do not attempt to import empty paths.
    if (path.empty() || std::all_of(path.begin(), path.end(), unicode_isspace))
    {
      context->error(error::code::import, U"Empty import path.");

      return false;
    }

    if ((module = m_module_manager->import_module(context, path)))
    {
      auto& dictionary = context->dictionary();

      // Transfer all exported words from the module into the calling execution
      // context.
      for (const auto& property : module->entries())
      {
        if (value::is(property.second, value::type::quote))
        {
          dictionary.insert(word(
            symbol(property.first),
            std::static_pointer_cast<quote>(property.second)
          ));
        }
      }

      return true;
    }
    else if (!context->error())
    {
      context->error(
        error::code::import,
        U"Unable to import from `" + path + U"`'"
      );
    }

    return false;
  }

  namespace module
  {
#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
# if defined(_WIN32)
    static const char file_separator = '\\';
# else
    static const char file_separator = '/';
# endif

    const std::u32string manager::default_module_file_extension = U".plorth";

    static bool is_absolute_path(const std::u32string&);
    static std::u32string dirname(const std::u32string&);
#endif

    namespace
    {
#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
      /**
       * Implementation of module manager which loads modules from file system.
       */
      class file_system_manager : public manager
      {
      public:
        using module_cache_type = std::unordered_map<
          std::u32string,
          std::shared_ptr<object>
        >;

        explicit file_system_manager(
          const std::vector<std::u32string>& lookup_paths,
          const std::u32string& module_file_extension
        )
          : m_lookup_paths(lookup_paths)
          , m_module_file_extension(utf8_encode(module_file_extension)) {}

        std::shared_ptr<object> import_module(
          const std::shared_ptr<context>& ctx,
          const std::u32string& path
        )
        {
          std::u32string resolved_path;
          module_cache_type::const_iterator cached_module;

          // First see if the given path actually resolves into actual file on
          // the file system.
          if (!resolve_path(ctx, path, resolved_path))
          {
            ctx->error(
              error::code::import,
              U"No such file or directory: " + path
            );

            return std::shared_ptr<object>();
          }

          // Then look from the module cache whether the module has already
          // been imported before, and use that cached module if such exists.
          cached_module = m_cache.find(resolved_path);
          if (cached_module != std::end(m_cache))
          {
            return cached_module->second;
          }

          // Otherwise begin loading the module from file system.
          return import_resolved_path(ctx, resolved_path);
        }

      private:
        /**
         * Attempts to resolve given arbitrary path into a path that exists in
         * the file system. If the given path seems to be an absolute path,
         * then it will be used as is, as long as a file corresponding to that
         * path exists in the file system. Otherwise, the list of lookup paths
         * will be iterated and for each directory in the lookup paths tested
         * whether a file exists under that directory that matches with the
         * given path.
         *
         * \param ctx           Execution context performing the import. This
         *                      is required for looking up directory where the
         *                      currently executed program has been read from.
         * \param path          Path to resolve.
         * \param resolved_path Where the resolved path will be placed into, if
         *                      the path was successfully resolved.
         * \return              Boolean flag telling whether the given path was
         *                      successfully resolved into a file or not.
         */
        bool resolve_path(const std::shared_ptr<context>& ctx,
                          const std::u32string& path,
                          std::u32string& resolved_path)
        {
          auto encoded_path = utf8_encode(path);
          char buffer[PATH_MAX];

          // If the path is absolute, resolve it into full path and use that
          // directly.
          if (is_absolute_path(path))
          {
            const auto dir = utf8_encode(dirname(ctx->filename()));

            if (!dir.empty())
            {
              encoded_path = dir + file_separator + encoded_path;
            }
            if (!::realpath(encoded_path.c_str(), buffer))
            {
              // Try again with appended file extension.
              encoded_path += m_module_file_extension;
              if (!::realpath(encoded_path.c_str(), buffer))
              {
                // Give up.
                return false;
              }
            }

            return resolve_into_file(buffer, resolved_path);
          }

          // Otherwise go through the module directories and look for a
          // matching file from each of them.
          for (const auto& directory : m_lookup_paths)
          {
            std::string module_path;

            // Skip empty paths.
            if (directory.empty() || std::all_of(directory.begin(),
                                                 directory.end(),
                                                 unicode_isspace))
            {
              continue;
            }

            module_path = utf8_encode(directory);
            if (module_path.back() != file_separator)
            {
              module_path += file_separator;
            }
            if (!::realpath(module_path.c_str(), buffer))
            {
              // Unable to locate the directory. Continue to next one.
              continue;
            }

            // Append the filename into the module directory path and try
            // whether that one exists or not.
            module_path = buffer;
            if (module_path.back() != file_separator)
            {
              module_path += file_separator;
            }
            module_path += encoded_path;
            if (!::realpath(module_path.c_str(), buffer))
            {
              // Try again with appended file extension.
              module_path += m_module_file_extension;
              if (!::realpath(module_path.c_str(), buffer))
              {
                // Give up.
                continue;
              }
            }

            // Finally try to resolve it into readable file.
            if (resolve_into_file(module_path, resolved_path))
            {
              return true;
            }
          }

          return false;
        }

        /**
         * Resolves arbitrary path which we know that exists in the file system
         * into path to a regular file which we can open and read source code
         * from.
         *
         * If the given path is pointing to a directory, see if that directory
         * contains a file called "index.plorth". If it does, use that one.
         * Otherwise refuse to use that directory for anything.
         *
         * If the path is pointing to a regular file that exists in the file
         * system, use it as the path from where to read source code from.
         *
         * \param path          Path to resolve.
         * \param resolved_path Where the resolved path will be placed into, if
         *                      the path was successfully resolved.
         * \return              Boolean flag telling whether the given path was
         *                      successfully resolved into a file or not.
         */
        bool resolve_into_file(const std::string& path,
                               std::u32string& resolved_path)
        {
          struct ::stat st;

          // Does the path even exist in the file system?
          if (::stat(path.c_str(), &st) < 0)
          {
            return false;
          }

          // Is it a directory? If so, look for a file called "index.plorth"
          // from it.
          if (S_ISDIR(st.st_mode))
          {
            auto index_file_path = path;

            if (index_file_path.back() != file_separator)
            {
              index_file_path += file_separator;
            }
            index_file_path += "index";
            index_file_path += m_module_file_extension;

            // OK. Does the index file exist?
            if (::stat(index_file_path.c_str(), &st) >= 0
                && S_ISREG(st.st_mode))
            {
              resolved_path = utf8_decode(index_file_path);

              return true;
            }
          }
          // Is it ordinary file? Then use that one.
          else if (S_ISREG(st.st_mode))
          {
            resolved_path = utf8_decode(path);

            return true;
          }

          // Otherwise use nothing.
          return false;
        }

        /**
         * Reads Plorth source code from given file system path, compiles it
         * into quote, then executes that quote under new execution context and
         * finally converts local dictionary of that execution context into an
         * object and returns the object.
         *
         * \param ctx  Execution context used for reporting errors.
         * \param path File system path to read source code from.
         * \return     Local dictionary of the module which source code has
         *             been read from the given path as an object or null
         *             reference if any kind of error occurred during the
         *             import.
         */
        std::shared_ptr<object> import_resolved_path(
          const std::shared_ptr<context>& ctx,
          const std::u32string& path
        )
        {
          std::ifstream is(utf8_encode(path));
          std::string raw_source;
          std::u32string source;
          std::shared_ptr<quote> compiled_module;
          std::shared_ptr<context> module_ctx;
          std::vector<object::value_type> result;
          std::shared_ptr<object> module;

          if (!is.good())
          {
            ctx->error(
              error::code::import,
              U"Unable to import from `" + path + U"'"
            );

            return std::shared_ptr<object>();
          }

          raw_source = std::string(
            std::istreambuf_iterator<char>(is),
            std::istreambuf_iterator<char>()
          );
          is.close();

          // First decode the source code with UTF-8 character encoding.
          if (!utf8_decode_test(raw_source, source))
          {
            ctx->error(
              error::code::import,
              U"Unable to decode source code into UTF-8."
            );

            return std::shared_ptr<object>();
          }

          // Then attempt to compile it.
          if (!(compiled_module = ctx->compile(source, path)))
          {
            return std::shared_ptr<object>();
          }

          // Run the module code inside new execution context.
          module_ctx = context::make(ctx->runtime());
          module_ctx->filename(path);
          if (!compiled_module->call(module_ctx))
          {
            if (module_ctx->error())
            {
              ctx->error(module_ctx->error());
            }

            return std::shared_ptr<object>();
          }

          // Finally convert the module into an object.
          for (const auto& word : module_ctx->dictionary().words())
          {
            result.push_back({ word->symbol()->id(), word->quote() });
          }

          module = ctx->runtime()->object(result);
          m_cache[path] = module;

          return module;
        }

      private:
        /** List of directories to look for modules. */
        const std::vector<std::u32string> m_lookup_paths;
        /** What file extension should be considered to be a module. */
        const std::string m_module_file_extension;
        /** Cache for already imported modules. */
        module_cache_type m_cache;
      };
#endif

      /**
       * Implementation of module manager which is unable to load any kind of
       * modules from anywhere.
       */
      class dummy_manager : public manager
      {
      public:
        std::shared_ptr<object> import_module(const std::shared_ptr<context>&,
                                              const std::u32string&)
        {
          return std::shared_ptr<object>();
        }
      };
    }

    std::shared_ptr<manager> manager::file_system(
      memory::manager& memory_manager,
      const std::vector<std::u32string>& lookup_paths,
      const std::u32string& module_file_extension
    )
    {
#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
      return std::shared_ptr<manager>(new (memory_manager) file_system_manager(
        lookup_paths,
        module_file_extension
      ));
#else
      return dummy(memory_manager);
#endif
    }

    std::shared_ptr<manager> manager::dummy(memory::manager& memory_manager)
    {
      return std::shared_ptr<manager>(new (memory_manager) dummy_manager());
    }

#if PLORTH_ENABLE_FILE_SYSTEM_MODULES
    /**
     * Tests whether given file path appears to be non-relative and pointing
     * directly to a specific file in the file system.
     */
    static bool is_absolute_path(const std::u32string& path)
    {
      const auto length = path.length();

      // Does the path begin file separator?
      if (length > 0 && path[0] == file_separator)
      {
        return true;
      }
#if defined(_WIN32)
      // Does the path begin with drive letter on Windows?
      else if (length > 3 &&
               std::isalpha(path[0]) &&
               path[1] == ':' &&
               path[2] == file_separator)
      {
        return true;
      }
#endif
      // Does the path begin with './'?
      if (length > 1 && path[0] == '.' && path[1] == file_separator)
      {
        return true;
      }
      // Does the path begin with '../'?
      else if (length > 2 &&
               path[0] == '.' &&
               path[1] == '.' &&
               path[2] == file_separator)
      {
        return true;
      }

      return false;
    }

    /**
     * Portable C++ implemenation of POSIX `dirname()` function.
     */
    static std::u32string dirname(const std::u32string& path)
    {
      const auto length = path.length();
      std::u32string::size_type index;

      if (!length)
      {
        return std::u32string();
      }

      index = path.find_last_of(file_separator);
      // No slashes found?
      if (index == std::u32string::npos)
      {
        return U".";
      }
      // Slash is the first character?
      else if (index == 0)
      {
        return std::u32string(1, file_separator);
      }
      // Slash is the last character?
      else if (index == length - 1)
      {
        return dirname(path.substr(0, index - 1));
      } else {
        return path.substr(0, index);
      }
    }
#endif
  }
}
