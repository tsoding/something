#include "something_config.hpp"

Config config = {};


void Config::load_file(const char *file_path)
{
    auto content =
        unwrap_or_panic(
            read_file_as_string_view(
                file_path,
                &config_file_buffer),
            "Could not read file: ", file_path);

    for (size_t line_number = 1; content.count > 0; ++line_number) {
        auto line = content.chop_by_delim('\n');
        auto var_def = line.chop_by_delim('#').trim();

        if (var_def.count > 0) {
            auto name = var_def.chop_by_delim(':').trim();
            auto type = var_def.chop_by_delim('=').trim();
            auto value = var_def.trim();
            (void) name;
            (void) value;
            if (type == "float"_sv) {
                todo("Parsing float is not implemented");
            } else if (type == "int"_sv) {
                todo("Parsing int is not implemented");
            } else if (type == "color"_sv) {
                todo("Parsing color is not implemented");
            } else {
                panic(file_path, ":", line_number, ": Unknown type `", type, "`");
            }
        }
    }
}
