[![Build Status](https://github.com/tsoding/something/workflows/CI/badge.svg)](https://github.com/tsoding/something/actions)

# something
**WARNING! The game is in an active development state and is not even
alpha yet. Use it at your own risk. Nothing is documented, anything
can be changed at any moment or stop working at all.**

## Quick Start
Install dependencies for your OS/Distro.
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
`Windows`:
```sh
$ mingw32-make -B
$ something.debug.exe
```

## License
[MIT](./LICENSE)

