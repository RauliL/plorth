# Plorth CLI

This directory contains source code for the command line interface of the
Plorth interpreter.

## REPL

By default, Plorth CLI comes with [REPL] utility that allows you to use Plorth
interactively from the console. Line editing of this REPL has been implemented
with customized version of [Linenoise] library, which supports only UTF-8
character encoding and does not currently compile under Windows. It can be
disabled by unsetting CMake option `PLORTH_CLI_ENABLE_REPL`.

[REPL]: https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop
[Linenoise]: https://github.com/antirez/linenoise/
