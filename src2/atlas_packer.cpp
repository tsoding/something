#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_image_write.h"

#include "./aids.hpp"

using namespace aids;

using RGBA32 = uint32_t;

struct Texture {
    int width;
    int height;
    RGBA32 *pixels;
    const char *file_path;
    String_View id;

    static Texture from_file(const char *file_path, String_View id)
    {
        Texture result = {};

        result.id = id;
        result.file_path = file_path;
        result.pixels =
            reinterpret_cast<RGBA32*>(
                stbi_load(
                    file_path,
                    &result.width,
                    &result.height,
                    NULL, 4));
        assert(result.pixels);

        return result;
    }
};

template <typename Ator = Mtor>
String_View id_of_file(const char *file_path, Ator *ator = &mtor)
{
    size_t len = strlen(file_path);
    char *result = ator->template alloc<char>(len);

    for (size_t i = 0; i < len; ++i) {
        if (isalnum(file_path[i])) {
            result[i] = toupper(file_path[i]);
        } else {
            result[i] = '_';
        }
    }

    return String_View {
        len,
        result
    };
}

bool is_path_sep(char x)
{
#ifndef _WIN32
    return x == '\\' || x == '/';
#else
    return x == '/';
#endif
}

String_View file_name_of_path(const char *begin)
{
    const char *dot = begin + strlen(begin) - 1;

    // try remove extension
    while (begin <= dot && *dot != '.' && !is_path_sep(*dot)) {
        dot -= 1;
    }

    if (dot < begin) {
        // no extension, no path separator, just return the whole thing then
        return cstr_as_string_view(begin);
    }

    if (is_path_sep(*dot)) {
        // no extension
        return cstr_as_string_view(dot + 1);
    }

    const char *sep = dot;
    while (begin <= sep && !is_path_sep(*sep)) {
        sep -= 1;
    }
    sep += 1;

    return String_View {static_cast<size_t>(dot - sep), sep};
}

void usage(FILE *stream, const char *program_name)
{
    println(stream, "Usage: ", program_name, " [-o <output-folder>] <atlas.conf>");
}

#ifdef TESTING
void test_file_name_of_path(void)
{
    struct Test_Case {
        const char *input;
        String_View expected_output;
    };

    Test_Case cases[] = {
        {"atlas", "atlas"_sv},
        {"atlas.conf", "atlas"_sv},
        {"foo/atlas.conf", "atlas"_sv},
        {"foo/atlas", "atlas"_sv},
        {"/", ""_sv},
        {"/.", ""_sv},
        {".", ""_sv},
        {"", ""_sv},
        {".conf", ""_sv},
        {"/.conf", ""_sv},
        {"/..conf", "."_sv},
        {"/..", "."_sv},
        {"..", "."_sv},
    };
    const size_t cases_size = sizeof(cases) / sizeof(cases[0]);

    for (size_t i = 0; i < cases_size; ++i) {
        const auto actual_output = file_name_of_path(cases[i].input);

        if (actual_output != cases[i].expected_output) {
            println(stderr, "TEST FAILED!");
            println(stderr, "  Expected: \"", cases[i].expected_output, '"');
            println(stderr, "  Actual:   \"", actual_output, '"');
            exit(1);
        }

        println(stdout, '"', cases[i].input, '"', " -> ", '"', actual_output, '"');
    }
}

int main(void)
{
    test_file_name_of_path();
    return 0;
}
#else
int main(int argc, char **argv)
{
    // TODO: atlas_packer should generate uv coords as well
    // TODO: customizable ids for textures in atlas.conf

    // Parse command line arguments
    Args args = {argc, argv};
    const char *program_name = args.shift();
    const char *output_folder = ".";
    const char *atlas_conf_path = NULL;

    while (!args.empty()) {
        const auto flag = args.shift();

        if (strcmp(flag, "-o") == 0) {
            if (args.empty()) {
                usage(stderr, program_name);
                panic("ERROR: no value is provided for `", flag, "` flag");
            }

            output_folder = args.shift();
        } else {
            atlas_conf_path = flag;
            // TODO: support for several atlas configs?
            break;
        }
    }

    if (atlas_conf_path == NULL) {
        usage(stderr, program_name);
        panic("ERROR: no atlas config file is provided");
    }

    const String_View atlas_conf_name = file_name_of_path(atlas_conf_path);

    // Load textures
    String_View atlas_conf_content =
        unwrap_or_panic(
            read_file_as_string_view(atlas_conf_path),
            "Could not read file `", atlas_conf_path, "`: ",
            strerror(errno));

    Dynamic_Array<Texture> textures = {};

    while (atlas_conf_content.count > 0) {
        String_View line = atlas_conf_content.chop_by_delim('\n').trim();
        String_View key_value = line.chop_by_delim('#').trim();

        if (key_value.count > 0) {
            String_View key = key_value.chop_by_delim('=').trim();
            String_View value = key_value.trim();

            const char *file_path = value.as_cstr();
            assert(file_path != nullptr);
            textures.push(Texture::from_file(file_path, key));
        }
    }

    // Build the Atlas

    int atlas_width = 0;
    int atlas_height = 0;

    for (size_t i = 0; i < textures.size; ++i) {
        atlas_width = max(atlas_width, textures.data[i].width);
        atlas_height += textures.data[i].height;
    }

    RGBA32 *atlas_pixels = static_cast<RGBA32*>(malloc(sizeof(RGBA32) * atlas_width * atlas_height));
    memset(atlas_pixels, 0, sizeof(RGBA32) * atlas_width * atlas_height);

    Dynamic_Array<char> path_buffer = {};

    path_buffer.concat(output_folder, strlen(output_folder));
    path_buffer.push('/');
    path_buffer.concat(atlas_conf_name.data, atlas_conf_name.count);
    path_buffer.concat(".hpp", 4);
    path_buffer.push('\0');
    println(stdout, "INFO: preparing ", path_buffer.data);

    FILE *atlas_hpp_file = fopen(path_buffer.data, "w");
    if (atlas_hpp_file == nullptr) {
        panic("ERROR: could not open file ", path_buffer.data, ": ",
              strerror(errno));
    }

    println(atlas_hpp_file, "#ifndef ATLAS_HPP_");
    println(atlas_hpp_file, "#define ATLAS_HPP_");
    println(atlas_hpp_file);

    int atlas_row = 0;
    for (size_t i = 0; i < textures.size; ++i) {
        const Texture *texture = &textures.data[i];

        println(atlas_hpp_file, "const int ", texture->id, "_X = ", 0, ";");
        println(atlas_hpp_file, "const int ", texture->id, "_Y = ", atlas_row, ";");
        println(atlas_hpp_file, "const int ", texture->id, "_WIDTH = ", texture->width, ";");
        println(atlas_hpp_file, "const int ", texture->id, "_HEIGHT = ", texture->height, ";");
        println(atlas_hpp_file);

        for (int row = 0; row < textures.data[i].height; ++row) {
            memcpy(&atlas_pixels[atlas_row * atlas_width],
                   &texture->pixels[row * texture->width],
                   sizeof(RGBA32) * texture->width);
            atlas_row += 1;
        }
    }

    println(atlas_hpp_file, "#endif  // ATLAS_HPP_");

    path_buffer.size = 0;
    path_buffer.concat(output_folder, strlen(output_folder));
    path_buffer.push('/');
    path_buffer.concat(atlas_conf_name.data, atlas_conf_name.count);
    path_buffer.concat(".png", 4);
    path_buffer.push('\0');
    println(stdout, "INFO: preparing ", path_buffer.data);

    if (!stbi_write_png(path_buffer.data,
                        atlas_width,
                        atlas_height,
                        4,
                        atlas_pixels,
                        sizeof(RGBA32) * atlas_width)) {
        panic("Could not save file `", path_buffer.data, "`");
    }

    return 0;
}
#endif // TESTING
