#include "something_fmw.hpp"

struct Fmw {};

Fmw *fmw_init(const char *)
{
    return NULL;
}

void fmw_free(Fmw *)
{
}

bool fmw_poll(Fmw *)
{
    return false;
}
