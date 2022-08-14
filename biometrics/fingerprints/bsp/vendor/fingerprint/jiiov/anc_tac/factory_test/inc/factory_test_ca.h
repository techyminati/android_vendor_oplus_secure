#ifndef __FACTORY_TEST_H__
#define __FACTORY_TEST_H__

#include "anc_error.h"
#include "anc_type.h"
#include "extension_command.h"

///////// FT FUNCTIONS ////////
ANC_RETURN_TYPE ExtensionFTInit();

/// 获取MMI配置信息
ANC_RETURN_TYPE ExtensionFTGetConfig(AncFTConfig *p_config);

/// 模组测试
ANC_RETURN_TYPE ExtensionFTModuleTest();

/// 将图从sensor模块转移到FT模块
ANC_RETURN_TYPE ExtensionFTTransmitImage(uint32_t idx, uint8_t **pp_buffer, uint32_t *p_buffer_length);

/// 拉取在ta中缓存的图
ANC_RETURN_TYPE ExtensionFTTransmitImageFromCache(uint32_t idx, int32_t *p_expo_time, uint8_t **pp_buffer,
                                         uint32_t *p_buffer_length);
/// 在ta中采图并将图缓存
ANC_RETURN_TYPE ExtensionFTCaptureImageToCache(uint32_t expo_time, uint32_t image_count);

/// 对多张图使用算法求得base sum
ANC_RETURN_TYPE ExtensionFTCalibrationBaseSum(uint32_t *base_sum_arr, uint32_t len, uint32_t *base_sum);

/// 安装偏差测试
ANC_RETURN_TYPE ExtensionFTMarkPositioning(AncFTMarkPositioningResult *p_result);

/// 肉色头防呆
ANC_RETURN_TYPE ExtensionWhitePrevent(uint32_t image_count, AncFTWhitePreventInfo *p_result);

/// Lens测试
ANC_RETURN_TYPE ExtensionFTLensTest(uint32_t image_index, AncFTLensTestResult *p_result);

/// 坏点坏块
ANC_RETURN_TYPE ExtensionFTDefectTest(uint32_t image_index, AncFTDefectResult *p_result);

/// base图校准
ANC_RETURN_TYPE ExtensionFTBaseImageCalibration(uint32_t image_count);

/// 黑色头曝光时间测试
ANC_RETURN_TYPE ExtensionFTBlackTest(AncFTBlackAlgoResult *p_result);

///signal/noise
ANC_RETURN_TYPE ExtensionFTSignalNoiseTest(uint32_t image_count, AncFTSnrResult *p_result);

/// freq/M-Factor
ANC_RETURN_TYPE ExtensionFTFreqMFactor(uint32_t image_count, AncFTChartAnalyseResult *p_result);

/// 保存校准数据(校准数据包含：base图、base sum、异常情况下的固定曝光时间、光心坐标)
ANC_RETURN_TYPE ExtensionSaveCalibration();

/// 重置TA中的一些状态
ANC_RETURN_TYPE ExtensionFTReset();

/// 释放TA资源
ANC_RETURN_TYPE ExtensionFTDeInit();

/// 曝光校准
ANC_RETURN_TYPE ExtensionFTExpoCalibration(int type, AncFTExpoResult* p_result);

ANC_RETURN_TYPE ExtensionFTModuleTestV2();
#endif

