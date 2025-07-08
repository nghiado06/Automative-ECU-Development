/*******************************************************************************
 * @file    Pwm_Lcfg.c
 * @author  Do Duc Nghia
 * @brief   File cấu hình PWM - Link-time Configuration
 * @details Cấu hình các kênh PWM bao gồm Timer, chế độ hoạt động, prescaler,
 *          polarity, trạng thái idle, độ lệch pha và hàm callback nếu có.
 *
 * @version 1.0
 * @date    2025-07-07
 *******************************************************************************/

/*====================================== [INCLUDE LIBRARY] =======================================*/
#include "Pwm_Cfg.h"

/*========================= [ STATIC CALLBACKS ] ==========================*/

/*******************************************************************************
 * @brief   Hàm Callback cho Fan Speed control
 * @details Hàm này được gọi khi có cạnh PWM được kích hoạt cho kênh quạt.
 *******************************************************************************/
void FanSpeed_Callback(void)
{
    // Giả định chưa có nội dung cụ thể
}

/*******************************************************************************
 * @brief   Hàm Callback cho LED Blink control
 * @details Hàm này được gọi khi có cạnh PWM được kích hoạt cho kênh LED.
 *******************************************************************************/
void LedBlink_Callback(void)
{
    // Giả định chưa có nội dung cụ thể
}

/*======================= [ CHANNEL CONFIG LIST ] ========================*/

/*******************************************************************************
 * @brief   Danh sách cấu hình các kênh PWM
 * @details Mỗi phần tử trong mảng này tương ứng với một kênh PWM cụ thể, bao gồm
 *          thông tin Timer sử dụng, chế độ hoạt động, prescaler, polarity,
 *          callback và các tham số khác.
 *******************************************************************************/
const Pwm_ChannelConfigType PwmChannelConfigs[MAX_CHANNELS] = {
    /* Kênh 0: Quạt DC */
    {
        .ChannelId = 0,
        .TimerChannel = TIM_Channel_1,
        .TimerInstance = TIM2, // Sử dụng TIM2 cho kênh
        .PwmChannelClass = PWM_FIXED_PERIOD,
        .IdleState = PWM_LOW,
        .NotificationType = PWM_RISING_EDGE,
        .Polarity = PWM_POLARITY_HIGH,
        .Prescaler = PWM_PRESCALER_DIV_8,
        .PhaseShift = 0,
        .NotificationCallback = FanSpeed_Callback},

    /* Kênh 1: LED blink PWM */
    {
        .ChannelId = 1,
        .TimerInstance = TIM2,
        .TimerChannel = TIM_Channel_2,
        .PwmChannelClass = PWM_VARIABLE_PERIOD,
        .IdleState = PWM_LOW,
        .NotificationType = PWM_BOTH_EDGES,
        .Polarity = PWM_POLARITY_HIGH,
        .Prescaler = PWM_PRESCALER_DIV_64,
        .PhaseShift = 0,
        .NotificationCallback = LedBlink_Callback},

    /* Kênh 2: Motor lệch pha */
    {
        .ChannelId = 2,
        .TimerInstance = TIM2,
        .TimerChannel = TIM_Channel_3,
        .PwmChannelClass = PWM_FIXED_PERIOD_SHIFTED,
        .IdleState = PWM_LOW,
        .NotificationType = PWM_FALLING_EDGE,
        .Polarity = PWM_POLARITY_HIGH,
        .Prescaler = PWM_PRESCALER_DIV_4,
        .PhaseShift = 300,
        .NotificationCallback = NULL_PTR}};

/*============================== [ CONFIG STRUCT ] ===========================*/

/*******************************************************************************
 * @brief   Cấu hình tổng cho PWM Driver
 * @details Trỏ tới mảng các kênh PWM và số lượng kênh đang được sử dụng. Cấu hình
 *          này sẽ được truyền vào hàm `Pwm_Init()` trong lúc khởi tạo.
 *******************************************************************************/
const Pwm_ConfigType PwmConfig = {
    .ChannelConfigs = PwmChannelConfigs,
    .NumChannels = MAX_CHANNELS};
