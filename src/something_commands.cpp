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
