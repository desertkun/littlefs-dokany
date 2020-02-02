#include "operations.h"
#include "context.h"
#include <lfs.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    lfs_file_t* file;
    lfs_dir_t * dir;
    uint8_t delete_file;
    uint8_t delete_dir;
} lfs_file_or_dir;

void fix_path(LPCWSTR FileName, char* fixed_file_name)
{
    uint32_t file_name_length = lstrlenW(FileName);
    fixed_file_name[file_name_length] = '\0';

    for (uint32_t i = 0; i < file_name_length; i++)
    {
        if (FileName[i] == L'\\')
        {
            fixed_file_name[i] = '/';
        }
        else
        {
            fixed_file_name[i] = (char)FileName[i];
        }
    }
}

NTSTATUS LFS_ZwCreateFile (LPCWSTR FileName,
    PDOKAN_IO_SECURITY_CONTEXT SecurityContext,
    ACCESS_MASK DesiredAccess,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PDOKAN_FILE_INFO DokanFileInfo)
{

    enum lfs_open_flags open_flags = 0;
    char fixed_file_name[255];
    fix_path(FileName, fixed_file_name);

    uint32_t is_dir = 0;

    {
        struct lfs_info file_info;
        if (lfs_stat(dokany_context.lfs, fixed_file_name, &file_info) == LFS_ERR_OK)
        {
            is_dir = file_info.type & LFS_TYPE_DIR;
        }
        else
        {
            is_dir = (!(CreateOptions & FILE_NON_DIRECTORY_FILE)) && (CreateDisposition & FILE_CREATE);

            if (is_dir)
            {
                if (lfs_mkdir(dokany_context.lfs, fixed_file_name) != LFS_ERR_OK)
                {
                    return STATUS_UNSUCCESSFUL;
                }
            }
        }
    }

    lfs_file_or_dir* f_d = calloc(1, sizeof(lfs_file_or_dir));
    DokanFileInfo->Context = (ULONG64)f_d;

    if (is_dir)
    {
        f_d->dir = calloc(1, sizeof(lfs_dir_t));
        if (lfs_dir_open(dokany_context.lfs, f_d->dir, fixed_file_name) != LFS_ERR_OK)
        {
            DokanFileInfo->IsDirectory = TRUE;
            free(f_d->dir);
            free(f_d);
            return STATUS_UNSUCCESSFUL;
        }

        return STATUS_SUCCESS;
    }

    switch (CreateDisposition)
    {
        case FILE_SUPERSEDE:
        {
            open_flags = LFS_O_TRUNC;
            break;
        }
        case FILE_CREATE:
        {
            open_flags = LFS_O_CREAT | LFS_O_EXCL;
            break;
        }
        case FILE_OPEN:
        {
            open_flags = LFS_O_APPEND;
            break;
        }
        case FILE_OPEN_IF:
        {
            open_flags = LFS_O_CREAT | LFS_O_APPEND;
            break;
        }
    }

    if ((DesiredAccess & (GENERIC_READ | GENERIC_WRITE)) == (GENERIC_READ | GENERIC_WRITE))
    {
        open_flags |= LFS_O_RDWR;
    }
    else if ((DesiredAccess & GENERIC_READ) == GENERIC_READ)
    {
        open_flags |= LFS_O_RDONLY;
    }
    else if ((DesiredAccess & GENERIC_WRITE) == GENERIC_WRITE)
    {
        open_flags |= LFS_O_WRONLY;
    }
    else
    {
        open_flags |= LFS_O_RDWR;
    }

    f_d->file = calloc(1, sizeof(lfs_file_t));
    int open_err = lfs_file_open(dokany_context.lfs, f_d->file, fixed_file_name, open_flags);
    if (open_err != LFS_ERR_OK)
    {
        free(f_d->file);
        free(f_d);

        switch (open_err)
        {
            case LFS_ERR_NOENT:
            {
                return STATUS_NO_SUCH_FILE;
            }
            default:
            {
                return STATUS_UNSUCCESSFUL;
            }
        }

    }

    return STATUS_SUCCESS;
}


void LFS_Cleanup (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo)
{

}


void LFS_CloseFile (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    lfs_file_or_dir* f_d = (lfs_file_or_dir*)DokanFileInfo->Context;
    if (f_d->file)
    {
        lfs_file_close(dokany_context.lfs, f_d->file);
        free(f_d->file);

        if (DokanFileInfo->DeleteOnClose == TRUE)
        {
            char fixed_path[255];
            fix_path(FileName, fixed_path);
            lfs_remove(dokany_context.lfs, fixed_path);
        }
    }
    if (f_d->dir)
    {
        lfs_dir_close(dokany_context.lfs, f_d->dir);
        free(f_d->dir);


        if (DokanFileInfo->DeleteOnClose == TRUE)
        {
            char fixed_path[255];
            fix_path(FileName, fixed_path);
            lfs_remove(dokany_context.lfs, fixed_path);
        }
    }
    free(f_d);
}


NTSTATUS LFS_ReadFile (LPCWSTR FileName,
    LPVOID Buffer,
    DWORD BufferLength,
    LPDWORD ReadLength,
    LONGLONG Offset,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    lfs_file_or_dir* f_d = (lfs_file_or_dir*)DokanFileInfo->Context;

    lfs_file_seek(dokany_context.lfs, f_d->file, Offset, LFS_SEEK_SET);
    *ReadLength = lfs_file_read(dokany_context.lfs, f_d->file, Buffer, BufferLength);
    return STATUS_SUCCESS;
}


NTSTATUS LFS_WriteFile (LPCWSTR FileName,
    LPCVOID Buffer,
    DWORD NumberOfBytesToWrite,
    LPDWORD NumberOfBytesWritten,
    LONGLONG Offset,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    lfs_file_or_dir* f_d = (lfs_file_or_dir*)DokanFileInfo->Context;

    lfs_file_seek(dokany_context.lfs, f_d->file, Offset, LFS_SEEK_SET);
    *NumberOfBytesWritten = lfs_file_write(dokany_context.lfs, f_d->file, Buffer, NumberOfBytesToWrite);
    return STATUS_SUCCESS;
}


NTSTATUS LFS_FlushFileBuffers (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    lfs_file_or_dir* f_d = (lfs_file_or_dir*)DokanFileInfo->Context;
    lfs_file_sync(dokany_context.lfs, f_d->file);
    return STATUS_SUCCESS;
}


NTSTATUS LFS_GetFileInformation (LPCWSTR FileName,
    LPBY_HANDLE_FILE_INFORMATION Buffer,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    lfs_file_or_dir* f_d = (lfs_file_or_dir*)DokanFileInfo->Context;

    Buffer->dwFileAttributes = 0;

    if (f_d->dir)
    {
        Buffer->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    }

    if (f_d->file)
    {
        Buffer->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
        Buffer->nFileSizeHigh = 0;
        Buffer->nFileSizeLow = lfs_file_size(dokany_context.lfs, f_d->file);
    }

    return STATUS_SUCCESS;
}


NTSTATUS LFS_FindFiles (LPCWSTR FileName,
    PFillFindData FillFindData,
    PDOKAN_FILE_INFO DokanFileInfo)
{

    lfs_file_or_dir* f_d = (lfs_file_or_dir*)DokanFileInfo->Context;

    struct lfs_info info;
    lfs_dir_rewind(dokany_context.lfs, f_d->dir);
    while (lfs_dir_read(dokany_context.lfs, f_d->dir, &info))
    {
        WIN32_FIND_DATAW found;

        wsprintfW(found.cFileName, L"%S", info.name);

        if (info.type == LFS_TYPE_DIR)
        {
            found.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        }
        else
        {
            found.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
            found.nFileSizeLow = info.size;
            found.nFileSizeHigh = 0;
        }

        FillFindData(&found, DokanFileInfo);
    }

    return STATUS_SUCCESS;
}


NTSTATUS LFS_FindFilesWithPattern (LPCWSTR PathName,
    LPCWSTR SearchPattern,
    PFillFindData FillFindData,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS LFS_SetFileAttributes (LPCWSTR FileName,
    DWORD FileAttributes,
    PDOKAN_FILE_INFO DokanFileInfo)
{
 return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS LFS_SetFileTime (LPCWSTR FileName,
    CONST FILETIME *CreationTime,
    CONST FILETIME *LastAccessTime,
    CONST FILETIME *LastWriteTime,
    PDOKAN_FILE_INFO DokanFileInfo)
{
 return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS LFS_DeleteFile (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    lfs_file_or_dir* f_d = (lfs_file_or_dir*)DokanFileInfo->Context;
    f_d->delete_file = DokanFileInfo->DeleteOnClose;
    return STATUS_SUCCESS;
}


NTSTATUS LFS_DeleteDirectory (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    char fixed_path[255];
    fix_path(FileName, fixed_path);
    lfs_dir_t dir;
    if (lfs_dir_open(dokany_context.lfs, &dir, fixed_path) != LFS_ERR_OK)
    {
        return STATUS_OBJECT_PATH_NOT_FOUND;
    }

    struct lfs_info info;
    while (lfs_dir_read(dokany_context.lfs, &dir, &info))
    {
        if (strcmp(info.name, ".") == 0)
        {
            continue;
        }

        if (strcmp(info.name, "..") == 0)
        {
            continue;
        }

        lfs_dir_close(dokany_context.lfs, &dir);
        return STATUS_DIRECTORY_NOT_EMPTY;
    }

    lfs_dir_close(dokany_context.lfs, &dir);

    DokanFileInfo->DeleteOnClose = TRUE;

    return STATUS_SUCCESS;
}


NTSTATUS LFS_MoveFile (LPCWSTR FileName,
    LPCWSTR NewFileName,
    BOOL ReplaceIfExisting,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    char fixed_from[255], fixed_to[255];
    fix_path(FileName, fixed_from);
    fix_path(NewFileName, fixed_to);

    return lfs_rename(dokany_context.lfs, fixed_from, fixed_to) == LFS_ERR_OK ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}


NTSTATUS LFS_SetEndOfFile (LPCWSTR FileName,
    LONGLONG ByteOffset,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS LFS_SetAllocationSize (LPCWSTR FileName,
    LONGLONG AllocSize,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS LFS_LockFile (LPCWSTR FileName,
    LONGLONG ByteOffset,
    LONGLONG Length,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS LFS_UnlockFile (LPCWSTR FileName,
    LONGLONG ByteOffset,
    LONGLONG Length,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS LFS_GetDiskFreeSpace (PULONGLONG FreeBytesAvailable,
    PULONGLONG TotalNumberOfBytes,
    PULONGLONG TotalNumberOfFreeBytes,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    uint32_t total_size = dokany_context.lfs->cfg->block_size * dokany_context.lfs->cfg->block_count;
    uint32_t used_size = dokany_context.lfs->cfg->block_size * lfs_fs_size(dokany_context.lfs);
    uint32_t free_size = total_size - used_size;

    *TotalNumberOfBytes = (ULONGLONG)total_size;
    *TotalNumberOfFreeBytes =  (ULONGLONG)free_size;
    *FreeBytesAvailable = (ULONGLONG)free_size;

    return STATUS_SUCCESS;
}


NTSTATUS LFS_GetVolumeInformation (LPWSTR VolumeNameBuffer,
    DWORD VolumeNameSize,
    LPDWORD VolumeSerialNumber,
    LPDWORD MaximumComponentLength,
    LPDWORD FileSystemFlags,
    LPWSTR FileSystemNameBuffer,
    DWORD FileSystemNameSize,
    PDOKAN_FILE_INFO DokanFileInfo)
{
    wsprintfW(FileSystemNameBuffer, L"littlefs");
    *VolumeSerialNumber = 0;
    *MaximumComponentLength = dokany_context.lfs->file_max;

    return STATUS_SUCCESS;
}


NTSTATUS LFS_Mounted (PDOKAN_FILE_INFO DokanFileInfo)
{
    return DOKAN_SUCCESS;
}


NTSTATUS LFS_Unmounted (PDOKAN_FILE_INFO DokanFileInfo)
{
    return DOKAN_SUCCESS;
}


NTSTATUS LFS_GetFileSecurity (LPCWSTR FileName,
    PSECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG BufferLength,
    PULONG LengthNeeded,
    PDOKAN_FILE_INFO DokanFileInfo)
{
 return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS LFS_SetFileSecurity (LPCWSTR FileName,
    PSECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG BufferLength,
    PDOKAN_FILE_INFO DokanFileInfo)
{
 return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS LFS_FindStreams (LPCWSTR FileName,
    PFillFindStreamData FillFindStreamData,
    PDOKAN_FILE_INFO DokanFileInfo)
{
 return STATUS_NOT_IMPLEMENTED;
}
