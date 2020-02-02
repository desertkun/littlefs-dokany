#ifndef LITTLEFS_DOKANY_CONTEXT_H
#define LITTLEFS_DOKANY_CONTEXT_H

#include <lfs.h>
#include <windows.h>

typedef struct
{
    lfs_t* lfs;
    HANDLE media_handle;
    uint32_t unit_size;
} littlefs_dokany_context_t;

extern littlefs_dokany_context_t dokany_context;

#endif //LITTLEFS_DOKANY_CONTEXT_H
