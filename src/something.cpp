#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <SDL.h>

#ifdef SOMETHING_RELEASE
#define STB_IMAGE_IMPLEMENTATION
#endif
#define STBI_ONLY_PNG
#include "./stb_image.h"

#include "aids.hpp"

using namespace aids;

#include "something_math.cpp"
#include "something_color.hpp"

#ifndef SOMETHING_RELEASE
#include "config_common.cpp"
#else
#include "./baked_config.hpp"
#endif

// READ THIS FIRST ---> https://en.wikipedia.org/wiki/Single_Compilation_Unit
#ifndef SOMETHING_RELEASE
// TODO(#173): config autoreloading does not work on Windows
#  if defined(__linux__)
#    include "something_fmw_inotify.cpp"
#  elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) \
	|| defined(__DragonFly__)
#    include "something_fmw_kqueue.cpp"
#  else
#    include "something_fmw_dummy.cpp"
#  endif // __linux__
#endif // SOMETHING_RELEASE
#ifdef _WIN32
#include "something_dirent.cpp"
#else
#include <dirent.h>
#endif // _WIN32
#include "something_error.cpp"
#include "something_color.cpp"
#include "something_render.cpp"
#include "something_font.cpp"
#include "something_camera.cpp"
#include "something_texture.cpp"
#include "something_sprite.cpp"
#include "something_tile_grid.cpp"
#include "something_sound.cpp"
#include "something_entity.cpp"
#include "something_popup.cpp"
#include "something_item.cpp"
#include "something_toolbar.cpp"
#include "something_commands.cpp"
#include "something_select_popup.cpp"
#include "something_edit_field.cpp"
#include "something_console.cpp"
#include "something_particles.cpp"
#include "something_background.cpp"
#include "something_game.cpp"
#include "something_main.cpp"
#include "something_assets.cpp"
