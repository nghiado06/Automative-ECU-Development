/*******************************************************************************
 * @file    Pwm.c
 * @author  Do Duc Nghia
 * @brief   PWM Driver Configuration Source File
 * @details File này định nghĩa các cấu hình compile-time và link-time cho PWM Driver,
 *          bao gồm định nghĩa danh sách channel, cấu hình group PWM, và cấu hình mặc
 *          định dùng cho hàm khởi tạo (Pwm_Init) và hủy khởi tạo (Pwm_DeInit).
 *
 *          Đây là phần cấu hình tĩnh (Link-time Configuration) của module PWM theo
 *          chuẩn AUTOSAR, thường được định nghĩa trong file Pwm_Lcfg.c tương ứng.
 *
 * @version 1.0
 * @date    2025-07-07
 *******************************************************************************/

/*================================================ [ INCLUDE LIBRARY ] ===============================================*/
#include "Pwm.h"

/*=============================================== [ MACRO DEFINITIONS ] ==============================================*/
/*******************************************************************************
 * @brief   Lấy giá trị Auto-Reload (ARR) của Timer
 * @param   TIMx    Timer cần đọc ARR
 * @return  Giá trị ARR hiện tại
 *******************************************************************************/
#define TIM_GetAutoreload(TIMx) ((TIMx)->ARR)

/*******************************************************************************
 * @brief   Thiết lập giá trị Auto-Reload (ARR) cho Timer
 * @param   TIMx    Timer cần cấu hình
 * @param   val     Giá trị ARR mới
 *******************************************************************************/
#define TIM_SetAutoreload(TIMx, val) ((TIMx)->ARR = (val))

/*============================================== [ EXTENSIVE VARIABLES ] =============================================*/
/*******************************************************************************
 * @brief   Con trỏ đến cấu hình PWM hiện tại
 * @details Biến toàn cục nội bộ, được gán khi gọi Pwm_Init() và dùng lại trong
 *          toàn bộ các hàm điều khiển PWM để truy cập thông tin kênh, timer,
 *          trạng thái, callback,...
 *******************************************************************************/
static const Pwm_ConfigType *Pwm_ConfigPtr = NULL_PTR;

/*================================================== [ ISR HANDLER ] ==================================================*/
/*******************************************************************************
 * @brief   Hàm xử lý ngắt cho Timer 2 (TIM2)
 * @details Xử lý ngắt Update (CNT = ARR) và Compare Match (CNT = CCRx) cho các
 *          kênh PWM đang sử dụng TIM2. Gọi callback tương ứng nếu được cấu hình.
 *******************************************************************************/
void TIM2_IRQHandler(void)
{
    for (uint8_t i = 0; i < Pwm_ConfigPtr->NumChannels; ++i)
    {
        const Pwm_ChannelConfigType *config = &Pwm_ConfigPtr->ChannelConfigs[i];

        if (config->TimerInstance != TIM2)
            continue; // Bỏ qua channel không dùng TIM2

        // --- Xử lý ngắt Update (CNT == ARR) ---
        if (TIM_GetITStatus(TIM2, TIM_IT_Update))
        {
            // Nếu là cạnh được notify tại thời điểm CNT = 0
            if ((config->Polarity == PWM_POLARITY_HIGH &&
                 config->NotificationType != PWM_FALLING_EDGE) ||
                (config->Polarity == PWM_POLARITY_LOW &&
                 config->NotificationType != PWM_RISING_EDGE))
            {
                if (config->NotificationCallback)
                    config->NotificationCallback();
            }
        }

        // --- Xử lý ngắt Compare Match (CNT == CCRx) ---
        if (TIM_GetITStatus(TIM2, TIM_IT_CC1) && config->TimerChannel == 1)
        {
            if ((config->Polarity == PWM_POLARITY_HIGH &&
                 config->NotificationType != PWM_RISING_EDGE) ||
                (config->Polarity == PWM_POLARITY_LOW &&
                 config->NotificationType != PWM_FALLING_EDGE))
            {
                if (config->NotificationCallback)
                    config->NotificationCallback();
            }
        }
        else if (TIM_GetITStatus(TIM2, TIM_IT_CC2) && config->TimerChannel == 2)
        {
            if ((config->Polarity == PWM_POLARITY_HIGH &&
                 config->NotificationType != PWM_RISING_EDGE) ||
                (config->Polarity == PWM_POLARITY_LOW &&
                 config->NotificationType != PWM_FALLING_EDGE))
            {
                if (config->NotificationCallback)
                    config->NotificationCallback();
            }
        }
        else if (TIM_GetITStatus(TIM2, TIM_IT_CC3) && config->TimerChannel == 3)
        {
            if ((config->Polarity == PWM_POLARITY_HIGH &&
                 config->NotificationType != PWM_RISING_EDGE) ||
                (config->Polarity == PWM_POLARITY_LOW &&
                 config->NotificationType != PWM_FALLING_EDGE))
            {
                if (config->NotificationCallback)
                    config->NotificationCallback();
            }
        }
        else if (TIM_GetITStatus(TIM2, TIM_IT_CC4) && config->TimerChannel == 4)
        {
            if ((config->Polarity == PWM_POLARITY_HIGH &&
                 config->NotificationType != PWM_RISING_EDGE) ||
                (config->Polarity == PWM_POLARITY_LOW &&
                 config->NotificationType != PWM_FALLING_EDGE))
            {
                if (config->NotificationCallback)
                    config->NotificationCallback();
            }
        }
    }

    // Cuối ISR: Clear cờ ngắt chung
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4);
}

/*====================================== [ FUNCTION DEFINITIONS ] ========================================*/

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
void Pwm_Init(const Pwm_ConfigType *ConfigPtr)
{
    if (ConfigPtr == NULL_PTR || ConfigPtr->ChannelConfigs == NULL_PTR)
        return;

    Pwm_ConfigPtr = ConfigPtr;

    for (uint8 i = 0; i < ConfigPtr->NumChannels; ++i)
    {
        const Pwm_ChannelConfigType *chCfg = &ConfigPtr->ChannelConfigs[i];

        TIM_TypeDef *Timer = chCfg->TimerInstance;
        uint8_t Channel = chCfg->TimerChannel;

        // 1. Enable Clock cho Timer tương ứng
        Pwm_Hw_EnableClock(Timer);

        // 2. Cấu hình prescaler
        Pwm_Hw_SetPrescaler(Timer, chCfg->Prescaler);

        // 3. Cấu hình polarity
        Pwm_Hw_SetPolarity(Timer, Channel, chCfg->Polarity);

        // 4. Cấu hình trạng thái idle ban đầu
        Pwm_Hw_ForceIdle(Timer, Channel, chCfg->IdleState);

        // 5. Cấu hình GPIO
        Pwm_Hw_ConfigGpio(chCfg->ChannelId);

        // 6. Cấu hình độ lệch pha
        if (chCfg->PwmChannelClass == PWM_FIXED_PERIOD_SHIFTED)
        {
            Pwm_Hw_SetPhase(chCfg->ChannelId, chCfg->PhaseShift);
        }
    }
}

/*******************************************************************************
 * @brief   Hủy khởi tạo PWM Driver
 * @details Dừng toàn bộ hoạt động PWM, đưa các chân PWM về trạng thái Idle nếu
 *          có cấu hình. Sau khi gọi hàm này, tất cả các hàm khác không nên được
 *          gọi cho đến khi Init lại.
 *******************************************************************************/
void Pwm_DeInit(void)
{
    if (Pwm_ConfigPtr == NULL_PTR || Pwm_ConfigPtr->ChannelConfigs == NULL_PTR)
        return;

    for (uint8 i = 0; i < Pwm_ConfigPtr->NumChannels; ++i)
    {
        const Pwm_ChannelConfigType *chCfg = &Pwm_ConfigPtr->ChannelConfigs[i];
        TIM_TypeDef *Timer = chCfg->TimerInstance;
        uint8_t Channel = chCfg->TimerChannel;

        // 1. Dừng phát xung, đưa về trạng thái Idle (mức LOW hoặc HIGH)
        Pwm_Hw_ForceIdle(Timer, Channel, chCfg->IdleState);

        // 2. Tắt xung ra trên channel (tùy theo channel)
        switch (Channel)
        {
        case 1:
            TIM_CCxCmd(Timer, TIM_Channel_1, TIM_CCx_Disable);
            break;
        case 2:
            TIM_CCxCmd(Timer, TIM_Channel_2, TIM_CCx_Disable);
            break;
        case 3:
            TIM_CCxCmd(Timer, TIM_Channel_3, TIM_CCx_Disable);
            break;
        case 4:
            TIM_CCxCmd(Timer, TIM_Channel_4, TIM_CCx_Disable);
            break;
        default:
            break;
        }
    }

    // 3. Xóa cấu hình đang dùng
    Pwm_ConfigPtr = NULL_PTR;
}

/*=================================== [ OUTPUT CONTROLLING FUNCTIONS ] ===================================*/
/*******************************************************************************
 * @brief   Thay đổi duty cycle của một kênh PWM
 * @details Áp dụng cho cả kênh có chu kỳ cố định (FIXED_PERIOD) hoặc lệch pha.
 *          Duty cycle được scale từ 0 đến PWM_MAX_DUTY_CYCLE.
 *
 * @param   ChannelNumber   ID của kênh PWM cần thay đổi duty
 * @param   DutyCycle       Giá trị duty cycle mới (0 – PWM_MAX_DUTY_CYCLE)
 *******************************************************************************/
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, Pwm_ValueType DutyCycle)
{
    if (Pwm_ConfigPtr == NULL_PTR || ChannelNumber >= Pwm_ConfigPtr->NumChannels)
        return;

    const Pwm_ChannelConfigType *chCfg = &Pwm_ConfigPtr->ChannelConfigs[ChannelNumber];
    uint16_t period = TIM_GetAutoreload(chCfg->TimerInstance);

    // Tính toán giá trị CCR mới (theo duty scale 0 – PWM_MAX_DUTY_CYCLE)
    uint16_t compare = (DutyCycle * period) / PWM_MAX_DUTY_CYCLE;

    // Nếu kiểu kênh là SHIFTED → có thể cộng thêm PhaseShift vào compare (nếu muốn tạo delay)
    if (chCfg->PwmChannelClass == PWM_FIXED_PERIOD_SHIFTED)
    {
        compare += chCfg->PhaseShift;
        if (compare > period)
        {
            compare -= period; // wrap-around để không vượt period
        }
    }

    switch (chCfg->TimerChannel)
    {
    case 1:
        TIM_SetCompare1(chCfg->TimerInstance, compare);
        break;
    case 2:
        TIM_SetCompare2(chCfg->TimerInstance, compare);
        break;
    case 3:
        TIM_SetCompare3(chCfg->TimerInstance, compare);
        break;
    case 4:
        TIM_SetCompare4(chCfg->TimerInstance, compare);
        break;
    }
}

/*******************************************************************************
 * @brief   Thay đổi chu kỳ và duty cycle của kênh PWM
 * @details Chỉ áp dụng cho kênh có class là PWM_VARIABLE_PERIOD. Giúp thay đổi
 *          cả tần số và độ rộng xung trong runtime.
 *
 * @param   ChannelNumber   ID của kênh PWM
 * @param   Period          Chu kỳ mới (đơn vị tick)
 * @param   DutyCycle       Giá trị duty cycle tương ứng với chu kỳ
 *******************************************************************************/
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, Pwm_ValueType DutyCycle)
{
    if (Pwm_ConfigPtr == NULL_PTR || ChannelNumber >= Pwm_ConfigPtr->NumChannels)
        return;

    const Pwm_ChannelConfigType *chCfg = &Pwm_ConfigPtr->ChannelConfigs[ChannelNumber];

    if (chCfg->PwmChannelClass != PWM_VARIABLE_PERIOD)
        return;

    TIM_TypeDef *timer = chCfg->TimerInstance;
    uint8_t channel = chCfg->TimerChannel;

    if (Period == 0 || Period > 0xFFFF)
        return;

    // Cập nhật chu kỳ mới (ARR)
    TIM_SetAutoreload(timer, Period);

    // Giới hạn duty cycle nếu cần
    if (DutyCycle > PWM_MAX_DUTY_CYCLE)
        DutyCycle = PWM_MAX_DUTY_CYCLE;

    uint16_t compareValue = (DutyCycle * Period) / PWM_MAX_DUTY_CYCLE;

    // Cập nhật duty mới
    switch (channel)
    {
    case 1:
        TIM_SetCompare1(timer, compareValue);
        break;
    case 2:
        TIM_SetCompare2(timer, compareValue);
        break;
    case 3:
        TIM_SetCompare3(timer, compareValue);
        break;
    case 4:
        TIM_SetCompare4(timer, compareValue);
        break;
    default:
        break;
    }
}

/*******************************************************************************
 * @brief   Đặt chân PWM về trạng thái idle
 * @details Dừng tạo xung PWM bằng cách đưa duty về 0 và đưa mức logic đầu ra
 *          về trạng thái Idle.
 *
 * @param   ChannelNumber   ID của kênh PWM cần set về idle
 *******************************************************************************/
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber)
{
    if (ChannelNumber >= Pwm_ConfigPtr->NumChannels)
        return;

    const Pwm_ChannelConfigType *config = &Pwm_ConfigPtr->ChannelConfigs[ChannelNumber];

    // Ngừng sinh PWM bằng cách set duty về 0
    switch (config->TimerChannel)
    {
    case 1:
        TIM_SetCompare1(config->TimerInstance, 0);
        break;
    case 2:
        TIM_SetCompare2(config->TimerInstance, 0);
        break;
    case 3:
        TIM_SetCompare3(config->TimerInstance, 0);
        break;
    case 4:
        TIM_SetCompare4(config->TimerInstance, 0);
        break;
    default:
        return;
    }

    // Ép mức logic ra chân về trạng thái Idle
    Pwm_Hw_ForceIdle(config->TimerInstance, config->TimerChannel, config->IdleState);
}

/*====================================== [ NOTIFICATION FUNCTIONS ] ======================================*/

/*******************************************************************************
 * @brief   Bật thông báo cạnh cho kênh PWM
 * @details Khi có cạnh xảy ra trên kênh PWM, hàm callback sẽ được gọi nếu được cấu hình.
 *          Hàm này cần được gọi sau khi Init nếu muốn dùng callback.
 *
 * @param   ChannelNumber   ID của kênh PWM
 * @param   Notification    Loại cạnh cần kích hoạt thông báo (rising/falling/both)
 *******************************************************************************/
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, Pwm_EdgeNotificationType Notification)
{
    const Pwm_ChannelConfigType *config = &Pwm_ConfigPtr->ChannelConfigs[ChannelNumber];

    // Cập nhật loại cạnh muốn thông báo
    ((Pwm_ChannelConfigType *)config)->NotificationType = Notification;

    // Bật interrupt trên Timer tương ứng
    TIM_TypeDef *timer = config->TimerInstance;
    uint8_t channel = config->TimerChannel;

    // Bật ngắt Update để bắt cạnh đầu chu kỳ (CNT = ARR)
    TIM_ITConfig(timer, TIM_IT_Update, ENABLE);

    // Bật ngắt CCx để bắt cạnh kết thúc xung (CNT = CCRx)
    switch (channel)
    {
    case 1:
        TIM_ITConfig(timer, TIM_IT_CC1, ENABLE);
        break;
    case 2:
        TIM_ITConfig(timer, TIM_IT_CC2, ENABLE);
        break;
    case 3:
        TIM_ITConfig(timer, TIM_IT_CC3, ENABLE);
        break;
    case 4:
        TIM_ITConfig(timer, TIM_IT_CC4, ENABLE);
        break;
    default:
        break;
    }

    if (timer == TIM2)
    {
        NVIC_EnableIRQ(TIM2_IRQn);
    }
    else if (timer == TIM3)
    {
        NVIC_EnableIRQ(TIM3_IRQn);
    }
    else if (timer == TIM4)
    {
        NVIC_EnableIRQ(TIM4_IRQn);
    }
}

/*******************************************************************************
 * @brief   Tắt thông báo cạnh cho kênh PWM
 * @details Ngừng gọi callback khi có cạnh xảy ra trên kênh PWM.
 *
 * @param   ChannelNumber   ID của kênh PWM
 *******************************************************************************/
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber)
{
    const Pwm_ChannelConfigType *config = &Pwm_ConfigPtr->ChannelConfigs[ChannelNumber];
    TIM_TypeDef *timer = config->TimerInstance;
    uint8_t channel = config->TimerChannel;

    // Tắt ngắt theo loại cạnh đã đăng ký
    switch (config->NotificationType)
    {
    case PWM_RISING_EDGE:
    case PWM_FALLING_EDGE:
        // Cạnh đơn → dùng 1 trong 2 ngắt
        if ((config->Polarity == PWM_POLARITY_HIGH && config->NotificationType == PWM_RISING_EDGE) ||
            (config->Polarity == PWM_POLARITY_LOW && config->NotificationType == PWM_FALLING_EDGE))
        {
            // Tắt ngắt Update
            TIM_ITConfig(timer, TIM_IT_Update, DISABLE);
        }
        else
        {
            // Tắt ngắt CCx tương ứng
            if (channel == 1)
                TIM_ITConfig(timer, TIM_IT_CC1, DISABLE);
            else if (channel == 2)
                TIM_ITConfig(timer, TIM_IT_CC2, DISABLE);
            else if (channel == 3)
                TIM_ITConfig(timer, TIM_IT_CC3, DISABLE);
            else if (channel == 4)
                TIM_ITConfig(timer, TIM_IT_CC4, DISABLE);
        }
        break;

    case PWM_BOTH_EDGES:
        // Tắt cả 2 loại ngắt
        TIM_ITConfig(timer, TIM_IT_Update, DISABLE);
        if (channel == 1)
            TIM_ITConfig(timer, TIM_IT_CC1, DISABLE);
        else if (channel == 2)
            TIM_ITConfig(timer, TIM_IT_CC2, DISABLE);
        else if (channel == 3)
            TIM_ITConfig(timer, TIM_IT_CC3, DISABLE);
        else if (channel == 4)
            TIM_ITConfig(timer, TIM_IT_CC4, DISABLE);
        break;

    default:
        break;
    }
}

/*========================================= [ STATUS FUNCTIONS ] =========================================*/

/*******************************************************************************
 * @brief   Lấy trạng thái logic hiện tại của chân PWM
 * @details Hàm này trả về trạng thái mức logic hiện tại (PWM_HIGH hoặc PWM_LOW).
 *          Kết quả phụ thuộc vào trạng thái chân vật lý và có thể khác nhau tùy MCU.
 *
 * @param   ChannelNumber   ID của kênh PWM
 * @return     Trạng thái hiện tại của chân PWM: PWM_HIGH hoặc PWM_LOW
 *******************************************************************************/
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber)
{
    const Pwm_ChannelConfigType *config = &Pwm_ConfigPtr->ChannelConfigs[ChannelNumber];
    GPIO_TypeDef *gpioPort;
    uint16_t gpioPin;

    // Giả định trước, sẽ cấu hình ánh xạ sau
    switch (ChannelNumber)
    {
    case 0:
        gpioPort = GPIOA;
        gpioPin = GPIO_Pin_0;
        break;
    case 1:
        gpioPort = GPIOA;
        gpioPin = GPIO_Pin_1;
        break;
    case 2:
        gpioPort = GPIOA;
        gpioPin = GPIO_Pin_2;
        break;
    case 3:
        gpioPort = GPIOA;
        gpioPin = GPIO_Pin_3;
        break;
    default:
        return PWM_LOW;
    }

    /* Đọc mức logic hiện tại của chân GPIO */
    if (GPIO_ReadOutputDataBit(gpioPort, gpioPin) == Bit_SET)
        return PWM_HIGH;
    else
        return PWM_LOW;
}