# Plorth ![Build Status]

Plorth is stack based, concatenative, strongly typed functional scripting
language which is easy to embed to applications written in C++. It's inspired
by [Forth] and [Factor] programming languages.

Features:
- Strong typing.
- Supports JSON data types and syntax.
- Garbage collected.
- Prototype based OO, just like in JavaScript.

[Browse documentation.][Documentation]

[Try Plorth in your browser.][REPL]

## Compilation

Plorth has no dependencies. Only C++11 capable compiler and [CMake] are
required to compile Plorth interpreter.

```bash
$ git submodule update --init
$ mkdir build
$ cd build
$ cmake ..
$ make
```

After the interpreter has been successfully compiled, you can run the `plorth`
binary to start Plorth REPL.

[Forth]: https://www.forth.com
[Factor]: http://www.factorcode.org
[CMake]: https://www.cmake.org
[Documentation]: http://plorth.org
[REPL]: https://raulil.github.io/plorth-ide/
[Build Status]: https://travis-ci.org/RauliL/plorth.svg?branch=master
