#ifndef SOMETHING_COMMANDS_HPP_
#define SOMETHING_COMMANDS_HPP_

struct Game;

void command_quit(Game *game, String_View args);
void command_reset(Game *game, String_View args);
void command_spawn_enemy(Game *game, String_View args);
void command_close(Game *game, String_View args);
#ifndef SOMETHING_RELEASE
void command_set(Game *game, String_View args);
#endif // SOMETHING_RELEASE

struct Command
{
    String_View name;
    void (*run)(Game *game, String_View args);
};

const Command commands[] = {
    {"quit"_sv, command_quit},
    {"reset"_sv, command_reset},
    {"spawn_enemy"_sv, command_spawn_enemy},
    {"close"_sv, command_close},
#ifndef SOMETHING_RELEASE
    {"set"_sv, command_set},
#endif // SOMETHING_RELEASE
};
const size_t commands_count = sizeof(commands) / sizeof(commands[0]);

#endif  // SOMETHING_COMMANDS_HPP_
