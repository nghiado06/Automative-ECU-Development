/*******************************************************************************
 * @file    Pwm_Hw.c
 * @author  Do Duc Nghia
 * @brief   Hardware Abstraction Layer for PWM Driver
 * @details File này định nghĩa các hàm phần cứng dùng để điều khiển Timer,
 *          cấu hình prescaler, polarity, trạng thái idle, độ lệch pha và chân GPIO
 *          cho các kênh PWM. Mục tiêu là tách biệt logic phần cứng khỏi driver chính.
 *
 * @version 1.0
 * @date    2025-07-07
 *******************************************************************************/

/*====================================== [INCLUDE LIBRARY] =======================================*/
#include "Pwm_Hw.h"

/*******************************************************************************
 * @brief   Kích hoạt clock cho Timer
 * @details Bật nguồn clock cho Timer tương ứng với kênh PWM đang sử dụng.
 *
 * @param   Timer   Con trỏ đến Timer (TIM2, TIM3, TIM4...)
 *******************************************************************************/
void Pwm_Hw_EnableClock(TIM_TypeDef *Timer)
{
    if (Timer == TIM2)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    }
    else if (Timer == TIM3)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    }
    else if (Timer == TIM4)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    }
}

/*******************************************************************************
 * @brief   Cấu hình Prescaler cho Timer
 * @details Thiết lập tần số đếm cho Timer bằng cách chia tần số clock đầu vào.
 *
 * @param   Timer       Con trỏ đến Timer tương ứng
 * @param   prescaler   Giá trị chia tần số theo kiểu Pwm_PrescalerType
 *******************************************************************************/

void Pwm_Hw_SetPrescaler(TIM_TypeDef *Timer, Pwm_PrescalerType prescaler)
{
    uint16_t psc = 0;
    switch (prescaler)
    {
    case PWM_PRESCALER_DIV_1:
        psc = 0;
        break;
    case PWM_PRESCALER_DIV_2:
        psc = 1;
        break;
    case PWM_PRESCALER_DIV_4:
        psc = 3;
        break;
    case PWM_PRESCALER_DIV_8:
        psc = 7;
        break;
    case PWM_PRESCALER_DIV_16:
        psc = 15;
        break;
    case PWM_PRESCALER_DIV_64:
        psc = 63;
        break;
    case PWM_PRESCALER_DIV_256:
        psc = 255;
        break;
    case PWM_PRESCALER_DIV_1024:
        psc = 1023;
        break;
    }

    TIM_PrescalerConfig(Timer, psc, TIM_PSCReloadMode_Immediate);
}

/*******************************************************************************
 * @brief   Cấu hình Polarity (đầu ra HIGH hay LOW trước)
 * @details Thiết lập chiều của xung PWM: HIGH trước rồi xuống LOW, hoặc ngược lại.
 *
 * @param   Timer      Con trỏ đến Timer tương ứng
 * @param   channel    Số kênh PWM (1–4)
 * @param   polarity   Kiểu Pwm_PolarityType (PWM_POLARITY_HIGH hoặc LOW)
 *******************************************************************************/

void Pwm_Hw_SetPolarity(TIM_TypeDef *Timer, uint8_t channel, Pwm_PolarityType polarity)
{
    TIM_OCInitTypeDef oc;
    TIM_OCStructInit(&oc);

    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = 0;
    oc.TIM_OCPolarity = (polarity == PWM_POLARITY_HIGH) ? TIM_OCPolarity_High : TIM_OCPolarity_Low;

    switch (channel)
    {
    case 1:
        TIM_OC1Init(Timer, &oc);
        TIM_OC1PreloadConfig(Timer, ENABLE);
        break;
    case 2:
        TIM_OC2Init(Timer, &oc);
        TIM_OC2PreloadConfig(Timer, ENABLE);
        break;
    case 3:
        TIM_OC3Init(Timer, &oc);
        TIM_OC3PreloadConfig(Timer, ENABLE);
        break;
    case 4:
        TIM_OC4Init(Timer, &oc);
        TIM_OC4PreloadConfig(Timer, ENABLE);
        break;
    default:
        // Không hợp lệ → bỏ qua hoặc log lỗi
        break;
    }
}

/*******************************************************************************
 * @brief   Cấu hình GPIO cho kênh PWM
 * @details Thiết lập chân GPIO liên quan đến kênh PWM, bao gồm chế độ alternate.
 *
 * @param   ch     ID của kênh PWM
 *******************************************************************************/
void Pwm_Hw_ConfigGpio(Pwm_ChannelType ch)
{
}

/*******************************************************************************
 * @brief   Cấu hình độ lệch pha cho kênh PWM
 * @details Dùng để tạo độ trễ pha giữa các kênh PWM khác nhau. Hiện chưa triển khai.
 *
 * @param   ch      ID của kênh PWM
 * @param   phase   Giá trị độ lệch pha (đơn vị tick)
 *******************************************************************************/
void Pwm_Hw_SetPhase(Pwm_ChannelType ch, Pwm_PhaseShiftType phase)
{
}

/*******************************************************************************
 * @brief   Đặt kênh PWM về trạng thái Idle
 * @details Ép mức logic của PWM về mức LOW hoặc HIGH tùy theo IdleState cấu hình.
 *
 * @param   Timer      Con trỏ đến Timer tương ứng
 * @param   channel    Số kênh PWM (1–4)
 * @param   state      Trạng thái đầu ra mong muốn (PWM_LOW hoặc PWM_HIGH)
 *******************************************************************************/
void Pwm_Hw_ForceIdle(TIM_TypeDef *Timer, uint8_t channel, Pwm_OutputStateType state)
{
    if (state == PWM_HIGH)
    {
        switch (channel)
        {
        case 1:
            TIM_SetCompare1(Timer, PWM_MAX_DUTY_CYCLE);
            break;
        case 2:
            TIM_SetCompare2(Timer, PWM_MAX_DUTY_CYCLE);
            break;
        case 3:
            TIM_SetCompare3(Timer, PWM_MAX_DUTY_CYCLE);
            break;
        case 4:
            TIM_SetCompare4(Timer, PWM_MAX_DUTY_CYCLE);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (channel)
        {
        case 1:
            TIM_SetCompare1(Timer, 0);
            break;
        case 2:
            TIM_SetCompare2(Timer, 0);
            break;
        case 3:
            TIM_SetCompare3(Timer, 0);
            break;
        case 4:
            TIM_SetCompare4(Timer, 0);
            break;
        default:
            break;
        }
    }
}
