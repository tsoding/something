#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <cctype>
#include <cstdint>

template <typename T>
constexpr T min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
constexpr T max(T x, T y)
{
    return x > y ? x : y;
}

// READ THIS FIRST ---> https://en.wikipedia.org/wiki/Single_Compilation_Unit
#include "./common_print.cpp"
#include "./common_string.cpp"
#include "./common_config.cpp"

int main()
{
    reload_config_file("./assets/config.vars");

    for (int var = 0; var < CONFIG_VAR_CAPACITY; ++var) {
        print(stdout, "#define ", config_defs[var].name, " ");
        switch (config_defs[var].type) {
        case CONFIG_TYPE_INT: {
            println(stdout, config[var].int_value);
        } break;

        case CONFIG_TYPE_FLOAT: {
            println(stdout, config[var].float_value, "f");
        } break;
        }
    }

    return 0;
}
