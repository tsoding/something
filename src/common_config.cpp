enum Config_Type
{
    CONFIG_TYPE_UNKNOWN = 0,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_COLOR
};

#include "./config_types.hpp"

Config_Type config_type_by_name(String_View type_name)
{
    if (type_name == "float"_sv) return CONFIG_TYPE_FLOAT;
    if (type_name == "int"_sv) return CONFIG_TYPE_INT;
    if (type_name == "color"_sv) return CONFIG_TYPE_COLOR;
    return CONFIG_TYPE_UNKNOWN;
}

union Config_Value
{
    float float_value;
    int int_value;
    SDL_Color color_value;
};

Config_Value config_values[CONFIG_VAR_CAPACITY] = {};

const size_t CONFIG_FILE_CAPACITY = 1 * 1024 * 1024;
char config_file_buffer[CONFIG_FILE_CAPACITY];

const size_t CONFIG_ERROR_CAPACITY = 1024;
char config_error_buffer[CONFIG_ERROR_CAPACITY];

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
    if (input.count != 8) return {};

    SDL_Color result = {};
    unwrap_into(result.r, input.subview(0, 2).from_hex<Uint8>());
    unwrap_into(result.g, input.subview(2, 2).from_hex<Uint8>());
    unwrap_into(result.b, input.subview(4, 2).from_hex<Uint8>());
    unwrap_into(result.a, input.subview(6, 2).from_hex<Uint8>());

    return {true, result};
}

Config_Parse_Result parse_config_text(String_View input)
{
    for (size_t line_number = 1; input.count > 0; ++line_number) {
        String_View line = input.chop_by_delim('\n').trim();

        if (line.count == 0) continue;   // Empty line
        if (*line.data == '#') continue; // Comment

        // NOTE: Format of Line:
        // VAR_NAME : TYPE = VALUE # COMMENT
        String_View name = line.chop_by_delim(':').trim();
        String_View type = line.chop_by_delim('=').trim();
        String_View value = line.chop_by_delim('#').trim();

        auto index = config_index_by_name(name);
        if (index < 0) {
            snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                     "Unknown variable `%.*s`",
                     (int) name.count, name.data);
            return parse_failure(config_error_buffer, line_number);
        }

        switch (config_type_by_name(type)) {
        case CONFIG_TYPE_COLOR: {
            auto x = string_view_as_color(value);
            if (!x.has_value) {
                snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                         "`%.*s` is not a color (variable `%.*s`)",
                         (int) value.count, value.data,
                         (int) name.count, name.data);
                return parse_failure(config_error_buffer, line_number);
            }
            config_values[index].color_value = x.unwrap;
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
            config_values[index].int_value = x.unwrap;
        } break;

        case CONFIG_TYPE_FLOAT: {
            auto x = value.as_float();
            if (!x.has_value) {
                snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                         "`%.*s` is not a float (variable `%.*s`)",
                         (int) value.count, value.data,
                         (int) name.count, name.data);
                return parse_failure(config_error_buffer, line_number);
            }
            config_values[index].float_value = x.unwrap;
        } break;

        case CONFIG_TYPE_UNKNOWN: {
            println(stderr, "[ERROR] Type ", type, " does not exist. ");
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
