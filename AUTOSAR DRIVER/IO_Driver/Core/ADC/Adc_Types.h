/*******************************************************************************
 * @file    Adc_Types.h
 * @author  Do Duc Nghia
 * @brief   ADC Driver Type Definitions Header File
 * @details File khai báo các kiểu dữ liệu dùng trong ADC Driver theo chuẩn AUTOSAR,
 *          bao gồm các loại typedef, enum và cấu trúc phục vụ việc cấu hình, vận hành
 *          và quản lý quá trình chuyển đổi ADC của vi điều khiển STM32.
 *
 * @version 1.0
 * @date    2025-06-28
 *******************************************************************************/

#ifndef ADC_TYPES_H
#define ADC_TYPES_H

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Std_Types.h"
#include "stm32f10x_adc.h"

/********************************************************************
 * ====================== [ TYPE DEFINITIONS ] =====================
 * @brief   Định nghĩa các kiểu dữ liệu theo tiêu chuẩn AUTOSAR
 ********************************************************************/
/*******************************************************************************
 * @brief   Định danh Group ADC
 * @details Mỗi Group là một tập hợp các channel ADC được chuyển đổi cùng lúc.
 *          Ví dụ: Group 0 bao gồm các channel CH1, CH2, CH5. Khi gọi chuyển đổi
 *          group, ADC sẽ đọc tuần tự từng channel này. Dùng để gom các channel
 *          có liên quan logic.
 ******************************************************************************/
typedef uint8 Adc_GroupType;

/*******************************************************************************
 * @brief   Kiểu dữ liệu chứa giá trị ADC sau chuyển đổi
 * @details Đây là kiểu dùng để lưu trữ giá trị số sau khi ADC chuyển đổi từ
 *          analog. Thường là uint16 vì độ phân giải phổ biến của ADC là 12-bit.
 ******************************************************************************/
typedef uint16 Adc_ValueGroupType;

/***********************************************************************************
 * @brief   Kiểu đại diện cho một channel cụ thể trong ADC
 * @details Channel là một chân analog cụ thể của vi điều khiển, ví dụ như PA0, PA1.
 **********************************************************************************/
typedef uint8 Adc_ChannelType;

/*******************************************************************************
 * @brief   Xác định phần cứng ADC (ADC1, ADC2,...)
 * @details Dùng để chọn ADC hardware unit nào sẽ xử lý group đó. Ví dụ STM32F1
 *          có ADC1, ADC2.
 ******************************************************************************/
typedef enum
{
    ADC_1,
    ADC_2
} Adc_HwUnitType;

/**********************************************************************************
 * @brief   Chế độ chuyển đổi ADC của một group
 * @details - ONESHOT: chỉ chuyển đổi một lần duy nhất sau khi gọi StartConversion.
 *          - CONTINUOUS: tự động chuyển đổi lặp liên tục cho đến khi bị dừng.
 **********************************************************************************/
typedef enum
{
    ADC_CONV_MODE_ONESHOT,
    ADC_CONV_MODE_CONTINUOUS
} Adc_GroupConvModeType;

/*******************************************************************************
 * @brief   Trạng thái hiện tại của một group ADC
 * @details Dùng để theo dõi quá trình chuyển đổi:
 *          - IDLE: chưa bắt đầu
 *          - BUSY: đang chuyển đổi
 *          - COMPLETED: đã hoàn thành 1 lần
 *          - STREAMCOMPLETED: đã thu đủ mẫu theo yêu cầu streaming
 ******************************************************************************/
typedef enum
{
    ADC_IDLE,
    ADC_BUSY,
    ADC_COMPLETED,
    ADC_STREAM_COMPLETED
} Adc_StatusType;

/*******************************************************************************
 * @brief   Nguồn kích hoạt chuyển đổi (trigger) của ADC
 * @details - SW: phần mềm gọi hàm bắt đầu chuyển đổi
 *          - HW: tín hiệu từ timer, chân ngoài hoặc ngoại vi khác
 ******************************************************************************/
typedef enum
{
    ADC_TRIGG_SRC_SW,
    ADC_TRIGG_SRC_HW
} Adc_TriggerSourceType;

/*******************************************************************************
 * @brief   Kiểu căn lề dữ liệu kết quả ADC
 * @details Một số ADC hỗ trợ căn trái hoặc phải trong thanh ghi kết quả.
 *          Ví dụ căn trái dùng khi chỉ cần giá trị 8-bit cao nhất.
 ******************************************************************************/
typedef enum
{
    ADC_ALIGN_LEFT,
    ADC_ALIGN_RIGHT
} Adc_ResultAlignmentType;

/*******************************************************************************
 * @brief   Số mẫu cần đọc trong chế độ streaming
 * @details Nếu dùng chế độ streaming, đây là số mẫu tối đa sẽ lưu vào buffer
 *          trước khi callback.
 ******************************************************************************/
typedef uint16 Adc_StreamNumSampleType;

/*******************************************************************************
 * @brief   Kiểu buffer sử dụng trong chế độ streaming
 * @details - LINEAR: ghi đầy buffer một lần rồi dừng
 *          - CIRCULAR: ghi lặp vòng tròn liên tục, đè mẫu cũ
 ******************************************************************************/
typedef enum
{
    ADC_STREAM_BUFFER_LINEAR,
    ADC_STREAM_BUFFER_CIRCULAR
} Adc_StreamBufferModeType;

/*******************************************************************************
 * @brief   Độ phân giải ADC (bit)
 * @details Tùy dòng chip có thể cấu hình 8, 10 hoặc 12-bit. Độ phân giải càng
 *          cao thì độ chính xác càng tốt.
 ******************************************************************************/
typedef enum
{
    ADC_RESOLUTION_8,
    ADC_RESOLUTION_10,
    ADC_RESOLUTION_12
} Adc_ResolutionType;

/*******************************************************************************
 * @brief   Nguồn tham chiếu ADC
 * @details Có thể dùng Vref nội (1.2V/1.8V...) hoặc nguồn ngoài cung cấp qua
 *          chân VREF+.
 ******************************************************************************/
typedef enum
{
    ADC_REF_INTERNAL,
    ADC_REF_EXTERNAL
} Adc_RefType;

/*******************************************************************************
 * @brief   Kiểu đầu vào ADC
 * @details - SINGLE_ENDED: đo điện áp so với GND
 *          - DIFFERENTIAL: đo điện áp giữa 2 chân khác nhau (chế độ vi sai)
 ******************************************************************************/
typedef enum
{
    ADC_SINGLE_ENDED,
    ADC_DIFFERENTIAL
} Adc_InputModeType;

/*******************************************************************************
 * @brief   Kiểu truy cập dữ liệu của Group ADC
 * @details - SINGLE: chỉ đọc một lần sau chuyển đổi
 *          - STREAMING: đọc liên tục và lưu vào buffer
 ******************************************************************************/
typedef enum
{
    ADC_ACCESS_MODE_SINGLE,
    ADC_ACCESS_MODE_STREAMING
} Adc_GroupAccessModeType;

/*******************************************************************************
 * @brief   Hành vi khi StartGroupConversion được gọi trong lúc group đang bận
 * @details - ABORT_RESTART: hủy và chạy lại từ đầu
 *          - SKIP: bỏ qua request mới
 ******************************************************************************/
typedef enum
{
    ADC_REPL_ABORT_RESTART,
    ADC_REPL_SKIP
} Adc_GroupReplacementType;

/*******************************************************************************
 * @brief   Thời gian lấy mẫu của ADC theo chu kỳ clock
 * @details Chọn càng lâu thì lấy mẫu càng chính xác (hạn chế nhiễu), nhưng tốc
 *          độ chậm hơn.
 ******************************************************************************/
typedef enum
{
    ADC_SAMPLINGTIME_1CYCLE5 = ADC_SampleTime_1Cycles5,
    ADC_SAMPLINGTIME_7CYCLES5 = ADC_SampleTime_7Cycles5,
    ADC_SAMPLINGTIME_13CYCLES5 = ADC_SampleTime_13Cycles5,
    ADC_SAMPLINGTIME_28CYCLES5 = ADC_SampleTime_28Cycles5,
    ADC_SAMPLINGTIME_41CYCLES5 = ADC_SampleTime_41Cycles5,
    ADC_SAMPLINGTIME_55CYCLES5 = ADC_SampleTime_55Cycles5,
    ADC_SAMPLINGTIME_71CYCLES5 = ADC_SampleTime_71Cycles5,
    ADC_SAMPLINGTIME_239CYCLES5 = ADC_SampleTime_239Cycles5
} Adc_SamplingTimeType;

/*******************************************************************************
 * @brief   Con trỏ hàm callback được gọi khi conversion hoàn tất
 * @details Dùng để gọi ngắt hoặc thông báo xử lý giá trị ADC sau khi thu đủ mẫu
 ******************************************************************************/
typedef void (*Adc_NotifyFnType)(void);

/*******************************************************************************
 * @brief   Mức độ ưu tiên giữa các group
 * @details Group có số priority nhỏ hơn sẽ được ưu tiên chạy trước nếu có xung
 *          trigger đồng thời
 ******************************************************************************/
typedef uint8 Adc_PriorityType;

/*******************************************************************************
 * @brief   Trạng thái năng lượng của ADC
 * @details Hữu ích khi hệ thống có quản lý tiết kiệm năng lượng, sleep hoặc
 *          shutdown ADC
 ******************************************************************************/
typedef enum
{
    ADC_POWER_STATE_ON,
    ADC_POWER_STATE_SLEEP,
    ADC_POWER_STATE_OFF
} Adc_PowerStateType;

/*******************************************************************************
 * @brief   Kiểu cạnh tín hiệu trigger phần cứng
 * @details - RISING: cạnh lên (LOW → HIGH)
 *          - FALLING: cạnh xuống (HIGH → LOW)
 ******************************************************************************/
typedef enum
{
    ADC_HW_TRIG_RISING,
    ADC_HW_TRIG_FALLING
} Adc_HardwareTriggerSignalType;

/*********************************************************************
 * @brief   Cấu trúc cấu hình cho từng Group ADC
 * @details Mỗi group đại diện cho một tập hợp các channel ADC sẽ được
 *          chuyển đổi cùng lúc theo thứ tự định sẵn. Group này lưu giữ
 *          đầy đủ thông tin cấu hình phần cứng (sampling, trigger, mode)
 *          và phần mềm (streaming, notification).
 *
 * groupId             ID định danh group
 * channelList         Danh sách các channel thuộc group
 * numberOfChannels    Số lượng channel trong group
 * conversionMode      ONESHOT hoặc CONTINUOUS
 * accessMode          SINGLE hay STREAMING
 * triggerSource       Trigger từ phần mềm hay phần cứng
 * hwTriggerSignal     Cạnh lên/xuống nếu dùng hardware trigger
 * streamBufferMode    Kiểu ghi buffer trong streaming (linear/circular)
 * numSamples          Số mẫu cần thu nếu streaming
 * notification        Callback khi ADC hoàn tất chuyển đổi group
 * priority            Mức ưu tiên thực thi giữa các group
 * hwUnit              ADC phần cứng sử dụng (ADC1/ADC2)
 * refVoltage          Tham chiếu ADC (nội/ngoại)
 * samplingTime        Thời gian lấy mẫu áp dụng cho toàn group
 * resolution          Độ phân giải ADC (8/10/12 bit)
 * inputMode           Chế độ đầu vào (single-ended/differential)
 * groupReplacement    Cách xử lý nếu gọi lại group khi đang bận
 *********************************************************************/
typedef struct
{
    Adc_GroupType groupId;
    const Adc_ChannelType *channelList;
    uint8 numberOfChannels;

    Adc_GroupConvModeType conversionMode;
    Adc_GroupAccessModeType accessMode;

    Adc_TriggerSourceType triggerSource;
    Adc_HardwareTriggerSignalType hwTriggerSignal;

    Adc_StreamBufferModeType streamBufferMode;
    Adc_StreamNumSampleType numSamples;

    Adc_NotifyFnType notification;

    Adc_PriorityType priority;
    Adc_HwUnitType hwUnit;

    Adc_RefType refVoltage;
    Adc_SamplingTimeType samplingTime;
    Adc_ResolutionType resolution;
    Adc_InputModeType inputMode;

    Adc_GroupReplacementType groupReplacement;
} Adc_GroupConfigType;

/*******************************************************************************
 * @brief   Cấu hình tổng ADC Driver – chứa các group đã khai báo
 * @details Dữ liệu này sẽ được truyền vào hàm Adc_Init(). Có thể khai báo nhiều
 *          group với các cấu hình khác nhau
 ******************************************************************************/
typedef struct
{
    const Adc_GroupConfigType *groupConfigPtr;
    uint8 numberOfGroups;
} Adc_ConfigType;

#endif /* ADC_TYPES_H */
