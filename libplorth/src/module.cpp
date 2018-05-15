#include <plorth/context.hpp>
#if PLORTH_ENABLE_MODULES
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
#if PLORTH_ENABLE_MODULES
  static const char* plorth_file_extension = ".plorth";

  static ref<object> module_import(context*, const unistring&);
  static bool module_resolve_path(context*,
                                  const unistring&,
                                  unistring&);
#endif

  bool context::import(const unistring& path)
  {
    // Do not import empty paths.
    if (path.empty() || std::all_of(path.begin(), path.end(), unichar_isspace))
    {
      error(error::code_import, U"Empty import path.");

      return false;
    }

#if PLORTH_ENABLE_MODULES
    unistring resolved_path;
    auto& imported_modules = m_runtime->imported_modules();
    runtime::module_container::iterator entry;
    ref<object> module;

    // First attempt to resolve the module path into actual file system path.
    if (!module_resolve_path(this, path, resolved_path))
    {
      error(error::code_import, U"No such file or directory.");

      return false;
    }

    // Then look from the module cache whether the module has already been
    // loaded.
    entry = imported_modules.find(resolved_path);

    // If the module has already been loaded, just use the cached module.
    // Otherwise begin the process of reading source code from the file,
    // compiling that into a quote and finally executing the quote in new
    // separate execution context.
    if (entry != std::end(imported_modules))
    {
      module = entry->second.cast<object>();
    }
    else if ((module = module_import(this, resolved_path)))
    {
      imported_modules[resolved_path] = module;
    } else {
      return false;
    }

    // Transfer all exported words from the module into the calling execution
    // context.
    for (const auto& property : module->entries())
    {
      if (property.second && property.second->is(value::type_quote))
      {
        m_dictionary.insert(m_runtime->word(
          m_runtime->symbol(property.first),
          property.second.cast<quote>()
        ));
      }
    }

    return true;
#else
    error(error::code_import, U"Modules have been disabled.");

    return false;
#endif
  }

#if PLORTH_ENABLE_MODULES
  static ref<object> module_import(context* ctx, const unistring& path)
  {
    std::ifstream is(utf8_encode(path));
    std::string raw_source;
    unistring source;
    ref<quote> compiled_module;
    ref<context> module_ctx;
    std::vector<object::value_type> result;

    if (!is.good())
    {
      ctx->error(error::code_import, U"Unable to import `" + path + U"'");

      return ref<object>();
    }

    raw_source = std::string(
      std::istreambuf_iterator<char>(is),
      std::istreambuf_iterator<char>()
    );
    is.close();

    // First decode the source as UTF-8.
    if (!utf8_decode_test(raw_source, source))
    {
      ctx->error(
        error::code_import,
        U"Unable to decode source code into UTF-8."
      );

      return ref<object>();
    }

    // Then attempt to compile it.
    if (!(compiled_module = ctx->compile(source, path)))
    {
      return ref<object>();
    }

    // Run the module code inside new execution context.
    module_ctx = context::make(ctx->runtime());
    module_ctx->filename(path);
    if (!compiled_module->call(module_ctx))
    {
      if (module_ctx->error())
      {
        ctx->error(module_ctx->error());
      } else {
        ctx->error(error::code_import, U"Module import failed.");
      }

      return ref<object>();
    }

    // Finally convert the module into object.
    for (const auto& word : module_ctx->dictionary().words())
    {
      result.push_back(std::make_pair(word->symbol()->id(), word->quote()));
    }

    return ctx->runtime()->value<object>(result);
  }

  static bool is_absolute_path(const unistring& path)
  {
    const auto length = path.length();

    // Does the path begin file separator?
    if (length > 0 && path[0] == PLORTH_FILE_SEPARATOR)
    {
      return true;
    }
#if defined(_WIN32)
    // Does the path begin with drive letter on Windows?
    else if (length > 3 &&
             std::isalpha(path[0]) &&
             path[1] == ':' &&
             path[2] == PLORTH_FILE_SEPARATOR)
    {
      return true;
    }
#endif
    // Does the path begin with './'?
    if (length > 1 && path[0] == '.' && path[1] == PLORTH_FILE_SEPARATOR)
    {
      return true;
    }
    // Does the path begin with '../'?
    else if (length > 2 &&
             path[0] == '.' &&
             path[1] == '.' &&
             path[2] == PLORTH_FILE_SEPARATOR)
    {
      return true;
    }

    return false;
  }

  /**
   * Resolves arbitrary path which we know that exist in the file system into
   * path to a regular file which we can open and read source code from.
   *
   * If the given path is pointing to a directory, see if that directory
   * contains file called "index.plorth". If it does, use that one. Otherwise
   * refuse to use that directory for anything.
   *
   * If the path is pointing to a regular file that exist in the file system,
   * use it as the path from where to read source code from.
   */
  static bool resolve_into_file(const std::string& path,
                                unistring& resolved_path)
  {
    struct ::stat st;

    // Does the path even exist in the filesystem?
    if (::stat(path.c_str(), &st) < 0)
    {
      return false;
    }

    // Is it a directory? If so, look for a file called "index.plorth"
    // from it.
    if (S_ISDIR(st.st_mode))
    {
      std::string index_file_path = path;

      if (index_file_path.back() != PLORTH_FILE_SEPARATOR)
      {
        index_file_path += PLORTH_FILE_SEPARATOR;
      }
      index_file_path += "index";
      index_file_path += plorth_file_extension;

      // OK. Does the index file exist?
      if (::stat(index_file_path.c_str(), &st) >= 0 && S_ISREG(st.st_mode))
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

  static bool module_resolve_path(context* ctx,
                                  const unistring& path,
                                  unistring& resolved_path)
  {
    std::string encoded_path = utf8_encode(path);
    char buffer[PATH_MAX];

    // If the path is absolute, resolve it into full path and use that directly.
    if (is_absolute_path(path))
    {
      const auto dir = utf8_encode(dirname(ctx->filename()));

      if (!dir.empty())
      {
        encoded_path = dir + PLORTH_FILE_SEPARATOR + encoded_path;
      }
      if (!::realpath(encoded_path.c_str(), buffer))
      {
        // Try again with appended file extension.
        encoded_path += plorth_file_extension;
        if (!::realpath(encoded_path.c_str(), buffer))
        {
          // Give up.
          return false;
        }
      }

      return resolve_into_file(buffer, resolved_path);
    }

    // Otherwise go through the module directories and look for a matching file
    // from each of them.
    for (const auto& directory : ctx->runtime()->module_paths())
    {
      std::string module_path;

      // Skip empty paths.
      if (directory.empty() || std::all_of(directory.begin(),
                                           directory.end(),
                                           unichar_isspace))
      {
        continue;
      }

      module_path = utf8_encode(directory);
      if (module_path.back() != PLORTH_FILE_SEPARATOR)
      {
        module_path += PLORTH_FILE_SEPARATOR;
      }
      if (!::realpath(module_path.c_str(), buffer))
      {
        // Unable to locate the directory. Continue to next one.
        continue;
      }

      // Append the filename into the module directory path and try whether that
      // one exists or not.
      module_path = buffer;
      if (module_path.back() != PLORTH_FILE_SEPARATOR)
      {
        module_path += PLORTH_FILE_SEPARATOR;
      }
      module_path += encoded_path;
      if (!::realpath(module_path.c_str(), buffer))
      {
        // Try again with appended file extension.
        module_path += plorth_file_extension;
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
#endif
}
