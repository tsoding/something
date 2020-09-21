#ifndef SOMETHING_FMW_HPP_
#define SOMETHING_FMW_HPP_

// NOTE: FMW stands for File Modification Watcher

struct Fmw;

Fmw *fmw_init(const char *filepath);
void fmw_free(Fmw *fmw);
bool fmw_poll(Fmw *fmw);

#endif  // SOMETHING_FMW_HPP_
