#ifndef __ANC_FP_INFO_H__
#define __ANC_FP_INFO_H__

#include "anc_type.h"

// init info
typedef struct
{
// ic info
char        product_id[16];         // 产品id，例如JV0301
int32_t	    lens_id;                // 镜头id
char	    chip_id[32];            // 模组id
int32_t	    module_integrator_id;   // 模组厂id
// algo vesion
char 	    algo_version[32];       // 算法版本号
int32_t	    template_verison;       // 模板版本号
// calabration info
int32_t	    defect_total;           // 怀点数
int32_t	    defect_cluster;         // 坏块数
int32_t	    expo_param_1;           // 曝光参数1：base_sum或固定曝光时间
int32_t	    expo_param_2;           // 曝光参数2：最小曝光时间阈值
int32_t	    expo_param_3;           // 曝光参数3：环境光上限阈值
int32_t	    signal_min;             // 最小信号量
int32_t	    freqpeak;               // 高频信号量
int32_t	    line_pair_count;        //线对数
}  __attribute__((packed)) anc_fingerprint_init_info_t;


// enroll info
typedef struct
{
int32_t     cmd_status;             // cmd返回值
char        algo_version[32];       // 算法版本号
int32_t     finger_quality_score;   // 质量分
int32_t     seg_feat_mean;          // 指纹有效区域像素均值
int32_t     finger_seg_score;       // 指纹有效区域像素面积
int32_t     finger_light_score;     // 漏光分
int32_t     compress_image_pixel_1; // raw图0、0.25分位数
int32_t     compress_image_pixel_2; // raw图0.5、0.75分位数
int32_t     compress_image_pixel_3; // raw图1分位数、均值
int32_t     finger_live_score;      // 活体防伪分
int32_t     expo_time;              // 曝光时间
int32_t     env_light;              // 环境光值
int32_t     is_abnormal_expo;       // 是否异常曝光
int32_t     hbm_time;               // 手指按下到UI ready的时间
int32_t     capture_time;           // 采图时间
int32_t     extract_time;           // 抽feature时间
int32_t     enroll_time;            // 录入模板时间
int32_t     ta_time;                // TA流程时间
int32_t     kpi_time;               // 计入kpi的总时间
}  __attribute__((packed)) anc_fingerprint_enroll_info_t;


// enroll finish info
typedef struct
{
int32_t     cmd_status;             // cmd返回值
char        algo_version[32];       // 算法版本号
int32_t     template_version;       // 模板版本号
int32_t     dup_enroll_cnt;         // 超出判重次数后且重复录入的次数
int32_t     total_enroll_cnt;       // 总的录入次数
int32_t     enrolled_template_count;// 录入模板个数
}  __attribute__((packed)) anc_fingerprint_enroll_finish_info_t;

// auth info
typedef struct
{
int32_t     cmd_status;             // cmd返回值
int32_t     algo_status;            // 算法返回值
char        algo_version[32];       // sdk算法版本
char        chip_id[32];            // 模组id
int32_t     finger_quality_score;   // 质量分
int32_t     compare_cache_score;    // 比对cache分
int32_t     seg_feat_mean;          // 指纹有效区域的像素均值
int32_t     finger_seg_score;       // 指纹有效区域面积
int32_t     fp_direction;           // 指纹方向
int32_t     retry_count;            // retry次数
int32_t     finger_light_score;     // 漏光得分
int32_t     is_studied;             // 学习状态
int32_t     enrolled_template_count;// 录入模板个数
int32_t     fp_protector_score;     // 贴膜判断分数
int32_t     fp_temperature_score;   // 指纹场景温度分数
int32_t     finger_live_score;      // 活体防伪得分
int32_t     raw_img_sum;            // raw图像素的sum值
int32_t     matched_template_idx;   // 比对上的模板序号
int32_t     compare_final_score;    // 最终比对得分
int32_t     img_variance;           // raw图的方差
int32_t     img_contrast;           // raw图的对比度
int32_t     finger_strange_score;   // 异物得分
int32_t     debase_quality_score;   // debase综合质量得分
int32_t     matched_image_sum;      // 匹配图片的sum值
int32_t     matched_user_id;        // 匹配上的指纹组虚拟id
int32_t     matched_finger_id;      // 匹配上的指纹组内指纹虚拟id
int32_t     mat_s;                  // 比对图到静态底库的矩阵值
int32_t     mat_d;                  // 比对图到动态底库的矩阵值
int32_t     mat_c;                  // 比对图到缓存底库的矩阵值
int32_t     mat_cs;                 // 匹配上的缓存底库到静态底库的矩阵值
int32_t     compress_cls_score;     // 复合二分类得分
int32_t     compress_tnum_score;    // 复合匹配点数得分
int32_t     compress_area_score;    // 复合底库面积得分
int32_t     compress_island_score;  // 复合底库岛得分
int32_t     compress_image_pixel_1; // raw图0、0.25分位数
int32_t     compress_image_pixel_2; // raw图0.5、0.75分位数
int32_t     compress_image_pixel_3; // raw图1分位数、均值
int32_t     verify_index;           // 比对序号
int32_t     env_light;              // 环境光值
int32_t     is_abnormal_expo;       // 是否异常曝光
int32_t     fusion_cnt;             // 融合序号
int32_t     expo_time;              // 曝光时间
int32_t     ghost_cache_behavior;   // ghost cache行为统计
int32_t     dynamic_liveness_th;    // 动态活体阈值
int32_t     hbm_time;               // 手指按下到UI ready的时间
int32_t     keyghost_score;         // 残影算法关键帧得分
int32_t     ctnghost_score;         // 残影算法连续帧得分
int32_t     small_cls_fast_reject_count; // 小二分类快拒数目
int32_t     small_cls_fast_accept_count; // 小二分类快A数目
int32_t     tnum_fast_accept_count; // tnum快A数目
int32_t     glb_fast_reject_count;  // glb快拒数目
int32_t     lf_fast_reject_count;   // lf快拒数目
int32_t     total_cmp_cls_times;    // 进入二分类图片次数
int32_t     total_cache_count;      // 候选cache数目
int32_t     capture_time;           // 采图时间
int32_t     extract_time;           // 抽feature时间
int32_t     verify_time;            // 比对时间
int32_t     ta_time;                // TA流程时间
int32_t     kpi_time;               // 计入kpi的总时间
int32_t     study_time;             // 学习时间
// MMI数据
int32_t     signal;                 // 时域信号
int32_t     noise;                  // 时域噪声
int32_t     hi_freq;                // 高频分量
int32_t     screen_leak_ratio;      // 漏光比
int32_t     fov;                    // FOV
int32_t     shading;                // shading
int32_t     cal_expo_time;          // 校准曝光时间
int32_t     magnification;          // 放大倍数
}  __attribute__((packed)) anc_fingerprint_auth_info_t;

#endif
