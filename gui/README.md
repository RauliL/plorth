# Plorth GUI

Experimental non-fancy GUI REPL for Plorth, using GTK3 as the widget toolkit.
It features simple line editor, input/output history and live stack display.

## Compilation

Plorth GUI depends on [gtkmm] and `libplorth` libraries. C++11 capable compiler
and [CMake] are also required to compile Plorth GUI.

```bash
mkdir build
cd build
cmake ..
make
```

After the GUI has been successfully compiled, you can run the `plorth-gui`
binary to start the graphical version of Plorth REPL.

[gtkmm]: https://gtkmm.org
[CMake]: https://www.cmake.org
