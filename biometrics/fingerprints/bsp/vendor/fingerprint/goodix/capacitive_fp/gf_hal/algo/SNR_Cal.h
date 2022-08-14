#ifndef SNR_CAL_H
#define SNR_CAL_H

#define CREATEDLL_API  //_declspec(dllexport)

#define uint8_t  unsigned char
#define int8     char
#define uint16_t unsigned short
#define int16_t  short
#define uint32_t unsigned int
#define int32_t  int

typedef enum
{
    NoSito = 0,  //不进行Sito处理
    FrameSito = 1, //帧sito
    RowSito = 2, //行sito
    ColSito = 3 //列sito
}emSito_t;

typedef struct
{
    uint16_t frameNum;  //pBaseData、pTouchRawData所包含的图像的帧数（两者数量必须相等）
    uint16_t row;  //Sensor的行数
    uint16_t col;  //Sensor的列数
    uint16_t isSaturationTcode;  //是否饱和打码，0代表不是饱和打码，非0代表饱和打码
    uint16_t sito;  //emSito_t sito类型
    uint16_t touchFrameNum; //pTouchRawData所包含的图像的帧数，仅供F系列使用
}SNR_Init_t;

typedef struct
{
    float signal; //信号量
    float noise; //噪声
    float snr; //信噪比
    float selectPercentage; //被选中的片数与总的片数的比值
}SNR_t;

#ifdef __cplusplus
extern "C"{
    CREATEDLL_API uint8_t SNR_Cal(const uint16_t *pBaseData, const uint16_t *pTouchRawData, const SNR_Init_t *pInit, SNR_t *pSNR);

    CREATEDLL_API char *GetSNRVersion(void);
    CREATEDLL_API uint8_t SNR_Cal_F_Series(const uint16_t *pBaseData, const uint16_t *pTouchRawData, const SNR_Init_t *pInit, SNR_t *pSNR);
}
#else
    CREATEDLL_API uint8_t SNR_Cal(const uint16_t *pBaseData, const uint16_t *pTouchRawData, const SNR_Init_t *pInit, SNR_t *pSNR);

    CREATEDLL_API char *GetSNRVersion(void);
    CREATEDLL_API uint8_t SNR_Cal_F_Series(const uint16_t *pBaseData, const uint16_t *pTouchRawData, const SNR_Init_t *pInit, SNR_t *pSNR);
#endif

#endif
