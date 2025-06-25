#ifndef PORT_H
#define PORT_H

#include <stm32f10x_gpio.h>
#include "Std_Types.h"

#define PORT_VENDOR_ID 1234U
#define PORT_MODULE_ID 81U
#define PORT_SW_MAJOR_VERSION 1U
#define PORT_SW_MINOR_VERSION 0U
#define PORT_SW_PATCH_VERSION 0U

/* ======================= [ MACRO PORT IDs ] ======================= */
#define PORT_A 0
#define PORT_B 1
#define PORT_C 2
#define PORT_D 3

/* ======================= [ MACRO GET PIN MASK ] =================== */
#define PORT_GET_PIN(PinNum) ((uint16)(1U << (PinNum)))

/* ======================= [ TYPE DEFINITIONS ] ===================== */
typedef uint8 Port_PinType;
typedef uint8 Port_PortType;

/* Chế độ pin (mode chức năng) */
typedef enum
{
    PORT_PIN_MODE_DIO = 0,
    PORT_PIN_MODE_ADC,
    PORT_PIN_MODE_SPI,
    PORT_PIN_MODE_CAN,
    PORT_PIN_MODE_LIN
} Port_PinModeType;

/* Hướng pin */
typedef enum
{
    PORT_PIN_IN = 0,
    PORT_PIN_OUT
} Port_PinDirectionType;

/* Mức logic mặc định */
typedef enum
{
    PORT_PIN_LEVEL_LOW = STD_LOW,
    PORT_PIN_LEVEL_HIGH = STD_HIGH
} Port_PinLevelType;

/* Kiểu kéo trong chế độ input */
typedef enum
{
    PORT_PULL_NONE = 0,
    PORT_PULL_UP,
    PORT_PULL_DOWN
} Port_PinPullType;

/* Kiểu output: push-pull / open-drain */
typedef enum
{
    PORT_OUTPUT_PUSHPULL = 0,
    PORT_OUTPUT_OPENDRAIN
} Port_OutputType;

/* Tốc độ đầu ra */
typedef enum
{
    PORT_SPEED_2MHZ = GPIO_Speed_2MHz,
    PORT_SPEED_10MHZ = GPIO_Speed_10MHz,
    PORT_SPEED_50MHZ = GPIO_Speed_50MHz
} Port_PinSpeedLevel;

/* ======================= [ STRUCTURES ] =========================== */
/* Cấu hình cho từng chân pin */
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

/* Cấu hình tổng toàn bộ các pin */
typedef struct
{
    const Port_PinConfigType *PinConfigArray;
    uint8 NumPins;
} Port_ConfigType;

/* ======================= [ INLINE HELPERS ] ======================= */
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

/* ======================= [ API PROTOTYPES ] ======================= */
void Port_Init(const Port_ConfigType *ConfigPtr);
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction);
void Port_RefreshPortDirection(void);
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode);
void Port_GetVersionInfo(Std_VersionInfoType *versioninfo);

#endif