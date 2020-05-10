enum Config_Type
{
    CONFIG_TYPE_NONE = 0,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_COLOR
};

enum Config_Var
{
    PLAYER_SPEED = 0,
    ENTITY_COOLDOWN_WEAPON,
    ENTITY_GRAVITY,
    ENTITY_INITIAL_LIVES,
    ENTITY_GUN_LENGTH,
    ENTITY_PROJECTILE_DAMAGE,
    ENTITY_LIVEBAR_WIDTH,
    ENTITY_LIVEBAR_HEIGHT,
    ENTITY_LIVEBAR_PADDING_BOTTOM,
    ENTITY_LIVEBAR_COLOR,
    ENTITY_MAX_LIVES,
    ROOM_NEIGHBOR_DIM_ALPHA,

    CONFIG_VAR_CAPACITY,
    CONFIG_VAR_UNKNOWN,
};

union Config_Value
{
    float float_value;
    int int_value;
    SDL_Color color_value;
};

bool config_types_inited = false;
Config_Type config_types[CONFIG_VAR_CAPACITY] = {};
Config_Value config_values[CONFIG_VAR_CAPACITY] = {};

const size_t CONFIG_FILE_CAPACITY = 1 * 1024 * 1024;
char config_file_buffer[CONFIG_FILE_CAPACITY];

const size_t CONFIG_ERROR_CAPACITY = 1024;
char config_error_buffer[CONFIG_ERROR_CAPACITY];

void init_config_types()
{
    config_types[PLAYER_SPEED]                  = CONFIG_TYPE_FLOAT;
    config_types[ENTITY_COOLDOWN_WEAPON]        = CONFIG_TYPE_FLOAT;
    config_types[ENTITY_GRAVITY]                = CONFIG_TYPE_FLOAT;
    config_types[ENTITY_GUN_LENGTH]             = CONFIG_TYPE_FLOAT;
    config_types[ENTITY_INITIAL_LIVES]          = CONFIG_TYPE_INT;
    config_types[ENTITY_LIVEBAR_WIDTH]          = CONFIG_TYPE_FLOAT;
    config_types[ENTITY_LIVEBAR_HEIGHT]         = CONFIG_TYPE_FLOAT;
    config_types[ENTITY_LIVEBAR_PADDING_BOTTOM] = CONFIG_TYPE_FLOAT;
    config_types[ROOM_NEIGHBOR_DIM_ALPHA]       = CONFIG_TYPE_INT;
    config_types[ENTITY_PROJECTILE_DAMAGE]      = CONFIG_TYPE_INT;
    config_types[ENTITY_MAX_LIVES]              = CONFIG_TYPE_INT;
    config_types[ENTITY_LIVEBAR_COLOR]          = CONFIG_TYPE_COLOR;
    config_types_inited = true;
}

String_View config_var_as_string_view(Config_Var var)
{
    switch(var) {
    case PLAYER_SPEED:                  return "PLAYER_SPEED"_sv;
    case ENTITY_COOLDOWN_WEAPON:        return "ENTITY_COOLDOWN_WEAPON"_sv;
    case ENTITY_GRAVITY:                return "ENTITY_GRAVITY"_sv;
    case ENTITY_INITIAL_LIVES:          return "ENTITY_INITIAL_LIVES"_sv;
    case ENTITY_GUN_LENGTH:             return "ENTITY_GUN_LENGTH"_sv;
    case ENTITY_PROJECTILE_DAMAGE:      return "ENTITY_PROJECTILE_DAMAGE"_sv;
    case ENTITY_LIVEBAR_WIDTH:          return "ENTITY_LIVEBAR_WIDTH"_sv;
    case ENTITY_LIVEBAR_HEIGHT:         return "ENTITY_LIVEBAR_HEIGHT"_sv;
    case ENTITY_LIVEBAR_PADDING_BOTTOM: return "ENTITY_LIVEBAR_PADDING_BOTTOM"_sv;
    case ROOM_NEIGHBOR_DIM_ALPHA:       return "ROOM_NEIGHBOR_DIM_ALPHA"_sv;
    case ENTITY_MAX_LIVES:              return "ENTITY_MAX_LIVES"_sv;
    case ENTITY_LIVEBAR_COLOR:          return "ENTITY_LIVEBAR_COLOR"_sv;

    case CONFIG_VAR_CAPACITY:
    case CONFIG_VAR_UNKNOWN:
    {}
    }

    return {};
}

Config_Var string_view_as_config_var(String_View name)
{
    for (int var = 0; var < CONFIG_VAR_CAPACITY; ++var) {
        if (name == config_var_as_string_view((Config_Var) var)) {
            return (Config_Var) var;
        }
    }

    return CONFIG_VAR_UNKNOWN;
}

Maybe<float> string_view_as_float(String_View input)
{
    char buffer[300] = {};
    memcpy(buffer, input.data, min(sizeof(buffer) - 1, input.count));
    char *endptr = NULL;
    float result = strtof(buffer, &endptr);

    if (buffer > endptr || (size_t) (endptr - buffer) != input.count) {
        return {};
    }

    return {true, result};
}

struct Config_Parse_Result
{
    bool is_error;
    const char *message;
    size_t line;
};

inline
Config_Parse_Result parse_success()
{
    Config_Parse_Result result = {};
    return result;
}

inline
Config_Parse_Result parse_failure(const char *message, size_t line)
{
    Config_Parse_Result result = {};
    result.is_error = true;
    result.message = message;
    result.line = line;
    return result;
}

Maybe<SDL_Color> string_view_as_color(String_View input)
{
    // input == "ff0000ff"
    if (input.count != 8) return {};

    SDL_Color result = {};
    result.r = input.subview(0, 2).from_hex<Uint8>();
    result.g = input.subview(2, 2).from_hex<Uint8>();
    result.b = input.subview(4, 2).from_hex<Uint8>();
    result.a = input.subview(6, 2).from_hex<Uint8>();

    return {true, result};
}

Config_Parse_Result parse_config_text(String_View input)
{
    if (!config_types_inited) init_config_types();

    for (size_t line_number = 1; input.count > 0; ++line_number) {
        String_View line = input.chop_by_delim('\n').trim();

        if (line.count == 0) continue;   // Empty line
        if (*line.data == '#') continue; // Comment

        String_View name = line.chop_by_delim('=').trim();
        // We are choping value by '#' for a potention comment on the
        // same line as the definition.
        String_View value = line.chop_by_delim('#').trim();

        auto var = string_view_as_config_var(name);
        if (var >= CONFIG_VAR_UNKNOWN) {
            snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                     "Unknown variable `%.*s`",
                     (int) name.count, name.data);
            return parse_failure(config_error_buffer, line_number);
        }

        switch (config_types[var]) {
        case CONFIG_TYPE_COLOR: {
            auto x = string_view_as_color(value);
            if (!x.has_value) {
                snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                         "`%.*s` is not a color (variable `%.*s`)",
                         (int) value.count, value.data,
                         (int) name.count, name.data);
                return parse_failure(config_error_buffer, line_number);
            }
            config_values[var].color_value = x.unwrap;
        } break;

        case CONFIG_TYPE_INT: {
            auto x = value.as_integer<int>();
            if (!x.has_value) {
                snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                         "`%.*s` is not an int (variable `%.*s`)",
                         (int) value.count, value.data,
                         (int) name.count, name.data);
                return parse_failure(config_error_buffer, line_number);
            }
            config_values[var].int_value = x.unwrap;
        } break;

        case CONFIG_TYPE_FLOAT: {
            auto x = string_view_as_float(value);
            if (!x.has_value) {
                snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                         "`%.*s` is not a float (variable `%.*s`)",
                         (int) value.count, value.data,
                         (int) name.count, name.data);
                return parse_failure(config_error_buffer, line_number);
            }
            config_values[var].float_value = x.unwrap;
        } break;

        case CONFIG_TYPE_NONE: {
            println(stderr,
                    "[ERROR] Could not find the type definition of ", name, " variable. ",
                    "Please add it to the init_config_types() function.");
            abort();
        } break;
        }
    }

    return parse_success();
}

Config_Parse_Result reload_config_file(const char *file_path)
{
    FILE *f = fopen(file_path, "rb");
    if (!f) {
        println(stderr, "Could not open file `", file_path, "`: ",
                strerror(errno));
        abort();
    }

    String_View input = {};
    input.count = fread(config_file_buffer, 1, CONFIG_FILE_CAPACITY, f);
    input.data = config_file_buffer;
    fclose(f);

    memset(config_values, 0, sizeof(Config_Value) * CONFIG_VAR_CAPACITY);
    return parse_config_text(input);
}

#define CONFIG_INT(x) (config_values[x].int_value)
#define CONFIG_FLOAT(x) (config_values[x].float_value)
#define CONFIG_COLOR(x) (config_values[x].color_value)
