/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _DUMPENCODER_H_
#define _DUMPENCODER_H_

#include <utils/Vector.h>
#include "DumpTypeDefine.h"
#include "gf_base_types.h"
#include "Mutex.h"

using ::android::Vector;

#ifndef ENCRYPT_VERSION
#define ENCRYPT_VERSION          (1001)
#endif  // ENCRYPT_VERSION

namespace goodix
{
    struct EncryptDataHeader
    {
    public:
        uint32_t encrypt_info;  // encryptor version
        uint32_t filecount;
        uint32_t total_data_len;
    };

    typedef enum {
        OP_TYPE_DEFAULT,
        OP_TYPE_ENROLL,
        OP_TYPE_AUTH,
        OP_TYPE_CALI,
        OP_TYPE_PRODUCT,
    } GF_DUMP_OP_TYPE;

    typedef enum {
        CATEGORY_TYPE_BMP,
        CATEGORY_TYPE_RAW,
        CATEGORY_TYPE_CALI,
        CATEGORY_TYPE_TEST,
    } GF_DUMP_CATEGORY_TYPE;
    class DumpEncoder
    {
    public:
        DumpEncoder();
        ~DumpEncoder();
        gf_error_t init(bool encrypt, const char* dataRoot);
        gf_error_t dumpBegin(bool appendTimeStamp = true);   // NOLINT(575)
        gf_error_t dumpEnd(uint32_t operation = OP_TYPE_DEFAULT);
        gf_error_t dumpMultipleFrameRawData(const char* filepath, uint16_t* rawData,
                uint32_t dataLen, uint32_t width, uint32_t height);
        gf_error_t dumpImageToBmpFile(const char *filepath, uint8_t *image, uint32_t width,
                uint32_t height);
        gf_error_t dumpUnsignedDataToCsvFile(const char *filepath, void *data, uint8_t byteSize,
                uint32_t width, uint32_t height);
        gf_error_t dumpSignedDataToCsvFile(const char *filepath, void *data, uint8_t byteSize,
                uint32_t width, uint32_t height);
        gf_error_t dumpFloatDataToCsvFile(const char *filepath, void *data, uint8_t byteSize,
                uint32_t width, uint32_t height);
        gf_error_t dumpDataToFileDirectly(const char *filepath, uint8_t *data, uint32_t len);
        gf_error_t dumpRawDataToBmp(const char *filepath, void* data, uint32_t width, uint32_t height);
        gf_error_t customizedEncryptDumpImageToBmpFile(const char *filepath, uint8_t *image,
                                               uint32_t width, uint32_t height, uint32_t operation);
        gf_error_t dumpDumplicatUnsignedDataToCsvFile(const char *filepath, void *data, uint8_t byteSize,
                uint32_t width, uint32_t height, uint32_t frame);
        /**
         * Relative the root dir
         */
        void setEncryptDir(const char *encrypt_dir);
        // relative to the data root
        void setPlainDir(const char *plainDir);

        bool getEncryptFlag();

        void setEncryptFlag(bool encrypt);

    private:
        gf_error_t writeBufferToEncryptFile(const char* filepath, const uint8_t* buffer,
                uint32_t buffer_len);
        gf_error_t writeBufferToFile(const char* filepath, const uint8_t* buffer,
                uint32_t buffer_len);

        bool mEncrypt;
        bool mDumpEnable;
        EncryptDataHeader mEncryptHeader;
        Vector<uint8_t> mDataRoot;
        Vector<uint8_t> mEncryptDir;
        Vector<uint8_t> mEncryptFileName;
        Vector<uint8_t> mEncryptData;
        Vector<uint8_t> mPlainDir;
        Mutex mEncryptLock;
    };
}  // namespace goodix

#endif /* _DUMPENCODER_H_ */