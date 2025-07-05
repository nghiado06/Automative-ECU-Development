/*******************************************************************************
 * @file    Adc_Cfg.h
 * @author  Do Duc Nghia
 * @brief   ADC Driver Configuration Header File
 * @details File này khai báo các cấu hình compile-time và link-time cho ADC Driver,
 *          bao gồm khai báo danh sách channel, cấu hình group ADC, và cấu hình mặc định
 *          dùng cho hàm khởi tạo (Adc_Init) và hủy khởi tạo (Adc_DeInit).
 *
 *          Đây là phần cấu hình tĩnh (Link-time Configuration) của module ADC theo chuẩn AUTOSAR,
 *          thường được định nghĩa trong file Adc_Lcfg.c tương ứng.
 *
 * @version 1.0
 * @date    2025-06-28
 *******************************************************************************/

#ifndef ADC_CFG_H
#define ADC_CFG_H

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Adc_Types.h"
#include "stm32f10x_adc.h"

#define ADC_MAX_GROUPS 10
#define ADC_MAX_SAMPLES 5

/*******************************************************************************
 * @brief   Danh sách channel giả dùng cho cấu hình mặc định
 ******************************************************************************/
extern const Adc_ChannelType DummyChannels[1];

/*******************************************************************************
 * @brief   Danh sách channel thực tế của group
 ******************************************************************************/
extern const Adc_ChannelType channels[2];

/*******************************************************************************
 * @brief   Cấu hình group ADC thực tế sử dụng
 ******************************************************************************/
extern const Adc_GroupConfigType AdcGroupConfigs[ADC_MAX_GROUPS];

/*******************************************************************************
 * @brief   Cấu hình tổng ADC thực tế sử dụng
 ******************************************************************************/
extern const Adc_ConfigType Adc_Config;

/*******************************************************************************
 * @brief   Cấu hình mặc định dùng để DeInit (tắt ADC, reset trạng thái)
 ******************************************************************************/
extern const Adc_ConfigType Adc_ConfigDefault;

#endif
