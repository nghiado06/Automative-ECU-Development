# ADC DRIVER

ADC (Analog-to-Digital Converter) Driver là một driver trực thuộc thấp nhất trong hệ thống AUTOSAR MCAL (Microcontroller Abstraction Layer). Driver này chịu trách nhiệm trực tiếp giao tiếp với module ADC của vi điều khiển nhằm chuyển đổi tín hiệu analog thành số digital.

**Chức năng chính**:
- Cung cấp API cho các module cao hơn truy cập dữ liệu ADC
- Cài đặt các group, chế độ đọc, trigger, buffer
- Tích hợp ngắt hoặc DMA để tối ưu hiệu suất truy nhập ADC

Trong các hệ thống nhúng (embedded), việc trẫn hoá ADC là cần thiết để:
- Đồng bộ điều khiển ADC từ nhiều module khác nhau
- Gọi API mà không quan tâm đến chi tiết phần cứng
- Dễ dàng chuyển đổi qua vi điều khiển khác trong tương lai

**Hôm nay mình sẽ chia sẻ về các nội dung sau:**

- **Phần 1: Tổng quan về cấu trúc ADC Driver**
  - Các kiểu dữ liệu (type)
  - Các API cơ bản

- **Phần 2: Cấu trúc hoạt động chi tiết**
  - So sánh streaming vs one-shot
  - Phân tích buffer, sample index
  - Tích hợp ngắt, DMA

- **Phần 3: Luồng hoạt động API**
  - Thứ tự các hành động gọi API
  - Biểu đồ trình tự driver hoạt động

- **Phần 4: Tổng kết và ví dụ ứng dụng**
  - Đánh giá driver
  - 3 ví dụ ứng dụng thực tế

Và để cho các bạn có thể nẵm kỹ hơn về adc các bạn có thể đọc kỹ hơn về ADC ở đây [ADC_SPL](../../../../STM32_SPL_Programming/ADC/ADC.md).

---

# Phần 1: Tổng quan về cấu trúc của ADC Driver

## 1.1 Các type của ADC Driver – Ý nghĩa và Chức năng

AUTOSAR ADC Driver định nghĩa rất nhiều kiểu dữ liệu (type) để hỗ trợ khả năng trừu tượng hóa và mở rộng linh hoạt. Dưới đây là những kiểu dữ liệu quan trọng nhất, cùng phân tích bản chất và vai trò của chúng:

| STT | Tên Type                        | Kiểu / Giá trị                                        | Chức năng / Ý nghĩa                                                                                     |
| --- | ------------------------------- | ----------------------------------------------------- | ------------------------------------------------------------------------------------------------------- |
| 1   | `Adc_GroupType`                 | `uint8`                                               | Đại diện cho một nhóm chuyển đổi ADC. Mỗi group chứa các thông tin cấu hình riêng như channel, trigger. |
| 2   | `Adc_ChannelType`               | enum / uint8                                          | Đại diện cho các kênh vật lý của ADC, dùng trong `channelList` để định nghĩa kênh thuộc group.          |
| 3   | `Adc_GroupAccessModeType`       | `ADC_ACCESS_MODE_SINGLE`, `ADC_ACCESS_MODE_STREAMING` | Quy định cách lấy dữ liệu: 1 lần hay liên tục. SINGLE là one-shot, STREAMING là lấy nhiều mẫu.          |
| 4   | `Adc_GroupConvModeType`         | `ADC_CONV_MODE_ONE_SHOT`, `ADC_CONV_MODE_CONTINUOUS`  | Điều khiển chế độ chuyển đổi phần cứng: 1 lần hoặc liên tục.                                            |
| 5   | `Adc_TriggerSourceType`         | `ADC_TRIGG_SRC_SW`, `ADC_TRIGG_SRC_HW`,...            | Loại trigger (phần mềm hoặc phần cứng).                                                                 |
|     | `Adc_HardwareTriggerSignalType` | `ADC_HW_TRIG_RISING`, `ADC_HW_TRIG_FALLING`,...       | Xác định tín hiệu trigger phần cứng cụ thể.                                                             |
| 6   | `Adc_StreamNumSampleType`       | `uint16`                                              | Số mẫu cần lấy trong một group streaming.                                                               |
| 7   | `Adc_NotifyFnType`              | `void (*)(void)`                                      | Hàm callback được gọi khi chuyển đổi hoàn tất (nếu enable notification).                                |
| 8   | `Adc_ValueGroupType`            | `uint16`                                              | Giá trị ADC đọc được. Dùng làm phần tử của buffer.                                                      |
| 9   | `Adc_GroupConfigType`           | struct                                                | Cấu trúc chính chứa toàn bộ cấu hình cho 1 group.                                                       |

### Cấu trúc chi tiết `Adc_GroupConfigType`

```c
typedef struct {
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
    Adc_ReadModeType readMode;          // [Tùy chọn mở rộng] Polling hay Interrupt
    Adc_TransferModeType transferMode;  // [Tùy chọn mở rộng] DMA hay trực tiếp
} Adc_GroupConfigType;
```

**Phân tích:**  
`Adc_GroupConfigType` là cấu trúc trung tâm cho mỗi group, gom toàn bộ thông tin cấu hình như số kênh, trigger, chế độ đọc, kiểu lưu buffer,...  
Có thể mở rộng thêm trường (như readMode, transferMode) để bổ sung tính năng mà không ảnh hưởng tới các nhóm khác.


## 1.2. Các API chính trong ADC Driver

ADC Driver theo chuẩn AUTOSAR cung cấp một tập các API để quản lý quá trình khởi tạo, điều khiển, đọc dữ liệu và xử lý callback. Dưới đây là danh sách các API cùng với phân tích chức năng, điều kiện sử dụng và vai trò của từng hàm.

| Tên hàm                                                                              | Chức năng                    | Mô tả chi tiết                                                                                                                                                                       |
| ------------------------------------------------------------------------------------ | ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `Adc_Init(const Adc_ConfigType *ConfigPtr)`                                          | Khởi tạo ADC                 | Cấu hình toàn bộ các nhóm ADC theo thông tin trong `ConfigPtr`: chọn ADCx, số kênh, trigger, mode liên tục hay không. Bắt buộc phải gọi trước khi sử dụng bất kỳ chức năng nào khác. |
| `Adc_DeInit(void)`                                                                   | Gỡ cấu hình ADC              | Đưa ADC về trạng thái mặc định, reset trạng thái các group, có thể dùng trong tình huống tắt hệ thống hoặc re-init.                                                                  |
| `Adc_SetupResultBuffer(Adc_GroupType group, Adc_ValueGroupType *DataBufferPtr)`      | Cấu hình buffer              | Gán vùng nhớ để driver lưu giá trị chuyển đổi của ADC. Hàm này là bắt buộc nếu muốn sử dụng ở chế độ streaming.                                                                      |
| `Adc_StartGroupConversion(Adc_GroupType group)`                                      | Bắt đầu chuyển đổi           | Kích hoạt quá trình chuyển đổi của một nhóm ADC. Dựa vào cấu hình, hàm này sẽ khởi tạo ngắt hoặc DMA nếu cần. Tự động bật trigger phần mềm nếu được cấu hình.                        |
| `Adc_StopGroupConversion(Adc_GroupType group)`                                       | Dừng chuyển đổi              | Ngưng quá trình lấy mẫu ADC của một group. Dùng khi muốn dừng thủ công trong ứng dụng hoặc khi đủ số mẫu cần lấy.                                                                    |
| `Adc_ReadGroup(Adc_GroupType group, Adc_ValueGroupType *DataBufferPtr)`              | Đọc dữ liệu ADC              | Đọc kết quả chuyển đổi (theo polling). Nếu conversion chưa xong (cờ EOC chưa bật), hàm trả về `E_NOT_OK`. Có thể gọi trong vòng lặp để lấy dữ liệu từng mẫu hoặc toàn bộ.            |
| `Adc_GetGroupStatus(Adc_GroupType group)`                                            | Truy vấn trạng thái          | Trả về trạng thái hiện tại của group: `ADC_IDLE`, `ADC_BUSY`, hoặc `ADC_COMPLETED`. Hữu ích để kiểm tra tiến độ từ bên ngoài.                                                        |
| `Adc_EnableGroupNotification(Adc_GroupType group)`                                   | Bật callback                 | Kích hoạt gọi hàm callback sau khi group hoàn tất chuyển đổi. Điều kiện: group đã có hàm `notification` hợp lệ trong cấu hình.                                                       |
| `Adc_DisableGroupNotification(Adc_GroupType group)`                                  | Tắt callback                 | Ngưng gọi callback của group sau khi chuyển đổi. Dùng để chủ động tắt xử lý không cần thiết.                                                                                         |
| `Adc_GetStreamLastPointer(Adc_GroupType group, Adc_ValueGroupType **PtrToSamplePtr)` | Lấy mẫu cuối cùng            | Trả về con trỏ đến phần tử cuối cùng đã ghi trong buffer streaming. Dùng cho ứng dụng xử lý dữ liệu online.                                                                          |
| `Adc_EnableHardwareTrigger(Adc_GroupType group)`                                     | (Dự phòng) Bật trigger ngoài | Dành cho hệ thống dùng trigger từ phần cứng (ví dụ: timer, PWM). Tùy platform có thể mở rộng thêm.                                                                                   |
| `Adc_DisableHardwareTrigger(Adc_GroupType group)`                                    | (Dự phòng) Tắt trigger ngoài | Ngược lại với hàm trên.                                                                                                                                                              |
| `Adc_GetVersionInfo(Std_VersionInfoType *versioninfo)`                               | Truy xuất version            | Cho phép kiểm tra thông tin phiên bản của driver. Hữu ích cho mục đích kiểm thử, traceability.                                                                                       |

> **Lưu ý:** Trong chuẩn AUTOSAR, nhiều hàm phải được gọi theo thứ tự bắt buộc để tránh lỗi logic. Ví dụ: `Adc_SetupResultBuffer()` phải được gọi trước `Adc_StartGroupConversion()` nếu sử dụng streaming mode.

# 2. CẤU TRÚC HOẠT ĐỘNG CHI TIẾT

## 2.1 Phân tích chế độ Streaming và One-shot trong ADC Driver

Trong ADC Driver theo chuẩn AUTOSAR, chế độ truy cập dữ liệu kết quả được xác định thông qua `Adc_GroupAccessModeType`, với hai chế độ chính: **SINGLE (One-shot)** và **STREAMING**. Mỗi chế độ phản ánh một cách thu thập và xử lý dữ liệu khác nhau, phù hợp với các yêu cầu ứng dụng riêng biệt.

| Thuộc tính                          | SINGLE (One-shot)                                  | STREAMING (Liên tục)                                         |
| ----------------------------------- | -------------------------------------------------- | ------------------------------------------------------------ |
| **Enum tương ứng**                  | `ADC_ACCESS_MODE_SINGLE`                           | `ADC_ACCESS_MODE_STREAMING`                                  |
| **Số lần chuyển đổi**               | 1 lần sau mỗi trigger                              | Nhiều lần, liên tục cho đến khi đủ số mẫu hoặc bị dừng       |
| **Ứng dụng điển hình**              | Đọc cảm biến đơn lẻ, kiểm tra nhanh giá trị analog | Ghi nhận tín hiệu dao động, phân tích dữ liệu thời gian thực |
| **Buffer**                          | Không yêu cầu hoặc chỉ 1 phần tử                   | Cần buffer lớn tương ứng với `numSamples * số kênh`          |
| **Cấu trúc xử lý**                  | Đọc xong → gọi callback / trả về ngay              | Mỗi mẫu được ghi vào buffer → callback khi đủ số lượng       |
| **Chỉ số mẫu (SampleIndex)**        | Không cần                                          | Cần theo dõi để ghi đúng vào buffer và xác định khi đủ mẫu   |
| **Kiểm tra kết thúc** (`GetStatus`) | Trả về `ADC_COMPLETED` ngay sau khi hoàn tất       | Chỉ trả về `ADC_COMPLETED` khi đủ số mẫu như cấu hình        |
| **Tích hợp ngắt hoặc DMA**          | Có thể không cần (dùng polling)                    | Gợi ý dùng ngắt/DMA để giảm tải CPU                          |

### Phân tích sâu:

#### **One-shot (Single Access Mode)**

- Cách hoạt động:
  - Sau khi gọi `Adc_StartGroupConversion()`, ADC sẽ thực hiện chuyển đổi cho tất cả kênh trong group một lần duy nhất.
  - Sau khi hoàn thành, trạng thái group chuyển sang `ADC_COMPLETED`.
  - Dữ liệu được lấy qua `Adc_ReadGroup()` hoặc thông qua callback nếu có.

- Ưu điểm:
  - Cấu hình đơn giản.
  - Không yêu cầu quản lý buffer phức tạp.

- Nhược điểm:
  - Không phù hợp khi cần thu thập nhiều mẫu theo thời gian (ví dụ: theo dõi tín hiệu dao động).

#### **Streaming Mode**

- Cách hoạt động:
  - Khi gọi `Adc_StartGroupConversion()`, ADC sẽ liên tục thu thập giá trị từ các kênh trong group và lưu vào buffer.
  - Bộ đếm `adcSampleIndex[group]` sẽ tăng dần theo từng mẫu.
  - Khi đủ số mẫu (`adcSampleIndex >= numSamples`), trạng thái group chuyển sang `ADC_COMPLETED`, ADC sẽ tự dừng và callback sẽ được gọi (nếu có).

- Ưu điểm:
  - Phù hợp với ứng dụng thời gian thực, phân tích tín hiệu, giám sát liên tục.
  - Có thể kết hợp với DMA để tăng tốc độ và giảm tải CPU.

- Nhược điểm:
  - Yêu cầu cấu hình buffer đúng kích thước.
  - Cần logic quản lý chỉ số mẫu, luồng dữ liệu phức tạp hơn.

**Kết luận**:  
Việc chọn giữa `SINGLE` và `STREAMING` phụ thuộc hoàn toàn vào yêu cầu ứng dụng cụ thể. Trong hệ thống nhúng thời gian thực, chế độ streaming kết hợp với DMA là phổ biến và hiệu quả nhất.

## 2.2 Cấu trúc buffer trong ADC Driver: cách tổ chức và lưu trữ dữ liệu

ADC Driver theo chuẩn AUTOSAR sử dụng buffer để lưu trữ kết quả chuyển đổi ADC. Việc tổ chức buffer và cách ghi dữ liệu vào nó là yếu tố cốt lõi quyết định khả năng hoạt động song song, liên tục của driver. Phần này sẽ phân tích cấu trúc, nguyên lý và vai trò từng thành phần liên quan đến buffer.

### 1. Mục tiêu của buffer trong ADC Driver

| Mục đích                       | Giải thích                                                                                          |
| ------------------------------ | --------------------------------------------------------------------------------------------------- |
| Lưu trữ dữ liệu ADC            | Kết quả chuyển đổi từ phần cứng ADC được lưu vào RAM tạm thời để ứng dụng truy cập.                 |
| Hỗ trợ các chế độ khác nhau    | STREAMING cần ghi nhiều mẫu liên tục, ONE_SHOT chỉ cần một mẫu → buffer phải thích ứng cả 2 chế độ. |
| Quản lý dữ liệu cho từng group | Mỗi group sẽ có buffer riêng → độc lập giữa các group → driver dễ mở rộng & đồng thời.              |

### 2. Cấu trúc tổ chức bộ nhớ trong driver

| Biến toàn cục            | Kiểu dữ liệu           | Vai trò                                                            |
| ------------------------ | ---------------------- | ------------------------------------------------------------------ |
| `adcGroupBuffers[group]` | `Adc_ValueGroupType*`  | Con trỏ tới buffer lưu giá trị ADC của group tương ứng             |
| `adcSampleIndex[group]`  | `uint16`               | Chỉ số mẫu hiện tại trong buffer (dùng cho STREAMING)              |
| `buffer_group_X[]`       | `Adc_ValueGroupType[]` | Mảng thật lưu dữ liệu cho group X → được gán vào `adcGroupBuffers` |

💡 **Lưu ý:** Driver không tự cấp phát bộ nhớ – người dùng hoặc file cấu hình cần khai báo sẵn `buffer_group_X[]`, rồi gán bằng `Adc_SetupResultBuffer()`.

### 3. Cách ghi dữ liệu vào buffer

- **Trong chế độ ONESHOT:**
  - Ghi 1 lần duy nhất vào `buffer[0]` sau khi chuyển đổi hoàn tất.

- **Trong chế độ STREAMING:**
  - Mỗi lần có ngắt hoặc đọc thủ công:
    - Lấy `adcSampleIndex[group]` → ghi vào `buffer[index]`
    - Tăng `adcSampleIndex[group]++`
    - Kiểm tra `index >= numSamples` → nếu đủ thì:
      - Dừng ADC
      - Gọi callback nếu có
      - Cập nhật trạng thái group là `ADC_COMPLETED`

### 4. Sơ đồ minh họa luồng dữ liệu buffer

```
        +-----------------------------+
        |  ADC chuyển đổi hoàn tất   |
        +-----------------------------+
                      |
                      v
        +-----------------------------+
        |   Đọc giá trị từ hardware   |
        +-----------------------------+
                      |
                      v
        +----------------------------------------+
        | Ghi vào buffer[group][adcSampleIndex]  |
        +----------------------------------------+
                      |
                      v
        +-------------------------------+
        |   Tăng adcSampleIndex[group] |
        +-------------------------------+
                      |
             [Nếu đủ mẫu]
                      |
                      v
        +-------------------------------+
        |  Dừng ADC + gọi callback     |
        +-------------------------------+
```

### 5. So sánh cấu trúc với Polling và DMA

| Cách truyền kết quả    | Cần buffer không? | Ai điều khiển việc ghi?          | Khi nào ghi vào buffer?                                  |
| ---------------------- | ----------------- | -------------------------------- | -------------------------------------------------------- |
| **Polling (thủ công)** | Có                | Người dùng qua `Adc_ReadGroup()` | Sau khi kiểm tra `EOC` và đọc `ADC_GetConversionValue()` |
| **Interrupt**          | Có                | ISR (trong driver)               | Khi `EOC` interrupt xảy ra                               |
| **DMA**                | Có                | Bộ DMA hardware                  | Tự động ghi khi mỗi mẫu ADC hoàn thành                   |
