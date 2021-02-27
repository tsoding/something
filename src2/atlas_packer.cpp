#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#include "./aids.hpp"

using namespace aids;

struct Texture {
    using RGBA32 = uint32_t;

    int width;
    int height;
    RGBA32 *pixels;
    const char *file_path;

    static Texture from_file(const char *file_path)
    {
        Texture result = {};

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

int main(int argc, char **argv)
{
    const char *const atlas_path = "./atlas.conf";
    String_View atlas_content =
        unwrap_or_panic(
            read_file_as_string_view(atlas_path),
            "Could not read file `", atlas_path, "`: ",
            strerror(errno));

    Dynamic_Array<Texture> textures = {};

    while (atlas_content.count > 0) {
        String_View line = atlas_content.chop_by_delim('\n').trim();
        if (line.count > 0) {
            const char *file_path = line.as_cstr();
            assert(file_path != nullptr);
            textures.push(Texture::from_file(file_path));
        }
    }

    println(stdout, "Loaded Textures: ", textures.size);
    for (size_t i = 0; i < textures.size; ++i) {
        println(stdout, Pad {2, ' '}, "File Path: ", textures.data[i].file_path);
        println(stdout, Pad {2, ' '}, "Width: ", textures.data[i].width);
        println(stdout, Pad {2, ' '}, "Height: ", textures.data[i].height);
        println(stdout, Pad {30, '-'});
    }

    return 0;
}
