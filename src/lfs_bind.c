#include "lfs_bind.h"
#include "context.h"
#include <stdio.h>
#include <windows.h>

int lfs_bind_read(const struct lfs_config *c, lfs_block_t block,
    lfs_off_t off, void *buffer, lfs_size_t size)
{
    littlefs_dokany_context_t* context = (littlefs_dokany_context_t*)c->context;

    uint64_t ptr = block * c->block_size + off;
    long ptr_high = ptr >> 32;
    uint32_t ptr_low = ptr & 0xFFFFFFFF;

    OVERLAPPED ol;
    memset(&ol, 0, sizeof(ol));
    ol.Offset = ptr_low;
    ol.OffsetHigh = ptr_high;

    DWORD bytes_read;
    if (ReadFile(context->media_handle, buffer, size, &bytes_read, &ol) == TRUE)
    {
        return LFS_ERR_OK;
    }

    return LFS_ERR_IO;
}

int lfs_bind_prog(const struct lfs_config *c, lfs_block_t block,
    lfs_off_t off, const void *buffer, lfs_size_t size)
{
    littlefs_dokany_context_t* context = (littlefs_dokany_context_t*)c->context;

    uint64_t ptr = block * c->block_size + off;
    long ptr_high = ptr >> 32;
    uint32_t ptr_low = ptr & 0xFFFFFFFF;

    OVERLAPPED ol;
    memset(&ol, 0, sizeof(ol));
    ol.Offset = ptr_low;
    ol.OffsetHigh = ptr_high;

    DWORD bytes_written;
    if (WriteFile(context->media_handle, buffer, size, &bytes_written, &ol) == TRUE)
    {
        return LFS_ERR_OK;
    }

    return LFS_ERR_IO;
}

static uint8_t empty_block[8192];

int lfs_bind_erase(const struct lfs_config *c, lfs_block_t block)
{
    memset(empty_block, 0xFF, c->block_size);
    littlefs_dokany_context_t* context = (littlefs_dokany_context_t*)c->context;
    DWORD bytes_written;

    uint64_t ptr = block * c->block_size;
    long ptr_high = ptr >> 32;
    uint32_t ptr_low = ptr & 0xFFFFFFFF;

    OVERLAPPED ol;
    memset(&ol, 0, sizeof(ol));
    ol.Offset = ptr_low;
    ol.OffsetHigh = ptr_high;

    if (WriteFile(context->media_handle, empty_block, c->block_size, &bytes_written, &ol) == TRUE)
    {
        return LFS_ERR_OK;
    }


    return LFS_ERR_IO;
}

int lfs_bind_sync(const struct lfs_config *c)
{
    littlefs_dokany_context_t* context = (littlefs_dokany_context_t*)c->context;
    FlushFileBuffers(context->media_handle);
    return LFS_ERR_OK;
}