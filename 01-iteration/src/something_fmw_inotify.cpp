#include <sys/inotify.h>
#include <unistd.h>

#include "something_fmw.hpp"

struct Fmw
{
    int fd;
    int wd;
};

Fmw *fmw_init(const char *filepath)
{
    Fmw *fmw = (Fmw*) malloc(sizeof(Fmw));

    fmw->fd = inotify_init1(IN_NONBLOCK);
    if (fmw->fd == -1) {
        println(stderr, "inotify_init() failed: ", strerror(errno));
        abort();
    }

    fmw->wd = inotify_add_watch(fmw->fd, filepath, IN_MODIFY);
    if (fmw->wd == -1) {
        println(stderr, "inotify_add_watch() failed: ", strerror(errno));
        abort();
    }

    return fmw;
}

void fmw_free(Fmw *fmw)
{
    close(fmw->fd);
    free(fmw);
}

bool fmw_poll(Fmw *fmw)
{
    inotify_event event = {};
    char buffer[1024];

    auto n = read(fmw->fd, &event, sizeof(event));
    if (n != -1 || errno != EAGAIN) {
        assert(n == sizeof(event));

        // Skiping the name
        size_t m = event.len;
        while (m > 0) {
            m -= read(fmw->fd, buffer, min(sizeof(buffer), m));
        }

        return true;
    }

    return false;
}
