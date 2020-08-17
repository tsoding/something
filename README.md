[![Build Status](https://github.com/tsoding/something/workflows/CI/badge.svg)](https://github.com/tsoding/something/actions)

# something

**WARNING! The game is in an active development state and is not even
alpha yet. Use it at your own risk. Nothing is documented, anything
can be changed at any moment or stop working at all.**

## Quick Start

```console
$ # Dependencies
$ ## Debian
$ sudo apt-get install libsdl2-dev
$ ## Manjaro
$ sudo pacman -S sdl2
$ ## (add your distro here)
$ ## Windows
$ ### MinGW (with MSYS)
$ pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkg-config
$ # Build
$ ## UNIX-like system
$ make -B
$ ./something.debug
$ ## Windows
$ mingw32-make
$ something.debug
```
