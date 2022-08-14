#define LOG_TAG "[ANC_TAC][File]"

#include "anc_file.h"

#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


#include "anc_log.h"
#include "anc_memory_wrapper.h"




ANC_FILE_HANDLE  AncFileOpen(const char *p_path, int mode) {
    int fd = -1;

    fd = open(p_path, mode, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        ANC_LOGE("%s: fail to open file %s, errno = %d", __func__, p_path, errno);
        return NULL;
    }

    return (ANC_FILE_HANDLE)((intptr_t)fd);
}

int32_t  AncFileRead(ANC_FILE_HANDLE fh, void *buffer, unsigned int length) {
    int fd = (int)((int *)fh);
    ssize_t size = read(fd , buffer , length);
    if (size != (ssize_t)length) {
        ANC_LOGE("%s: fail to read file, size = %d, real read size = %d", __func__, length, size);
    }
    return (int32_t)size;
}

/*avoid confuse write return meaning, return write buffer size */
int32_t  AncFileWrite(ANC_FILE_HANDLE fh, void *buffer, unsigned int length) {
    int fd = (int)((int *)fh);
    ssize_t size = write(fd , buffer , length);
    if (size != (ssize_t)length) {
        ANC_LOGE("%s: fail to write file, size = %d, real write size = %d", __func__, length, size);
    }
    return (int32_t)size;
}

ANC_RETURN_TYPE  AncFileClose(ANC_FILE_HANDLE fh) {
    int fd = (int)((int *)fh);
    int ret = close(fd);
    if (0 != ret) {
        ANC_LOGE("%s: fail to close file, errno = %d", __func__, ret);
        return ANC_FAIL_CLOSE_FILE;
    }
    return ANC_OK;
}

ANC_RETURN_TYPE AncFileSync(ANC_FILE_HANDLE fh) {
    int fd = (int)((int *)fh);
    int ret = fsync(fd);
    if (0 != ret) {
        ANC_LOGE("%s: fail to sync file, errno = %d", __func__, ret);
        return ANC_FAIL;
    }
    return ANC_OK;
}

ANC_RETURN_TYPE AncTestDir(const char *p_dir_path) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int ret = access(p_dir_path, F_OK);
    if (0 != ret) {
        ANC_LOGE("%s: fail to access file %s, errno = %d", __func__, p_dir_path, ret);
        ret_val = ANC_FAIL_NO_DIR;
    }

    return ret_val;
}

ANC_RETURN_TYPE AncFstat(ANC_FILE_HANDLE fh, int64_t *p_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int ret = 0;
    int fd = (int)((int *)fh);
    struct stat file_stat;

    AncMemset(&file_stat, 0, sizeof(file_stat));
    ret = fstat(fd, &file_stat);
    if (0 == ret) {
        *p_size = file_stat.st_size;
    } else {
        ANC_LOGE("%s: fail to fstat, errno = %d", __func__, ret);
        ret_val = ANC_FAIL_FSTAT_FILE;
    }

    return ret_val;
}

ANC_RETURN_TYPE AncMkDir(const char *p_dir_path, uint32_t mode) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (0 != mkdir(p_dir_path, (mode_t)mode)) {
        ANC_LOGE("%s: fail to mkdir %s, errno = %d, errnostr = %s", __func__, p_dir_path, errno, strerror(errno));
        ret_val = ANC_FAIL;
    }
    return ret_val;
}

ANC_RETURN_TYPE AncRemoveFile(const char *p_file_name) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int ret = remove(p_file_name);
    if (0 != ret) {
        ANC_LOGE("%s: fail to remove file %s, errno = %d", __func__, p_file_name, ret);
        ret_val = ANC_FAIL;
    }

    return ret_val;
}

int32_t  AncReadClusterFileDecrypt(const char* p_path, void *buffer, unsigned int length) {
    ANC_LOGW("%s: Doesn't Support Cluster File Operation" , __func__);
    ANC_UNUSED(p_path);
    ANC_UNUSED(buffer);
    ANC_UNUSED(length);
    return -1;
}

int32_t  AncWriteClusterFileEncrypt(const char* p_path, const void *buffer, unsigned int length) {
    ANC_LOGW("%s: Doesn't Support Cluster File Operation" , __func__);
    ANC_UNUSED(p_path);
    ANC_UNUSED(buffer);
    ANC_UNUSED(length);
    return -1;
}
ANC_RETURN_TYPE AncRemoveClusterFile(const char *p_file_name) {
    ANC_LOGW("%s: Doesn't Support Cluster File Operation" , __func__);
    ANC_UNUSED(p_file_name);
    return ANC_FAIL;
}

ANC_RETURN_TYPE AncGetClusterFileTotalSize(const char* p_file_name, uint32_t* p_size) {
    ANC_LOGW("%s: Doesn't Support Cluster File Operation" , __func__);
    ANC_UNUSED(p_file_name);
    ANC_UNUSED(p_size);
    return ANC_FAIL;
}
ANC_RETURN_TYPE AncRenameClusterFile(const char *p_oldfilename, const char *p_newfilename) {
    ANC_LOGW("%s: Doesn't Support Cluster File Operation" , __func__);
    ANC_UNUSED(p_oldfilename);
    ANC_UNUSED(p_newfilename);
    return ANC_FAIL;
}

ANC_RETURN_TYPE AncGetFileSize(const char* p_file_name, uint32_t* p_size) {
    if ((NULL == p_file_name) || (NULL == p_size)) {
        ANC_LOGE("%s input param NULL" , __func__);
        return ANC_FAIL;
    }
    ANC_FILE_HANDLE handler = AncFileOpen(p_file_name, ANC_O_RDONLY);
    if (NULL == handler) {
        *p_size = 0;
        return ANC_FAIL;
    }
    int64_t size = 0;
    AncFstat(handler, &size);
    AncFileClose(handler);

    *p_size = (size > 0) ? (uint32_t)size : 0;
    return ANC_OK;
}
