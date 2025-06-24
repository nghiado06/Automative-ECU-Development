/*******************************************************************************
 * @file    Dio.c
 * @author  Do Duc Nghia
 * @brief   Digital I/O Driver File
 * @details File này định nghĩa cụ thể cho các API đã được khai báo trong header
 *          file
 *
 * @version 1.0
 * @date    2025-06-23
 * *****************************************************************************
 */

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Dio.h"
#include "Dio_Cfg.h"

/********************************************************************
 * ================= [ API FUNCTION DEFINITIONS ] ===================
 * @brief   Định nghĩa các hàm API điều khiển theo chuẩn AUTOSAR
 ********************************************************************/

/********************************************************************
 * @brief   Định nghĩa hàm Dio_ReadChannel
 * @details Hàm giúp đọc mức logic của một channel
 *
 * @param   ChannelId        Channel muốn đọc
 * @return  Dio_LevelType    Kiểu dữ liệu thể hiện mức logic
 * ******************************************************************
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId)
{
    GPIO_TypeDef *GPIO_Port;
    uint16 GPIO_Pin;

    GPIO_Port = DIO_GET_PORT(ChannelId);
    GPIO_Pin = DIO_GET_PIN(ChannelId);

    if (GPIO_Port == NULL_PTR)
    {
        return STD_LOW;
    }
    else
    {
        if (GPIO_ReadInputDataBit(GPIO_Port, GPIO_Pin) == Bit_SET)
        {
            return STD_HIGH;
        }
        else
        {
            return STD_LOW;
        }
    }
    return STD_LOW;
}

/********************************************************************
 * @brief   Định nghĩa hàm Dio_WriteChannel
 * @details Hàm giúp ghi một mức logic cho một channel
 *
 * @param   ChannelId         Channel muốn ghi
 * @param   Level             Mức logic muốn ghi
 * ******************************************************************
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level)
{
    GPIO_TypeDef *GPIO_Port;
    uint16 GPIO_Pin;

    GPIO_Port = DIO_GET_PORT(ChannelId);
    GPIO_Pin = DIO_GET_PIN(ChannelId);

    if (GPIO_Port == NULL_PTR)
    {
        return;
    }
    else
    {
        if (Level == STD_HIGH)
        {
            GPIO_SetBits(GPIO_Port, GPIO_Pin);
        }
        else
        {
            GPIO_ResetBits(GPIO_Port, GPIO_Pin);
        }
    }
}

/********************************************************************
 * @brief   Định nghĩa hàm Dio_ReadPort
 * @details Hàm giúp đọc mức logic của toàn bộ chân trong một port
 *
 * @param   PortId               Port muốn đọc
 * @return  Dio_PortLevelType    Mức logic của các chân
 * ******************************************************************
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId)
{
    GPIO_TypeDef *GPIO_Port;
    GPIO_Port = DIO_PORT(PortId);

    if (GPIO_Port == NULL_PTR)
    {
        return 0;
    }
    else
    {
        return (Dio_PortLevelType)(GPIO_ReadInputData(GPIO_Port));
    }
}

/********************************************************************
 * @brief   Định nghĩa hàm Dio_WritePort
 * @details Hàm giúp ghi mức logic của các chân trong cùng 1 port
 *
 * @param   PortId    Port muốn ghi
 * @param   Level     Mức logic của các chân sẽ ghi
 * ******************************************************************
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level)
{
    GPIO_TypeDef *GPIO_Port;
    GPIO_Port = DIO_PORT(PortId);

    if (GPIO_Port == NULL_PTR)
    {
        return;
    }
    else
    {
        GPIO_Write(GPIO_Port, Level);
    }
}

/********************************************************************
 * @brief   Định nghĩa hàm Dio_ReadChannelGroup
 * @details Hàm giúp đọc mức logic của một nhóm các chân trên cùng
 *          một port
 *
 * @param   ChannelGroupIdPtr    Nhóm chân muốn đọc
 * @return  Dio_PortLevelType    Thể hiện mức logic của các chân
 * ******************************************************************
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr)
{
    GPIO_TypeDef *GPIO_Port = DIO_PORT(ChannelGroupIdPtr->port);
    uint16 portVal = GPIO_ReadInputData(GPIO_Port);

    if (GPIO_Port == NULL_PTR)
    {
        return 0;
    }
    else
    {
        return (Dio_PortLevelType)(((ChannelGroupIdPtr->mask) & portVal) >> ChannelGroupIdPtr->offset);
    }
}

/********************************************************************
 * @brief   Định nghĩa hàm Dio_WriteChannelGroup
 * @details Hàm giúp ghi mức logic của một nhóm các chân trên cùng một
 *          port
 *
 * @param   ChannelGroupIdPtr     Nhóm chân muốn ghi
 * @param   Level                 Mức logic muốn ghi
 * ******************************************************************
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr, Dio_PortLevelType Level)
{
    GPIO_TypeDef *GPIO_Port;
    GPIO_Port = DIO_PORT(ChannelGroupIdPtr->port);

    if (GPIO_Port == NULL_PTR)
    {
        return;
    }
    else
    {
        uint16_t portVal = GPIO_ReadOutputData(GPIO_Port);

        // Xóa các bit được chọn trong group
        portVal &= ~(ChannelGroupIdPtr->mask);

        // Ghi level mới vào vị trí offset, đảm bảo đúng mask
        portVal |= ((Level << ChannelGroupIdPtr->offset) & ChannelGroupIdPtr->mask);

        GPIO_Write(GPIO_Port, portVal);
    }
}

/********************************************************************
 * @brief   Định nghĩa hàm Dio_GetVersionInfo
 * @details Hàm lấy thông tin phiên bản
 *
 * @param   VersionInfo   Thông tin phiên bản
 * ******************************************************************
 */
void Dio_GetVersionInfo(Std_VersionInfoType *VersionInfo)
{
    VersionInfo->vendorID = 0x1234;
    VersionInfo->moduleID = 0x5678;
    VersionInfo->sw_major_version = 1;
    VersionInfo->sw_minor_version = 0;
    VersionInfo->sw_patch_version = 0;
}

/********************************************************************
 * @brief   Định nghĩa hàm Dio_FlipChannel
 * @details Hàm giúp đảo mức logic của một chân
 *
 * @param   ChannelId       Chân muốn đảo
 * @return  Dio_LevelType   Mức logic sau khi đảo
 * ******************************************************************
 */
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId)
{
    Dio_LevelType channelLevel = Dio_ReadChannel(ChannelId);

    if (channelLevel == STD_HIGH)
    {
        Dio_WriteChannel(ChannelId, STD_LOW);
        return STD_LOW;
    }
    else
    {
        Dio_WriteChannel(ChannelId, STD_HIGH);
        return STD_HIGH;
    }
}
