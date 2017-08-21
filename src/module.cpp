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

namespace plorth
{
#if PLORTH_ENABLE_MODULES
  static const char* plorth_file_extension = ".plorth";
# if defined(_WIN32)
  static const unichar file_separator = '\\';
# else
  static const unichar file_separator = '/';
# endif

  static bool resolve_path(const unistring&,
                           unistring&,
                           const std::vector<unistring>&);
#endif

  bool runtime::import(const ref<context>& ctx, const unistring& path)
  {
    // Do not import empty paths.
    if (path.empty() || std::all_of(path.begin(), path.end(), unichar_isspace))
    {
      ctx->error(error::code_import, "Empty import path.");

      return false;
    }

#if PLORTH_ENABLE_MODULES
    unistring resolved_path;
    object::container_type::const_iterator entry;
    ref<object> module;

    // First attempt to resolve the module path into actual file system path.
    if (!resolve_path(path, resolved_path, m_module_paths))
    {
      ctx->error(error::code_import, "No such file or directory.");

      return false;
    }

    // Then look from the module cache whether the module has already been
    // loaded.
    entry = m_imported_modules.find(resolved_path);

    // If the module has already been loaded, just use the cached module.
    // Otherwise begin the process of reading source code from the file,
    // compiling that into a quote and finally executing the quote in new
    // separate execution context.
    if (entry != std::end(m_imported_modules))
    {
      module = entry->second.cast<object>();
    } else {
      std::fstream is(utf8_encode(resolved_path));

      if (is.good())
      {
        const std::string input = std::string(
          std::istreambuf_iterator<char>(is),
          std::istreambuf_iterator<char>()
        );
        unistring source;
        ref<quote> compiled_module;
        ref<context> module_context;

        is.close();

        if (!utf8_decode_test(input, source))
        {
          ctx->error(error::code_import, "Unable to decode source code into UTF-8.");

          return false;
        }

        // Did the compilation fail?
        if (!(compiled_module = ctx->compile(source)))
        {
          return false;
        }

        // Run the module code inside new execution context.
        module_context = new_context();
        if (!compiled_module->call(module_context))
        {
          if (module_context->error())
          {
            ctx->error(module_context->error());
          } else {
            ctx->error(error::code_import, "Module import failed.");
          }

          return false;
        }

        // TODO: Add some kind of way to selectively export words from a module.
        module = value<object>(module_context->dictionary());
        m_imported_modules[resolved_path] = module;
      } else {
        ctx->error(error::code_import, "Unable to import `" + resolved_path + "'");

        return false;
      }
    }

    // Transfer all exported words from the module into the calling execution
    // context.
    for (const auto& property : module->properties())
    {
      ctx->dictionary()[property.first] = property.second;
    }

    return true;
#else
    ctx->error(error::code_import, "Modules have been disabled.");

    return false;
#endif
  }

#if PLORTH_ENABLE_MODULES
  static bool is_absolute_path(const unistring& path)
  {
    const auto length = path.length();

    // Does the path begin file separator?
    if (length > 0 && path[0] == file_separator)
    {
      return true;
    }
#if defined(_WIN32)
    // Does the path begin with drive letter on Windows?
    else if (length > 3 && std::isalpha(path[0]) && path[1] == ':' && path[2] == file_separator)
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
    else if (length > 2 && path[0] == '.' && path[1] == '.' && path[2] == file_separator)
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

      if (index_file_path.back() != file_separator)
      {
        index_file_path += file_separator;
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

  static bool resolve_path(const unistring& path,
                           unistring& resolved_path,
                           const std::vector<unistring>& module_directories)
  {
    std::string encoded_path = utf8_encode(path);
    char buffer[PATH_MAX];

    // If the path is absolute, resolve it into full path and use that directly.
    if (is_absolute_path(path))
    {
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
    for (const auto& directory : module_directories)
    {
      std::string module_path;

      // Skip empty paths.
      if (directory.empty() || std::all_of(directory.begin(),
                                           directory.end(),
                                           unichar_isspace))
      {
        continue;
      }

      module_path = encoded_path;
      if (!::realpath(module_path.c_str(), buffer))
      {
        // Unable to locate the directory. Continue to next one.
        continue;
      }

      // Append the filename into the module directory path and try whether that
      // one exists or not.
      module_path = buffer;
      if (module_path.back() != file_separator)
      {
        module_path += file_separator;
      }
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
