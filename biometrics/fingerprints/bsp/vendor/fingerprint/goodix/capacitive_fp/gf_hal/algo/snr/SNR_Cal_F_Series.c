/******************************************************************************
 * $SNR Calculate file  $Date: 2017/01/16 10:30:00 $
 *
 * Copyright 2017-2025 All rights reserved. .
 *
 * Description: Provide the entry of system.
 *
 * Note:
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Long.Liu     2019/02/11        modify for coverity
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <float.h>
#include "SNR_Cal.h"
//#include "cpl_math.h"
#include <math.h>


#define DataError 1 //数据存在异常，有大量坏点或者是测试头按压不全
#define MemAllocFail 2 //内存分配失败
#define FrameLess 3 //数据不足30帧
#define NULL_Point 4 //空指针异常

#define ROW_SLICE  5  //一个切片的行数
#define COL_SLICE  12  //一个切片的列数，必须为偶数，根据测试头的规格有所不同

#define OFFSET 4096 //diff的偏置，保证diff为正数

#define UNTOUCH_DIFF_THRE 0 //判断untouch用的diff的阈值

#define EuDisIncrementThre 400 //欧氏距离增量的阈值

#define MIN_TEMP  10000
#define MAX_TEMP  -10000

#define SELECT_PERCENTAGE_THR 0.5 //被选中的片数与总的片数的比值的阈值

#define Extend_Col 6 //向边缘扩展的列数(目前是取片宽的一半)

#define Swap(a,b) {a=a+b;b=a-b;a=a-b;}

#define SNR_STATIC static

SNR_STATIC void RemoveAvr1(short* pSigData, uint16_t x, uint16_t y)
{
	//uint16_t* DataSave = new T[x*y];
	short* DataSave = (short *)calloc((short)(x*y), sizeof(short));
	double* RowAvr = (double *)calloc(x, sizeof(double));
	double* ColAvr = (double *)calloc(y,sizeof(double));
	double Avr = 0;
	int i = 0;
	int j = 0;

	if ((NULL == DataSave) || (NULL == RowAvr) || (NULL == ColAvr))
	{
		if (NULL != DataSave)free(DataSave);
		if (NULL != RowAvr)free(RowAvr);
		if (NULL != ColAvr)free(ColAvr);
		return;
	}


	memcpy(DataSave, pSigData, sizeof(short)*x*y);
	memset(RowAvr, 0, sizeof(double)*x);
	memset(ColAvr, 0, sizeof(double)*y);


	/******************************全屏SITO*********************/
	for (i = 0; i < x; i++)
	{
		for (j = 0; j < y; j++)
		{
			Avr += (DataSave[i*y + j] / (double)((x+2)*(y+2)));
			if (0 == j || y - 1 == j || 0 == i || x - 1 == i)
			{
				Avr += (DataSave[i*y + j] / (double)((x + 2)*(y + 2)));
			}
			
		}
	}
	Avr += 2*(DataSave[0] / (double)((x + 2)*(y + 2)));
	Avr += 2*(DataSave[y-1] / (double)((x + 2)*(y + 2)));
	Avr += 2*(DataSave[(x-1)*y] / (double)((x + 2)*(y + 2)));
	Avr += 2*(DataSave[(x - 1)*y+y-1] / (double)((x + 2)*(y + 2)));

	for (i = 0; i < x; i++)
	{
		for (j = 0; j < y; j++)
		{
			DataSave[i*y + j] = DataSave[i*y + j] - (short)Avr;
		}
	}
	
	memcpy(pSigData, DataSave, sizeof(short)*x*y);
	if(NULL != DataSave)free(DataSave);
	if (NULL != RowAvr)free(RowAvr);
	if (NULL != ColAvr)free(ColAvr);

}
//***********************************************噪声计算************************************************
SNR_STATIC uint8_t Noise_Cal1(const uint16_t *pBaseDataRmOneCycle, uint16_t allPixel, uint16_t col, uint16_t frameNum, SNR_t *pSNR)
{
	float sumTemp = 0;
	float avgTemp = 0;
	float temp = 0;
	float sumSTD = 0;
	uint16_t pixelCount = 0; //位于片内的像素点统计

	uint16_t i = 0;
    uint16_t j = 0;
	uint32_t pixelIndex = 0;
	uint16_t frameIndex = 0;
	short * pBaseData = NULL;
	uint32_t frameStIndex = 0;

    pBaseData = (short *)(uint16_t *)calloc((uint32_t)(allPixel)*frameNum, sizeof(short));
	
	if (NULL == pBaseData)
		return 0;

	for (pixelIndex = 0; pixelIndex < allPixel*frameNum; ++pixelIndex)
	{
		pBaseData[pixelIndex] = pBaseDataRmOneCycle[pixelIndex];
	}

#if 1
	
	for (frameIndex = 0; frameIndex < frameNum; ++frameIndex)
	{
		frameStIndex = frameIndex*allPixel;

		RemoveAvr1(&pBaseData[frameStIndex], allPixel / col, col);
	}
#endif
	for (j = 0; j < allPixel; j++)
	{
		avgTemp = 0;
		sumTemp = 0;

			pixelCount++;
			for (i = 0; i < frameNum; i++)
			{
				avgTemp = avgTemp + (float)pBaseData[i*allPixel + j];
			}
			avgTemp = avgTemp / frameNum;

			for (i = 0; i < frameNum; i++)
			{
				temp = (float)pBaseData[i*allPixel + j] - avgTemp;
				sumTemp += temp*temp;
			}

			sumSTD += sqrt(sumTemp / frameNum) * 6;  //std*6

	}
	//噪声输出
        if(pixelCount != 0){
            pSNR->noise = sumSTD / pixelCount;
        }
	if (NULL != pBaseData)free(pBaseData);
	return 0;
}


//**************************************************基准曲面计算并校准diff平面*********************************
SNR_STATIC uint8_t Middle_Surface_Fliter1(float **ppDiffValidData, uint16_t AllSlicePixelRow, uint16_t AllSlicePixelCol, float **ppfTempValid)
{
	float minTemp;
	float maxTemp;
	uint16_t extendCol = Extend_Col; //向左右两边各扩展Extend_Col列
	uint16_t tempStartColIndex;
	uint16_t tempEndColIndex;
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t k = 0;
	float sumTemp = 0;

	//寻找行最大最小值包络
	for (i = 0; i < AllSlicePixelRow; i++)
	{
		for (j = 0; j < AllSlicePixelCol; j++)
		{
			minTemp = MIN_TEMP;
			maxTemp = MAX_TEMP;
			tempStartColIndex = j < extendCol ? 0 : (j - extendCol);
			tempEndColIndex = j + extendCol > AllSlicePixelCol ? AllSlicePixelCol : (j + extendCol);
			for (k = tempStartColIndex; k < tempEndColIndex; k++)
			{
				if (ppDiffValidData[i][k]>maxTemp)
					maxTemp = ppDiffValidData[i][k]; //求该像素点左右两边extendCol个像素的最大值
				if (ppDiffValidData[i][k]<minTemp)
					minTemp = ppDiffValidData[i][k]; //求该像素点左右两边extendCol个像素的最小值
			}
			ppfTempValid[i][j] = (maxTemp + minTemp) / 2.0;
		}
	}

	
	for (i = 0; i < AllSlicePixelRow; i++)
	{
		for (j = 0; j < AllSlicePixelCol; j++)
		{
			sumTemp = 0;
			if (j < extendCol)
			{
				for (k = 0; k < (j + extendCol); k++)
				{
					sumTemp += ppfTempValid[i][k];
				}
				sumTemp += ppfTempValid[i][0] * (extendCol - j);
			}
			else if (j + extendCol > AllSlicePixelCol)
			{
				for (k = j - extendCol; k < AllSlicePixelCol; k++)
				{
					sumTemp += ppfTempValid[i][k];
				}
				sumTemp += ppfTempValid[i][AllSlicePixelCol - 1] * (extendCol - (AllSlicePixelCol - j));
			}
			else
			{
				for (k = (j - extendCol); k < (j + extendCol); k++)
				{
					sumTemp = sumTemp + ppfTempValid[i][k];
				}	
			}
			ppDiffValidData[i][j] = ppDiffValidData[i][j] - (sumTemp / (2 * extendCol)); //diff减去基准曲面，获得校准后的值
		}
	}
	return 0;
}


//***********************************************欧氏距离以及片的有效性判断************************************************
SNR_STATIC int32_t Partition1(uint32_t *arr, int32_t low, int32_t high)
{
	int32_t key;
	key = arr[low];
	while (low < high)
	{
		while (low < high && arr[high] >= key)
			high--;
		if (low < high)
			arr[low++] = arr[high];
		while (low < high && arr[low] <= key)
			low++;
		if (low < high)
			arr[high--] = arr[low];
	}
	arr[low] = key;
	return low;
}
SNR_STATIC void Quick_Sort1(uint32_t *arr, int32_t start, int32_t end)
{
	int32_t pos;
	if (start < end)
	{
		pos = Partition1(arr, start, end);
		Quick_Sort1(arr, start, pos - 1);
		Quick_Sort1(arr, pos + 1, end);
	}
	return;
}
SNR_STATIC uint8_t JudgeValidByEuDistance1(float **ppDiffValidData, const uint16_t *pTouchRawData, float **ppfTemp, uint8_t **ValidMatrix, uint16_t SliceMatRow, uint16_t SliceMatCol, uint16_t col, uint16_t pixelCount, uint16_t SliceRowStart, uint16_t SliceColStart)
{
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t k = 0;
    uint16_t l = 0;
	uint8_t isMallocFailed = 0;
	int16_t **ppDiffSliceMax = NULL;
	int16_t **ppDiffSliceMin = NULL;
	float min_temp = MIN_TEMP;
	float max_temp = MAX_TEMP;
	float temp1 = 0;
	float temp2 = 0;
	uint32_t *tempEuDistance = NULL;
	uint32_t tempEuDistanceThr = 0;
	//int16_t **ppDiffSliceMax = (int16_t **)MallocArray2D(SliceMatRow, SliceMatCol, sizeof(int16_t *), sizeof(int16_t));
	//if (NULL == ppDiffSliceMax)
	//	return MemAllocFail;
	//int16_t **ppDiffSliceMin = (int16_t **)MallocArray2D(SliceMatRow, SliceMatCol, sizeof(int16_t *), sizeof(int16_t));
	//if (NULL == ppDiffSliceMin)
	//{
	//	free(ppDiffSliceMax);
	//	ppDiffSliceMax = NULL;
	//	return MemAllocFail;
	//}
	ppDiffSliceMax = (int16_t **)calloc(SliceMatRow, sizeof(int16_t *));
	if (NULL == ppDiffSliceMax)
		return MemAllocFail;
	ppDiffSliceMin = (int16_t **)calloc(SliceMatRow, sizeof(int16_t *));
	if (NULL == ppDiffSliceMin)
	{
		free(ppDiffSliceMax);
		ppDiffSliceMax = NULL;
		return MemAllocFail;
	}
	for (i = 0; i < SliceMatRow; i++)
	{
		ppDiffSliceMax[i] = (int16_t *)calloc(SliceMatCol, sizeof(int16_t));
		ppDiffSliceMin[i] = (int16_t *)calloc(SliceMatCol, sizeof(int16_t));
		if (NULL == ppDiffSliceMax[i] || NULL == ppDiffSliceMin[i])
		{
			isMallocFailed = 1;
		}
	}
	if (1 == isMallocFailed)
	{
		for (i = 0; i < SliceMatRow; i++)
		{
			if (ppDiffSliceMax[i] != NULL)
				free(ppDiffSliceMax[i]);
			if (ppDiffSliceMin[i] != NULL)
				free(ppDiffSliceMin[i]);
		}
		free(ppDiffSliceMax);
		free(ppDiffSliceMin);
			return MemAllocFail;
	}

	for (j = 0; j < pixelCount; j++)
	{
		ppfTemp[j / col][j % col] = (float)pTouchRawData[j]; //转成矩阵形式  这里只取了第一张图像的原始数据进行饱和判断
	}

	for (i = 0; i < SliceMatRow; i++)
	{
		for (j = 0; j < SliceMatCol; j++)
		{
			min_temp = MIN_TEMP;
			max_temp = MAX_TEMP;

			for (k = (uint16_t)(i * ROW_SLICE); k < (i * ROW_SLICE + ROW_SLICE); k++)
			{
				for (l = (uint16_t)(j * COL_SLICE); l < (j * COL_SLICE + COL_SLICE); l++)
				{
					if (ppDiffValidData[k][l] < min_temp)
						min_temp = ppDiffValidData[k][l];
					if (ppDiffValidData[k][l] > max_temp)
						max_temp = ppDiffValidData[k][l];

					if (ValidMatrix[i][j] == 1)
					{
						if (ppfTemp[k + SliceRowStart][l + SliceColStart]> 3900 || ppfTemp[k + SliceRowStart][l + SliceColStart] < 1)
						{
							ValidMatrix[i][j] = 0;
						}
					}	
				}
			}
			ppDiffSliceMin[i][j] = min_temp;
			ppDiffSliceMax[i][j] = max_temp;
		}
	}

	 temp1 = 0;
	 temp2 = 0;
	//每个切片计算出对其他切片点最大值最小值的欧氏距离
	for (i = 0; i < SliceMatRow; i++) //SliceMatRow
	{
		for (j = 0; j < SliceMatCol; j++) //SliceMatCol
		{
			ppfTemp[i][j] = 0;
			for (k = 0; k < SliceMatRow; k++) //SliceMatRow
			{
				for (l = 0; l < SliceMatCol; l++) //SliceMatCol
				{
					temp1 = ppDiffSliceMax[k][l] - ppDiffSliceMax[i][j];
					temp2 = ppDiffSliceMin[k][l] - ppDiffSliceMin[i][j];
					//ppfTemp[i][j] = ppfTemp[i][j] + cpl_sqrt((temp1*temp1 + temp2*temp2));
					ppfTemp[i][j] = ppfTemp[i][j] + sqrt((temp1*temp1 + temp2*temp2));
				}
			}
		}
	}

	//对欧氏距离进行增量统计，找出增量阈值处对应的欧氏距离大小
	tempEuDistance = (uint32_t *)malloc((int32_t)(SliceMatRow*SliceMatCol)*sizeof(uint32_t));
	if (NULL == tempEuDistance)
	{
		for (i = 0; i < SliceMatRow; i++)
		{
			if (ppDiffSliceMax[i] != NULL)
				free(ppDiffSliceMax[i]);
			if (ppDiffSliceMin[i] != NULL)
				free(ppDiffSliceMin[i]);
		}
		free(ppDiffSliceMax);
		free(ppDiffSliceMin);
		return MemAllocFail;
	}
	for (i = 0; i < SliceMatRow; i++)
	{
		for (j = 0; j < SliceMatCol; j++)
		{
			tempEuDistance[i*SliceMatCol + j] = (uint32_t)ppfTemp[i][j];
		}
	}

	Quick_Sort1(tempEuDistance, 0, SliceMatRow*SliceMatCol - 1);
    tempEuDistanceThr = tempEuDistance[SliceMatRow*SliceMatCol - 1]; //欧氏距离阈值

	for (i = 0; i < SliceMatRow*SliceMatCol-1; i++)
	{
		if ((tempEuDistance[i + 1] - tempEuDistance[i]) > EuDisIncrementThre) //大于欧式距离增量阈值
		{
			tempEuDistanceThr = tempEuDistance[i + 1];
			break;
		}
	}

	for (i = 0; i < SliceMatRow; i++)
	{
		for (j = 0; j < SliceMatCol; j++)
		{
			if (ValidMatrix[i][j] == 1 && ppfTemp[i][j] > tempEuDistanceThr)
			{
				ValidMatrix[i][j] = 0;
			}
		}
	}
	if (NULL != tempEuDistance)
	{
		free(tempEuDistance);
		tempEuDistance = NULL;
	}

	for (i = 0; i < SliceMatRow; i++)
	{
		if (ppDiffSliceMax[i] != NULL)
			free(ppDiffSliceMax[i]);
		if (ppDiffSliceMin[i] != NULL)
			free(ppDiffSliceMin[i]);
	}
	free(ppDiffSliceMax);
	free(ppDiffSliceMin);
	ppDiffSliceMax = NULL;
	ppDiffSliceMin = NULL;
	return 0;
}


//**************************************************信号计算部分**************************************************
SNR_STATIC uint8_t Singal_Cal1(float **ppDiffValidData, uint8_t **ppValidMat, uint16_t SliceMatRow, uint16_t SliceMatCol, SNR_t *pSNR)
{
	uint16_t count = 0;
	float min_temp;
	float max_temp;
	float Signal = 0;
	float signal_1 = 0; //最大的信号量
	float signal_2 = 0; //次最大的信号量
	float signalTemp;

    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t k = 0;
    uint16_t l = 0;
	for (i = 0; i < SliceMatRow; i++)
	{
		for (j = 0; j < SliceMatCol; j++)
		{
			//如果该区域有效则累加信号和
			if (ppValidMat[i][j])
			{
				min_temp = MIN_TEMP;
				max_temp = MAX_TEMP;
				for (k = (uint16_t)(i * ROW_SLICE); k < (i * ROW_SLICE + ROW_SLICE); k++)
				{
					for (l = (uint16_t)(j * COL_SLICE); l < (j * COL_SLICE + COL_SLICE); l++)
					{
						if (ppDiffValidData[k][l] < min_temp)
							min_temp = ppDiffValidData[k][l];
						if (ppDiffValidData[k][l] > max_temp)
							max_temp = ppDiffValidData[k][l];
					}
					signalTemp = max_temp - min_temp; //该片内一行的信号量
					if (k == i*ROW_SLICE)
					{
						signal_1 = signalTemp;
					}
					else if (k == (i*ROW_SLICE + 1))
					{
						signal_2 = signalTemp;
						if (signal_2 > signal_1)
						{
							Swap(signal_1, signal_2);
						}
					}
					else if (signalTemp > signal_1)
					{
						signal_1 = signalTemp;
					}
					else if (signalTemp > signal_2)
					{
						signal_2 = signalTemp;
					}
				}
				count++;
				//Signal += (signal_1 + signal_2) / 2.0;
				Signal += signal_2;
			}
		}
	}

	pSNR->selectPercentage = (float)count / (SliceMatRow * SliceMatCol); //已被选择的片区数与总的片区数之比

	//计算出信号值，信号输出
	if (count != 0)
		pSNR->signal = Signal / (float)count;
	else
		pSNR->signal = 0;

	if (pSNR->selectPercentage > SELECT_PERCENTAGE_THR)
		return 0;
	else
		return DataError;
}

SNR_STATIC void FreeMemory1(float **ppfTemp1, int row_1, float **ppfTemp2, int row_2, uint8_t **ppiTemp, int row_3)
{
	int i;
	for (i = 0; i < row_1; i++)
	{
		if (NULL != ppfTemp1[i])
			free(ppfTemp1[i]);
	}
	free(ppfTemp1);
	ppfTemp1 = NULL;
	for (i = 0; i < row_2; i++)
	{
		if (NULL != ppfTemp2[i])
			free(ppfTemp2[i]);
	}
	free(ppfTemp2);
	ppfTemp2 = NULL;
	for (i = 0; i < row_3; i++)
	{
		if (NULL != ppiTemp[i])
			free(ppiTemp[i]);
	}
	free(ppiTemp);
	ppiTemp = NULL;
}

/// <summary>
/// SNR计算
/// v1.5.1更新：针对三星MILANJ修改 UNTOUCH_DIFF_THRE 500 -> 300,增加touchFrameNum到SNR_Init_t使untouch帧数和touch帧数可分开配置
/// v1.4更新: 增加预留sito功能，传入形参封装成结构体
/// v1.3更新: 大幅优化计算逻辑以及内存占用，提升性能
/// v1.2更新: 增加选择片区数的比例阈值，小于阈值(50%)不计算SNR(此时返回值为1，SNR为0)
/// v1.1更新: 修改接口和算法，校正曲面后才计算欧氏距离，并增加是否按压、是否饱和的综合判断 
/// v1.02更新: 加入噪声计算，函数SNR返回值
/// v1.01更新: 切片宽度改为12pixel, 使用宏定义风格做参数
/// v1.00更新: 首次发布, 带动态切片选取
/// </summary>
/// <param name="pBaseData">base的原始数据(多帧连续)，必须>=30帧</param>
/// <param name="pTouchRawData">touch时的原始数据(多帧连续)，和base的帧数相等</param>
/// <param name="pInit">用于初始化参数的结构体</param>
/// <param name="pSNR">信噪比结构体指针(输出)</param>
///<returns>0表示正常返回；1表示被选片数太少(按压不全或者坏点太多)，无法计算SNR，需要重新采集数据计算;2表示内存分配失败;3表示数据不足30帧;4表示传入空指针</returns>
uint8_t SNR_Cal_F_Series(const uint16_t *pBaseData, const uint16_t *pTouchRawData, const SNR_Init_t *pInit, SNR_t *pSNR)
{
	int i = 0;
	int j = 0;
	uint8_t isMallocFailed = 0;
	SNR_Init_t InitTmp;
	SNR_Init_t *pInitModifyRowCol;
	uint16_t ignoreNum;
	int cutIndex = 0;
	uint16_t maxFrameNum;
	uint16_t *pBaseDataRmOneCycle = NULL;
	uint16_t *pTouchRawDataRmOneCycle = NULL;

	uint16_t SliceMatRow;
	uint16_t SliceMatCol;
	uint16_t AllSlicePixelCol;
	uint16_t AllSlicePixelRow;
	uint8_t **ValidMatrix = NULL;
	float sumTemp;
	uint16_t validRow; //位于切片内的像素点的row
	uint16_t validCol; //位于切片内的像素点的col
	uint8_t ret;

	uint16_t SliceRowStart;  //切片开始的位置的row坐标
	uint16_t SliceRowEnd;  //切片结束的位置的row坐标
	uint16_t SliceColStart; //切片开始的位置的col坐标
	uint16_t SliceColEnd;  //切片结束的位置的col坐标
	uint16_t pixelCount;//像素总数
    uint32_t frameindex;
    uint32_t rowIndex;
    uint32_t colIndex;

	if (NULL == pBaseData || NULL == pTouchRawData || NULL == pInit || NULL == pSNR)
		return NULL_Point;

	InitTmp = *pInit;
	pInitModifyRowCol = &InitTmp;

	if (pInit->frameNum < 30)
		return FrameLess;

	pSNR->signal = 0;
	pSNR->noise = 0;
	pSNR->selectPercentage = 0;

	//去掉周围一圈数据
	ignoreNum = 1;
	cutIndex = 0;
	maxFrameNum = pInitModifyRowCol->frameNum > pInitModifyRowCol->touchFrameNum ? pInitModifyRowCol->frameNum : pInitModifyRowCol->touchFrameNum;
	int FRAME_SIZE = pInitModifyRowCol->col * pInitModifyRowCol->row;

       uint32_t tmp_base_data = (pInitModifyRowCol->col - 2)*(pInitModifyRowCol->row - 2)*pInitModifyRowCol->frameNum;
       uint32_t tmp_touch_data = (pInitModifyRowCol->col - 2)*(pInitModifyRowCol->row - 2)*pInitModifyRowCol->touchFrameNum;
	//pBaseDataRmOneCycle = (uint16_t *)calloc(tmp_base_data, sizeof(uint16_t));
	pBaseDataRmOneCycle = (uint16_t *)calloc(tmp_base_data, sizeof(uint16_t));
	if (NULL == pBaseDataRmOneCycle)
		return MemAllocFail;

	//pTouchRawDataRmOneCycle = (uint16_t *)calloc(tmp_touch_data, sizeof(uint16_t));
	pTouchRawDataRmOneCycle = (uint16_t *)calloc(tmp_touch_data, sizeof(uint16_t));
    if (NULL == pTouchRawDataRmOneCycle)
    {
        free(pBaseDataRmOneCycle);
        return MemAllocFail;
    }
    for (frameindex = 0; frameindex < maxFrameNum; ++frameindex)
    {
        for (rowIndex = ignoreNum; rowIndex < pInitModifyRowCol->row - ignoreNum; ++rowIndex)
        {
            for (colIndex = ignoreNum; colIndex < pInitModifyRowCol->col - ignoreNum; ++colIndex)
			{
				if (frameindex < pInitModifyRowCol->frameNum)
				{
					pBaseDataRmOneCycle[cutIndex] = pBaseData[frameindex*FRAME_SIZE + rowIndex*pInitModifyRowCol->col + colIndex];
				}
				if (frameindex < pInitModifyRowCol->touchFrameNum)
				{
					pTouchRawDataRmOneCycle[cutIndex] = pTouchRawData[frameindex*FRAME_SIZE + rowIndex*pInitModifyRowCol->col + colIndex];
				}
				cutIndex++;
			}

		}
	}
	pInitModifyRowCol->col -= 2 * ignoreNum;
	pInitModifyRowCol->row -= 2 * ignoreNum;

	pixelCount = pInitModifyRowCol->row * pInitModifyRowCol->col; //整个sensor总共的像素点数
	SliceMatRow = (uint16_t)(pInitModifyRowCol->row / ROW_SLICE); //所有片所组成的片与片之间的矩阵，这是指能分的片数(行方向)
	SliceRowStart = (uint16_t)((pInitModifyRowCol->row - (ROW_SLICE * SliceMatRow)) / 2); //位于片矩阵内的像素点起始坐标(行方向)
	SliceRowEnd = (uint16_t)((ROW_SLICE * SliceMatRow) + SliceRowStart); //位于片矩阵内的像素点终点坐标(行方向)

	SliceMatCol = (uint16_t)(pInitModifyRowCol->col / COL_SLICE); //所有片所组成的片与片之间的矩阵，这是指能分的片数(列方向)
	SliceColStart = (uint16_t)((pInitModifyRowCol->col - (COL_SLICE * SliceMatCol)) / 2); //位于片矩阵内的像素点起始坐标(列方向)
	SliceColEnd = (uint16_t)((COL_SLICE * SliceMatCol) + SliceColStart); //位于片矩阵内的像素点终点坐标(列方向)

	AllSlicePixelCol = (uint16_t)(SliceMatCol * COL_SLICE); //全部片的列像素点个数
	AllSlicePixelRow = (uint16_t)(SliceMatRow * ROW_SLICE); //全部片的行像素点个数

	float **ppTempValid = (float **)calloc(pInitModifyRowCol->row, sizeof(float *));
	float **DiffSignalAllSlice = (float **)calloc(AllSlicePixelRow, sizeof(float *));
	ValidMatrix = (uint8_t **)calloc(SliceMatRow, sizeof(uint8_t *));
	if (NULL == ppTempValid || NULL == DiffSignalAllSlice || NULL == ValidMatrix)
	{
		free(pBaseDataRmOneCycle);
		free(pTouchRawDataRmOneCycle);
		if (ppTempValid != NULL)
			free(ppTempValid);
		if (DiffSignalAllSlice != NULL)
			free(DiffSignalAllSlice);
		if (ValidMatrix != NULL)
			free(ValidMatrix);
		return MemAllocFail;
	}	
	for (i = 0; i < pInitModifyRowCol->row; i++)
	{
		ppTempValid[i] = (float *)calloc(pInitModifyRowCol->col, sizeof(float));
		if (NULL == ppTempValid[i])
			isMallocFailed = 1;
	}
	for (i = 0; i < AllSlicePixelRow; i++)
	{
		DiffSignalAllSlice[i] = (float *)calloc(AllSlicePixelCol, sizeof(float));
		if (NULL == DiffSignalAllSlice[i])
			isMallocFailed = 1;
	}
	for (i = 0; i < SliceMatRow; i++)
	{
		ValidMatrix[i] = (uint8_t *)calloc(SliceMatCol, sizeof(uint8_t));
		if (NULL == ValidMatrix[i])
			isMallocFailed = 1;
	}
	if (isMallocFailed)
	{
		//释放内存
		free(pBaseDataRmOneCycle);
		free(pTouchRawDataRmOneCycle);
		FreeMemory1(ppTempValid, pInitModifyRowCol->row, DiffSignalAllSlice, AllSlicePixelRow, ValidMatrix, SliceMatRow);
		return MemAllocFail;
	}

	for (i = 0; i < SliceMatRow; i++)
	{
		for (j = 0; j < SliceMatCol; j++)
		{
			ValidMatrix[i][j] = 1; //开始时设定全部片选择
		}
	}

	sumTemp = 0;
	//对输入信号作帧平均
	for (j = 0; j < pixelCount; j++)
	{
		sumTemp = 0;
		for (i = 0; i < pInitModifyRowCol->touchFrameNum; i++)
		{
			sumTemp = sumTemp + (float)(pBaseDataRmOneCycle[i*pixelCount + j] - pTouchRawDataRmOneCycle[i*pixelCount + j] + OFFSET); //+4096保证为正数
		}
		validRow = j / pInitModifyRowCol->col;
		validCol = j % pInitModifyRowCol->col;
		if (validRow >= SliceRowStart  && validRow < SliceRowEnd && validCol >= SliceColStart && validCol < SliceColEnd)
		{
			validRow = validRow - SliceRowStart;
			validCol = validCol - SliceColStart;
			sumTemp = sumTemp / pInitModifyRowCol->touchFrameNum;
			DiffSignalAllSlice[validRow][validCol] = sumTemp; //整理为矩阵
			if (!pInitModifyRowCol->isSaturationTcode)
			{
				if (sumTemp < (UNTOUCH_DIFF_THRE + OFFSET))  //一旦出现diff小于阈值的情况
				{
					ValidMatrix[validRow / ROW_SLICE][validCol / COL_SLICE] = 0;
				}
			}
		}
	}

	
	Noise_Cal1(pBaseDataRmOneCycle, pixelCount, pInitModifyRowCol->col, pInitModifyRowCol->frameNum, pSNR);
	Middle_Surface_Fliter1(DiffSignalAllSlice, AllSlicePixelRow, AllSlicePixelCol, ppTempValid);
	ret = JudgeValidByEuDistance1(DiffSignalAllSlice, pTouchRawDataRmOneCycle, ppTempValid, ValidMatrix, SliceMatRow, SliceMatCol, pInitModifyRowCol->col,pixelCount,SliceRowStart,SliceColStart);
	if (ret)
	{
		//释放内存
		free(pBaseDataRmOneCycle);
		free(pTouchRawDataRmOneCycle);
		FreeMemory1(ppTempValid, pInitModifyRowCol->row, DiffSignalAllSlice, AllSlicePixelRow, ValidMatrix, SliceMatRow);
		return ret;
	}
	ret = Singal_Cal1(DiffSignalAllSlice, ValidMatrix, SliceMatRow, SliceMatCol, pSNR);
	//释放内存
	free(pBaseDataRmOneCycle);
	free(pTouchRawDataRmOneCycle);
	FreeMemory1(ppTempValid, pInitModifyRowCol->row, DiffSignalAllSlice, AllSlicePixelRow, ValidMatrix, SliceMatRow);
	
	//SNR输出
	if (pSNR->noise > FLT_EPSILON && pSNR->selectPercentage > SELECT_PERCENTAGE_THR)
		pSNR->snr = pSNR->signal / pSNR->noise;
	else 
		pSNR->snr = 0;

	return ret;
}



