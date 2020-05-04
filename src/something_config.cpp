enum Config_Type
{
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT
};

enum Config_Var
{
    PLAYER_SPEED = 0,
    GRAVITY,

    CONFIG_VAR_CAPACITY,
    CONFIG_VAR_UNKNOWN,
};

struct Config_Def
{
    String_View name;
    Config_Type type;
};

union Config_Value
{
    float float_value;
    int int_value;
};

Config_Def config_defs[CONFIG_VAR_CAPACITY] = {
    {"PLAYER_SPEED"_sv , CONFIG_TYPE_FLOAT},
    {"GRAVITY"_sv      , CONFIG_TYPE_FLOAT},
};

Config_Value config[CONFIG_VAR_CAPACITY] = {};

const size_t CONFIG_FILE_CAPACITY = 1 * 1024 * 1024;
char config_file_buffer[CONFIG_FILE_CAPACITY];

Config_Var string_view_as_config_var(String_View name)
{
    for (int var = 0; var < CONFIG_VAR_CAPACITY; ++var) {
        if (name == config_defs[var].name) {
            return (Config_Var) var;
        }
    }

    return CONFIG_VAR_UNKNOWN;
}

float string_view_as_float(String_View input)
{
    char buffer[300] = {};
    memcpy(buffer, input.data, min(sizeof(buffer) - 1, input.count));
    return strtof(buffer, NULL);
}

int string_view_as_int(String_View input)
{
    char buffer[300] = {};
    memcpy(buffer, input.data, min(sizeof(buffer) - 1, input.count));
    return atoi(buffer);
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

Config_Parse_Result parse_config_text(String_View input)
{
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
            return parse_failure("Unknown variable", line_number);
        }

        switch (config_defs[var].type) {
        case CONFIG_TYPE_INT: {
            auto x = value.as_integer<int>();
            if (!x.has_value) {
                return parse_failure("Value is not a valid integer", line_number);
            }
            config[var].int_value = x.unwrap;
        } break;

        case CONFIG_TYPE_FLOAT: {
            config[var].float_value = string_view_as_float(value);
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

    memset(config, 0, sizeof(Config_Value) * CONFIG_VAR_CAPACITY);
    return parse_config_text(input);
}
