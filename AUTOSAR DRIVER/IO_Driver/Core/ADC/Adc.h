/*******************************************************************************
 * @file    Adc.h
 * @author  Do Duc Nghia
 * @brief   ADC Driver Header File
 * @details Cung cấp các API điều khiển ADC theo chuẩn AUTOSAR, bao gồm khởi tạo,
 *          điều khiển chuyển đổi, đọc dữ liệu và quản lý thông báo.
 *
 * @version 1.0
 * @date    2025-06-28
 *******************************************************************************/

#ifndef ADC_H
#define ADC_H

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Adc_Types.h"
#include "Std_Types.h"
#include "stm32f10x_rcc.h"
#include "Adc_Cfg.h"
#include "stm32f10x_dma.h"
#include "stm32f10x.h"

/********************************************************************
 * ================= [ API FUNCTION DEFINITIONS ] ===================
 * @brief   Định nghĩa các hàm API điều khiển theo chuẩn AUTOSAR
 ********************************************************************/

/*******************************************************************************
 * @brief   Khởi tạo ADC Driver
 * @param   ConfigPtr   Con trỏ tới cấu hình ADC
 * @details Hàm sẽ cấu hình toàn bộ ADC dựa trên thông tin trong Adc_ConfigType,
 *          bao gồm các group, chế độ trigger, độ phân giải, thời gian lấy mẫu,..
 ******************************************************************************/
void Adc_Init(const Adc_ConfigType *ConfigPtr);

/*******************************************************************************
 * @brief   Hủy khởi tạo ADC
 * @details Đưa ADC về trạng thái ban đầu, tắt module và clear flag nội bộ.
 ******************************************************************************/
void Adc_DeInit(void);

/*******************************************************************************
 * @brief   Thiết lập buffer để lưu kết quả streaming
 * @param   group             Group muốn thiết lập buffer
 * @param   DataBufferPtr     Con trỏ tới buffer lưu kết quả
 * @return  Std_ReturnType    E_OK nếu thành công, E_NOT_OK nếu thất bại
 ******************************************************************************/
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType group, Adc_ValueGroupType *DataBufferPtr);

/*******************************************************************************
 * @brief   Bắt đầu chuyển đổi cho một group
 * @param   group   Group cần chuyển đổi
 * @details Có thể trigger bằng phần mềm hoặc phần cứng tuỳ cấu hình group
 ******************************************************************************/
void Adc_StartGroupConversion(Adc_GroupType group);

/*******************************************************************************
 * @brief   Dừng chuyển đổi cho một group
 * @param   group   Group cần dừng
 ******************************************************************************/
void Adc_StopGroupConversion(Adc_GroupType group);

/*******************************************************************************
 * @brief   Đọc dữ liệu từ một group đã chuyển đổi xong
 * @param   group           Group cần đọc
 * @param   DataBufferPtr   Con trỏ buffer để lưu dữ liệu đọc được
 * @return  Std_ReturnType  E_OK nếu đọc thành công, E_NOT_OK nếu chưa có dữ liệu
 ******************************************************************************/
Std_ReturnType Adc_ReadGroup(Adc_GroupType group, Adc_ValueGroupType *DataBufferPtr);

/*******************************************************************************
 * @brief   Lấy trạng thái hiện tại của một group
 * @param   group   Group cần kiểm tra
 * @return  Adc_StatusType (IDLE, BUSY, COMPLETED, ...)
 ******************************************************************************/
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType group);

/*******************************************************************************
 * @brief   Cho phép gọi callback khi group chuyển đổi xong
 * @param   group   Group cần bật notification
 ******************************************************************************/
void Adc_EnableGroupNotification(Adc_GroupType group);

/*******************************************************************************
 * @brief   Tắt callback khi group chuyển đổi xong
 * @param   group   Group cần tắt notification
 ******************************************************************************/
void Adc_DisableGroupNotification(Adc_GroupType group);

/*******************************************************************************
 * @brief   Lấy con trỏ tới mẫu cuối cùng trong buffer streaming
 * @param   group             Group cần kiểm tra
 * @param   PtrToSamplePtr    Con trỏ tới con trỏ mẫu cuối cùng
 * @return  Std_ReturnType    E_OK nếu thành công, E_NOT_OK nếu group chưa chạy
 ******************************************************************************/
Std_ReturnType Adc_GetStreamLastPointer(Adc_GroupType group, Adc_ValueGroupType **PtrToSamplePtr);

/*******************************************************************************
 * @brief   Cho phép trigger bằng phần cứng cho group
 * @param   group   Group cần bật HW trigger
 ******************************************************************************/
void Adc_EnableHardwareTrigger(Adc_GroupType group);

/*******************************************************************************
 * @brief   Tắt trigger phần cứng cho group
 * @param   group   Group cần tắt HW trigger
 ******************************************************************************/
void Adc_DisableHardwareTrigger(Adc_GroupType group);

/*******************************************************************************
 * @brief   Lấy thông tin phiên bản của driver
 * @param   versioninfo   Con trỏ tới struct chứa thông tin version
 ******************************************************************************/
void Adc_GetVersionInfo(Std_VersionInfoType *versioninfo);

#endif /* ADC_H */
