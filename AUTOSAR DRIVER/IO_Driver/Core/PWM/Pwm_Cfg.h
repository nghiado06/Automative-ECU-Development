/*******************************************************************************
 * @file    Pwm_Cfg.h
 * @author  Do Duc Nghia
 * @brief   Header cấu hình PWM cho từng kênh
 * @details File này khai báo số lượng kênh PWM, các callback và biến cấu hình
 *          toàn cục được sử dụng trong `Pwm_Lcfg.c` và `Pwm.c`.
 *
 * @version 1.0
 * @date    2025-07-07
 *******************************************************************************/

#ifndef PWM_CFG_H
#define PWM_CFG_H

/*====================================== [INCLUDE LIBRARY] =======================================*/
#include "Pwm.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/*******************************************************************************
 * @brief   Số lượng kênh PWM tối đa trong hệ thống
 *******************************************************************************/
#define MAX_CHANNELS 3

/*******************************************************************************
 * @brief   Callback cho điều khiển tốc độ quạt
 *******************************************************************************/
void FanSpeed_Callback(void);

/*******************************************************************************
 * @brief   Callback cho điều khiển LED nhấp nháy
 *******************************************************************************/
void LedBlink_Callback(void);

/*******************************************************************************
 * @brief   Mảng cấu hình cho các kênh PWM
 * @note    Được định nghĩa trong file `Pwm_Lcfg.c`
 *******************************************************************************/
const Pwm_ChannelConfigType PwmChannelConfigs[MAX_CHANNELS];

/*******************************************************************************
 * @brief   Cấu hình tổng thể cho PWM Driver
 * @note    Bao gồm danh sách các kênh và số lượng kênh, dùng trong `Pwm_Init`
 *******************************************************************************/
const Pwm_ConfigType PwmConfig;

#endif