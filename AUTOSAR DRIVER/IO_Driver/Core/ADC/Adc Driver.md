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

# Phần 2. Cấu trúc Hoạt động chi tiết

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

---

### 5. So sánh cấu trúc với Polling và DMA

| Cách truyền kết quả    | Cần buffer không? | Ai điều khiển việc ghi?          | Khi nào ghi vào buffer?                                  |
| ---------------------- | ----------------- | -------------------------------- | -------------------------------------------------------- |
| **Polling (thủ công)** | Có                | Người dùng qua `Adc_ReadGroup()` | Sau khi kiểm tra `EOC` và đọc `ADC_GetConversionValue()` |
| **Interrupt**          | Có                | ISR (trong driver)               | Khi `EOC` interrupt xảy ra                               |
| **DMA**                | Có                | Bộ DMA hardware                  | Tự động ghi khi mỗi mẫu ADC hoàn thành                   |

## 2.3 Tích hợp ngắt và DMA trong ADC Driver

Trong các hệ thống nhúng thời gian thực, việc tích hợp **ngắt (Interrupt)** và **DMA (Direct Memory Access)** giúp nâng cao hiệu suất đọc ADC, giảm tải CPU, và tối ưu hóa luồng dữ liệu. Phần này phân tích bản chất, cách triển khai và mối liên hệ của hai cơ chế trên trong ADC Driver chuẩn AUTOSAR.

---

### 1. **Ngắt (Interrupt-based conversion)**

| Mục                | Mô tả                                                                                                                     |
| ------------------ | ------------------------------------------------------------------------------------------------------------------------- |
| **Ý nghĩa**        | Cho phép ADC tự động thông báo cho CPU (qua ngắt) khi đã có kết quả chuyển đổi. CPU không cần polling.                    |
| **Cách hoạt động** | Sau khi ADC hoàn thành 1 mẫu (EOC – End of Conversion), cờ EOC được set và ISR (Interrupt Service Routine) sẽ được gọi.   |
| **Ứng dụng**       | Phù hợp các hệ thống yêu cầu phản hồi nhanh mà vẫn tiết kiệm CPU, ví dụ cảnh báo ngưỡng, thu thập dữ liệu không liên tục. |

#### Phân tích triển khai:

- Cần bật `ADC_IT_EOC` bằng `ADC_ITConfig(...)`.
- Cấu hình `NVIC` để bật ngắt ADC1_2.
- ISR xác định `groupId` đang hoạt động bằng biến toàn cục `adcActiveGroupId[]`.
- ISR gọi `Adc_HandleConversionComplete(groupId, rawValue)` để xử lý lưu buffer + gọi callback.

```c
// Trong ADC ISR:
if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
    Adc_HandleConversionComplete(groupId, ADC_GetConversionValue(ADC1));
}
```

---

### 2. **DMA (Direct Memory Access)**

| Mục                | Mô tả                                                                                                          |
| ------------------ | -------------------------------------------------------------------------------------------------------------- |
| **Ý nghĩa**        | DMA cho phép phần cứng ADC ghi trực tiếp dữ liệu vào bộ nhớ mà không cần CPU can thiệp từng mẫu.               |
| **Cách hoạt động** | ADC sau khi chuyển đổi sẽ tự động đẩy kết quả vào buffer qua DMA channel. CPU chỉ cần xử lý kết quả cuối cùng. |
| **Ứng dụng**       | Rất phù hợp với STREAMING hoặc multi-channel sampling – giảm tải CPU, tăng throughput.                         |

#### Phân tích triển khai:

- Cần bật clock cho DMA1 (`RCC_AHBPeriph_DMA1`).
- Gán DMA_MemoryBaseAddr là buffer riêng của `group`:
  ```c
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adcGroupBuffers[group];
  ```
- Gán PeripheralBaseAddr là thanh ghi `ADCx->DR`.
- Bật `ADC_DMACmd(adc, ENABLE)` để cho phép ADC sử dụng DMA.
- Không cần polling hoặc ISR, chỉ cần đọc buffer khi đủ mẫu.

---

### So sánh ngắt và DMA:

| Tiêu chí                | Ngắt                           | DMA                                   |
| ----------------------- | ------------------------------ | ------------------------------------- |
| Cần CPU xử lý từng mẫu? | Có (ISR gọi mỗi lần EOC)       | Không (tự động ghi vào RAM)           |
| Phù hợp với             | Oneshot, low-sample apps       | Streaming, high-speed sampling        |
| Phức tạp cài đặt        | Trung bình                     | Cao hơn (cần DMA + cấu hình chi tiết) |
| Sử dụng RAM             | Ít hơn (có thể xử lý từng mẫu) | Nhiều hơn (phải cấp sẵn buffer lớn)   |

---

### Mối liên hệ với AUTOSAR

AUTOSAR cho phép cấu hình **Read Mode** và **Transfer Mode** trong mỗi `Adc_GroupConfigType` để chọn giữa:

- `Adc_ReadModeType` = `ADC_READ_MODE_POLLING` hoặc `ADC_READ_MODE_INTERRUPT`
- `Adc_TransferModeType` = `ADC_TRANSFER_MODE_DIRECT` hoặc `ADC_TRANSFER_MODE_DMA`

Điều này cho phép mở rộng driver linh hoạt, không cần thay đổi lõi API.


# 3. Luồng hoạt động API

## 3.1. Luồng hoạt động tổng thể của ADC Driver

ADC Driver theo chuẩn AUTOSAR hoạt động theo mô hình định hướng cấu hình (configuration-based), nơi mà tất cả các thông tin như trigger, buffer, số kênh... đều được định nghĩa trước trong file cấu hình. Sau đó, các API được gọi tuần tự để điều khiển quá trình chuyển đổi ADC. Phần này trình bày luồng tổng thể từ khởi tạo đến kết thúc một chu kỳ đọc dữ liệu, kèm sơ đồ minh họa rõ ràng.

---

### 1. Khởi tạo – `Adc_Init()`

**Mục đích:**
- Bật clock ADCx, thực hiện reset + calibration
- Thiết lập các thông số phần cứng như scan mode, trigger, số channel, chế độ chuyển đổi (one-shot/continuous)

**Luồng thực hiện:**
```
User → Adc_Init(&Adc_Config) 
    └─> Vòng lặp qua từng group trong Config
           └─> Cấu hình ADCx (ADC_Init)
           └─> Bật ADC + thực hiện Calibration
```

---

### 2. Thiết lập buffer – `Adc_SetupResultBuffer()`

**Mục đích:**
- Gán con trỏ buffer mà kết quả ADC sẽ được lưu vào
- Phân biệt rõ từng group có vùng nhớ riêng

**Luồng:**
```
User → Adc_SetupResultBuffer(group, bufferPtr)
    └─> adcGroupBuffers[group] = bufferPtr
```

---

### 3. Bắt đầu chuyển đổi – `Adc_StartGroupConversion()`

**Mục đích:**
- Cấu hình danh sách kênh cho ADC
- Bắt đầu trigger (phần mềm hoặc phần cứng)
- Nếu có interrupt hoặc DMA thì khởi động tương ứng

**Luồng:**
```
User → Adc_StartGroupConversion(group)
    ├─> Gán activeGroupId
    ├─> Cấu hình channelList cho ADCx
    ├─> Nếu dùng DMA → gọi Adc_Dma_Config()
    ├─> Nếu dùng Interrupt → gọi Adc_Nvic_Config()
    └─> ADC_SoftwareStartConvCmd(ENABLE)
```

---

### 4. Trong quá trình chuyển đổi:

#### Trường hợp 1: Dùng Interrupt
- Mỗi lần ADC hoàn tất 1 mẫu → ngắt xảy ra
- Hàm ISR gọi `Adc_HandleConversionComplete()`
- Ghi vào buffer, tăng sampleIndex, gọi callback nếu xong

```
EOC → ISR → Adc_HandleConversionComplete(group, value)
    ├─> Ghi buffer
    ├─> Kiểm tra đủ mẫu → gọi callback (nếu bật)
```

#### Trường hợp 2: Dùng DMA
- DMA tự động copy giá trị từ ADC_DR → buffer
- Người dùng có thể kiểm tra số mẫu đầy theo cơ chế riêng (flag, polling hoặc callback từ DMA)

---

### 5. Đọc dữ liệu – `Adc_ReadGroup()`

**Mục đích:**
- Đọc dữ liệu (nếu không dùng interrupt/DMA)
- Được gọi thủ công bởi ứng dụng

**Luồng:**
```
User → Adc_ReadGroup(group, bufferPtr)
    ├─> Đọc từng giá trị từ ADC_GetConversionValue
    └─> Gọi Adc_HandleConversionComplete() cho mỗi giá trị
```

---

### 6. Dừng chuyển đổi – `Adc_StopGroupConversion()`

**Mục đích:** Dừng phần cứng ADC và cập nhật trạng thái
```
User → Adc_StopGroupConversion(group)
    └─> ADC_SoftwareStartConvCmd(DISABLE)
    └─> Status = IDLE
```

---

### 7. Callback (nếu enable notification)
Nếu người dùng bật notification bằng:
```c
Adc_EnableGroupNotification(group);
```
→ thì `Adc_HandleConversionComplete()` sẽ gọi hàm `notification()` được định nghĩa trong cấu hình sau khi group hoàn tất lấy đủ mẫu.

---

## Sơ đồ tổng quát

```
+-------------+         +------------------------+         +-----------------+
| Adc_Init()  | ----->  | Adc_SetupResultBuffer()| ----->  | Adc_StartGroupConversion() |
+-------------+         +------------------------+         +-----------------+
                                                                |
                                                      ┌─────────┴──────────────┐
                                                      ▼                        ▼
                                            +-------------------+    +-------------------+
                                            | Adc_Nvic_Config() |    | Adc_Dma_Config()  |
                                            +-------------------+    +-------------------+
                                                      |
                                              +-------------------+
                                              | Conversion Ongoing |
                                              +-------------------+
                                                      |
                                       +-------------------------------+
                                       | ISR → Adc_HandleConversion... |
                                       +-------------------------------+
                                                      |
                                     ┌──────────────┬───────────────┐
                                     ▼              ▼               ▼
                            Ghi buffer    Check sampleCount    Gọi callback
```

---



## 3.2 – Phân tích chi tiết từng nhánh luồng theo cấu hình

Sau khi đã trình bày tổng thể luồng hoạt động ADC Driver, phần này đi sâu vào phân tích từng trường hợp cấu hình khác nhau mà driver hỗ trợ, bao gồm:

---

### A. Trường hợp 1 – Chế độ SINGLE + POLLING

| **Đặc điểm** | **Chi tiết**                               |
| ------------ | ------------------------------------------ |
| AccessMode   | `ADC_ACCESS_MODE_SINGLE`                   |
| ConvMode     | `ADC_CONV_MODE_ONE_SHOT` hoặc `CONTINUOUS` |
| ReadMode     | `POLLING`                                  |
| Trigger      | Phần mềm (`ADC_TRIGG_SRC_SW`)              |
| DMA          | Không dùng                                 |

**Luồng hoạt động:**

1. `Adc_Init()` cấu hình ADCx.
2. `Adc_SetupResultBuffer()` gán buffer cho group.
3. `Adc_StartGroupConversion(group)`:
   - Gọi `ADC_SoftwareStartConvCmd(adc, ENABLE)`
   - Đặt trạng thái group = BUSY.
4. Trong main loop:
   - Gọi `Adc_ReadGroup(group, buffer)`:
     - Polling `ADC_FLAG_EOC`, đọc giá trị.
     - Ghi vào buffer.
     - Gọi `Adc_HandleConversionComplete()`.
5. Trạng thái group chuyển sang `COMPLETED`.

**Phù hợp khi:** cần thao tác đơn giản, ít kênh, không yêu cầu thời gian thực.

---

### B. Trường hợp 2 – Chế độ STREAMING + POLLING

| **Đặc điểm** | **Chi tiết**                |
| ------------ | --------------------------- |
| AccessMode   | `ADC_ACCESS_MODE_STREAMING` |
| ConvMode     | `ADC_CONV_MODE_CONTINUOUS`  |
| ReadMode     | `POLLING`                   |
| Trigger      | Phần mềm                    |
| DMA          | Không dùng                  |

**Luồng hoạt động:**

1. `Adc_Init()`, `Adc_SetupResultBuffer()`, `Adc_StartGroupConversion(group)`
2. Trong main loop:
   - Lặp nhiều lần `Adc_ReadGroup()`, mỗi lần ghi vào `adcGroupBuffers[group][adcSampleIndex[group]]`
   - Tăng `adcSampleIndex`, so sánh với `numSamples`.
3. Khi đủ mẫu:
   - Dừng chuyển đổi.
   - Trạng thái `COMPLETED`, gọi callback nếu có.

**Phù hợp khi:** cần lấy liên tục nhiều mẫu nhưng không muốn dùng interrupt.

---

### C. Trường hợp 3 – STREAMING hoặc SINGLE + INTERRUPT

| **Đặc điểm** | **Chi tiết** |
| ------------ | ------------ |
| ReadMode     | `INTERRUPT`  |
| DMA          | Không dùng   |

**Luồng hoạt động:**

1. `Adc_StartGroupConversion()`:
   - Gọi `ADC_ITConfig()`, cấu hình NVIC.
2. Khi chuyển đổi xong:
   - `ISR` được gọi (`ADC_1_2_IRQHandler`)
   - Gọi `Adc_HandleConversionComplete(group, value)`:
     - Ghi vào buffer.
     - Tăng index.
     - Nếu đủ mẫu → dừng, cập nhật trạng thái, gọi callback.

**Ưu điểm:** không chiếm CPU, phù hợp real-time system.

---

### D. Trường hợp 4 – STREAMING + DMA

| **Đặc điểm** | **Chi tiết**                     |
| ------------ | -------------------------------- |
| AccessMode   | `STREAMING`                      |
| TransferMode | `DMA`                            |
| ReadMode     | Không cần quan tâm (DMA tự động) |

**Luồng hoạt động:**

1. `Adc_StartGroupConversion()`:
   - Gọi `Adc_Dma_Config(group)`
   - Gán địa chỉ `adcGroupBuffers[group]` vào DMA.
   - Gọi `ADC_DMACmd(adc, ENABLE)`.
2. DMA sẽ tự động ghi dữ liệu vào buffer.
3. Khi đủ mẫu (hoặc dùng DMA interrupt):
   - Gọi callback từ ISR của DMA hoặc kiểm tra số lượng mẫu.

**Ưu điểm:** hiệu suất cao, không chiếm CPU, dùng cho hệ thống nhiều mẫu/ tốc độ cao.

---

### Tổng kết phân nhánh luồng

| Chế độ        | ReadMode  | Phương pháp đọc          | Ghi chú                |
| ------------- | --------- | ------------------------ | ---------------------- |
| SINGLE        | POLLING   | `Adc_ReadGroup` thủ công | Đơn giản, dễ dùng      |
| STREAM        | POLLING   | `ReadGroup` nhiều lần    | Đủ mẫu thì tự dừng     |
| SINGLE/STREAM | INTERRUPT | ISR tự xử lý             | Tốt cho thời gian thực |
| STREAM        | DMA       | Không cần gọi hàm đọc    | Tối ưu hiệu năng       |

---

# 4. Tổng kết và ví dụ thực tiễn

## 4.1 Tổng kết kiến thức chính

| Mục tiêu học                              | Nội dung tóm tắt                                                                                                                                                                                            |
| ----------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Hiểu cấu trúc driver**                  | Driver ADC chia thành nhiều group độc lập, mỗi group có thể tùy chỉnh danh sách channel, trigger, mode đọc, số mẫu,…                                                                                        |
| **Nắm được các mode chính**               | Oneshot – đọc 1 lần; Streaming – đọc liên tục; Continuous – tự động lặp lại; Trigger từ SW/HW                                                                                                               |
| **Phân biệt luồng Polling/Interrupt/DMA** | Polling: người dùng gọi `Adc_ReadGroup()` để lấy dữ liệu. <br> Interrupt: tự ngắt sau mỗi lần đọc xong và gọi callback. <br> DMA: tự động lưu vào buffer sau mỗi lần chuyển đổi mà không cần CPU can thiệp. |
| **Sử dụng buffer hiệu quả**               | Mỗi group cần gán buffer trước bằng `Adc_SetupResultBuffer()` để có nơi lưu dữ liệu. Buffer cần đủ dung lượng = số channel × số mẫu.                                                                        |
| **Hiểu luồng thực thi API**               | Init → SetupBuffer → StartGroupConversion → [chuyển đổi/ghi dữ liệu] → [callback hoặc đọc thủ công] → Stop hoặc lặp lại                                                                                     |

## 4.2 Ví dụ ứng dụng thực tế

### Bài toán: Giám sát điện áp pin trong robot

**Yêu cầu**:

- Đọc điện áp từ 2 pin (ADC_Channel_0 và ADC_Channel_1)
- Đọc mỗi 5ms (sử dụng timer trigger)
- Lưu liên tục 10 mẫu gần nhất để phân tích trung bình
- Sử dụng DMA để giảm tải CPU
- Callback sau mỗi 10 mẫu để tính toán điện áp trung bình

**Cấu hình** `Adc_GroupConfigType`:

```c
static const Adc_ChannelType batMonitorChannels[] = {ADC_CHANNEL_0, ADC_CHANNEL_1};

const Adc_GroupConfigType AdcGroup_BatteryMonitor = {
    .groupId = 0,
    .channelList = batMonitorChannels,
    .numberOfChannels = 2,
    .conversionMode = ADC_CONV_MODE_CONTINUOUS,
    .accessMode = ADC_ACCESS_MODE_STREAMING,
    .triggerSource = ADC_TRIGG_SRC_HW,
    .hwTriggerSignal = ADC_HW_TRIG_RISING,
    .streamBufferMode = ADC_STREAM_BUFFER_LINEAR,
    .numSamples = 10,
    .notification = BatteryMonitor_Callback,
    .priority = 1,
    .hwUnit = ADC_1,
    .refVoltage = ADC_REF_VOLTAGE_DEFAULT,
    .samplingTime = ADC_SAMPLE_TIME_28CYCLES,
    .resolution = ADC_RES_12BIT,
    .inputMode = ADC_INPUT_SINGLE_ENDED,
    .groupReplacement = ADC_GROUP_REPL_ABORT_RESTART,
    .readMode = ADC_READ_MODE_INTERRUPT,
    .transferMode = ADC_TRANSFER_MODE_DMA
};
```

**Hàm callback**:

```c
void BatteryMonitor_Callback(void) {
    Adc_ValueGroupType *buffer;
    if (Adc_GetStreamLastPointer(0, &buffer) == E_OK) {
        float avg0 = 0, avg1 = 0;
        for (int i = 0; i < 10; i++) {
            avg0 += buffer[i * 2];
            avg1 += buffer[i * 2 + 1];
        }
        avg0 /= 10.0;
        avg1 /= 10.0;

        // Cập nhật hệ thống hoặc báo hiệu nếu pin thấp
    }
}
```

## 4.3 Gợi ý mở rộng

| Hướng mở rộng                             | Ý tưởng                                                               |
| ----------------------------------------- | --------------------------------------------------------------------- |
| **Tối ưu bộ nhớ**                         | Dùng buffer dạng vòng để lưu liên tục nhưng tiết kiệm RAM.            |
| **Tích hợp vào hệ thống RTOS**            | Callback có thể sử dụng semaphore hoặc queue để gửi dữ liệu cho task. |
| **Chuyển sang dùng DMA ngắt cuối chu kỳ** | Giúp cập nhật dữ liệu song song với CPU.                              |
| **Kết hợp nhiều group đồng thời**         | Tăng khả năng đo đa dạng sensor với thời gian khác nhau.              |
