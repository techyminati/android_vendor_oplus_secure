#ifndef _HALDSP_H_
#define _HALDSP_H_

#include "gf_error.h"
#include <semaphore.h>
#include <dlfcn.h>
#include "proxy.h"
#include "Thread.h"

typedef enum
   {
       DSP_NOT_AVAILABLE,
       DSP_AVAILABLE,
       DSP_INITIALIZING,
       DSP_NOT_SECURE,
       DSP_NONE,
   } gf_dsp_status_t;

   typedef enum
   {
       DSP_CMD_GET_VERSION,
       DSP_CMD_GET_FEATURE_TWO,
       DSP_CMD_GET_FEATURE_FOUR,
       DSP_CMD_AGING_TEST,
       DSP_CMD_SET_HIGH_SPEED,
       DSP_CMD_SET_NORMAL_SPEED,
       DSP_CMD_MAX,
   } gf_dsp_cmd_t;

   typedef enum
   {
       DSP_PROXY_SEND_CMD,
       DSP_PROXY_SET_HIGH_FREQ,
       DSP_PROXY_SET_NORMAL_FREQ,
       DSP_PROXY_EXIT,
       DSP_PROXY_NONE,
   } gf_dsp_proxy_cmd_type_t;

   typedef enum
   {
       SEND_CMD_DOING,
       SEND_CMD_ERROR,
       SEND_CMD_FINISH,
   } gf_dsp_proxy_send_cmd_status_t;

   typedef struct
   {
       int32_t power_level;
       int32_t latency;
       int32_t dcvs_enable;
   } gf_dsp_proxy_set_high_freq_t;

   typedef struct
   {
       gf_dsp_proxy_cmd_type_t type;
       gf_dsp_proxy_send_cmd_status_t status;
       union
       {
           dsp_cmd_type cmd_type;
           gf_dsp_proxy_set_high_freq_t high_freq;
       };
   } gf_dsp_proxy_cmd_t;


namespace goodix {
    class CaEntry;
    class HalDsp : public Thread {
    public:
        explicit HalDsp(CaEntry *caEntry);
        virtual ~HalDsp();
        virtual gf_error_t init();
        virtual gf_error_t reinit();
        virtual gf_error_t deinit();
        gf_error_t sendCmdToDsp(gf_dsp_cmd_t cmd);
        gf_error_t waitDspNotify();
        gf_dsp_status_t checkDspValid();
        int64_t mDspTime;
#ifdef SUPPORT_DSP_COMPATIBLE_VERSION_G3
        static HalDsp* sInstance;
        virtual int32_t onDspCmdResult(int cmd, int result);
        static inline int32_t dspCmdCallback(int32_t cmd, int32_t result) {
            if (HalDsp::sInstance != NULL) {
                return HalDsp::sInstance->onDspCmdResult(cmd, result);
            }
            return 0;
        };
#endif
    protected:
        virtual bool threadLoop();
    private:
        void clearDspSem();
        gf_error_t proxyHandleInit(void);
        void proxyHandleDeinit(void);
        gf_error_t setDspHighFreq(void);
        gf_error_t setDspNormalFreq(void);
#ifdef SUPPORT_DSP_COMPATIBLE_VERSION_G3
        gf_error_t dspSemInit();
        void dspSemDeinit();
        uint32_t mDspResult;
        static sem_t sSem;
#else
        sem_t* getDspSem();
#endif
        gf_error_t waitDspNotify(uint32_t wait_time_sec, uint32_t wait_time_us);
        gf_error_t dspProxySendCmd(gf_dsp_proxy_cmd_t *proxy_cmd);

        volatile gf_dsp_status_t mDspStatus;
        gf_dsp_proxy_cmd_t mProxyCmd;
        CaEntry *mCaEntry;
        Mutex mDspMutex;
        Condition mCond;
    };
}  // namespace goodix
#endif  /* _HALDSP_H_ */
