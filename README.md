# Plorth

Plorth is a stack based, statically typed, functional and minimal scripting language
which is easy to embed in applications written in C++. It's inspired by [Forth] and
[Factor] programming languages.

## Compilation

Plorth has no dependencies. Only C++11 capable compiler and [CMake] are required to
compile Plorth interpreter.

```bash
mkdir build
cd build
cmake ..
make
```

After the interpreter has been successfully compiled, you can run the `plorth-cli`
binary to start Plorth REPL.

[Forth]: https://www.forth.com
[Factor]: http://www.factorcode.org
[CMake]: https://www.cmake.org
