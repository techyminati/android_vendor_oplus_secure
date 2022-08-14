#ifndef __ANC_DATA_COLLECT_H__
#define __ANC_DATA_COLLECT_H__

#include "anc_type.h"


/*
//auth
typedef struct
{
//base_info
    int32_t auth_result;    //匹配结果
    int32_t fail_reason;    //0
    int32_t fail_reason_retry[3];   //各个retry过程中的失败原因
    char algo_version[32];  //算法版本号，如v03.02.01.06

//请提供数值范围、尽量用正数、特定的数值请穷举其含义。
    int32_t quality_score;  //质量分数, finger_quality_score
    int32_t match_score;    //匹配分数, compare_cache_score
    int32_t signal_value;   //指纹有效区域的像素均值，seg_feat_mean
    int32_t img_area;       //指纹有效区域面积，finger_seg_score
    int32_t img_direction;  //指纹方向，fp_direction
    int32_t finger_type;    //0
    int32_t ta_retry_times; //retry次数
    int32_t recog_round;    //0
    int32_t exposure_flag;  //漏光得分，light_score
    int32_t study_flag;     //学习状态，study_status
    int32_t fdt_base_flag;  //0
    int32_t image_base_flag;//0
    int32_t finger_number;  //手指个数
    int32_t errtouch_flag;  //0
    int32_t memory_info;    //0
    int32_t screen_protector_type;  //贴膜判断分数，fp_protector_score
    int32_t touch_diff;     //指纹场景温度分数，如干冷，常温，强光，fp_temperature_score
    int32_t mp_touch_diff;  //0
    int32_t fake_result;    //活体得分，finger_live_score

//rawdata_info
    int32_t auth_rawdata[3];//每张图片的rawdata值，与retry对应。retry0和retry2对应auth_rawdata[0];auth_rawdata[1]

//template_info
    int32_t one_finger_template_num[5]; //0
    int32_t all_template_num;   //0

//kpi_info
    int32_t capture_time[4];    //3次retry各自的采图时间
    int32_t preprocess_time[4]; //0
    int32_t get_feature_time[4];//特征提取时间
    int32_t auth_time[4];       //比对时间
    int32_t detect_fake_time[4];//0
    int32_t kpi_time_all[4];    //retry的kpi时间，各模块时间相加
    int32_t study_time;         //解锁成功之后的模板学习时间
//calabration_info

//other_info
    int32_t auth_ta_data[32];
    //jiiov special
    //0  int32_t    matched_template_idx;   //比对上的模板序号. 主要看用户解锁习惯，是均匀用手指还是只用常用手指
    //1  int32_t    compare_final_score;    //最终比对得分
    //2  int32_t    img_variance; //raw图的方差
    //3  int32_t    img_contrast; //raw图的对比度
    //4  int32_t    strange_score;  //异物得分
    //5  int32_t    debase_quality_score;   //debase综合质量得分
    //6  int32_t    matched_image_sum;  //解锁图片的sum值
    //7  int32_t    matched_user_id;    //匹配上的指纹组虚拟id
    //8  int32_t    matched_finger_id;  //匹配上的指纹组内指纹虚拟id
    //9  int32_t    mat_s; //比对图到静态底库的矩阵值
    //10 int32_t    mat_d; //比对图到动态底库的矩阵值
    //11 int32_t    mat_c; //比对图到缓存底库的矩阵值
    //12 int32_t    mat_cs; //匹配上的缓存底库到静态底库的矩阵值
    //13 int32_t    compress_cls_score; //复合二分类得分
    //14 int32_t    compress_tnum_score;    //复合匹配点数得分
    //15 int32_t    compress_area_score;    //复合底库面积得分
    //16 int32_t    compress_island_score;  //复合底库岛得分
    //17 int32_t    compress_image_pixel_1; //raw图0、0.25分位数
    //18 int32_t    compress_image_pixel_2; //raw图0.5、0.75分位数
    //19 int32_t    compress_image_pixel_3; //raw图1分位数、均值
    //20 int32_t    verify_index;   //比对序号
    //21 int32_t    env_light;  //环境光值
    //22 int32_t    fusion_cnt; //融合序号
    //23 int32_t    expo_time; //曝光时间


//bak_info
    int32_t auth_event_state1;  //0
    int32_t auth_event_state2;  //0
    char auth_event_string1[8]; //0
    char auth_event_string2[8]; //0
}  __attribute__((packed)) jiiov_oplus_fingerprint_auth_ta_info_t;
*/






/*// init
typedef struct
{
    int32_t init_result;
    int32_t init_fail_reason;

//ic info
    int32_t sensor_id;      //vendor_id
    int32_t lens_type;      //lens_id
    char chip_type[8];      //chip_id，如jv0302
    char factory_type[8];   //module_integrator_id，模组厂，如oflim等、直接返回字符串

//algo vesion
    char algo_version[32];  //example: v03.02.02.206
    int32_t algo_version1;  //0
    int32_t algo_version2;  //0
    int32_t algo_version3;  //0
    int32_t algo_version4;  //0
    int32_t algo_version5;  //0

//ic_status
    int32_t badpixel_num;       //坏点个数，total defect number
    int32_t badpixel_num_local; //坏块个数，total cluster number
    int32_t init_finger_number; //指纹的个数
    int32_t template_verison;   //example::0x22 模板的版本号
    int32_t template_num[5];    //各个指纹分别有多少张模板图
    int32_t all_template_num;   //总模板图个数

//calabration_info
    int32_t exposure_value;     //曝光校正参数，base_sum
    int32_t exposure_time;      //0
    int32_t calabration_signal_value;   //最小信号量，s_signal_min
    int32_t calabration_tsnr;   //0
    int32_t flesh_touch_diff;   //高频信号量，freqpeak
    int32_t scale;              //线对数，line_pair_count
    int32_t gain;               //0

//reserve_info
    int32_t init_event_state1;  //0
    int32_t init_event_state2;  //chip_id头备份
    char init_event_sting1[8];  //chip_id_1
    char init_event_sting2[8];  //chip_id_2

//jiiov special
    // int32_t calabration_rawdata;//校准时候的rawdata值
    // uint32_t base_sum;
    // uint32_t white_mean;
    // uint32_t black_mean;
    // uint32_t x;
    // uint32_t y;
    // uint32_t freqpeak;
    // uint32_t shading_highest;
    // uint32_t shading_lowest;
    // uint32_t fov_size;
    // struct {
    //     uint32_t total;
    //     uint32_t lines;
    //     uint32_t clusters;
    //     uint32_t max_cluster_size;
    // } defect_info[2];
    // uint32_t offset_x;
    // uint32_t offset_y;
    // int32_t s_signal_max;
    // int32_t s_signal_min;
    // int32_t s_nosie_max;
    // int32_t s_nosie_min;
    // int32_t t_signal_max;
    // int32_t t_signal_min;
    // int32_t t_nosie_max;
    // int32_t t_nosie_min;
    // uint32_t line_pair_count;
    // int32_t rotation_degree;
    // uint32_t frequency_difference;
}  __attribute__((packed)) oplus_fingerprint_init_ta_info_t;


//auth
typedef struct
{
//base_info
    int32_t auth_result;    //匹配结果
    int32_t fail_reason;    //0
    int32_t fail_reason_retry[3];   //各个retry过程中的失败原因
    char algo_version[32];  //算法版本号，如v03.02.01.06

//请提供数值范围、尽量用正数、特定的数值请穷举其含义。
    int32_t quality_score;  //质量分数, finger_quality_score
    int32_t match_score;    //匹配分数, compare_cache_score
    int32_t signal_value;   //指纹有效区域的像素均值，seg_feat_mean
    int32_t img_area;       //指纹有效区域面积，finger_seg_score
    int32_t img_direction;  //指纹方向，fp_direction
    int32_t finger_type;    //残影分数1
    int32_t ta_retry_times; //retry次数
    int32_t recog_round;    //残影分数2
    int32_t exposure_flag;  //漏光得分，light_score
    int32_t study_flag;     //学习状态，study_status
    int32_t fdt_base_flag;  //复合快返分数1
    int32_t image_base_flag;//复合快返分数2
    int32_t finger_number;  //手指个数
    int32_t errtouch_flag;  //手指按压时间
    int32_t memory_info;    //缓存底库个数
    int32_t screen_protector_type;  //贴膜判断分数，fp_protector_score
    int32_t touch_diff;     //指纹场景温度分数，如干冷，常温，强光，fp_temperature_score
    int32_t mp_touch_diff;  //复合快返分数3
    int32_t fake_result;    //活体得分，finger_live_score

//rawdata_info
    int32_t auth_rawdata[3];//每张图片的rawdata值，与retry对应。retry0和retry2对应auth_rawdata[0];auth_rawdata[1]

//template_info
    int32_t one_finger_template_num[5]; //0
    int32_t all_template_num;   //0

//kpi_info
    int32_t capture_time[4];    //3次retry各自的采图时间
    int32_t preprocess_time[4]; //0
    int32_t get_feature_time[4];//特征提取时间
    int32_t auth_time[4];       //比对时间
    int32_t detect_fake_time[4];//0
    int32_t kpi_time_all[4];    //retry的kpi时间，各模块时间相加
    int32_t study_time;         //解锁成功之后的模板学习时间
//calabration_info

//other_info
    int32_t auth_ta_data[32];
    //jiiov special
    //0  int32_t	matched_template_idx;	//比对上的模板序号. 主要看用户解锁习惯，是均匀用手指还是只用常用手指
    //1  int32_t	compare_final_score;	//最终比对得分
    //2  int32_t	img_variance; //raw图的方差
    //3  int32_t	img_contrast; //raw图的对比度
    //4  int32_t	finger_strange_score;	//异物得分
    //5  int32_t	debase_quality_score;	//debase综合质量得分
    //6  int32_t	matched_image_sum;	//解锁图片的sum值
    //7  int32_t	matched_user_id;	//匹配上的指纹组虚拟id
    //8  int32_t	matched_finger_id;	//匹配上的指纹组内指纹虚拟id
    //9  int32_t	mat_s; //比对图到静态底库的矩阵值
    //10 int32_t	mat_d; //比对图到动态底库的矩阵值
    //11 int32_t	mat_c; //比对图到缓存底库的矩阵值
    //12 int32_t	mat_cs; //匹配上的缓存底库到静态底库的矩阵值
    //13 int32_t	compress_cls_score;	//复合二分类得分
    //14 int32_t	compress_tnum_score;	//复合匹配点数得分
    //15 int32_t	compress_area_score;	//复合底库面积得分
    //16 int32_t	compress_island_score;	//复合底库岛得分
    //17 int32_t	compress_image_pixel_1;	//raw图0、0.25分位数
    //18 int32_t	compress_image_pixel_2;	//raw图0.5、0.75分位数
    //19 int32_t	compress_image_pixel_3;	//raw图1分位数、均值
    //20 int32_t	verify_index;	//比对序号
    //21 int32_t	env_light;	//环境光值
    //22 int32_t    fusion_cnt; //融合序号
    //23 int32_t    expo_time; //曝光时间
    //24 int32_t    ghost_cache_behavior; //ghost cache行为统计
    //25 int32_t    dynamic_liveness_th; //动态活体阈值
    //26 int32_t    signal;     //复合工模测试信号
    //27 int32_t    noise;      //复合工模测试噪声
    //28 int32_t    fov + hi_freq;  //fov + 高频分量
    //29 int32_t    expo + screen_leak_ratio;   //校准固定曝光时间 + 屏幕漏光比
    //30 int32_t    magnification + shading;    //放大倍数 + 暗角比值


//bak_info
    int32_t auth_event_state1;  //息屏状态
    int32_t auth_event_state2;  //chip_id头备份
    char auth_event_string1[8]; //chip_id_1
    char auth_event_string2[8]; //chip_id_2
}  __attribute__((packed)) oplus_fingerprint_auth_ta_info_t;


//single_enroll
typedef struct
{
//base_info
    int32_t singleenroll_result;    //最终的失败原因
    int32_t fail_reason;            //0
    int32_t fail_reason_param1;     //0
    int32_t fail_reason_param2;     //0
    char algo_version[32];          //算法版本号，大的版本号即可
    int32_t current_enroll_times;   //0

//请提供数值范围、特定的数值请穷举其含义
    int32_t quality_score;          //质量分数，finger_quality_score
    int32_t signal_value;           //指纹有效区域像素均值，seg_feat_mean
    int32_t img_area;               //指纹有效区域像素面积，finger_seg_score
    int32_t img_direction;          //0
    int32_t finger_type;            //0
    int32_t ta_retry_times;         //retry次数
    int32_t exposure_flag;          //漏光分数，light_score
    int32_t fdt_base_flag;          //0
    int32_t image_base_flag;        //0
    int32_t repetition_rate;//重复率;//0
    int32_t enroll_rawdata;         //复合像素值，compress_image_pixel_3
    int32_t anomaly_flag;           //防伪分数，live_score
    int32_t screen_protector_type;  //0
    int32_t key_point_num;          //0
    int32_t increase_rate;          //0
    //int32_t icon_status;

//kpi_infp
    int32_t capture_time;           //采图时间
    int32_t preprocess_time;        //0
    int32_t get_feature_time;       //特征提取时间
    int32_t enroll_time;            //录入时间
    int32_t detect_fake_time;       //0
    int32_t kpi_time_all;           //kpi时间，各模块时间相加

//bak_info
    int32_t singleenroll_event_state1;  //0
    int32_t singleenroll_event_state2;  //0
    char singleenroll_event_string1[8]; //0
    char singleenroll_event_string2[8]; //0
//jiiov special
    // int32_t finger_number;
    // int32_t live_score;
    // int32_t light_score;
    // int32_t finger_seg_score;
    // int32_t img_sum;
    // int32_t img_variance;
    // int32_t img_contrast;
    // int32_t seg_feat_mean;
    // int32_t compress_image_pixel_1;
    // int32_t compress_image_pixel_2;
    // int32_t compress_image_pixel_3;
}  __attribute__((packed)) oplus_fingerprint_singleenroll_ta_info_t;

//enroll_end
typedef struct
{
//base_info
    int32_t enroll_result;      //录入的最后结果
    int32_t enroll_reason;      //0
    int32_t cdsp_flag;          //0

    int32_t repetition_enroll_number;   //重复且强制录入的次数
    int32_t total_enroll_times;         //总的录入次数
    int32_t finger_number;              //总手指个数
    int32_t lcd_type;                   //0

//version_info
    char algo_version[32];              //算法版本号，最大的版本号即可
    int32_t template_version;           //模板版本号

//calabration_info

//bak_info
    int32_t enroll_event_state1;        //0
    int32_t enroll_event_state2;        //0
    char enroll_event_string1[8];       //0
    char enroll_event_string2[8];       //0
}  __attribute__((packed)) oplus_fingerprint_enroll_ta_info_t;

// typedef enum oplus_fingerprint_dcs_event_type {
//     DCS_INTI_EVENT_INFO = 0,
//     DCS_AUTH_EVENT_INFO = 1,
//     DCS_SINGLEENROLL_EVENT_INFO = 2,
//     DCS_ENROLL_EVENT_INFO = 3,
//     DCS_SPECIAL_EVENT_INFO = 4,	
//     DCS_DEFAULT_EVENT_INFO,
// } oplus_fingerprint_dcs_event_type_t;


// typedef struct {
//     oplus_fingerprint_dcs_event_type_t dcs_type;
//     union {
// 		oplus_fingerprint_init_event_info_t init_event_info;
// 		oplus_fingerprint_auth_event_info_t auth_event_info;
// 		oplus_fingerprint_singleenroll_event_info_t singleenroll_event_info;
// 		oplus_fingerprint_enroll_event_info_t enroll_event_info;
// 		oplus_fingerprint_special_event_info_t special_event_info;
//     } data;
// } oplus_fingerprint_dcs_info_t;*/


#endif