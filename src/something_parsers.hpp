#ifndef SEPARATE_FILE_HPP_
#define SEPARATE_FILE_HPP_

template <typename F>
void parse_assets_conf(String_View input, F f)
{
    for (int line_number = 1; input.count > 0; ++line_number) {
        String_View line = input.chop_by_delim('\n').trim();

        if (line.count == 0) continue; // Skip empty lines
        if (*line.data == '#') continue; // Skip single line comments

        String_View asset_type = line.chop_by_delim('[').trim();
        String_View asset_id = line.chop_by_delim(']').trim();
        line.chop_by_delim('=');
        String_View asset_path = line.chop_by_delim('#').trim();

        f(line_number, asset_type, asset_id, asset_path);
    }
}

// TODO(#289): add vars.conf parser to something_parsers.hpp
// TODO(#290): add animat files parser to something_parsers.hpp

#endif  // SEPARATE_FILE_HPP_
