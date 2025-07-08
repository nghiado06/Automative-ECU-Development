/*******************************************************************************
 * @file    Pwm.h
 * @author  Do Duc Nghia
 * @brief   PWM Driver Configuration Header File
 * @details File này khai báo các cấu hình compile-time và link-time cho PWM Driver,
 *          cấu hình mặc định dùng cho hàm khởi tạo (Pwm_Init) và hủy khởi tạo (Pwm_DeInit).
 *
 *          Đây là phần cấu hình tĩnh (Link-time Configuration) của module PWM theo
 *          chuẩn AUTOSAR, thường được định nghĩa trong file Pwm_Lcfg.c tương ứng.
 *
 * @version 1.0
 * @date    2025-07-07
 *******************************************************************************/

#ifndef PWM_H
#define PWM_H

/*====================================== [INCLUDE LIBRARY] =======================================*/
#include "Std_Types.h"
#include "Pwm_Cfg.h"
#include "Pwm_Hw.h"
#include "Pwm_Types.h"

/*======================================= [ FUNCTION PROTOTYPES ] ========================================*/

/*==================================== [ INITIALIZATION FUNCTIONS ] ======================================*/

/*******************************************************************************
 * @brief   Khởi tạo PWM Driver
 * @details Hàm này sẽ khởi tạo tất cả các kênh PWM theo cấu hình tĩnh được
 *          cung cấp trong tham số `ConfigPtr`. Bao gồm setup timer, chân GPIO,
 *          prescaler, polarity và trạng thái ban đầu của từng kênh.
 *
 * @param   ConfigPtr    Con trỏ trỏ tới cấu trúc Pwm_ConfigType chứa thông tin
 *                       cấu hình.
 *******************************************************************************/
void Pwm_Init(const Pwm_ConfigType *ConfigPtr);

/*******************************************************************************
 * @brief   Hủy khởi tạo PWM Driver
 * @details Dừng toàn bộ hoạt động PWM, đưa các chân PWM về trạng thái Idle nếu
 *          có cấu hình. Sau khi gọi hàm này, tất cả các hàm khác không nên được
 *          gọi cho đến khi Init lại.
 *******************************************************************************/
void Pwm_DeInit(void);

/*=================================== [ OUTPUT CONTROLLING FUNCTIONS ] ===================================*/
/*******************************************************************************
 * @brief   Thay đổi duty cycle của một kênh PWM
 * @details Áp dụng cho cả kênh có chu kỳ cố định (FIXED_PERIOD) hoặc lệch pha.
 *          Duty cycle được scale từ 0 đến PWM_MAX_DUTY_CYCLE.
 *
 * @param   ChannelNumber   ID của kênh PWM cần thay đổi duty
 * @param   DutyCycle       Giá trị duty cycle mới (0 – PWM_MAX_DUTY_CYCLE)
 *******************************************************************************/
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, Pwm_ValueType DutyCycle);

/*******************************************************************************
 * @brief   Thay đổi chu kỳ và duty cycle của kênh PWM
 * @details Chỉ áp dụng cho kênh có class là PWM_VARIABLE_PERIOD. Giúp thay đổi
 *          cả tần số và độ rộng xung trong runtime.
 *
 * @param   ChannelNumber   ID của kênh PWM
 * @param   Period          Chu kỳ mới (đơn vị tick)
 * @param   DutyCycle       Giá trị duty cycle tương ứng với chu kỳ
 *******************************************************************************/
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, Pwm_ValueType DutyCycle);

/*******************************************************************************
 * @brief   Đặt chân PWM về trạng thái idle
 * @details Dừng tạo xung PWM bằng cách đưa duty về 0 và đưa mức logic đầu ra
 *          về trạng thái Idle.
 *
 * @param   ChannelNumber   ID của kênh PWM cần set về idle
 *******************************************************************************/
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber);

/*====================================== [ NOTIFICATION FUNCTIONS ] ======================================*/

/*******************************************************************************
 * @brief   Bật thông báo cạnh cho kênh PWM
 * @details Khi có cạnh xảy ra trên kênh PWM, hàm callback sẽ được gọi nếu được cấu hình.
 *          Hàm này cần được gọi sau khi Init nếu muốn dùng callback.
 *
 * @param   ChannelNumber   ID của kênh PWM
 * @param   Notification    Loại cạnh cần kích hoạt thông báo (rising/falling/both)
 *******************************************************************************/
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, Pwm_EdgeNotificationType Notification);

/*******************************************************************************
 * @brief   Tắt thông báo cạnh cho kênh PWM
 * @details Ngừng gọi callback khi có cạnh xảy ra trên kênh PWM.
 *
 * @param   ChannelNumber   ID của kênh PWM
 *******************************************************************************/
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber);

/*========================================= [ STATUS FUNCTIONS ] =========================================*/

/*******************************************************************************
 * @brief   Lấy trạng thái logic hiện tại của chân PWM
 * @details Hàm này trả về trạng thái mức logic hiện tại (PWM_HIGH hoặc PWM_LOW).
 *          Kết quả phụ thuộc vào trạng thái chân vật lý và có thể khác nhau tùy MCU.
 *
 * @param   ChannelNumber   ID của kênh PWM
 * @return     Trạng thái hiện tại của chân PWM: PWM_HIGH hoặc PWM_LOW
 *******************************************************************************/
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber);

#endif