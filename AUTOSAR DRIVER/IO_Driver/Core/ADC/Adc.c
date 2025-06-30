/*******************************************************************************
 * @file    Adc.c
 * @author  Do Duc Nghia
 * @brief   ADC Driver Source File
 * @details Cung cấp các API điều khiển ADC theo chuẩn AUTOSAR, bao gồm khởi tạo,
 *          điều khiển chuyển đổi, đọc dữ liệu và quản lý thông báo.
 *
 * @version 1.0
 * @date    2025-06-28
 *******************************************************************************/

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Adc.h"

/********************************************************************
 * ==================== [ INTERNAL DEFINITIONS ] ====================
 * @brief   Định nghĩa các cấu hình hỗ trợ
 ********************************************************************/
/*******************************************************************************
 * @brief   Buffer dùng để lưu dữ liệu của group
 ******************************************************************************/
static Adc_ValueGroupType *adcGroupBuffers[ADC_MAX_GROUPS] = {NULL_PTR};

/*******************************************************************************
 * @brief   Mảng chứa status của các group
 ******************************************************************************/
static Adc_StatusType adcGroupStatus[ADC_MAX_GROUPS] = {ADC_IDLE};

/*******************************************************************************
 * @brief   Mảng boolean để theo dõi trạng thái callback của từng group
 * @details TRUE nếu callback của group đó đã được bật bằng Adc_EnableGroupNotification().
 *******************************************************************************/
static boolean adcGroupNotificationEnabled[ADC_MAX_GROUPS] = {FALSE};

/*******************************************************************************
 * @brief   Vị trí ghi mẫu hiện tại trong buffer streaming
 * @details Được cập nhật mỗi lần ADC ghi một mẫu vào buffer.
 *          Chỉ có tác dụng với group có accessMode là STREAMING.
 *******************************************************************************/
static uint16 adcSampleIndex[ADC_MAX_GROUPS] = {0};

/********************************************************************
 * =============== [ INTERNAL FUNCTION DEFINITIONS ] ================
 * @brief   Định nghĩa các hàm hỗ trợ
 ********************************************************************/
/*************************************************************************************
 * @brief   Xử lý khi một chuyển đổi ADC hoàn tất (EOC flag được set)
 * @param   group   Group ADC đã hoàn thành chuyển đổi
 * @details Hàm này được gọi khi một mẫu chuyển đổi hoàn tất (thường trong ngắt),
 *          nhằm thực hiện:
 *          - Đọc giá trị ADC từ hardware
 *          - Lưu vào buffer đã thiết lập sẵn
 *          - Cập nhật chỉ số mẫu trong streaming buffer
 *          - Kiểm tra đã đủ số mẫu yêu cầu trong chế độ STREAMING
 *              - Nếu đủ: dừng ADC, cập nhật trạng thái là COMPLETED
 *                        và gọi callback nếu được bật
 *          - Nếu chưa đủ: tiếp tục chuyển đổi (nếu ở chế độ liên tục)
 *
 * @note    Hàm này chỉ hoạt động với group đã được cấu hình buffer và đang hoạt động.
 *************************************************************************************/
static void Adc_HandleConversionComplete(Adc_GroupType group, uint16 rawValue)
{
    const Adc_GroupConfigType *cfg = &Adc_Config.groupConfigPtr[group];

    // STREAMING MODE
    if (cfg->accessMode == ADC_ACCESS_MODE_STREAMING)
    {
        if (adcGroupBuffers[group] != NULL_PTR)
        {
            // Ghi vào buffer
            adcGroupBuffers[group][adcSampleIndex[group]] = rawValue;

            // Tăng chỉ số
            adcSampleIndex[group]++;

            // Nếu đã đủ số mẫu
            if (adcSampleIndex[group] >= cfg->numSamples)
            {
                adcGroupStatus[group] = ADC_COMPLETED;
                ADC_SoftwareStartConvCmd(getInstance(cfg->hwUnit), DISABLE); // Stop ADC
                adcSampleIndex[group] = 0;

                // Gọi callback nếu có
                if (adcGroupNotificationEnabled[group] && cfg->notification != NULL_PTR)
                {
                    cfg->notification();
                }
            }
        }
    }
    else // NORMAL MODE
    {
        adcGroupStatus[group] = ADC_COMPLETED;

        if (adcGroupNotificationEnabled[group] && cfg->notification != NULL_PTR)
        {
            cfg->notification();
        }
    }
}

/********************************************************************
 * ================= [ API FUNCTION DEFINITIONS ] ===================
 * @brief   Định nghĩa các hàm API điều khiển theo chuẩn AUTOSAR
 ********************************************************************/

/*******************************************************************************
 * @brief   Khởi tạo ADC Driver
 * @details Hàm sẽ cấu hình toàn bộ ADC dựa trên thông tin trong Adc_ConfigType,
 *          bao gồm các group, chế độ trigger, độ phân giải, thời gian lấy mẫu,..
 *
 * @param   ConfigPtr   Con trỏ tới cấu hình ADC
 ******************************************************************************/
void Adc_Init(const Adc_ConfigType *ConfigPtr)
{
    if (ConfigPtr == NULL_PTR || ConfigPtr->groupConfigPtr == NULL_PTR)
        return;

    for (uint8 i = 0; i < ConfigPtr->numberOfGroups; ++i)
    {
        const Adc_GroupConfigType *group = &ConfigPtr->groupConfigPtr[i];
        ADC_TypeDef *adcInstance = NULL;

        // 1. Chọn ADCx instance & bật Clock
        switch (group->hwUnit)
        {
        case ADC_1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
            adcInstance = ADC1;
            break;
        case ADC_2:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
            adcInstance = ADC2;
            break;
        default:
            continue;
        }

        // 2. Cấu hình ADC
        ADC_InitTypeDef ADC_InitStruct;
        ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
        ADC_InitStruct.ADC_ScanConvMode = ENABLE;

        switch (group->conversionMode)
        {
        case ADC_CONV_MODE_CONTINUOUS:
            ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
            break;
        default:
            ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
            break;
        }

        switch (group->triggerSource)
        {
        case ADC_TRIGG_SRC_HW:
            switch (group->hwTriggerSignal)
            {
            case ADC_HW_TRIG_RISING:
                ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
                break;
            case ADC_HW_TRIG_FALLING:
                ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC2;
                break;
            default:
                ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
                break;
            }
            break;
        default:
            ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
            break;
        }

        ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
        ADC_InitStruct.ADC_NbrOfChannel = group->numberOfChannels;

        ADC_Init(adcInstance, &ADC_InitStruct);

        // 3. Bật ADC
        ADC_Cmd(adcInstance, ENABLE);

        // 4. Reset ADC (cần cho ADC1)
        ADC_ResetCalibration(adcInstance);
        while (ADC_GetResetCalibrationStatus(adcInstance))
            ;
        ADC_StartCalibration(adcInstance);
        while (ADC_GetCalibrationStatus(adcInstance))
            ;
    }
}

/*******************************************************************************
 * @brief   Hủy khởi tạo ADC
 * @details Đưa ADC về trạng thái ban đầu, tắt module và clear flag nội bộ.
 ******************************************************************************/
void Adc_DeInit(void)
{
    Adc_Init(&Adc_ConfigDefault);

    for (int i = 0; i < ADC_MAX_GROUPS; i++)
    {
        adcGroupStatus[i] = ADC_IDLE;
    }
}

/*******************************************************************************
 * @brief   Thiết lập buffer để lưu kết quả streaming
 * @param   group             Group muốn thiết lập buffer
 * @param   DataBufferPtr     Con trỏ tới buffer lưu kết quả
 * @return  Std_ReturnType    E_OK nếu thành công, E_NOT_OK nếu thất bại
 ******************************************************************************/
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType group, Adc_ValueGroupType *DataBufferPtr)
{
    /* Kiểm tra group ID hợp lệ */
    if (group >= Adc_Config.numberOfGroups)
    {
        return E_NOT_OK;
    }

    /* Kiểm tra con trỏ buffer hợp lệ */
    if (DataBufferPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Lưu con trỏ buffer vào vùng quản lý nội bộ của driver */
    adcGroupBuffers[group] = DataBufferPtr;

    return E_OK;
}

/*******************************************************************************
 * @brief   Bắt đầu chuyển đổi cho một group
 * @details Có thể trigger bằng phần mềm hoặc phần cứng tuỳ cấu hình group
 *
 * @param   group   Group cần chuyển đổi
 ******************************************************************************/
void Adc_StartGroupConversion(Adc_GroupType group)
{

    const Adc_GroupConfigType *groupConf = &Adc_Config.groupConfigPtr[group];
    ADC_TypeDef *adcx = getInstance(groupConf->hwUnit);

    for (uint8 ch = 0; ch < groupConf->numberOfChannels; ++ch)
    {
        ADC_RegularChannelConfig(adcx,
                                 groupConf->channelList[ch],
                                 ch + 1,
                                 groupConf->samplingTime);
    }

    ADC_SoftwareStartConvCmd(adcx, ENABLE);

    adcGroupStatus[group] = ADC_BUSY;
}

/*******************************************************************************
 * @brief   Dừng chuyển đổi cho một group
 * @param   group   Group cần dừng
 ******************************************************************************/
void Adc_StopGroupConversion(Adc_GroupType group)
{
    if (group >= Adc_Config.numberOfGroups || Adc_Config.groupConfigPtr == NULL_PTR)
        return;

    const Adc_GroupConfigType *grpCfg = &Adc_Config.groupConfigPtr[group];
    ADC_TypeDef *adcx = (grpCfg->hwUnit == ADC_1) ? ADC1 : ADC2;

    // Tắt chuyển đổi phần mềm nếu đang bật
    if (grpCfg->triggerSource == ADC_TRIGG_SRC_SW)
    {
        ADC_SoftwareStartConvCmd(adcx, DISABLE);
    }

    adcGroupStatus[group] = ADC_IDLE;
}

/*******************************************************************************
 * @brief   Đọc dữ liệu từ một group đã chuyển đổi xong
 * @param   group           Group cần đọc
 * @param   DataBufferPtr   Con trỏ buffer để lưu dữ liệu đọc được
 * @return  Std_ReturnType  E_OK nếu đọc thành công, E_NOT_OK nếu chưa có dữ liệu
 ******************************************************************************/
Std_ReturnType Adc_ReadGroup(Adc_GroupType group, Adc_ValueGroupType *DataBufferPtr)
{
    if (group >= Adc_Config.numberOfGroups || DataBufferPtr == NULL_PTR)
        return E_NOT_OK;

    const Adc_GroupConfigType *grpCfg = &Adc_Config.groupConfigPtr[group];
    ADC_TypeDef *adcx = (grpCfg->hwUnit == ADC_1) ? ADC1 : ADC2;

    // Nếu conversion chưa xong thì không đọc được
    if (ADC_GetFlagStatus(adcx, ADC_FLAG_EOC) == RESET)
        return E_NOT_OK;

    for (uint8 i = 0; i < grpCfg->numberOfChannels; ++i)
    {
        while (ADC_GetFlagStatus(adcx, ADC_FLAG_EOC) == RESET)
            ; // Chờ chuyển đổi xong

        uint16 rawValue = ADC_GetConversionValue(adcx);
        ADC_ClearFlag(adcx, ADC_FLAG_EOC); // Xóa cờ chuyển đổi

        // Nếu có buffer truyền vào thì lưu
        if (DataBufferPtr != NULL_PTR)
            DataBufferPtr[i] = rawValue;

        // Gọi xử lý STREAMING
        Adc_HandleConversionComplete(group, rawValue);
        adcGroupStatus[group] = ADC_COMPLETED;

        return E_OK;
    }

    /*******************************************************************************
     * @brief   Lấy trạng thái hiện tại của một group
     * @param   group   Group cần kiểm tra
     * @return  Adc_StatusType (IDLE, BUSY, COMPLETED, ...)
     ******************************************************************************/
    Adc_StatusType Adc_GetGroupStatus(Adc_GroupType group)
    {
        if (group >= ADC_MAX_GROUPS)
            return ADC_IDLE;

        return adcGroupStatus[group];
    }

    /*******************************************************************************
     * @brief   Cho phép gọi callback khi group chuyển đổi xong
     * @param   group   Group cần bật notification
     ******************************************************************************/
    void Adc_EnableGroupNotification(Adc_GroupType group)
    {
        if (group >= ADC_MAX_GROUPS)
            return;

        // Gán cờ kích hoạt notification
        adcGroupNotificationEnabled[group] = TRUE;
    }

    /*******************************************************************************
     * @brief   Tắt callback khi group chuyển đổi xong
     * @param   group   Group cần tắt notification
     ******************************************************************************/
    void Adc_DisableGroupNotification(Adc_GroupType group)
    {
        if (group >= ADC_MAX_GROUPS)
            return;

        // Xóa cờ kích hoạt notification
        adcGroupNotificationEnabled[group] = FALSE;
    }

    /*******************************************************************************
     * @brief   Lấy con trỏ tới mẫu cuối cùng trong buffer streaming
     * @param   group             Group cần kiểm tra
     * @param   PtrToSamplePtr    Con trỏ tới con trỏ mẫu cuối cùng
     * @return  Std_ReturnType    E_OK nếu thành công, E_NOT_OK nếu group chưa chạy
     ******************************************************************************/
    Std_ReturnType Adc_GetStreamLastPointer(Adc_GroupType group, Adc_ValueGroupType * *PtrToSamplePtr)
    {
        if (group >= ADC_MAX_GROUPS || PtrToSamplePtr == NULL_PTR)
            return E_NOT_OK;

        const Adc_GroupConfigType *groupCfg = &Adc_Config.groupConfigPtr[group];

        // Kiểm tra xem group đó có đang ở chế độ STREAMING không
        if (groupCfg->accessMode != ADC_ACCESS_MODE_STREAMING)
            return E_NOT_OK;

        // Kiểm tra buffer hợp lệ
        if (adcGroupBuffers[group] == NULL_PTR)
            return E_NOT_OK;

        // Trả về con trỏ đến phần tử cuối cùng đã ghi (dựa vào chỉ số hiện tại)
        *PtrToSamplePtr = &adcGroupBuffers[group][adcSampleIndex[group]];

        return E_OK;
    }

    /*******************************************************************************
     * @brief   Cho phép trigger bằng phần cứng cho group
     * @param   group   Group cần bật HW trigger
     ******************************************************************************/
    void Adc_EnableHardwareTrigger(Adc_GroupType group)
    {
    }

    /*******************************************************************************
     * @brief   Tắt trigger phần cứng cho group
     * @param   group   Group cần tắt HW trigger
     ******************************************************************************/
    void Adc_DisableHardwareTrigger(Adc_GroupType group)
    {
    }

    /*******************************************************************************
     * @brief   Lấy thông tin phiên bản của driver
     * @param   versioninfo   Con trỏ tới struct chứa thông tin version
     ******************************************************************************/
    void Adc_GetVersionInfo(Std_VersionInfoType * versioninfo)
    {
    }