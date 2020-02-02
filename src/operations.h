#ifndef LITTLEFS_DOKANY_OPERATIONS_H
#define LITTLEFS_DOKANY_OPERATIONS_H

#include <dokan/dokan.h>
#include <dokan/fileinfo.h>


extern NTSTATUS LFS_ZwCreateFile (LPCWSTR FileName,
    PDOKAN_IO_SECURITY_CONTEXT SecurityContext,
    ACCESS_MASK DesiredAccess,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PDOKAN_FILE_INFO DokanFileInfo);

extern void LFS_Cleanup (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo);

extern void LFS_CloseFile (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_ReadFile (LPCWSTR FileName,
    LPVOID Buffer,
    DWORD BufferLength,
    LPDWORD ReadLength,
    LONGLONG Offset,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_WriteFile (LPCWSTR FileName,
    LPCVOID Buffer,
    DWORD NumberOfBytesToWrite,
    LPDWORD NumberOfBytesWritten,
    LONGLONG Offset,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_FlushFileBuffers (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_GetFileInformation (LPCWSTR FileName,
    LPBY_HANDLE_FILE_INFORMATION Buffer,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_FindFiles (LPCWSTR FileName,
    PFillFindData FillFindData,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_FindFilesWithPattern (LPCWSTR PathName,
    LPCWSTR SearchPattern,
    PFillFindData FillFindData,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_SetFileAttributes (LPCWSTR FileName,
    DWORD FileAttributes,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_SetFileTime (LPCWSTR FileName,
    CONST FILETIME *CreationTime,
    CONST FILETIME *LastAccessTime,
    CONST FILETIME *LastWriteTime,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_DeleteFile (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_DeleteDirectory (LPCWSTR FileName,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_MoveFile (LPCWSTR FileName,
    LPCWSTR NewFileName,
    BOOL ReplaceIfExisting,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_SetEndOfFile (LPCWSTR FileName,
    LONGLONG ByteOffset,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_SetAllocationSize (LPCWSTR FileName,
    LONGLONG AllocSize,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_LockFile (LPCWSTR FileName,
    LONGLONG ByteOffset,
    LONGLONG Length,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_UnlockFile (LPCWSTR FileName,
    LONGLONG ByteOffset,
    LONGLONG Length,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_GetDiskFreeSpace (PULONGLONG FreeBytesAvailable,
    PULONGLONG TotalNumberOfBytes,
    PULONGLONG TotalNumberOfFreeBytes,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_GetVolumeInformation (LPWSTR VolumeNameBuffer,
    DWORD VolumeNameSize,
    LPDWORD VolumeSerialNumber,
    LPDWORD MaximumComponentLength,
    LPDWORD FileSystemFlags,
    LPWSTR FileSystemNameBuffer,
    DWORD FileSystemNameSize,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_Mounted (PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_Unmounted (PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_GetFileSecurity (LPCWSTR FileName,
    PSECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG BufferLength,
    PULONG LengthNeeded,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_SetFileSecurity (LPCWSTR FileName,
    PSECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG BufferLength,
    PDOKAN_FILE_INFO DokanFileInfo);

extern NTSTATUS LFS_FindStreams (LPCWSTR FileName,
    PFillFindStreamData FillFindStreamData,
    PDOKAN_FILE_INFO DokanFileInfo);

#endif //LITTLEFS_DOKANY_OPERATIONS_H
