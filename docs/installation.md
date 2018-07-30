# Installing Plorth

This page describes how to get Plorth up and running on your computer.

Plorth has no dependencies, only [CMake] and C++11 capable compiler are
required to compile the interpreter.

## Retrieving the source code

Your best bet of getting your hands on Plorth interpreter source code is to
clone it from [GitHub](https://github.com/RauliL/plorth.git). This can be done
by executing the following command:

```bash
$ git clone https://github.com/RauliL/plorth.git
```

If you are planning on using the command line interface, you will also need to
execute the following command to pull in external dependencies used by it:

```bash
$ git submodule --init
```

## Compiling

After you have cloned the source code from GitHub, you need to compile the
interpreter. For this you need modern C++ compiler and [CMake]. Change into
`plorth` directory and run following commands:

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

This will begin compilation of the interpreter. The interpreter consists of
two parts: Plorth interpreter library and it's command line user interface
known as `plorth` which uses the interpreter library. By tweaking the CMake
options, it's also possible to compile only the library without `plorth`
executable, if you plan to embed the interpreter to your own C++ application.

## Installation

After the interpreter has been compiled, you can run the `plorth` executable
located in `build/plorth` directory. Alternatively, you can also install Plorth
into your system by running the following command:

```bash
sudo make install
```

The installation however is not necessary if you plan only to play with the
interpreter's REPL and possibly run some examples.

[CMake]: https://cmake.org
