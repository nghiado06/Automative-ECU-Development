/*******************************************************************************
 * @file    Port.c
 * @author  Do Duc Nghia
 * @brief   Port Driver Source File
 * @details File này định nghĩa các Port Driver API
 *
 * @version 1.0
 * @date    2025-06-25
 * *****************************************************************************
 */

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Port.h"
#include "Port_Cfg.h"
#include <stm32f10x_rcc.h>

/********************************************************************
 * ================= [ Static / Internal Variables ] ===================
 * @brief   Đặt biến trạng thái xác định Port đã init hay chưa
 ********************************************************************/
static uint8_t Port_Initialized = 0;

/********************************************************************
 * ================= [ INTERNAL SUPPORT FUNCTION ] ==================
 * @brief   Hàm giúp hỗ trợ việc cấu hình từng chân
 ********************************************************************/

/********************************************************************
 * @brief   Định nghĩa hàm Port_ApplyPinConfig
 * @details Hàm giúp cấu hình từng chân riêng lẻ
 *
 * @param   ConfigPtr        Con trỏ trỏ đến chân cần cấu hình
 * ******************************************************************
 */
void Port_ApplyPinConfig(const Port_PinConfigType *ConfigPtr)
{
    /* Cấp xung clock */
    switch (ConfigPtr->PortNum)
    {
    case PORT_A:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        break;
    case PORT_B:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        break;
    case PORT_C:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        break;
    case PORT_D:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
        break;
    default:
        break;
    }

    /* Lấy con trỏ GPIOx tương ứng từ PortNum */
    GPIO_TypeDef *GPIOx;
    GPIOx = Port_GetGPIOx(ConfigPtr->PortNum);

    /* Cấu hình Chân */
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = PORT_GET_PIN(ConfigPtr->PinNum);
    gpio.GPIO_Speed = ConfigPtr->Speed;

    /* Phân tích trường hợp Mode */

    switch (ConfigPtr->Mode)
    {
    case PORT_PIN_MODE_DIO:
        switch (ConfigPtr->Direction)
        {
        case PORT_PIN_IN:
            if (ConfigPtr->Pull == PORT_PULL_UP)
            {
                gpio.GPIO_Mode = GPIO_Mode_IPU;
            }
            else if (ConfigPtr->Pull == PORT_PULL_DOWN)
            {
                gpio.GPIO_Mode = GPIO_Mode_IPD;
            }
            else if (ConfigPtr->Pull == PORT_PULL_NONE)
            {
                gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            }
            break;
        case PORT_PIN_OUT:
            if (ConfigPtr->OutputType == PORT_OUTPUT_PUSHPULL)
            {
                gpio.GPIO_Mode = GPIO_Mode_Out_PP;
            }
            else if (ConfigPtr->OutputType == PORT_OUTPUT_OPENDRAIN)
            {
                gpio.GPIO_Mode = GPIO_Mode_Out_OD;
            }
            break;
            /*Extra Mode...*/
        }
        break;

    case PORT_PIN_MODE_ADC:
        gpio.GPIO_Mode = GPIO_Mode_AIN;
        break;

    default:
        break;
    }

    /* Cấu hình GPIO */
    GPIO_Init(GPIOx, &gpio);

    /* Nếu là output, gán mức logic ban đầu */
    if (ConfigPtr->Direction == PORT_PIN_OUT)
    {
        if (ConfigPtr->Level == PORT_PIN_LEVEL_HIGH)
            GPIO_SetBits(GPIOx, gpio.GPIO_Pin);
        else
            GPIO_ResetBits(GPIOx, gpio.GPIO_Pin);
    }
}

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
void Port_Init(const Port_ConfigType *ConfigPtr)
{
    if (ConfigPtr == NULL_PTR)
        return;

    for (uint16_t i = 0; i < ConfigPtr->NumPins; i++)
    {
        Port_ApplyPinConfig(&ConfigPtr->PinConfigArray[i]);
    }
    Port_Initialized = 1;
}

/********************************************************************
 * @brief   Định nghĩa hàm Port_SetPinDirection
 * @details Hàm giúp xác định chiều của chân trong runtime
 *
 * @param   Pin              Phần tử chân cần đổi chiều
 * @param   Direction        Chiều muốn đổi
 * ******************************************************************
 */
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction)
{
    if (!Port_Initialized)
        return;
    if (Pin >= Port_Config.NumPins)
        return;
    if (!pinConfigs[Pin].DirectionChangeable)
        return;

    Port_PinConfigType *pinCfg = (Port_PinConfigType *)&pinConfigs[Pin];
    pinCfg->Direction = Direction;
    Port_ApplyPinConfig(pinCfg);
}

/********************************************************************
 * @brief   Định nghĩa hàm Port_RefreshPortDirection
 * @details Hàm giúp đưa các chiều của các chân về trạng thái mặc định
 *          đã cấu hình sẵn
 * ******************************************************************
 */
void Port_RefreshPortDirection(void)
{
    if (!Port_Initialized)
        return;
    for (uint16_t i = 0; i < Port_Config.NumPins; i++)
    {
        if (!pinConfigs[i].DirectionChangeable)
        {
            Port_ApplyPinConfig(&pinConfigs[i]);
        }
    }
}

/********************************************************************
 * @brief   Định nghĩa hàm Port_GetVersionInfo
 * @details Hàm lấy thông tin phiên bản
 *
 * @param   VersionInfo   Thông tin phiên bản
 * ******************************************************************
 */
void Port_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
    if (versioninfo == NULL_PTR)
        return;
    versioninfo->vendorID = PORT_VENDOR_ID;
    versioninfo->moduleID = PORT_MODULE_ID;
    versioninfo->sw_major_version = PORT_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = PORT_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = PORT_SW_PATCH_VERSION;
}

/********************************************************************
 * @brief   Định nghĩa hàm Port_SetPinMode
 * @details Hàm giúp xác định Mode của chân trong runtime
 *
 * @param   Pin              Phần tử chân cần đổi mode
 * @param   Mode             Mode muốn đổi
 * ******************************************************************
 */
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
    if (!Port_Initialized)
        return;
    if (Pin >= Port_Config.NumPins)
        return;
    if (!pinConfigs[Pin].ModeChangeable)
        return;

    Port_PinConfigType *pinCfg = (Port_PinConfigType *)&pinConfigs[Pin];
    pinCfg->Mode = Mode;
    Port_ApplyPinConfig(pinCfg);
}