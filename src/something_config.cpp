// TODO: integrate config vars into the code
// TODO: hot reloading the vars

enum Config_Type
{
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT
};

enum Config_Var
{
    X = 0,
    Y,
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
    {"X"_sv, CONFIG_TYPE_FLOAT},
    {"Y"_sv, CONFIG_TYPE_INT  },
};

Config_Value config[CONFIG_VAR_CAPACITY] = {};

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

void parse_config_text(String_View input)
{
    memset(config, 0, sizeof(Config_Value) * CONFIG_VAR_CAPACITY);

    while (input.count > 0) {
        String_View line = input.chop_by_delim('\n').trim();

        if (line.count == 0) continue;
        if (*line.data == '#') continue;

        String_View name = line.chop_by_delim('=').trim();
        String_View value = line.trim();

        // TODO: better error reporting on parsing config vars

        auto var = string_view_as_config_var(name);
        if (var >= CONFIG_VAR_UNKNOWN) {
            println(stderr, "Unknown variable `", name, "`");
            abort();
        }

        switch (config_defs[var].type) {
        case CONFIG_TYPE_INT: {
            auto x = value.as_integer<int>();
            if (!x.has_value) {
                println(stderr, "`", name, "` is not a valid integer");
                abort();
            }
            config[var].int_value = x.unwrap;
        } break;

        case CONFIG_TYPE_FLOAT: {
            config[var].float_value = string_view_as_float(value);
        } break;
        }
    }
}

void parse_config_file(const char *file_path)
{
    parse_config_text(file_as_string_view(file_path));
}
