#ifndef SOMETHING_DIRENT_HPP_
#define SOMETHING_DIRENT_HPP_

struct dirent
{
    char d_name[MAX_PATH+1];
};

typedef struct DIR DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
void closedir(DIR *dirp);

#endif  // SOMETHING_DIRENT_HPP_
