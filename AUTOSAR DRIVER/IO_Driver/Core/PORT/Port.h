/*******************************************************************************
 * @file    Port.h
 * @author  Do Duc Nghia
 * @brief   Port Driver Source File
 * @details File này định nghĩa các Port Driver API
 *
 * @version 1.0
 * @date    2025-06-25
 * *****************************************************************************
 */
#ifndef PORT_H
#define PORT_H

#include <stm32f10x_gpio.h>
#include "Std_Types.h"

#define PORT_VENDOR_ID 1234U
#define PORT_MODULE_ID 81U
#define PORT_SW_MAJOR_VERSION 1U
#define PORT_SW_MINOR_VERSION 0U
#define PORT_SW_PATCH_VERSION 0U

/************************************************************************
 * ==================== [ MACRO PORT DEFINITIONS ] ======================
 * @brief   Khai báo các port bằng macro
 ************************************************************************/
#define PORT_A 0
#define PORT_B 1
#define PORT_C 2
#define PORT_D 3

/************************************************************************
 * ==================== [ PORT TYPE DEFINITIONS ] =======================
 * @brief   Định nghĩa các Type trong Port Driver theo tiêu chuẩn AUTOSAR
 ************************************************************************/

/************************************************************************
 * @brief   Định nghĩa kiểu dữ liệu khai báo số chân ứng với port
 ************************************************************************/
typedef uint8 Port_PinType;

/************************************************************************
 * @brief   Định nghĩa kiểu dữ liệu khai báo số port
 ************************************************************************/
typedef uint8 Port_PortType;

/************************************************************************
 * @brief   Định nghĩa các mode của chân
 * @details Khai báo các mode, giao thức của chân (GPIO, ADC, CAN, SPI)
 ************************************************************************/
typedef enum
{
    PORT_PIN_MODE_DIO = 0,
    PORT_PIN_MODE_ADC,
    PORT_PIN_MODE_SPI,
    PORT_PIN_MODE_CAN,
    PORT_PIN_MODE_LIN
} Port_PinModeType;

/************************************************************************
 * @brief   Định nghĩa mode in hoặc output của chân
 ************************************************************************/
typedef enum
{
    PORT_PIN_IN = 0,
    PORT_PIN_OUT
} Port_PinDirectionType;

/************************************************************************
 * @brief   Định nghĩa mức logic của chân
 ************************************************************************/
typedef enum
{
    PORT_PIN_LEVEL_LOW = STD_LOW,
    PORT_PIN_LEVEL_HIGH = STD_HIGH
} Port_PinLevelType;

/************************************************************************
 * @brief   Định nghĩa các chế độ input của chân
 ************************************************************************/
typedef enum
{
    PORT_PULL_NONE = 0,
    PORT_PULL_UP,
    PORT_PULL_DOWN
} Port_PinPullType;

/************************************************************************
 * @brief   Định nghĩa các chế độ output của chân
 ************************************************************************/
typedef enum
{
    PORT_OUTPUT_PUSHPULL = 0,
    PORT_OUTPUT_OPENDRAIN
} Port_OutputType;

/************************************************************************
 * @brief   Định nghĩa các mức độ tốc độ đầu ra của chân
 ************************************************************************/
typedef enum
{
    PORT_SPEED_2MHZ = GPIO_Speed_2MHz,
    PORT_SPEED_10MHZ = GPIO_Speed_10MHz,
    PORT_SPEED_50MHZ = GPIO_Speed_50MHz
} Port_PinSpeedLevel;

/************************************************************************
 * @brief   Định nghĩa các cấu hình của một chân
 ************************************************************************/
typedef struct
{
    Port_PortType PortNum;           // A = 0, B = 1, ...
    Port_PinType PinNum;             // 0..15
    Port_PinModeType Mode;           // GPIO / AF / ANALOG / ...
    Port_PinDirectionType Direction; // IN / OUT
    Port_PinPullType Pull;           // Pull-up/down
    Port_PinLevelType Level;         // HIGH / LOW (áp dụng cho output)
    Port_OutputType OutputType;      // PP / OD
    Port_PinSpeedLevel Speed;        // 2MHz / 10MHz / 50MHz

    boolean DirectionChangeable; // Có cho phép thay đổi hướng trong runtime không?
    boolean ModeChangeable;      // Có cho phép thay đổi mode trong runtime không?
} Port_PinConfigType;

/************************************************************************
 * @brief   Cấu hình tổng toàn bộ các pin
 ************************************************************************/
typedef struct
{
    const Port_PinConfigType *PinConfigArray;
    uint8 NumPins;
} Port_ConfigType;

/********************************************************************
 * ==================== [ INTERNAL HELPERS ] ========================
 * @brief   Các hàm và macro hỗ trợ hoạt động
 ********************************************************************/

/********************************************************************
 * @brief   Định nghĩa hàm Port_GetGPIOx
 * @details Hàm giúp xác định Port dựa trên NumPort
 *
 * @param   Port            Port cần xác định
 * ******************************************************************
 */
/* Lấy con trỏ GPIOx từ PortNum */
static inline GPIO_TypeDef *Port_GetGPIOx(Port_PortType port)
{
    switch (port)
    {
    case PORT_A:
        return GPIOA;
    case PORT_B:
        return GPIOB;
    case PORT_C:
        return GPIOC;
    case PORT_D:
        return GPIOD;
    default:
        return NULL_PTR;
    }
}

/********************************************************************
 * @brief   Macro giúp xác định Pin dựa trên số pin đầu vào
 *
 * @param   PinNum              Số Pin cần xác định
 * ******************************************************************
 */
#define PORT_GET_PIN(PinNum) ((uint16)(1U << (PinNum)))

/********************************************************************
 * ================= [ API FUNCTION DEFINITIONS ] ===================
 * @brief   Định nghĩa các hàm API điều khiển theo chuẩn AUTOSAR
 ********************************************************************/

/********************************************************************
 * @brief   Định nghĩa hàm Port_Init
 * @details Hàm giúp cấu hình một mảng các chân đã được set trước
 *
 * @param   ConfigPtr        Con trỏ trỏ đến mảng chân cần cấu hình
 * ******************************************************************
 */
void Port_Init(const Port_ConfigType *ConfigPtr);

/********************************************************************
 * @brief   Định nghĩa hàm Port_SetPinDirection
 * @details Hàm giúp xác định chiều của chân trong runtime
 *
 * @param   Pin              Phần tử chân cần đổi chiều
 * @param   Direction        Chiều muốn đổi
 * ******************************************************************
 */
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction);

/********************************************************************
 * @brief   Định nghĩa hàm Port_RefreshPortDirection
 * @details Hàm giúp đưa các chiều của các chân về trạng thái mặc định
 *          đã cấu hình sẵn
 * ******************************************************************
 */
void Port_RefreshPortDirection(void);

/********************************************************************
 * @brief   Định nghĩa hàm Port_GetVersionInfo
 * @details Hàm lấy thông tin phiên bản
 *
 * @param   VersionInfo   Thông tin phiên bản
 * ******************************************************************
 */
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode);

/********************************************************************
 * @brief   Định nghĩa hàm Port_SetPinMode
 * @details Hàm giúp xác định Mode của chân trong runtime
 *
 * @param   Pin              Phần tử chân cần đổi mode
 * @param   Mode             Mode muốn đổi
 * ******************************************************************
 */
void Port_GetVersionInfo(Std_VersionInfoType *versioninfo);

#endif