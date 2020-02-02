
#ifndef LITTLEFS_DOKANY_LFS_BIND_H
#define LITTLEFS_DOKANY_LFS_BIND_H

#include <lfs.h>

extern int lfs_bind_read(const struct lfs_config *c, lfs_block_t block,
    lfs_off_t off, void *buffer, lfs_size_t size);

extern int lfs_bind_prog(const struct lfs_config *c, lfs_block_t block,
    lfs_off_t off, const void *buffer, lfs_size_t size);

extern int lfs_bind_erase(const struct lfs_config *c, lfs_block_t block);

extern int lfs_bind_sync(const struct lfs_config *c);

#endif //LITTLEFS_DOKANY_LFS_BIND_H
