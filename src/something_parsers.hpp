#ifndef SEPARATE_FILE_HPP_
#define SEPARATE_FILE_HPP_

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

template <typename F>
Config_Parse_Result parse_vars_conf(String_View input, F f)
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

        auto result = f(line_number, name, type, value);
        if (result.is_error) {
            return result;
        }
    }

    return parse_success();
}

// TODO(#290): add animat files parser to something_parsers.hpp

#endif  // SEPARATE_FILE_HPP_
