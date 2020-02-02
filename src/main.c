
#include <dokan/dokan.h>
#include <lfs.h>
#include <stdio.h>

#include "operations.h"
#include "lfs_bind.h"
#include "context.h"

int __cdecl wmain(ULONG argc, PWCHAR argv[])
{
    ULONG debug_options = 0;
    ULONG positional_arg = 0;
    LPCWSTR mount_point = NULL;
    LPCWSTR media = NULL;
    uint32_t unit_size = 512;
    uint32_t block_size = 8192;

    for (ULONG i = 1; i < argc; i++)
    {
        PWCHAR* arg = argv[i];

        if (wcscmp(arg, L"-d") == 0)
        {
            debug_options |= DOKAN_OPTION_DEBUG | DOKAN_OPTION_STDERR;
        }
        else if (wcscmp(arg, L"--block-size") == 0)
        {
            block_size = _wtoi(argv[++i]);
        }
        else if (wcscmp(arg, L"--uint-size") == 0)
        {
            unit_size = _wtoi(argv[++i]);
        }
        else
        {
            switch (positional_arg++)
            {
                case 0:
                {
                    // mount point
                    mount_point = (LPCWSTR)arg;
                    break;
                }
                case 1:
                {
                    // media
                    media = (LPCWSTR)arg;
                    break;
                }
            }
        }
    }

    if (mount_point == NULL || media == NULL)
    {
        fprintf(stderr, "Usage: <mount point> <media>\n");
        return 1;
    }

    char media_path[255];
    sprintf(media_path, "\\\\.\\%ls", media);

    HANDLE media_handle =
        CreateFile(media_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (media_handle == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Cannot open media file: %ld\n", GetLastError());
        return 2;
    }

    LARGE_INTEGER media_file_size;
    GetFileSizeEx(media_handle, &media_file_size);

    DISK_GEOMETRY disk_geometry;

    DWORD bytes_returned;
    if (DeviceIoControl(media_handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &disk_geometry, sizeof(disk_geometry), &bytes_returned, NULL) == 0)
    {
        fprintf(stderr, "Cannot manage media file\n");
        return 3;
    }

    if (disk_geometry.BytesPerSector != unit_size)
    {
        fprintf(stderr, "Media secor side does not match %ld: \n", disk_geometry.BytesPerSector);
        return 4;
    }

    GET_LENGTH_INFORMATION length_information;

    if (DeviceIoControl(media_handle, IOCTL_DISK_GET_LENGTH_INFO , NULL, 0, &length_information, sizeof(length_information), &bytes_returned, NULL) == 0)
    {
        fprintf(stderr, "Get media file size\n");
        return 5;
    }

    uint32_t block_count = length_information.Length.QuadPart / unit_size;

    lfs_t lfs;

    dokany_context.lfs = &lfs;
    dokany_context.media_handle = media_handle;
    dokany_context.unit_size = unit_size;

    struct lfs_config lfs_config = {
        // context
        &dokany_context,
        // read
        lfs_bind_read,
        // prog
        lfs_bind_prog,
        // erase
        lfs_bind_erase,
        // sync
        lfs_bind_sync,
        // read_size
        unit_size,
        // prog_size
        unit_size,
        // block_size
        block_size,
        // block_count
        block_count,
        // block_cycles
        512,
        // cache_size
        unit_size * 16,
        // lookahead_size
        128,
        // read_buffer
        NULL,
        // prog_buffer
        NULL,
        // lookahead_buffer
        NULL,
        // name_max
        0,
        // file_max
        0,
        // attr_max
        0
    };

    int lfs_mount_result = lfs_mount(&lfs, &lfs_config);
    if (lfs_mount_result != LFS_ERR_OK)
    {
        fprintf(stderr, "Failed to mount littlefs\n");
        return lfs_mount_result;
    }

    DOKAN_OPTIONS dokan_options = {
        // Version
        131,
        // ThreadCount
        1,
        // Options
        debug_options,
        // GlobalContext
        (ULONG64)&dokany_context,
        // MountPoint
        mount_point,
        // UNCName
        NULL,
        // Timeout
        100,
        // AllocationUnitSize
        unit_size,
        // SectorSize
        block_size
    };

    DOKAN_OPERATIONS dokan_operations = {
        LFS_ZwCreateFile,
        LFS_Cleanup,
        LFS_CloseFile,
        LFS_ReadFile,
        LFS_WriteFile,
        LFS_FlushFileBuffers,
        LFS_GetFileInformation,
        LFS_FindFiles,
        LFS_FindFilesWithPattern,
        LFS_SetFileAttributes,
        LFS_SetFileTime,
        LFS_DeleteFile,
        LFS_DeleteDirectory,
        LFS_MoveFile,
        LFS_SetEndOfFile,
        LFS_SetAllocationSize,
        LFS_LockFile,
        LFS_UnlockFile,
        LFS_GetDiskFreeSpace,
        LFS_GetVolumeInformation,
        LFS_Mounted,
        LFS_Unmounted,
        LFS_GetFileSecurity,
        LFS_SetFileSecurity,
        LFS_FindStreams
    };

    int status = DokanMain(&dokan_options, &dokan_operations);
    switch (status) {
        case DOKAN_SUCCESS:
            fprintf(stderr, "Success\n");
            break;
        case DOKAN_ERROR:
            fprintf(stderr, "Error\n");
            break;
        case DOKAN_DRIVE_LETTER_ERROR:
            fprintf(stderr, "Bad Drive letter\n");
            break;
        case DOKAN_DRIVER_INSTALL_ERROR:
            fprintf(stderr, "Can't install driver\n");
            break;
        case DOKAN_START_ERROR:
            fprintf(stderr, "Driver something wrong\n");
            break;
        case DOKAN_MOUNT_ERROR:
            fprintf(stderr, "Can't assign a drive letter\n");
            break;
        case DOKAN_MOUNT_POINT_ERROR:
            fprintf(stderr, "Mount point error\n");
            break;
        case DOKAN_VERSION_ERROR:
            fprintf(stderr, "Version error\n");
            break;
        default:
            fprintf(stderr, "Unknown error: %d\n", status);
            break;
    }

    lfs_unmount(&lfs);

	return status;
}