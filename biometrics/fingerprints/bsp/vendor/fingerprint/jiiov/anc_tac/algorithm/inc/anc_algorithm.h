#ifndef __ANC_ALGORITHM_H__
#define __ANC_ALGORITHM_H__

#include <stdint.h>

#include "anc_type.h"
#include "anc_error.h"

extern uint8_t g_algo_version_string[100];
ANC_RETURN_TYPE AlgorithmInit();
ANC_RETURN_TYPE GetAlgorithmVersion(uint8_t *p_version, uint32_t version_length);
ANC_RETURN_TYPE AlgorithmDeinit();

ANC_RETURN_TYPE AlgoInitEnroll();
ANC_RETURN_TYPE AlgoEnroll(uint32_t *p_remaining, uint32_t *p_fingerid);
ANC_RETURN_TYPE AlgoEnrollExtractFeature();
ANC_RETURN_TYPE AlgoEnrollFeature(uint32_t *p_remaining, uint32_t *p_fingerid);
ANC_RETURN_TYPE AlgoDeinitEnroll(uint32_t *p_finger_id, ANC_BOOL is_finished);
ANC_RETURN_TYPE AlgoGetEnrollTotalTimes(int32_t *p_enroll_total_times);

ANC_RETURN_TYPE AlgoInitVerify();
ANC_RETURN_TYPE AlgoVerify(uint32_t *p_finger_id, uint32_t *p_need_study, uint32_t *p_algo_status, uint32_t retry_count);
ANC_RETURN_TYPE AlgoVerifyExtractFeature(uint32_t retry_count, int32_t abnormal_exp, uint32_t *p_exp_ratio);
ANC_RETURN_TYPE AlgoCompareFeature(uint32_t *p_finger_id, uint32_t *p_need_study, uint32_t *p_algo_status);
ANC_RETURN_TYPE AlgoFeatureStudy(uint32_t finger_id);
ANC_RETURN_TYPE AlgoDeinitVerify();

ANC_RETURN_TYPE TemplateSetActiveGroup(uint32_t gid, const char *p_store_path);
ANC_RETURN_TYPE GetAuthenticatorId(uint64_t *p_authenticator_id);
ANC_RETURN_TYPE TemplateLoadDatabase();
ANC_RETURN_TYPE DeleteFingerprint(uint32_t finger_id);
ANC_RETURN_TYPE GetAllFingerprintsId(uint32_t *p_id_array, uint32_t id_array_size, uint32_t *p_id_count);
uint32_t GetAllFingerprintsCount(void);

ANC_RETURN_TYPE AlgoGetImageQualityScore(uint32_t *P_image_quality_score);

void AppSetSaveFileConfig(uint8_t on_off);

ANC_RETURN_TYPE AlgoGetHBResult(uint32_t retry_count, uint32_t *p_feat, uint32_t *p_bpm);

#endif
