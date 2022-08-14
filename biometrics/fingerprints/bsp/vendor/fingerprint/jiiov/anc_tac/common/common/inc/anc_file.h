#ifndef __ANC_FILE_H__
#define __ANC_FILE_H__

#include "anc_type.h"
#include "anc_error.h"

#define ANC_O_ACCMODE 00000003
#define ANC_O_RDONLY  00000000
#define ANC_O_WRONLY  00000001
#define ANC_O_RDWR    00000002
#define ANC_O_CREAT   00000100
#define ANC_O_TRUNC   00001000
#define ANC_O_APPEND  00002000

#define ANC_FILE_HANDLE void*

ANC_FILE_HANDLE AncFileOpen(const char* path, int    mode);
int32_t  AncFileRead(ANC_FILE_HANDLE fh, void *buffer, uint32_t length);
/*avoid confuse write return meaning, return write buffer size */
int32_t  AncFileWrite(ANC_FILE_HANDLE fh, void *buffer, uint32_t length);
ANC_RETURN_TYPE  AncFileClose(ANC_FILE_HANDLE fh);
ANC_RETURN_TYPE AncFileSync(ANC_FILE_HANDLE fh);
ANC_RETURN_TYPE AncFlush(ANC_FILE_HANDLE fh);
ANC_RETURN_TYPE AncTestDir(const char *p_dir_path);
ANC_RETURN_TYPE AncFstat(ANC_FILE_HANDLE fh, int64_t* p_size);
ANC_RETURN_TYPE AncMkDir(const char* path, uint32_t mode);
ANC_RETURN_TYPE AncCopyFile(const char *p_src_path,const char *p_dst_folder_path,const char* p_dst_file_name);
ANC_RETURN_TYPE AncGetFileSize(const char* p_file_name, uint32_t* p_size);
ANC_RETURN_TYPE AncRemoveFile(const char* p_file_name);
ANC_RETURN_TYPE AncRename(const char *p_oldfilename, const char *p_newfilename);

ANC_RETURN_TYPE AncGetClusterFileTotalSize(const char* p_file_name, uint32_t* p_size);
ANC_RETURN_TYPE AncRenameClusterFile(const char *p_oldfilename, const char *p_newfilename);
int32_t  AncReadClusterFileDecrypt(const char* path, void *buffer, uint32_t length);
int32_t  AncWriteClusterFileEncrypt(const char* path, const void *buffer, uint32_t length);
ANC_RETURN_TYPE AncRemoveClusterFile(const char* p_file_name);

#endif
