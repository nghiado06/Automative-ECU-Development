/********************************************************************************
 * @file    Adc_Lcfg.c
 * @author  Do Duc Nghia
 * @brief   ADC Driver Link-Time Configuration Source File
 * @details File này định nghĩa toàn bộ cấu hình ADC cụ thể tại thời điểm liên kết
 *          (link-time), bao gồm danh sách channel, cấu hình group ADC và cấu hình
 *          mặc định khi deinit. Đây là phần hiện thực tương ứng với các khai báo
 *          trong file Adc_Cfg.h, và được truyền vào hàm Adc_Init để khởi tạo driver.
 *
 * @version 1.0
 * @date    2025-06-28
 ********************************************************************************/

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Adc_Cfg.h"

/**********************************************************************************
 * @brief   Danh sách channel thực tế sử dụng
 * @details Mỗi phần tử đại diện cho một channel vật lý trong STM32 (ADC_Channel_x)
 *          - Channel này sẽ được ánh xạ trong group để tiến hành chuyển đổi
 *
 *          Ví dụ: channels[0] = ADC_Channel_1 tương ứng với chân PA1
 *********************************************************************************/
const Adc_ChannelType channels[2] = {
    ADC_Channel_1,
    ADC_Channel_2};

/*******************************************************************************
 * @brief   Cấu hình từng group ADC cụ thể
 * @details Mỗi group có thể chứa nhiều channel (multi-channel scan)
 *          - Hỗ trợ ONESHOT hoặc CONTINUOUS mode
 *          - Có thể kích hoạt bằng SW hoặc HW trigger
 *          - Có thể định nghĩa callback khi conversion hoàn tất
 *
 *          Trong ví dụ dưới đây, groupId = 0 bao gồm 2 channel, chạy ONESHOT,
 *          kích hoạt bằng phần mềm
 ******************************************************************************/
const Adc_GroupConfigType AdcGroupConfigs[ADC_MAX_GROUPS] = {
    {.groupId = 0,
     .channelList = channels,
     .numberOfChannels = 2,
     .conversionMode = ADC_CONV_MODE_ONESHOT,
     .accessMode = ADC_ACCESS_MODE_SINGLE,
     .triggerSource = ADC_TRIGG_SRC_SW,
     .hwTriggerSignal = ADC_HW_TRIG_RISING,
     .streamBufferMode = ADC_STREAM_BUFFER_LINEAR,
     .numSamples = 1,
     .notification = NULL_PTR,
     .priority = 0,
     .hwUnit = ADC_1,
     .refVoltage = ADC_REF_INTERNAL,
     .samplingTime = ADC_SAMPLINGTIME_28CYCLES5,
     .resolution = ADC_RESOLUTION_12,
     .inputMode = ADC_SINGLE_ENDED,
     .groupReplacement = ADC_REPL_SKIP}};

/*******************************************************************************
 * @brief   Cấu hình tổng ADC sử dụng thực tế
 * @details Biến này sẽ được truyền vào hàm Adc_Init để cấu hình driver
 ******************************************************************************/
const Adc_ConfigType Adc_Config = {
    .groupConfigPtr = AdcGroupConfigs,
    .numberOfGroups = 1};

/*******************************************************************************
 * @brief   Danh sách channel giả dùng cho cấu hình mặc định
 * @details Dùng để deinit hoặc fallback khi không có cấu hình thật
 ******************************************************************************/
const Adc_ChannelType DummyChannels[1] = {
    ADC_Channel_0};

/*******************************************************************************
 * @brief   Cấu hình mặc định của group ADC
 * @details Dùng trong Adc_DeInit để reset về trạng thái ban đầu
 ******************************************************************************/
const Adc_GroupConfigType AdcGroupConfigDefault[] = {
    {.groupId = 0,
     .channelList = DummyChannels,
     .numberOfChannels = 1,
     .conversionMode = ADC_CONV_MODE_ONESHOT,
     .accessMode = ADC_ACCESS_MODE_SINGLE,
     .triggerSource = ADC_TRIGG_SRC_SW,
     .hwTriggerSignal = ADC_HW_TRIG_RISING,
     .streamBufferMode = ADC_STREAM_BUFFER_LINEAR,
     .numSamples = 1,
     .notification = NULL_PTR,
     .priority = 0,
     .hwUnit = 0,
     .refVoltage = ADC_REF_INTERNAL,
     .samplingTime = ADC_SAMPLINGTIME_1CYCLE5,
     .resolution = ADC_RESOLUTION_12,
     .inputMode = ADC_SINGLE_ENDED,
     .groupReplacement = ADC_REPL_SKIP}};

/*******************************************************************************
 * @brief   Cấu hình tổng ADC mặc định (fallback)
 * @details Sử dụng trong Adc_DeInit để vô hiệu hóa ADC và xóa cấu hình hiện hành
 ******************************************************************************/
const Adc_ConfigType Adc_ConfigDefault = {
    .groupConfigPtr = AdcGroupConfigDefault,
    .numberOfGroups = 1};
