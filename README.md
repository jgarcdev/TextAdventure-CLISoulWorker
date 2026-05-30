# SoulWorker CLI Text Aventure Game


## Building

### Windows

The main way to build the game itself is via cmake. This also includes the installer and the launcher.
CMake calls the zig compiler itself and not the build system.

The game, however, can also be built using zig build.

Currently, the cmake build process depends on XWIN for a copy of the MSVC headers.
This is done from a cross-compilation environment from Linux to Windows.

> Note: Windows MSYS2 users can use the Makefile however, it is not valid for the launcher target.
> The Makefile uses the C file which does not handle for Windows.

### Linux

Use the Makefile.

Since the launcher is written in Zig, it can be built on Linux as well.
However, the Makefile uses the C file.

The game itself can also be built using zig build.