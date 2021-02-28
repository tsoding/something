#include "something_fmw.hpp"

const int fmw_watch_buffer_size = 4096;

struct Fmw {
    HANDLE watch_dir;
    HANDLE iocp;
    int watch_bytes_received;
    unsigned char *watch_buffer;

    OVERLAPPED overlapped;
};

Fmw *fmw_init(const char *filepath)
{
    Fmw *fmw = (Fmw*) malloc(sizeof(Fmw));
    *fmw = {};

    fmw->watch_buffer = (unsigned char *) malloc(fmw_watch_buffer_size);

    char file_dir[1024];

    size_t length = strlen(filepath);
    size_t last_slash = length - 1;

    for (size_t i = 0; i < length; ++i) {
        if (filepath[length-i] == '\\' || filepath[length-i] == '/') {
            last_slash = length-i;
            break;
        }
    }

    memcpy(file_dir, filepath, last_slash);
    file_dir[last_slash] = 0;

    fmw->watch_dir = CreateFileA(file_dir, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);

    ReadDirectoryChangesW(fmw->watch_dir, fmw->watch_buffer, fmw_watch_buffer_size, 1, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
                          (LPDWORD)&fmw->watch_bytes_received, &fmw->overlapped, 0);
    fmw->iocp = CreateIoCompletionPort(fmw->watch_dir, 0, 0, 0);

    return fmw;
}

void fmw_free(Fmw *fmw)
{
    if (fmw->watch_dir && fmw->watch_dir != INVALID_HANDLE_VALUE) {
        CloseHandle(fmw->watch_dir);
    }

    if (fmw->iocp && fmw->iocp != INVALID_HANDLE_VALUE) {
        CloseHandle(fmw->iocp);
    }

    if (fmw->watch_buffer) {
        free(fmw->watch_buffer);
    }

    free(fmw);
}

bool fmw_poll(Fmw *fmw)
{
    bool file_modified = false;

    OVERLAPPED *overlapped;
    DWORD overlapped_bytes;
    ULONG_PTR overlapped_key;

    if (GetQueuedCompletionStatus(fmw->iocp, &overlapped_bytes, &overlapped_key, &overlapped, 1)) {
        unsigned char *data = fmw->watch_buffer;

        /* fyi, there's a fancy way to restore Fmw struct pointer here.

            'overlapped' pointer returned by GetQueuedCompletionStatus()
            points to 'OVERLAPPED overlapped;' member of Fmw struct
            Ahere's a macro in winapi to get the base struct pointer from that:

                Fmw *fmw = CONTAINING_RECORD(overlapped, fmw, overlapped);

            this is useful when you have lots of different types of async tasks */

        int data_offset = 0;

        do {
            FILE_NOTIFY_INFORMATION *file_info = (FILE_NOTIFY_INFORMATION *)data;
            data_offset = file_info->NextEntryOffset;

            if (file_info->Action == FILE_ACTION_MODIFIED) {
                file_modified = true;
            }

            data += data_offset;
        } while (data_offset);

        ReadDirectoryChangesW(fmw->watch_dir, fmw->watch_buffer, fmw_watch_buffer_size, 1, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
                              (LPDWORD)&fmw->watch_bytes_received, &fmw->overlapped, 0);
    }

    return file_modified;
}
