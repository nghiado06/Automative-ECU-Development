/*******************************************************************************
 * @file    Dio.h
 * @author  Do Duc Nghia
 * @brief   Digital I/O Driver Header File
 * @details File này cấu hình các kiểu dữ liệu, API theo chuẩn AUTOSAR
 *
 * @version 1.0
 * @date    2025-06-23
 * *****************************************************************************
 */

#ifndef DIO_H
#define DIO_H

/*===================== [ INCLUDE LIBRARIES ] ======================*/
#include "..\Std_Types.h"

/********************************************************************
 * ===================== [ AUTOSAR STANDARD ] =======================
 * @brief   Dưới đây sẽ là các API Functions theo tiêu chuẩn AUTSAR
 *          áp dụng với mọi dòng vi điều khiển 8-bit.
 ********************************************************************/

/********************************************************************
 * ===================== [ TYPE DEFINITIONS ] =======================
 * @brief   Định nghĩa các kiểu dữ liệu theo tiêu chuẩn AUTOSAR
 ********************************************************************/

/**********************************************************************
 * @brief   Định nghĩa kiểu dữ liệu Dio_ChannelType
 * @details kiểu dữ liệu này biểu diễn một pin cụ thể của vi điều khiển
 *          - ví dụ như PA0, PC13, PB0,..
 * ********************************************************************
 */
typedef uint8 Dio_ChannelType;

/*********************************************************************
 * @brief   Định nghĩa kiểu dữ liệu Dio_PortType
 * @details kiểu dữ liệu Dio_PortType biểu diễn cho một cổng digital,
 *          tức là một nhóm chân IO (Ví dụ: Port A, Port B,..)
 * ********************************************************************
 */
typedef uint8 Dio_PortType;

/**********************************************************************
 * @brief   Định nghĩa struct Dio_ChannelGroupType
 * @details Dio_ChannelGroupType đại diện cho một nhóm nhiều chân liên
 *          tiếp nằm trên cùng một port.
 *
 * mask     Mặt nạ giúp xác định những bit nào trong port thuộc group
 * offset   Bit thấp nhất trong mask (Vị trí bắt đầu của nhóm)
 * port     Port chứa các chân trong group
 * ********************************************************************
 */
typedef struct
{
    uint32 mask;
    uint8 offset;
    Dio_PortType port;
} Dio_ChannelGroupType;

/*********************************************************************
 * @brief   Định nghĩa kiểu dữ liệu Dio_LevelType
 * @details kiểu dữ liệu dùng để biểu diễn mức logic (HIGH/LOW) của
 *          một chân DIO hoặc một port DIO
 * ********************************************************************
 */
typedef uint8 Dio_LevelType;

/*********************************************************************
 * @brief   Định nghĩa kiểu dữ liệu Dio_PortLevelType
 * @details kiểu dữ liệu biểu diễn mức logic của toàn bộ chân trong một
 *          port
 * ********************************************************************
 */
typedef uint8 Dio_PortLevelType;

/********************************************************************
 * ================= [ API FUNCTION DEFINITIONS ] ===================
 * @brief   Định nghĩa các hàm API điều khiển theo chuẩn AUTOSAR
 ********************************************************************/

/********************************************************************
 * @brief   Định nghĩa hàm Dio_ReadChannel
 * @details Hàm giúp đọc mức logic của một channel
 *
 * @param   ChannelId         Channel muốn đọc
 * @return  Dio_LevelType    Kiểu dữ liệu thể hiện mức logic
 * ******************************************************************
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId);

/********************************************************************
 * @brief   Định nghĩa hàm Dio_WriteChannel
 * @details Hàm giúp ghi một mức logic cho một channel
 *
 * @param   ChannelId         Channel muốn ghi
 * @param   Level             Mức logic muốn ghi
 * ******************************************************************
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/********************************************************************
 * @brief   Định nghĩa hàm Dio_ReadPort
 * @details Hàm giúp đọc mức logic của toàn bộ chân trong một port
 *
 * @param   PortId               Port muốn đọc
 * @return  Dio_PortLevelType    Mức logic của các chân
 * ******************************************************************
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId);

/********************************************************************
 * @brief   Định nghĩa hàm Dio_WritePort
 * @details Hàm giúp ghi mức logic của các chân trong cùng 1 port
 *
 * @param   PortId    Port muốn ghi
 * @param   Level     Mức logic của các chân sẽ ghi
 * ******************************************************************
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);

/********************************************************************
 * @brief   Định nghĩa hàm Dio_ReadChannelGroup
 * @details Hàm giúp đọc mức logic của một nhóm các chân trên cùng
 *          một port
 *
 * @param   ChannelGroupIdPtr     Nhóm chân muốn đọc
 * @return  Dio_PortLevelType    Thể hiện mức logic của các chân
 * ******************************************************************
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr);

/********************************************************************
 * @brief   Định nghĩa hàm Dio_WriteChannelGroup
 * @details Hàm giúp ghi mức logic của một nhóm các chân trên cùng một
 *          port
 *
 * @param   ChannelGroupIdPtr     Nhóm chân muốn ghi
 * @param   Level                 Mức logic muốn ghi
 * ******************************************************************
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr, Dio_PortLevelType Level);

/********************************************************************
 * @brief   Định nghĩa hàm Dio_GetVersionInfo
 * @details Hàm lấy thông tin phiên bản
 *
 * @param   VersionInfo   Thông tin phiên bản
 * ******************************************************************
 */
void Dio_GetVersionInfo(Std_VersionInfoType *VersionInfo);

/********************************************************************
 * @brief   Định nghĩa hàm Dio_FlipChannel
 * @details Hàm giúp đảo mức logic của một chân
 *
 * @param   ChannelId       Chân muốn đảo
 * @return  Dio_LevelType   Mức logic sau khi đảo
 * ******************************************************************
 */
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId);

#endif