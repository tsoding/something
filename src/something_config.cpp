// TODO: integrate config vars into the code
// TODO: hot reloading the vars

const size_t CONFIG_VAR_CAPACITY = 256;
const size_t CONFIG_NAME_CAPACITY = 50;

union Config_Value
{
    float float_value;
    int int_value;

    template <typename T> T as();
};

template <>
float Config_Value::as()
{
    return float_value;
}

template <>
int Config_Value::as()
{
    return int_value;
}

String_View names[CONFIG_VAR_CAPACITY] = {};
Config_Value values[CONFIG_VAR_CAPACITY] = {};
size_t config_size = 0;

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
    config_size = 0;

    while (input.count > 0) {
        String_View line = input.chop_by_delim('\n').trim();

        if (line.count == 0) continue;
        if (*line.data == '#') continue;

        String_View name = line.chop_by_delim(':').trim();
        String_View type = line.chop_by_delim('=').trim();
        String_View value = line.trim();

        assert(config_size < CONFIG_VAR_CAPACITY);
        names[config_size] = name;
        if (type == "float"_sv) {
            values[config_size].float_value = string_view_as_float(value);
        } else if (type == "int"_sv) {
            values[config_size].int_value = string_view_as_int(value);
        } else {
            println(stderr, "Unexpected type `", type, "`");
            abort();
        }

        config_size++;
    }
}

void parse_config_file(const char *file_path)
{
    parse_config_text(file_as_string_view(file_path));
}

template <typename T>
T get_config_var(const char *name)
{
    // TODO: better asymptotic for get_config_var
    for (size_t i = 0; i < config_size; ++i) {
        if (names[i] == cstr_as_string_view(name)) {
            return values[i].as<T>();
        }
    }

    assert(0 && "Non-existing config variable");
    return T();
}
