[![Build Status](https://github.com/tsoding/something/workflows/CI/badge.svg)](https://github.com/tsoding/something/actions)

# something
**WARNING! The game is in an active development state and is not even
alpha yet. Use it at your own risk. Nothing is documented, anything
can be changed at any moment or stop working at all.**

## Quick Start
Install dependencies for your OS/Distro, here is a table for help, this does not include compile and make.
| OS / Distro                   | Dependency package names                            |
|-------------------------------|-----------------------------------------------------|
| Debian or based (Ubuntu)      | libsdl2-dev pkg-config                              |
| Arch Linux or based (Manjaro) | sdl2 pkgconf                                        |
| Void Linux                    | SDL2-devel pkg-config                               |
| Window (MinGW (with MSYS2))   | mingw-w64-x86\_64-SDL2 mingw-w64-x86\_64-pkg-config |
| Window (Visual Studio)        | Instructions below                                  |

### Building
`*NIX`:
```sh
$ make -B
$ ./something.debug
```
`Windows (Visual Studio)`:
```
> curl -fsSL -o SDL2-devel-2.0.12-VC.zip https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip
> tar -xf SDL2-devel-2.0.12-VC.zip
> move SDL2-2.0.12 SDL2
> del SDL2-devel-2.0.12-VC.zip
> build_msvc
```
`Windows (MinGW)`:
```sh
$ mingw32-make -B
$ something.debug.exe
```

## License
[MIT](./LICENSE)

