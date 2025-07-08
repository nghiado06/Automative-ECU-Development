/*******************************************************************************
 * @file    Pwm_Hw.h
 * @author  Do Duc Nghia
 * @brief   Hardware Abstraction Layer for PWM Driver
 * @details File này khai báo các hàm phần cứng dùng để điều khiển Timer, cấu hình
 *          prescaler, polarity, trạng thái idle, độ lệch pha và chân GPIO cho các
 *          kênh PWM. Mục tiêu là tách biệt logic phần cứng khỏi driver chính.
 *
 * @version 1.0
 * @date    2025-07-07
 *******************************************************************************/

#ifndef PWM_HW_H
#define PWM_HW_H

/*====================================== [INCLUDE LIBRARY] =======================================*/
#include "Std_Types.h"
#include "Pwm.h"

/*******************************************************************************
 * @brief   Kích hoạt clock cho Timer
 * @details Bật nguồn clock cho Timer tương ứng với kênh PWM đang sử dụng.
 *
 * @param   Timer   Con trỏ đến Timer (TIM2, TIM3, TIM4...)
 *******************************************************************************/
void Pwm_Hw_EnableClock(TIM_TypeDef *Timer);

/*******************************************************************************
 * @brief   Cấu hình Prescaler cho Timer
 * @details Thiết lập tần số đếm cho Timer bằng cách chia tần số clock đầu vào.
 *
 * @param   Timer       Con trỏ đến Timer tương ứng
 * @param   prescaler   Giá trị chia tần số theo kiểu Pwm_PrescalerType
 *******************************************************************************/
void Pwm_Hw_SetPrescaler(TIM_TypeDef *Timer, Pwm_PrescalerType prescaler);

/*******************************************************************************
 * @brief   Cấu hình Polarity (đầu ra HIGH hay LOW trước)
 * @details Thiết lập chiều của xung PWM: HIGH trước rồi xuống LOW, hoặc ngược lại.
 *
 * @param   Timer      Con trỏ đến Timer tương ứng
 * @param   channel    Số kênh PWM (1–4)
 * @param   polarity   Kiểu Pwm_PolarityType (PWM_POLARITY_HIGH hoặc LOW)
 *******************************************************************************/
void Pwm_Hw_SetPolarity(TIM_TypeDef *Timer, uint8_t channel, Pwm_PolarityType polarity);

/*******************************************************************************
 * @brief   Cấu hình độ lệch pha cho kênh PWM
 * @details Dùng để tạo độ trễ pha giữa các kênh PWM khác nhau. Hiện chưa triển khai.
 *
 * @param   ch      ID của kênh PWM
 * @param   phase   Giá trị độ lệch pha (đơn vị tick)
 *******************************************************************************/
void Pwm_Hw_SetPhase(Pwm_ChannelType ch, Pwm_PhaseShiftType phase);

/*******************************************************************************
 * @brief   Đặt kênh PWM về trạng thái Idle
 * @details Ép mức logic của PWM về mức LOW hoặc HIGH tùy theo IdleState cấu hình.
 *
 * @param   Timer      Con trỏ đến Timer tương ứng
 * @param   channel    Số kênh PWM (1–4)
 * @param   state      Trạng thái đầu ra mong muốn (PWM_LOW hoặc PWM_HIGH)
 *******************************************************************************/
void Pwm_Hw_ForceIdle(TIM_TypeDef *Timer, uint8_t channel, Pwm_OutputStateType state);

/*******************************************************************************
 * @brief   Cấu hình GPIO cho kênh PWM
 * @details Thiết lập chân GPIO liên quan đến kênh PWM, bao gồm chế độ alternate.
 *
 * @param   ch     ID của kênh PWM
 *******************************************************************************/
void Pwm_Hw_ConfigGpio(Pwm_ChannelType ch);

#endif