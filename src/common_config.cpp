enum Config_Type
{
    CONFIG_TYPE_UNKNOWN = 0,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_COLOR,
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_SDL_BLENDFACTOR,
    CONFIG_TYPE_SDL_BLENDOPERATION,
};

#include "./config_types.hpp"

Config_Type config_type_by_name(String_View type_name)
{
    if (type_name == "float"_sv) return CONFIG_TYPE_FLOAT;
    if (type_name == "int"_sv) return CONFIG_TYPE_INT;
    if (type_name == "color"_sv) return CONFIG_TYPE_COLOR;
    if (type_name == "string"_sv) return CONFIG_TYPE_STRING;
    if (type_name == "SDL_BlendFactor"_sv) return CONFIG_TYPE_SDL_BLENDFACTOR;
    if (type_name == "SDL_BlendOperation"_sv) return CONFIG_TYPE_SDL_BLENDOPERATION;
    return CONFIG_TYPE_UNKNOWN;
}

String_View config_name_by_type(Config_Type type)
{
    switch (type) {
    case CONFIG_TYPE_UNKNOWN: return "<unknown>"_sv;
    case CONFIG_TYPE_INT: return "int"_sv;
    case CONFIG_TYPE_FLOAT: return "float"_sv;
    case CONFIG_TYPE_COLOR: return "color"_sv;
    case CONFIG_TYPE_STRING: return "string"_sv;
    case CONFIG_TYPE_SDL_BLENDFACTOR: return "SDL_BlendFactor"_sv;
    case CONFIG_TYPE_SDL_BLENDOPERATION: return "SDL_BlendOperation"_sv;
    }

    return "<unknown>"_sv;
}

union Config_Value
{
    float float_value;
    int int_value;
    SDL_Color color_value;
    String_View string_value;
    SDL_BlendFactor SDL_BlendFactor_value;
    SDL_BlendOperation SDL_BlendOperation_value;
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

Maybe<String_View> string_view_of_string_literal(String_View input)
{
    if (input.count < 2) return {};

    if (*input.data != '"') return {};
    input.chop(1);

    if (input.data[input.count - 1] != '"') return {};
    input.chop_back(1);

    return {true, input};
}

String_View string_view_of_sdl_blend_factor(SDL_BlendFactor factor)
{
    switch (factor) {
    case SDL_BLENDFACTOR_ZERO:
        return "SDL_BLENDFACTOR_ZERO"_sv;
    case SDL_BLENDFACTOR_ONE:
        return "SDL_BLENDFACTOR_ONE"_sv;
    case SDL_BLENDFACTOR_SRC_COLOR:
        return "SDL_BLENDFACTOR_SRC_COLOR"_sv;
    case SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR:
        return "SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR"_sv;
    case SDL_BLENDFACTOR_SRC_ALPHA:
        return "SDL_BLENDFACTOR_SRC_ALPHA"_sv;
    case SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA:
        return "SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA"_sv;
    case SDL_BLENDFACTOR_DST_COLOR:
        return "SDL_BLENDFACTOR_DST_COLOR"_sv;
    case SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR:
        return "SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR"_sv;
    case SDL_BLENDFACTOR_DST_ALPHA:
        return "SDL_BLENDFACTOR_DST_ALPHA"_sv;
    case SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA:
        return "SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA"_sv;
    default:
        return ""_sv;
    }
}

Maybe<SDL_BlendFactor> sdl_blend_factor_of_string_view(String_View input)
{
    if (input == "SDL_BLENDFACTOR_ZERO"_sv) {
        return {true, SDL_BLENDFACTOR_ZERO};
    }

    if (input == "SDL_BLENDFACTOR_ONE"_sv) {
        return {true, SDL_BLENDFACTOR_ONE};
    }

    if (input == "SDL_BLENDFACTOR_SRC_COLOR"_sv) {
        return {true, SDL_BLENDFACTOR_SRC_COLOR};
    }

    if (input == "SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR"_sv) {
        return {true, SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR};
    }

    if (input == "SDL_BLENDFACTOR_SRC_ALPHA"_sv) {
        return {true, SDL_BLENDFACTOR_SRC_ALPHA};
    }

    if (input == "SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA"_sv) {
        return {true, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA};
    }

    if (input == "SDL_BLENDFACTOR_DST_COLOR"_sv) {
        return {true, SDL_BLENDFACTOR_DST_COLOR};
    }

    if (input == "SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR"_sv) {
        return {true, SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR};
    }

    if (input == "SDL_BLENDFACTOR_DST_ALPHA"_sv) {
        return {true, SDL_BLENDFACTOR_DST_ALPHA};
    }

    if (input == "SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA"_sv) {
        return {true, SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA};
    }

    return {};
}

Maybe<SDL_BlendOperation> sdl_blend_operation_of_string_view(String_View input)
{
    if (input == "SDL_BLENDOPERATION_ADD"_sv) {
        return {true, SDL_BLENDOPERATION_ADD};
    }

    if (input == "SDL_BLENDOPERATION_SUBTRACT"_sv) {
        return {true, SDL_BLENDOPERATION_SUBTRACT};
    }

    if (input == "SDL_BLENDOPERATION_REV_SUBTRACT"_sv) {
        return {true, SDL_BLENDOPERATION_REV_SUBTRACT};
    }

    if (input == "SDL_BLENDOPERATION_MINIMUM"_sv) {
        return {true, SDL_BLENDOPERATION_MINIMUM};
    }

    if (input == "SDL_BLENDOPERATION_MAXIMUM"_sv) {
        return {true, SDL_BLENDOPERATION_MAXIMUM};
    }

    return {};
}

String_View string_view_of_sdl_blend_operation(SDL_BlendOperation blend_operation)
{
    switch (blend_operation) {
    case SDL_BLENDOPERATION_ADD:
        return "SDL_BLENDOPERATION_ADD"_sv;
    case SDL_BLENDOPERATION_SUBTRACT:
        return "SDL_BLENDOPERATION_SUBTRACT"_sv;
    case SDL_BLENDOPERATION_REV_SUBTRACT:
        return "SDL_BLENDOPERATION_REV_SUBTRACT"_sv;
    case SDL_BLENDOPERATION_MINIMUM:
        return "SDL_BLENDOPERATION_MINIMUM"_sv;
    case SDL_BLENDOPERATION_MAXIMUM:
        return "SDL_BLENDOPERATION_MAXIMUM"_sv;
    default:
        return ""_sv;
    }
}

Config_Parse_Result parse_config_text(String_View input)
{
    for (size_t line_number = 1; input.count > 0; ++line_number) {
        String_View line = input.chop_by_delim('\n').trim();

        if (line.count == 0) continue;   // Empty line
        if (*line.data == '#') continue; // Comment

        // NOTE: Format of Line:
        // VAR_NAME : TYPE = VALUE # COMMENT
        String_View name  = line.chop_by_delim(':').trim();
        String_View type  = line.chop_by_delim('=').trim();
        String_View value = line.chop_by_delim('#').trim();

        auto index = config_index_by_name(name);
        if (index < 0) {
            snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                     "Unknown variable `%.*s`",
                     (int) name.count, name.data);
            return parse_failure(config_error_buffer, line_number);
        }

        auto actual_config_type = config_type_by_name(type);
        auto expected_config_type = config_types[index];

        if (actual_config_type != expected_config_type) {
            auto expected_type = config_name_by_type(expected_config_type);
            snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                     "Expected type of `%.*s` was `%.*s`, but found `%.*s`.",
                     (int) name.count, name.data,
                     (int) expected_type.count, expected_type.data,
                     (int) type.count, type.data);
            return parse_failure(config_error_buffer, line_number);
        }

        switch (actual_config_type) {
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

        case CONFIG_TYPE_STRING: {
            auto x = string_view_of_string_literal(value);
            if (!x.has_value) {
                snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                         "`%.*s` is not a string (variable `%.*s`)",
                         (int) value.count, value.data,
                         (int) name.count, name.data);
                return parse_failure(config_error_buffer, line_number);
            }
            config_values[index].string_value = x.unwrap;
        } break;

        case CONFIG_TYPE_SDL_BLENDFACTOR: {
            auto x = sdl_blend_factor_of_string_view(value);
            if (!x.has_value) {
                snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                         "`%.*s` is not an SDL_BlendFactor (variable `%.*s`)",
                         (int) value.count, value.data,
                         (int) name.count, name.data);
                return parse_failure(config_error_buffer, line_number);
            }
            config_values[index].SDL_BlendFactor_value = x.unwrap;
        } break;

        case CONFIG_TYPE_SDL_BLENDOPERATION: {
            auto x = sdl_blend_operation_of_string_view(value);
            if (!x.has_value) {
                snprintf(config_error_buffer, CONFIG_ERROR_CAPACITY,
                         "`%.*s` is not an SDL_BlendOperation (variable `%.*s`)",
                         (int) value.count, value.data,
                         (int) name.count, name.data);
                return parse_failure(config_error_buffer, line_number);
            }
            config_values[index].SDL_BlendOperation_value = x.unwrap;
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
