#include "./something_game.hpp"
#include "./something_commands.hpp"

void command_help(Game *game, String_View)
{
    for (size_t i = 0; i < commands_count; ++i) {
        game->console.println(commands[i].name, " - ", commands[i].description);
    }
}

void command_quit(Game *, String_View)
{
    exit(0);
}

void command_reset(Game *game, String_View)
{
    game->reset_entities();
}

void command_spawn_enemy(Game *game, String_View)
{
    game->spawn_enemy_at(game->mouse_position);
}

void command_close(Game *game, String_View)
{
    game->console.toggle();
}


#ifndef SOMETHING_RELEASE
void command_set(Game *game, String_View args)
{
    const auto varname = args.chop_word();
    const auto varindex = config_index_by_name(varname);
    if (varindex < 0) {
        game->console.println("Variable `", varname, "` does not exist!");
        return;
    }

    auto varvalue = args.trim();

    switch (config_types[varindex]) {
    case CONFIG_TYPE_INT: {
        auto x = varvalue.as_integer<int>();
        if (!x.has_value) {
            game->console.println("`", varvalue, "` is not an int");
        } else {
            config_values[varindex].int_value = x.unwrap;
        }
    } break;
    case CONFIG_TYPE_FLOAT: {
        auto x = varvalue.as_float();
        if (!x.has_value) {
            game->console.println("`", varvalue, "` is not a float");
        } else {
            config_values[varindex].float_value = x.unwrap;
        }
    } break;
    case CONFIG_TYPE_COLOR: {
        auto x = string_view_as_color(varvalue);
        if (!x.has_value) {
            game->console.println("`", varvalue, "` is not a color");
        } else {
            config_values[varindex].color_value = x.unwrap;
        }
    } break;
    case CONFIG_TYPE_STRING: {
        if (varvalue.count > 0 && *varvalue.data != '"') {
            game->console.println("`", varvalue, "` is not a string");
            break;
        }

        varvalue.chop(1);
        auto string_value = varvalue.chop_by_delim('"');

        if (config_file_buffer_size + string_value.count > CONFIG_FILE_CAPACITY) {
            game->console.println("Not enough config file memory to set this variable");
            break;
        }

        memcpy(config_file_buffer + config_file_buffer_size,
               string_value.data,
               string_value.count);

        config_values[varindex].string_value = {
            string_value.count,
            config_file_buffer + config_file_buffer_size
        };

        config_file_buffer_size += string_value.count;
    } break;
    case CONFIG_TYPE_UNKNOWN:
    default: {
        game->console.println("Variable `", varname, "` has unknown type");
    }
    }
}

void command_reload(Game *game, String_View)
{
    auto result = reload_config_file(VARS_CONF_FILE_PATH);
    if (result.is_error) {
        game->console.println(VARS_CONF_FILE_PATH, ":", result.line, ": ", result.message);
        game->popup.notify(FONT_FAILURE_COLOR, "%s:%d: %s", VARS_CONF_FILE_PATH, result.line, result.message);
    } else {
        game->console.println("Reloaded config file `", VARS_CONF_FILE_PATH, "`");
        game->popup.notify(FONT_SUCCESS_COLOR, "Reloaded config file\n\n%s", VARS_CONF_FILE_PATH);
    }
}

#endif // SOMETHING_RELEASE

void command_save_room(Game *game, String_View)
{
    auto &player = game->entities[PLAYER_ENTITY_INDEX];
    Recti *lock = NULL;
    for (size_t i = 0; i < game->camera_locks_count; ++i) {
        Rectf lock_abs = rect_cast<float>(game->camera_locks[i]) * TILE_SIZE;
        if (rect_contains_vec2(lock_abs, player.pos)) {
            lock = &game->camera_locks[i];
        }
    }
    if(lock) {
        size_t tile_index = 0;
        for (int y = lock->y; y < lock->y + ROOM_HEIGHT; ++y) {
            for (int x = lock->x; x < lock->x + ROOM_WIDTH; ++x) {
                room_to_save[tile_index] = game->grid.tiles[y][x];
                tile_index++;
            }
        }

        const int rooms_count = game->get_rooms_count();

        char filepath[256];
        snprintf(filepath, sizeof(filepath), "./assets/rooms/room-%d.bin", rooms_count);
        FILE *f = fopen(filepath, "wb");
        if (!f) {
            game->console.println("Could not open file `", filepath, "`: ",
                    strerror(errno));
            return;
        }
        fwrite(room_to_save, sizeof(room_to_save[0]), ROOM_HEIGHT * ROOM_WIDTH, f);
        fclose(f);

        game->console.println("New room is saved");
    } else {
        game->console.println("Can't find a room with Player in it");
    }
}

void command_history(Game *game, String_View)
{
    game->console.println("--------------------");
    for (int i = 0; i < game->console.history.count; ++i) {
        const size_t j = (game->console.history.begin + i) % CONSOLE_HISTORY_CAPACITY;
        const String_View entry = {
            game->console.history.entry_sizes[j],
            game->console.history.entries[j]
        };
        game->console.println(entry);
    }
    game->console.println("--------------------");
}

void command_noclip(Game *game, String_View args)
{
    args = args.trim();

    if (args == "on"_sv) {
        game->noclip(true);
    } else if (args == "off"_sv) {
        game->noclip(false);
    } else {
        game->console.println("Unknown parameter `", args, "`. Expected 'on' or 'off'");
    }
}
