#ifndef SOMETHING_COMMANDS_HPP_
#define SOMETHING_COMMANDS_HPP_

struct Game;

void command_help(Game *game, String_View args);
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
    String_View description;
    void (*run)(Game *game, String_View args);
};

const Command commands[] = {
    {"help"_sv,        "Print this help"_sv,                 command_help},
    {"quit"_sv,        "Quit the game"_sv,                   command_quit},
    {"reset"_sv,       "Reset the state of the entities"_sv, command_reset},
    {"spawn_enemy"_sv, "Spawn an enemy"_sv,                  command_spawn_enemy},
    {"close"_sv,       "Close the console"_sv,               command_close},
#ifndef SOMETHING_RELEASE
    {"set"_sv,         "Set the value of a variable"_sv,     command_set},
#endif // SOMETHING_RELEASE
};
const size_t commands_count = sizeof(commands) / sizeof(commands[0]);

#endif  // SOMETHING_COMMANDS_HPP_
