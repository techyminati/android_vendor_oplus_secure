#ifndef _HAL_DSP_H_
#define _HAL_DSP_H_

#include "gf_error.h"
#include <semaphore.h>
#include <dlfcn.h>
#include "dsp_cmd.h"
#include "Thread.h"
#include "gf_base_types.h"
#include "gf_sensor_types.h"

namespace goodix {
    typedef enum {
        DSP_NOT_AVAILABLE,
        DSP_AVAILABLE,
        DSP_INITIALIZING,
        DSP_NOT_SECURE,
        DSP_NONE,
        DSP_HIGH_SPEED,
        DSP_NORMAL_SPEED
    } gf_dsp_status_t;

    typedef enum {
        DSP_CMD_GET_VERSION,
        DSP_CMD_GET_FEATURE_TWO,
        DSP_CMD_GET_FEATURE_FOUR,
        DSP_CMD_AGING_TEST,
        DSP_CMD_SET_HIGH_SPEED,
        DSP_CMD_SET_NORMAL_SPEED,
        DSP_CMD_START_LISTEN_TO_TA,
        DSP_CMD_MAX
    } gf_dsp_cmd_t;

    typedef enum {
        DSP_PROXY_INIT,
        DSP_PROXY_SEND_CMD,
        DSP_PROXY_SET_HIGH_FREQ,
        DSP_PROXY_SET_NORMAL_FREQ,
        DSP_PROXY_EXIT,
        DSP_PROXY_NONE,
    } gf_dsp_proxy_cmd_type_t;

    typedef enum {
        SEND_CMD_DOING,
        SEND_CMD_ERROR,
        SEND_CMD_FINISH,
    } gf_dsp_proxy_send_cmd_status_t;

    typedef enum {
        DSP_PROXY_CHIP_SHENZHEN,
        DSP_PROXY_CHIP_DELMAR_G5,
        DSP_PROXY_CHIP_DELMAR_G6,
        DSP_PROXY_CHIP_DELMAR_G7,
    } gf_dsp_proxy_chip_type_t;

    typedef struct {
        int32_t power_level;
        int32_t latency;
        int32_t dcvs_enable;
    } gf_dsp_proxy_set_high_freq_t;

    typedef struct {
        gf_dsp_proxy_cmd_type_t type;
        gf_dsp_proxy_send_cmd_status_t status;
        union {
           dsp_cmd_type cmd_type;
           gf_dsp_proxy_set_high_freq_t high_freq;
        };
    } gf_dsp_proxy_cmd_t;

    class CaEntry;
    class HalDsp : private Thread {
    public:
        explicit HalDsp(CaEntry *caEntry);
        virtual ~HalDsp();
        virtual gf_error_t init(gf_sensor_info_t info);
        virtual gf_error_t reinit();
        virtual gf_error_t deinit();
        virtual gf_error_t sendCmdToDsp(gf_dsp_cmd_t cmd);
        virtual gf_error_t waitDspNotify();
        virtual gf_dsp_status_t checkDspValid();
        virtual gf_dsp_status_t checkDspSpeed();
        virtual void markDspNotAvailable(void);
    private:
        static HalDsp* sInstance;
        static inline int32_t dspCmdCallback(int32_t cmd, int32_t result) {
            if (HalDsp::sInstance != NULL) {
                return HalDsp::sInstance->onDspCmdResult(cmd, result);
            }
            return 0;
        };
    protected:
        virtual bool threadLoop();
        virtual void processSendCmd();
        virtual gf_error_t dspProxySendCmd(gf_dsp_proxy_cmd_t *proxy_cmd);
        virtual gf_error_t proxyHandleInit(void);
        virtual void proxyHandleDeinit(void);
        virtual gf_error_t initInternal();
        virtual int32_t onDspCmdResult(int cmd, int result);
        virtual int32_t convertChipType(gf_sensor_info_t info);
        void clearDspSem();
        gf_error_t setDspHighFreq(void);
        gf_error_t setDspNormalFreq(void);
        gf_error_t dspSemInit();
        void dspSemDeinit();
        gf_error_t waitDspNotify(uint32_t wait_time_sec, uint32_t wait_time_us);

        volatile gf_dsp_status_t mDspStatus;
        volatile gf_dsp_status_t mDspSpeedStatus;
        gf_dsp_proxy_cmd_t mProxyCmd;
        CaEntry *mCaEntry;
        Mutex mDspMutex;
        Condition mCond;
        uint32_t mDspResult;
        static sem_t sSem;
        int32_t mChipType;
    };
}  // namespace goodix
#endif  /* _HAL_DSP_H_ */
