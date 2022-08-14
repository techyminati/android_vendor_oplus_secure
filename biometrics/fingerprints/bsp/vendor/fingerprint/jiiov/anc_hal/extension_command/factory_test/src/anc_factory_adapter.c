#include "anc_factory_adapter.h"
#include "anc_factory_common.h"
#include "anc_extension_command.h"
#include "anc_tac_time.h"
#include "anc_tac_sensor.h"


static ANC_RETURN_TYPE MMIExecute(AncFingerprintManager *p_manager, MMICmdFuncPtr func,  const char* p_cmd_desc) {
#ifndef ANC_DEBUG
    ANC_UNUSED(p_cmd_desc);
#endif
    long long mmi_time = 0;
    ANC_TIME_MEASURE_START(mmi_execute);

    ANC_RETURN_TYPE ret = func(p_manager);

    ANC_GET_TIME_MEASURE_END(mmi_execute, p_cmd_desc, &mmi_time);

    return ret;
}


ANC_RETURN_TYPE FactoryModeExtensionCommand(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    int32_t command_id = p_command->command_id;

    int32_t cmd_count = EXTENSION_COMMAND_FT_COUNT;
    int32_t index = command_id - EXTENSION_COMMAND_FT_INIT;
    if ((index < 0) || (index > cmd_count)) {
        ANC_LOGW("extension, invalid factory command id:%d", command_id);
        return ANC_FAIL_INVALID_COMMAND;
    }

    static CmdExecute cmds[EXTENSION_COMMAND_FT_COUNT];
    static uint32_t excute_count = EXTENSION_COMMAND_FT_COUNT;
    static uint32_t sensor_chipid = 0;

    do {
        if (index == 0) {
            if ((ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager,ANC_SENSOR_WAKEUP) != ANC_OK)) {
                ANC_LOGE("failed set power mode wakeup, ret_val = %d", ret_val);
                break;
            }
            if ((ret_val = SensorGetChipId(&sensor_chipid) != ANC_OK)) {
                ANC_LOGE("failed get sensor chipid, ret_val = %d", ret_val);
                break;
            }  
            ANC_LOGD(" sensor chipid:%#x",sensor_chipid);

            if ((ret_val = GetFactoryImpl(sensor_chipid, cmds, excute_count)) != ANC_OK) {
                ANC_LOGE("failed get factory all test item, ret_val = %d", ret_val);
                break;
            }
        }

        ANC_LOGD("exec mmi test. command_id:%d, cmd_count:%d, index:%d", command_id, cmd_count, index);
        CmdExecute *p_cmd_exec = &cmds[index];
        ret_val = MMIExecute(p_manager, p_cmd_exec->exec, p_cmd_exec->p_exec_desc);
    } while (0);

    return ret_val;
}



