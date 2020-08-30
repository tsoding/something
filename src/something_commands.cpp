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

void command_set(Game *game, String_View args)
{
    const auto varname = args.chop_word();
    const auto varindex = config_index_by_name(varname);
    if (varindex < 0) {
        game->console.println("Variable `", varname, "` does not exist!");
        return;
    }

    switch (config_types[varindex]) {
    case CONFIG_TYPE_INT:
        game->console.println(varname, " = ", config_values[varindex].int_value);
        break;
    case CONFIG_TYPE_FLOAT:
        game->console.println(varname, " = ", config_values[varindex].float_value);
        break;
    case CONFIG_TYPE_COLOR:
        game->console.println(varname, " = ", config_values[varindex].color_value);
        break;
    case CONFIG_TYPE_STRING:
        game->console.println(varname, " = ", config_values[varindex].string_value);
        break;
    case CONFIG_TYPE_UNKNOWN:
    case CONFIG_TYPE_SDL_BLENDFACTOR:
    case CONFIG_TYPE_SDL_BLENDOPERATION:
    default: {
        game->console.println("Variable `", varname, "` has unknown type");
    }
    }

    // const auto varvalue = args.trim();
}
