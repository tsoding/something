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
$ ### Visual Studio
$ curl -fsSL -o SDL2-devel-2.0.12-VC.zip https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip
$ tar -xf SDL2-devel-2.0.12-VC.zip
$ move SDL2-2.0.12 SDL2
$ del SDL2-devel-2.0.12-VC.zip
$ build_msvc
$ ### MinGW (with MSYS2)
$ pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkg-config
$ # Build
$ ## UNIX-like system
$ make -B
$ ./something.debug
$ ## Windows
$ set __MINGW32__=1 && mingw32-make -B
$ something.debug
```
## Mininum System Requirements / Dependencies

- libsdl2-dev (>= 2.0.5)
- g++ (>= 7.5)
