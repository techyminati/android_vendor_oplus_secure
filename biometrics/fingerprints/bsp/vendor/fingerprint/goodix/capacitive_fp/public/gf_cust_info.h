#ifndef __GF_CUST_INFO__
#define __GF_CUST_INFO__

#define DEC_STR_LEN   8
#define DEC_VER_LEN   16
#define DEC_VER2_LEN  32

// init
typedef struct
{
    int32_t init_result;//初始化是否成功
    int32_t init_fail_reason;//失败原因

//ic info
    int32_t sensor_id;//唯一标示模组的id
    int32_t lens_type;//之前的GF_SZ_CMD_FACTORY_TEST_GET_MT_INFO会尽量不用
    char chip_type[DEC_STR_LEN];//直接返回字符串、如G7、G3s、G3
    char factory_type[DEC_STR_LEN];//模组厂，如oflim等、直接返回字符串

//algo vesion
    char algo_version[DEC_VER2_LEN];// example: v03.02.02.206
    int32_t algo_version1;// production alog
    int32_t algo_version2;// fake algo
    int32_t algo_version3;// palm algo
    int32_t algo_version4;//
    int32_t algo_version5;//

//ic_status
    int32_t badpixel_num;//坏点个数
    int32_t badpixel_num_local;//局部坏点个数---0911电容新增
    int32_t init_finger_number;//指纹的个数
    int32_t template_verison;//模板的版本号
    int32_t template_num[5];//各个指纹分别有多少张模板图
    int32_t all_template_num;//总模板图个数

//calabration_info
    int32_t exposure_value;//曝光值
    int32_t exposure_time;//曝光时间
    int32_t calabration_signal_value;//校准信号量
    int32_t calabration_tsnr;//校准snr
    int32_t flesh_touch_diff;
    int32_t scale;
    int32_t gain;

//reserve_info
    int32_t init_event_state1;// reverse
    int32_t init_event_state2;// reverse
    char init_event_sting1[8];// reverse
    char init_event_sting2[8];// reverse

} oplus_fingerprint_init_ta_info_t;

typedef struct
{
//base_info
    int32_t auth_result;//匹配结果
    int32_t fail_reason;//最终的失败原因
    int32_t fail_reason_retry[3];//各个retry过程中的失败原因
    char algo_version[DEC_VER2_LEN];//算法版本号，如v03.02.01.06

//请提供数值范围、尽量用正数、特定的数值请穷举其含义。
    int32_t quality_score;//质量分数
    int32_t match_score;//匹配分数
    int32_t signal_value;//信号量
    int32_t img_area;//图片面积
    int32_t img_direction;//图像方向，全覆盖/左上角/右下角等
    int32_t finger_type;//手指类型，如干湿手指、
    int32_t ta_retry_times;//若从hal拿的，则直接填写也可以
    int32_t recog_round;//解锁成功的那次auth中的步数，若有
    int32_t exposure_flag;//曝光标记位，是否为强光？类比于o_high_light ？
    int32_t study_flag;//模板更新学习标记位
    int32_t fdt_base_flag;//fdt base更新标记位，若有
    int32_t image_base_flag;//image base更新标记位，若有
    int32_t finger_number;//手指个数，注意不要重复读取影响性能
    int32_t errtouch_flag;//误触，若有请标记
    int32_t memory_info;//内存占用情况
    int32_t screen_protector_type;//保护膜类型，如防偷窥膜
    int32_t touch_diff;//手指反光
    int32_t mp_touch_diff;//校准反射光
    int32_t fake_result;//防伪最终判断结果

//rawdata_info
    int32_t auth_rawdata[3];//每张图片的rawdata值，与retry对应。retry0和retry2对应auth_rawdata[0];auth_rawdata[1]

//template_info
    int32_t one_finger_template_num[5];//每个指纹的模板个数
    int32_t all_template_num;//5个指纹的总模板个数

//kpi_info
    int32_t capture_time[4]; // 4次retry各自的采图时间了，如果没有采图，就对应的写0
    int32_t preprocess_time[4]; // 预处理时间
    int32_t get_feature_time[4];
    int32_t auth_time[4];
    int32_t detect_fake_time[4];
    int32_t kpi_time_all[4]; // retry的kpi时间，capture_time、preprocess_time等时间相加
    int32_t study_time; // 解锁成功之后的模板学习时间
//other_info
    int32_t auth_ta_data[32];
    int32_t rawdata_max;//rawdata最大值-0911电容新增
    int32_t rawdata_min;//rawdata最小值-0911电容新增
    int32_t calbration_rawdata_avg;//校准时候的rawdata均值-0911电容新增
    int32_t calbration_rawdata_std;//校准rawdata标准差-0911电容新增

//bak_info
    int32_t auth_event_state1;//预留信息
    int32_t auth_event_state2;//预留信息
    char auth_event_string1[DEC_STR_LEN];//预留信息
    char auth_event_string2[DEC_STR_LEN];//预留信息
} oplus_fingerprint_auth_ta_info_t;


//single_enroll
typedef struct
{
//base_info
    int32_t singleenroll_result;//最终的失败原因
    int32_t fail_reason;//单次录入失败原因,如手指重复或者重复
    int32_t fail_reason_param1;//失败原因的参数,如重复区域、其重复的比例
    int32_t fail_reason_param2;//失败原因的参数,如重复区域、其重复的比例
    char algo_version[DEC_VER2_LEN];//算法版本号，大的版本号即可
    int32_t current_enroll_times;//当前录入次数

//请提供数值范围、特定的数值请穷举其含义
    int32_t quality_score;//质量分数
    int32_t signal_value;//信号量
    int32_t img_area;//图片面积
    int32_t img_direction;//图像方向，全覆盖/左上角/右下角等
    int32_t finger_type;//手指类型，如干湿手指、
    int32_t ta_retry_times;//若从hal拿的，则直接填写也可以
    int32_t exposure_flag;//曝光标记位，是否为强光？类比于o_high_light?
    int32_t fdt_base_flag;//fdt base更新标记位，若有
    int32_t image_base_flag;//image base更新标记位，若有
    int32_t repetition_rate;//重复率;0-100
    int32_t enroll_rawdata;//图片的rawdata值
    int32_t anomaly_flag;//异物标记位
    int32_t screen_protector_type;//保护膜类型，如防偷窥膜
    int32_t key_point_num;//关键点个数
    int32_t increase_rate;//增长率

//kpi_infp
    int32_t capture_time;
    int32_t preprocess_time;
    int32_t get_feature_time;
    int32_t enroll_time;
    int32_t detect_fake_time;
    int32_t kpi_time_all;//retry的kpi时间，capture_time、preprocess_time等时间相加

//bak_info
    int32_t singleenroll_reserved_int1;//预留信息
    int32_t singleenroll_reserved_int2;//预留信息
    char singleenroll_reserved_char1[DEC_STR_LEN];//预留信息
    char singleenroll_reserved_char2[DEC_STR_LEN];//预留信息
} oplus_fingerprint_single_enroll_ta_info_t;
//enroll_end

typedef struct
{
//base_info
    int32_t enroll_result;//录入的最后结果
    int32_t enroll_reason;//最终的失败原因
    int32_t cdsp_flag;//是否使用了cdsp

    int32_t repetition_enroll_number;//重复且强制录入的次数
    int32_t total_enroll_times;//总的录入次数
    int32_t finger_number;//总手指个数
    int32_t lcd_type;//lcd的类型

//version_info
    char algo_version[DEC_VER2_LEN];//算法版本号，最大的版本号即可
    int32_t template_version;//模板版本号
//calabration_info

//bak_info
    int32_t enroll_event_state1;
    int32_t enroll_event_state2;
    char enroll_event_string1[8];
    char enroll_event_string2[8];
} oplus_fingerprint_enroll_ta_info_t;

typedef enum oplus_fingerprint_dcs_event_type {
    DCS_INTI_EVENT_INFO = 0,
    DCS_AUTH_EVENT_INFO = 1,
    DCS_SINGLEENROLL_EVENT_INFO = 2,
    DCS_ENROLL_EVENT_INFO = 3,
    DCS_SPECIAL_EVENT_INFO = 4,
    DCS_DEFAULT_EVENT_INFO,
} oplus_fingerprint_dcs_event_type_t;


typedef struct {
    oplus_fingerprint_dcs_event_type_t dcs_type;
    union {
        oplus_fingerprint_init_ta_info_t init_event_info;
        oplus_fingerprint_auth_ta_info_t auth_event_info;
        oplus_fingerprint_single_enroll_ta_info_t single_enroll_event_info;
        oplus_fingerprint_enroll_ta_info_t enroll_event_info;
    } data;
} oplus_fingerprint_dcs_info_t;

typedef struct
{
    gf_cmd_header_t header;
    oplus_fingerprint_dcs_info_t dcs_ta_cmd_info;
    uint32_t sync_result;
    uint32_t dcs_type;
} oplus_dcs_event_ta_cmd_t;


#endif //__GF_CUST_INFO__

