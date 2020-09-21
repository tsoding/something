#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/event.h>

#include "something_fmw.hpp"

struct Fmw
{
    int fd;
    int kq;
};

Fmw *fmw_init(const char *filepath)
{
    Fmw *fmw = (Fmw*) malloc(sizeof(Fmw));

    fmw->kq = kqueue();
    if (fmw->kq == -1) {
        println(stderr, "kqueue() failed: ", strerror(errno));
        abort();
    }

    fmw->fd = open(filepath, O_RDONLY);
    if (fmw->fd == -1) {
        println(stderr, "open() failed: ", strerror(errno));
        abort();
    }
    return fmw;
}

void fmw_free(Fmw *fmw)
{
    close(fmw->kq);
    close(fmw->fd);
    free(fmw);
}

bool fmw_poll(Fmw *fmw)
{
    struct kevent event;
    struct kevent watch;

    // set the event parameters
    EV_SET(&watch, fmw->fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_ONESHOT,
        NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE, 0, 0);

    // setup a null timespec, to have a 0 timeout (ie. return immediately)
    struct timespec tm = { };

    // poll once.
    int num_events = kevent(fmw->kq, &watch, 1, &event, 1, &tm);

    if (num_events == -1) {
        println(stderr, "kevent() failed: ", strerror(errno));
        abort();
    }

    // check for writes (and deletes?? not sure how to handle deletes though)
    if (num_events > 0 && event.fflags & (NOTE_DELETE| NOTE_EXTEND | NOTE_WRITE)) {
        return true;
    } else {
        return false;
    }
}
