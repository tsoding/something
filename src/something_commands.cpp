#include "./something_game.hpp"
#include "./something_commands.hpp"

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

void sprint1(String_Buffer *sbuffer, SDL_Color color)
{
    sprint(sbuffer, "{", color.r, ",", color.g, ",", color.b, ",", color.a, "}");
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

    const auto varvalue = args.trim();

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
        game->console.println("TODO: setting string variables is not implemented yet");
    } break;
    case CONFIG_TYPE_UNKNOWN:
    default: {
        game->console.println("Variable `", varname, "` has unknown type");
    }
    }
}
#endif // SOMETHING_RELEASE
