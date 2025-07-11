/*******************************************************************************
 * @file    Adc.c
 * @author  Do Duc Nghia
 * @brief   ADC Driver Source File
 * @details Cung cấp các API điều khiển ADC theo chuẩn AUTOSAR, bao gồm khởi tạo,
 *          điều khiển chuyển đổi, đọc dữ liệu và quản lý thông báo.
 * @note    File này đang cấu hình ADC dạng Polling, chưa áp dụng Ngắt hay DMA
 *
 * @version 1.0
 * @date    2025-06-28
 *******************************************************************************/

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Adc.h"

#define ADC_INVALID_GROUP ((Adc_GroupType)0xFF)

#define ADC_VENDOR_ID 1234
#define ADC_MODULE_ID 5678
#define ADC_SW_MAJOR_VERSION 1
#define ADC_SW_MINOR_VERSION 0
#define ADC_SW_PATCH_VERSION 0

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
 * @details TRUE nếu callback của group đó đã được bật.
 *******************************************************************************/
static boolean adcGroupNotificationEnabled[ADC_MAX_GROUPS] = {FALSE};

/*******************************************************************************
 * @brief   Vị trí ghi mẫu hiện tại trong buffer streaming
 * @details Được cập nhật mỗi lần ADC ghi một mẫu vào buffer.
 *          Chỉ có tác dụng với group có accessMode là STREAMING.
 *******************************************************************************/
static uint16 adcSampleIndex[ADC_MAX_SAMPLES] = {0};

/*******************************************************************************
 * @brief   Bộ nhớ lưu trạng thái của group
 *******************************************************************************/
static Adc_GroupType adcActiveGroupId[2] = {ADC_INVALID_GROUP, ADC_INVALID_GROUP};

/*******************************************************************************
 * @brief   Bộ nhớ của từng group
 *******************************************************************************/
static Adc_ValueGroupType buffer_group_0[15]; // Điều chỉnh được số phần tử: phần tử = mẫu x số kênh
static Adc_ValueGroupType buffer_group_1[15];
static Adc_ValueGroupType buffer_group_2[15];

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
    adcActiveGroupId[cfg->hwUnit] = ADC_INVALID_GROUP;
}

/*************************************************************************************
 * @brief   Hàm ISR cho ADC
 *************************************************************************************/
void ADC1_2_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC))
    {
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

        Adc_GroupType group = adcActiveGroupId[ADC_1];
        if (group != ADC_INVALID_GROUP)
        {
            uint16 raw = ADC_GetConversionValue(ADC1);
            Adc_HandleConversionComplete(group, raw);
        }
    }

    // Nếu có ADC2:
    if (ADC_GetITStatus(ADC2, ADC_IT_EOC))
    {
        ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);

        Adc_GroupType group = adcActiveGroupId[ADC_2];
        if (group != ADC_INVALID_GROUP)
        {
            uint16 raw = ADC_GetConversionValue(ADC2);
            Adc_HandleConversionComplete(group, raw);
        }
    }
}

/*************************************************************************************
 * @brief   Hàm cấu hình sử dụng DMA
 *************************************************************************************/
static void Adc_Dma_Config(Adc_GroupType group)
{
    const Adc_GroupConfigType *cfg = &Adc_Config.groupConfigPtr[group];
    ADC_TypeDef *adc = getInstance(cfg->hwUnit);

    // Giả sử: bạn có adcGroupBuffers[group] là buffer đã gán
    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel1); // Tùy channel ADCx gắn vào DMA channel nào

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(adc->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adcGroupBuffers[group];
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = cfg->numSamples * cfg->numberOfChannels;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel1, ENABLE);

    ADC_DMACmd(adc, ENABLE);

    Adc_DmaNvic_Config(); // Cấu hình NVIC cho DMA;

    NVIC_EnableIRQ(DMA1_Channel1_IRQn); // Bật ngắt DMA

    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); // Bật ngắt chuyển hoàn thành
}

/*************************************************************************************
 * @brief   Hàm xử lý ngắt DMA Channel 1
 * @details Hàm này được gọi khi DMA hoàn thành chuyển đổi và gửi dữ liệu về ADC.
 *          Nó sẽ cập nhật trạng thái của group và gọi callback nếu có.
 *************************************************************************************/
void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC1);
        DMA_Cmd(DMA1_Channel1, DISABLE);
        ADC_DMACmd(ADC1, DISABLE);

        Adc_GroupType group = adcActiveGroupId[ADC_1];
        if (group != ADC_INVALID_GROUP)
        {
            adcGroupStatus[group] = ADC_COMPLETED;

            const Adc_GroupConfigType *cfg = &Adc_Config.groupConfigPtr[group];
            if (adcGroupNotificationEnabled[group] && cfg->notification != NULL_PTR)
            {
                cfg->notification();
            }
        }
    }
}

/*************************************************************************************
 * @brief   Hàm cấu hình NVIC cho DMA
 * @details Cấu hình NVIC để nhận ngắt từ DMA Channel 1, thường dùng cho ADC.
 *************************************************************************************/
static void Adc_DmaNvic_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);
}

/*************************************************************************************
 * @brief   Hàm cấu hình sử dụng Ngắt
 *************************************************************************************/
static void Adc_Nvic_Config(Adc_HwUnitType hwUnit)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;

    switch (hwUnit)
    {
    case ADC_1:
    case ADC_2:
        NVIC_InitStruct.NVIC_IRQChannel = ADC1_2_IRQn;
        break;
    default:
        return;
    }

    NVIC_Init(&NVIC_InitStruct);
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

    // Reset lại vị trí lưu sample
    adcSampleIndex[group] = 0;

    // Set trạng thái hoạt động cho group
    adcActiveGroupId[groupConf->hwUnit] = group;

    for (uint8 ch = 0; ch < groupConf->numberOfChannels; ++ch)
    {
        ADC_RegularChannelConfig(adcx,
                                 groupConf->channelList[ch],
                                 ch + 1,
                                 groupConf->samplingTime);
    }

    // Nếu dùng DMA
    if (groupConf->transferMode == ADC_TRANSFER_MODE_DMA)
    {
        Adc_Dma_Config(group);
    }

    // Nếu dùng ngắt
    if (groupConf->readMode == ADC_READ_MODE_INTERRUPT)
    {
        Adc_Nvic_Config(groupConf->hwUnit);
        NVIC_EnableIRQ(ADC1_2_IRQn); // Bật ngắt ADC1 và ADC2
        ADC_ITConfig(adcx, ADC_IT_EOC, ENABLE);
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
    }
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
Std_ReturnType Adc_GetStreamLastPointer(Adc_GroupType group, Adc_ValueGroupType **PtrToSamplePtr)
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
    if (group >= ADC_MAX_GROUPS)
        return;

    const Adc_GroupConfigType *groupCfg = &Adc_Config.groupConfigPtr[group];
    ADC_TypeDef *adcx = groupCfg->hwUnit == ADC_1 ? ADC1 : ADC2;

    // Kiểm tra xem group có hỗ trợ trigger phần cứng không
    if (groupCfg->triggerSource != ADC_TRIGG_SRC_HW)
        return;

    // Cấu hình trigger
    ADC_ExternalTrigConvCmd(adcx, ENABLE);
    ADC_ExternalTrigConvConfig(adcx, groupCfg->hwTriggerSignal);

    // Bật ngắt nếu cần
    if (groupCfg->readMode == ADC_READ_MODE_INTERRUPT)
    {
        NVIC_EnableIRQ(ADC1_2_IRQn);
        ADC_ITConfig(adcx, ADC_IT_EOC, ENABLE);
    }
}

/*******************************************************************************
 * @brief   Tắt trigger phần cứng cho group
 * @param   group   Group cần tắt HW trigger
 ******************************************************************************/
void Adc_DisableHardwareTrigger(Adc_GroupType group)
{
    if (group >= ADC_MAX_GROUPS)
        return;

    const Adc_GroupConfigType *groupCfg = &Adc_Config.groupConfigPtr[group];
    ADC_TypeDef *adcx = groupCfg->hwUnit == ADC_1 ? ADC1 : ADC2;

    // Kiểm tra xem group có hỗ trợ trigger phần cứng không
    if (groupCfg->triggerSource != ADC_TRIGG_SRC_HW)
        return;

    // Tắt trigger
    ADC_ExternalTrigConvCmd(adcx, DISABLE);
    ADC_ITConfig(adcx, ADC_IT_EOC, DISABLE);

    // Tắt ngắt nếu không cần nữa
    NVIC_DisableIRQ(ADC1_2_IRQn);
}

/*******************************************************************************
 * @brief   Lấy thông tin phiên bản của driver
 * @param   versioninfo   Con trỏ tới struct chứa thông tin version
 ******************************************************************************/
void Adc_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
    if (versioninfo == NULL_PTR)
        return;

    versioninfo->vendorID = ADC_VENDOR_ID;
    versioninfo->moduleID = ADC_MODULE_ID;
    versioninfo->sw_major_version = ADC_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = ADC_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = ADC_SW_PATCH_VERSION;
}