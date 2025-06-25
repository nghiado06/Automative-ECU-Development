/*******************************************************************************
 * @file    Port_Cfg.c
 * @author  Do Duc Nghia
 * @brief   Port Config Source File
 * @details File này cấu hình cụ thể cho port của vi điều khiển STM32F10x
 *
 * @version 1.0
 * @date    2025-06-25
 * *****************************************************************************
 */

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Port_Cfg.h"

/**********************************************************
 * Cấu hình chi tiết cho từng chân GPIO
 * - PortNum:   PORT_ID_A, PORT_ID_B, PORT_ID_C, ...
 * - PinNum:    Số thứ tự chân trong port (0-15)
 * - Mode:      PORT_PIN_MODE_DIO, ...
 * - Direction: PORT_PIN_OUT / PORT_PIN_IN
 * - DirectionChangeable: 1 = Cho phép đổi chiều runtime
 * - Level:     PORT_PIN_LEVEL_HIGH / PORT_PIN_LEVEL_LOW
 * - Pull:      PORT_PIN_PULL_NONE / UP / DOWN
 * - ModeChangeable: 1 = Cho phép đổi mode runtime
 **********************************************************/

/********************************************************************
 * @brief   Định nghĩa cụ thể cấu hình các chân
 ********************************************************************/
const Port_PinConfigType pinConfigs[NUM_PINS] = {

    /* PC13: DIO, Output, Không được phép đổi mode runtime, Push-pull */
    {
        .PortNum = PORT_C,
        .PinNum = 13,
        .Mode = PORT_PIN_MODE_DIO,
        .Direction = PORT_PIN_OUT,
        .DirectionChangeable = 0,
        .Level = PORT_PIN_LEVEL_LOW,
        .Pull = PORT_PULL_NONE,
        .OutputType = PORT_OUTPUT_PUSHPULL,
        .Speed = PORT_SPEED_50MHZ,
        .ModeChangeable = 0}
    /* ... Thêm cấu hình cho các chân khác tại đây ... */
};

/********************************************************************
 * @brief   Xác định cấu hình port chung
 ********************************************************************/
const Port_ConfigType Port_Config = {
    .PinConfigArray = pinConfigs,
    .NumPins = NUM_PINS};