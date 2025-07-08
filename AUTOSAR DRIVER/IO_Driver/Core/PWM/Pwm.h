/*******************************************************************************
 * @file    Pwm.h
 * @author  Do Duc Nghia
 * @brief   PWM Driver Configuration Header File
 * @details File này khai báo các cấu hình compile-time và link-time cho PWM Driver,
 *          bao gồm khai báo danh sách channel, cấu hình group PWM, và cấu hình mặc
 *          định dùng cho hàm khởi tạo (Pwm_Init) và hủy khởi tạo (Pwm_DeInit).
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
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/*========================================== [MACROS] ============================================*/

#define PWM_MAX_DUTY_CYCLE ((Pwm_ValueType)0x8000U)                            /**< Giá trị tối đa của duty cycle */
#define PWM_DUTY_100_PERCENT PWM_MAX_DUTY_CYCLE                                /**< Duty cycle 100% tương ứng với giá trị tối đa */
#define PWM_DUTY_75_PERCENT ((Pwm_ValueType)(PWM_MAX_DUTY_CYCLE * 75U / 100U)) /**< Duty cycle 75% */
#define PWM_DUTY_50_PERCENT ((Pwm_ValueType)(PWM_MAX_DUTY_CYCLE / 2U))         /**< Duty cycle 50% */
#define PWM_DUTY_25_PERCENT ((Pwm_ValueType)(PWM_MAX_DUTY_CYCLE / 4U))         /**< Duty cycle 25% */
#define PWM_DUTY_0_PERCENT ((Pwm_ValueType)0U)                                 /**< Duty cycle 0% tương ứng với giá trị 0 */

/*===================================== [ EXTENSIVE TYPES ] ======================================*/

/******************************************************************************
 * @typedef Pwm_NotificationCallbackType
 * @brief   Kiểu con trỏ hàm cho callback khi có sự kiện cạnh PWM.
 *
 * @details Callback này sẽ được gọi khi một cạnh (rising/falling/both)
 *          xuất hiện trên kênh PWM tương ứng (nếu được cấu hình và kích hoạt).
 ******************************************************************************/
typedef void (*Pwm_NotificationCallbackType)(void);

/******************************************************************************
 * @enum    Pwm_PrescalerType
 * @brief   Xác định tỉ lệ chia xung clock đầu vào cho Timer PWM.
 *
 * @details Giá trị này được sử dụng để giảm tần số đầu ra PWM bằng cách chia
 *          xung clock nội bộ của timer. Tuỳ thuộc phần cứng mà các hệ số chia
 *          được hỗ trợ sẽ khác nhau.
 ******************************************************************************/
typedef enum
{
    PWM_PRESCALER_DIV_1,   /**< Không chia, giữ nguyên clock */
    PWM_PRESCALER_DIV_2,   /**< Chia 2 */
    PWM_PRESCALER_DIV_4,   /**< Chia 4 */
    PWM_PRESCALER_DIV_8,   /**< Chia 8 */
    PWM_PRESCALER_DIV_16,  /**< Chia 16 */
    PWM_PRESCALER_DIV_64,  /**< Chia 64 */
    PWM_PRESCALER_DIV_256, /**< Chia 256 */
    PWM_PRESCALER_DIV_1024 /**< Chia 1024 */
} Pwm_PrescalerType;

/********************************************************************************
 * @typedef Pwm_PhaseShiftType
 * @brief   Xác định độ lệch pha (trễ) của kênh PWM tính theo đơn vị tick.
 *
 * @details Được sử dụng với kiểu kênh PWM_FIXED_PERIOD_SHIFTED để tạo ra các xung
 *          PWM lệch pha nhau (ví dụ trong điều khiển động cơ 3 pha).
 *******************************************************************************/
typedef uint16 Pwm_PhaseShiftType;

/********************************************************************************
 * @enum    Pwm_PolarityType
 * @brief   Xác định chiều xung của PWM – bắt đầu từ mức cao hay thấp.
 *
 * @details Điều này quyết định cạnh đầu tiên của xung PWM là cạnh lên hay cạnh xuống.
 *          Có thể dùng để đảo logic nếu phần cứng yêu cầu.
 *******************************************************************************/
typedef enum
{
    PWM_POLARITY_HIGH, /**< Xung bắt đầu bằng mức HIGH → xuống LOW */
    PWM_POLARITY_LOW   /**< Xung bắt đầu bằng mức LOW → lên HIGH */
} Pwm_PolarityType;

/*================================= [ AUTOSAR TYPE DEFINITIONS ] ======================================*/

/******************************************************************************
 * @typedef Pwm_ChannelType
 * @brief   Kiểu định danh cho từng kênh PWM.
 *
 * @details Mỗi channel ID tương ứng với một kênh vật lý hoặc logic của PWM.
 ******************************************************************************/
typedef uint8 Pwm_ChannelType;

/*******************************************************************************
 * @typedef Pwm_ValueType
 * @brief   Kiểu biểu diễn độ rộng xung PWM (Duty Cycle).
 *
 * @details Duty cycle được scale từ 0 đến PWM_MAX_DUTY_CYCLE (0x8000 = 100%).
 ******************************************************************************/
typedef uint16 Pwm_ValueType; /* Type for PWM value */

/*******************************************************************************
 * @typedef Pwm_PeriodType
 * @brief   Kiểu biểu diễn chu kỳ xung PWM (Period).
 *
 * @details Đơn vị có thể là tick hoặc micro giây, tùy cách cấu hình Timer.
 ******************************************************************************/
typedef uint16 Pwm_PeriodType; /* Type for PWM period */

/******************************************************************************
 * @enum    Pwm_OutputStateType
 * @brief   Xác định mức logic đầu ra khi kênh PWM tắt (idle).
 *
 * @details Dùng để đặt trạng thái chân PWM sau khi dừng hoạt động.
 ******************************************************************************/
typedef enum
{
    PWM_LOW,
    PWM_HIGH
} Pwm_OutputStateType;

/******************************************************************************
 * @enum    Pwm_EdgeNotificationType
 * @brief   Xác định loại cạnh nào sẽ gọi callback nếu được bật.
 ******************************************************************************/
typedef enum
{
    PWM_RISING_EDGE,  /**< Callback khi có cạnh lên (LOW sang HIGH) */
    PWM_FALLING_EDGE, /**< Callback khi có cạnh xuống (HIGH sang LOW) */
    PWM_BOTH_EDGES    /**< Callback khi có cả 2 loại cạnh */
} Pwm_EdgeNotificationType;

/******************************************************************************
 * @enum    Pwm_ChannelClassType
 * @brief   Xác định kiểu hoạt động của kênh PWM.
 ******************************************************************************/
typedef enum
{
    PWM_VARIABLE_PERIOD,     /**< Có thể thay đổi duty & period */
    PWM_FIXED_PERIOD,        /**< Chỉ thay đổi duty (chu kỳ cố định) */
    PWM_FIXED_PERIOD_SHIFTED /**< Chu kỳ cố định và hỗ trợ lệch pha */
} Pwm_ChannelClassType;

/******************************************************************************
 * @struct  Pwm_ChannelConfigType
 * @brief   Cấu trúc cấu hình cho từng kênh PWM
 * @details Mỗi kênh PWM tương ứng với một Timer channel trên vi điều khiển.
 *          Cấu trúc này chứa toàn bộ thông tin cấu hình phần cứng và logic
 *          điều khiển phần mềm, bao gồm kiểu hoạt động, chiều xung, thông số
 *          prescaler, độ lệch pha, cũng như hàm callback cho cạnh PWM.
 *
 * ChannelId              ID định danh kênh PWM (ví dụ 0, 1, 2...)
 * TimerInstance          Con trỏ tới instance của Timer (ví dụ TIM2, TIM3...)
 * TimerChannel           Số kênh của Timer (ví dụ TIM_CHANNEL_1, TIM_CHANNEL_2...)
 * PwmChannelClass        Loại hoạt động: VARIABLE_PERIOD / FIXED_PERIOD / SHIFTED
 * IdleState              Mức logic đầu ra khi kênh bị tắt (LOW hoặc HIGH)
 * NotificationType       Loại cạnh kích hoạt callback (rising / falling / both)
 * Polarity               Chiều xung PWM bắt đầu từ HIGH hay LOW
 * Prescaler              Bộ chia xung clock timer (1, 2, 4, ..., 1024)
 * PhaseShift             Độ lệch pha (theo tick), dùng nếu kiểu SHIFTED
 * NotificationCallback   Hàm callback được gọi khi có cạnh xảy ra
 ******************************************************************************/
typedef struct
{
    Pwm_ChannelType ChannelId;
    TIM_TypeDef *TimerInstance;
    uint8 TimerChannel;

    Pwm_ChannelClassType PwmChannelClass;
    Pwm_OutputStateType IdleState;
    Pwm_EdgeNotificationType NotificationType;
    Pwm_PolarityType Polarity;

    Pwm_PrescalerType Prescaler;
    Pwm_PhaseShiftType PhaseShift;
    Pwm_NotificationCallbackType NotificationCallback;
} Pwm_ChannelConfigType;

/******************************************************************************
 * @struct  Pwm_ConfigType
 * @brief   Cấu trúc tổng thể chứa cấu hình cho toàn bộ PWM Driver.
 ******************************************************************************/
typedef struct
{
    const Pwm_ChannelConfigType *ChannelConfigs; /**< Mảng cấu hình kênh */
    uint8 NumChannels;                           /**< Số lượng kênh PWM được cấu hình */
} Pwm_ConfigType;

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