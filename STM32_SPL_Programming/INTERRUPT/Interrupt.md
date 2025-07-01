# INTERRUPT - NGẮT
Trong lập trình vi điều khiển STM32F103, **Interrupt (Ngắt)** là một cơ chế quan trọng giúp vi điều khiển phản ứng nhanh chóng với các sự kiện xảy ra ngoài luồng chương trình chính. Không còn cần polling (kiểm tra liên tục), thay vào đó CPU sẽ tự động nhảy vào hàm xử lý khi ngắt xảy ra.

Việc sử dụng ngắt giúp tối ưu hiệu suất và phản ứng nhanh trong các hệ thống nhúng. Trong file README này, chúng ta sẽ cùng tìm hiểu từng nội dung quan trọng xoay quanh Interrupt:

### Nội dung chính bao gồm:
- **Khái niệm và các loại ngắt** trong STM32F103
- **NVIC**: Bộ quản lý ngắt và thiết lập độ ưu tiên
- **Quy trình xử lý ngắt** theo pipeline của CPU
- **Vector table**: Bản đồ liên kết IRQ với ISR
- **ISR (Interrupt Service Routine)**: Cách viết và quy tắc xử lý
- **Ví dụ ngắt ADC kèm ISR**
- **Tổng kết** và sơ đồ hoạt động

Trong các phần tiếp theo, chúng ta sẽ đi vào từng khía cạnh chi tiết của Interrupt trên STM32F103, bao gồm cả mã lệnh mẫu và gắn với thực tế sử dụng trong Embedded System.

## 1. KHÁI NIỆM VÀ CÁC LOẠI NGẮT TRONG STM32F103

### 1.1. Khái niệm ngắt (Interrupt)

Ngắt là một cơ chế cho phép vi điều khiển **tạm dừng chương trình chính** để xử lý một **sự kiện khẩn cấp** đến từ ngoại vi hoặc hệ thống, sau đó **quay lại tiếp tục chương trình** như chưa có gì xảy ra. Điều này giúp hệ thống phản ứng nhanh mà không cần phải kiểm tra liên tục trạng thái của ngoại vi.

Ví dụ: Khi một nút nhấn được nhấn xuống, vi điều khiển có thể dừng công việc hiện tại để đọc trạng thái nút nhấn, xử lý, rồi quay lại tiếp tục công việc cũ.

### 1.2. Các loại ngắt trong STM32F103

STM32F103 cung cấp nhiều nguồn ngắt khác nhau, được chia thành 3 nhóm chính. Mỗi nhóm có đặc điểm riêng về cách sinh ngắt và cấu hình xử lý.

#### a. Ngắt ngoài (External Interrupt – EXTI)

- Được sinh ra từ **chân GPIO**, sử dụng hệ thống **EXTI (External Interrupt/Event Controller)** để phát hiện cạnh lên/xuống hoặc mức logic.
- Thường dùng để xử lý sự kiện vật lý như **nút nhấn**, **tín hiệu từ cảm biến**...

**Ví dụ cấu hình EXTI từ chân PA0:**

```c
GPIO_InitTypeDef gpio;
EXTI_InitTypeDef exti;
NVIC_InitTypeDef nvic;

// Cấu hình chân PA0 làm input
gpio.GPIO_Pin = GPIO_Pin_0;
gpio.GPIO_Mode = GPIO_Mode_IPU;
GPIO_Init(GPIOA, &gpio);

// Liên kết EXTI0 với PA0
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

// Cấu hình EXTI
exti.EXTI_Line = EXTI_Line0;
exti.EXTI_Mode = EXTI_Mode_Interrupt;
exti.EXTI_Trigger = EXTI_Trigger_Falling;
exti.EXTI_LineCmd = ENABLE;
EXTI_Init(&exti);

// Cấu hình NVIC cho EXTI0
nvic.NVIC_IRQChannel = EXTI0_IRQn;
nvic.NVIC_IRQChannelPreemptionPriority = 1;
nvic.NVIC_IRQChannelSubPriority = 0;
nvic.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&nvic);
```

#### b. Ngắt từ ngoại vi tích hợp (Peripheral Interrupt)

- Sinh ra từ **các ngoại vi bên trong vi điều khiển**, như **Timer**, **ADC**, **USART**, **DMA**...
- Khi ngoại vi hoàn thành một sự kiện quan trọng (ví dụ ADC đo xong), nó sẽ **set cờ ngắt**, và nếu được cấp phép trong NVIC thì hệ thống sẽ gọi hàm xử lý.

**Ví dụ: cấu hình ngắt từ ADC1:**

```c
// Bật ngắt ADC khi đo xong (EOC)
ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

// Cấu hình NVIC
NVIC_InitTypeDef nvic;
nvic.NVIC_IRQChannel = ADC1_2_IRQn;
nvic.NVIC_IRQChannelPreemptionPriority = 2;
nvic.NVIC_IRQChannelSubPriority = 0;
nvic.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&nvic);
```

**Ví dụ: cấu hình ngắt Timer2:**

```c
// Cho phép Timer sinh ngắt khi tràn
TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

// Bật ngắt Timer2 trong NVIC
nvic.NVIC_IRQChannel = TIM2_IRQn;
nvic.NVIC_IRQChannelPreemptionPriority = 2;
nvic.NVIC_IRQChannelSubPriority = 1;
nvic.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&nvic);
```

**Ví dụ: cấu hình ngắt USART1 khi nhận được dữ liệu:**

```c
// Cho phép ngắt khi có dữ liệu đến
USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

// Bật ngắt USART1 trong NVIC
nvic.NVIC_IRQChannel = USART1_IRQn;
nvic.NVIC_IRQChannelPreemptionPriority = 2;
nvic.NVIC_IRQChannelSubPriority = 2;
nvic.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&nvic);
```

#### c. Ngắt hệ thống (System Exception)

- Là các ngắt đặc biệt sinh ra từ lõi Cortex-M3 như:
  - **SysTick**: bộ định thời hệ thống
  - **HardFault**: lỗi nghiêm trọng (truy cập sai địa chỉ, chia 0...)
  - **NMI**: Non-Maskable Interrupt, không thể bị vô hiệu hóa

Những ngắt này thường đã được định nghĩa sẵn và có độ ưu tiên cao nhất. Khi viết chương trình, chúng ta chỉ cần định nghĩa lại các hàm xử lý tương ứng nếu muốn can thiệp.

**Ví dụ:**

```c
void SysTick_Handler(void)
{
    // Thực hiện nhiệm vụ định kỳ mỗi 1ms
    millis++;
}
```
## 2. NVIC – BỘ QUẢN LÝ NGẮT TRÊN CORTEX-M3

### 2.1. NVIC là gì?
**NVIC (Nested Vectored Interrupt Controller)** là **bộ điều khiển ngắt tích hợp** trong lõi **Cortex-M3**. Đây là thành phần **chịu trách nhiệm toàn bộ việc quản lý ngắt** trong hệ thống STM32F103.

NVIC giúp hệ thống:
- Xác định khi nào một ngắt được xử lý (Enable/Disable)
- Xử lý khi có **nhiều ngắt xảy ra cùng lúc** (sắp xếp theo độ ưu tiên)
- Cho phép **lồng ngắt** (ngắt này có thể bị ngắt bởi ngắt ưu tiên cao hơn)
- Nhảy đến đúng **hàm xử lý ngắt (IRQHandler)**

> Không có NVIC thì CPU **không biết xử lý ngắt từ đâu đến**, **nên chọn ngắt nào**, và **nhảy vào đâu** để xử lý.

---

### 2.2. Vai trò cụ thể của NVIC trong hệ thống
Giả sử hệ thống có 3 nguồn ngắt: **Timer2, ADC, và EXTI0**, cùng xảy ra. NVIC sẽ:
1. Kiểm tra xem các ngắt có được ENABLE không
2. So sánh mức **Preemption Priority** của từng ngắt
3. Chọn ngắt có ưu tiên cao nhất để xử lý
4. Nhảy vào hàm xử lý IRQ tương ứng
5. Nếu trong lúc đang xử lý, có ngắt ưu tiên **cao hơn** xuất hiện, CPU sẽ **tạm dừng ngắt hiện tại** và nhảy sang xử lý ngắt mới → Đây gọi là **lồng ngắt (Nested Interrupt)**
6. Sau khi xử lý xong, quay về tiếp tục các ngắt còn lại hoặc chương trình chính

---

### 2.3. Cấu trúc ưu tiên trong NVIC
NVIC sử dụng 2 mức ưu tiên:
- **Preemption Priority** (ưu tiên chính): quyết định xem ngắt này có thể **giành quyền xử lý** so với ngắt khác hay không.
- **SubPriority** (ưu tiên phụ): nếu 2 ngắt có cùng Preemption Priority thì SubPriority quyết định **ai xử lý trước**.

> Ưu tiên **càng thấp (số càng nhỏ)** thì càng cao.

**Ví dụ:**
- Ngắt ADC có Preemption = 1, Sub = 0
- Ngắt Timer2 có Preemption = 2, Sub = 0
→ ADC sẽ được xử lý trước

Nếu cả hai có Preemption = 1:
- Ngắt ADC có Sub = 0, Timer2 có Sub = 1 → ADC vẫn ưu tiên hơn

---

### 2.4. Cấu hình NVIC trong STM32F103
Ta dùng struct `NVIC_InitTypeDef` như sau:

```c
NVIC_InitTypeDef nvic;
nvic.NVIC_IRQChannel = ADC1_2_IRQn;
nvic.NVIC_IRQChannelPreemptionPriority = 1;
nvic.NVIC_IRQChannelSubPriority = 0;
nvic.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&nvic);
```

**Các trường cần lưu ý:**
- `NVIC_IRQChannel`: chọn IRQ tương ứng với ngoại vi (ví dụ: `ADC1_2_IRQn`, `EXTI0_IRQn`, `USART1_IRQn`...)
- `PreemptionPriority`: Ưu tiên chính, cho phép ngắt này **chen vào ngắt khác** nếu nhỏ hơn
- `SubPriority`: Ưu tiên phụ, quyết định xử lý trước khi các ngắt cùng Preemption
- `Cmd = ENABLE`: Cho phép xử lý ngắt này qua NVIC

> Có thể tra bảng danh sách `IRQn_Type` trong `stm32f10x.h` để biết tên các kênh ngắt.

---

### 2.5. NVIC lồng ngắt (Nested Interrupts)
Khi một ngắt đang được xử lý, nếu một ngắt khác có **Preemption Priority cao hơn** xuất hiện, NVIC có thể **ngắt tạm thời ngắt hiện tại**, xử lý ngắt mới, rồi quay lại ngắt cũ → gọi là **nested interrupt**.

**Ví dụ lồng ngắt:**
- CPU đang xử lý ngắt Timer2 (Preemption = 2)
- Giữa chừng, EXTI0 (Preemption = 1) xảy ra → NVIC cho phép dừng Timer2 và chuyển sang xử lý EXTI0
- Sau khi EXTI0 xử lý xong → quay lại tiếp tục Timer2

Điều này giúp hệ thống **đảm bảo phản ứng nhanh với các sự kiện quan trọng hơn**, ngay cả khi đang xử lý một ngắt khác.

> Lưu ý: Việc lồng ngắt sâu có thể gây rối loạn nếu không kiểm soát tốt, cần giới hạn độ sâu hoặc quản lý qua cờ trạng thái.

---

## 3. QUY TRÌNH XỬ LÝ NGẮT TRONG STM32F103 (INTERRUPT PIPELINE)

Khi một ngắt xảy ra, hệ thống STM32F103 sẽ trải qua một chuỗi các bước **pipeline** xử lý để chuyển từ **chương trình chính** sang **hàm xử lý ngắt**, sau đó quay lại như chưa có gì xảy ra. Hiểu rõ quy trình này giúp bạn hình dung rõ **làm cách nào CPU biết được phải xử lý cái gì**, và **ở đâu**.

---

### 3.1. Tổng quan các bước xử lý ngắt

Giả sử bạn đã bật ngắt cho ngoại vi nào đó (ví dụ ADC) và đã bật NVIC, hệ thống sẽ xử lý theo các bước:

1. **Ngoại vi sinh ngắt**:

   - Khi một sự kiện xảy ra (VD: ADC đo xong), **cờ ngắt (IT flag)** của ngoại vi được **set**.

2. **NVIC kiểm tra ngắt**:

   - Nếu ngắt đó đã được **enable**, và đang không bị mask, NVIC đưa ngắt vào **Pending State** (chờ xử lý).

3. **CPU hoàn thành lệnh đang chạy và xác định ngắt**:

   - CPU dừng pipeline chính tạm thời và **đọc bảng Vector Table** để biết cần nhảy đến đâu.

4. **Tự động lưu trạng thái**:

   - CPU **push các thanh ghi quan trọng (PC, LR, xPSR, R0–R3)** vào Stack để đảm bảo sau này có thể quay lại đúng trạng thái ban đầu.

5. **Nhảy vào ISR (Interrupt Service Routine)**:

   - CPU gọi đúng **IRQHandler** tương ứng với ngắt.
   - Đây là nơi bạn viết mã xử lý sự kiện vừa xảy ra.

6. **Kết thúc ngắt – Trở về chương trình chính**:

   - Sau khi hàm IRQHandler kết thúc:
     - CPU **pop lại các thanh ghi đã lưu** từ stack
     - Quay lại nơi chương trình chính bị tạm dừng

---

### 3.2. Minh họa chuỗi pipeline xử lý ngắt

```
Ngoại vi (VD: ADC) đo xong ─► Set cờ ngắt ADC_IT_EOC
                             │
                     NVIC kiểm tra (enable + priority)
                             │
              Đưa vào hàng chờ (Pending IRQ)
                             │
                   CPU dừng chương trình chính
                             │
       ► Đọc Vector Table để xác định địa chỉ ISR
       ► Push các thanh ghi vào Stack
       ► Nhảy vào ISR: void ADC1_2_IRQHandler(void)
       ► Xử lý xong → Exit
       ► Pop lại trạng thái cũ
       ► Trở lại chương trình chính
```

> Quá trình trên diễn ra cực nhanh – chỉ vài chu kỳ máy (clock cycles)

---

### 3.3. Một số lưu ý quan trọng:

- **Nếu bạn không cấu hình NVIC**, cờ ngắt vẫn có thể được set, **nhưng chương trình sẽ không nhảy vào hàm IRQHandler** → ngắt bị "bỏ qua"
- ISR không nên chạy quá lâu, vì sẽ **block các ngắt ưu tiên thấp hơn**
- Nên **xóa cờ ngắt** thủ công trong ISR nếu không nó sẽ tiếp tục vào lại

```c
void ADC1_2_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
        uint16_t val = ADC_GetConversionValue(ADC1);
        // ... xử lý giá trị đo được

        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC); // XÓA CỜ!
    }
}
```

---

## 4. VECTOR TABLE – BẢNG ÁNH XẠ HÀM NGẮT

### 4.1. Vector Table là gì?

**Vector Table** là một bảng ánh xạ (mảng địa chỉ) đặt ở đầu bộ nhớ chương trình (Flash), dùng để **liên kết mỗi IRQ với một hàm xử lý ngắt tương ứng**. Khi ngắt xảy ra, CPU sẽ tìm đến Vector Table để biết phải nhảy vào địa chỉ nào.

> Có thể hiểu Vector Table như một "sổ địa chỉ" để CPU tra cứu và nhảy đến đúng ISR tương ứng với mỗi nguồn ngắt.

---

### 4.2. Cấu trúc của Vector Table

- Được lưu ở địa chỉ `0x00000000` trong bộ nhớ Flash
- Mỗi phần tử là một con trỏ đến hàm xử lý tương ứng
- Phần đầu bảng:
  - [0] Stack Pointer (SP): địa chỉ bắt đầu của stack
  - [1] Reset Handler: hàm chạy khi khởi động (entry point)
  - [2...] Các hàm xử lý ngắt: NMI, HardFault, SysTick, EXTI0, ADC1\_2, TIM2...

**Sơ đồ rút gọn:**

```
VectorTable[0] = Địa chỉ Stack Pointer ban đầu
VectorTable[1] = Reset_Handler
VectorTable[2] = NMI_Handler
VectorTable[3] = HardFault_Handler
...
VectorTable[n] = ADC1_2_IRQHandler
...
```

Các bạn có thể theo dõi kỹ hơn ở đây bảng sau: 

![alt text](<Documents/Vector Table.png>)

Và các phần sau nữa các bạn có thể tìm hiểu trong datasheet của stm32.

---

### 4.3. Tùy biến ISR bằng cách định nghĩa tên hàm tương ứng

Khi lập trình với CMSIS hoặc STM32 Standard Peripheral Library, bạn **chỉ cần định nghĩa đúng tên hàm IRQHandler**, trình biên dịch sẽ tự ánh xạ nó vào Vector Table nhờ file `startup_stm32f10x_md.s`

**Ví dụ:**

```c
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0)) {
        // Xử lý nút nhấn
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void ADC1_2_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
        uint16_t value = ADC_GetConversionValue(ADC1);
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    }
}
```

> Chỉ cần định nghĩa đúng tên, hệ thống sẽ tự động liên kết với Vector Table

---

### 4.4. Thay đổi Vector Table (nâng cao)

Trong một số trường hợp đặc biệt (Bootloader, đa ứng dụng), bạn có thể **thay đổi địa chỉ Vector Table** bằng cách ghi lại vào thanh ghi `SCB->VTOR` (Vector Table Offset Register):

```c
SCB->VTOR = 0x08003000; // Chuyển bảng vector sang vùng khác trong Flash
```

> Khi thay đổi vị trí Vector Table, bạn cần đảm bảo toàn bộ bảng mới đã có đầy đủ handler hợp lệ!

---

## 5. ISR – INTERRUPT SERVICE ROUTINE

### 5.1. ISR là gì?

**ISR (Interrupt Service Routine)** là **hàm xử lý ngắt**, được gọi tự động khi một ngắt xảy ra. Mỗi ISR tương ứng với một loại ngắt cụ thể, và được ánh xạ trong **Vector Table**.

> Ví dụ: Ngắt EXTI0 sẽ gọi hàm `void EXTI0_IRQHandler(void)`, ngắt ADC sẽ gọi `ADC1_2_IRQHandler()`...

---

### 5.2. Nguyên tắc viết hàm ISR

Khi viết ISR, cần tuân thủ những nguyên tắc sau:

- **Tên hàm phải đúng chuẩn** (được quy định bởi CMSIS hoặc startup file)
- **Không có đối số và không có giá trị trả về**
- ISR **phải chạy nhanh**, không nên dùng `delay` hoặc xử lý phức tạp bên trong
- **Phải xóa cờ ngắt** sau khi xử lý, nếu không sẽ bị lặp lại vô hạn

---

### 5.3. Ví dụ các hàm ISR

#### a. ISR cho EXTI0 (nút nhấn)

```c
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0)) {
        // Thực hiện hành động khi nút được nhấn
        toggleLED();

        // Xóa cờ ngắt
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
```

#### b. ISR cho ADC1_2

```c
void ADC1_2_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
        uint16_t value = ADC_GetConversionValue(ADC1);
        // Lưu hoặc xử lý giá trị ADC
        adcResult = value;

        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    }
}
```

#### c. ISR cho USART1 (khi nhận dữ liệu)

```c
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        char c = USART_ReceiveData(USART1);
        // Lưu ký tự hoặc đưa vào buffer
        buffer[index++] = c;

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
```

---

### 5.4. Quản lý trạng thái bên trong ISR

Vì ISR **không có tham số truyền vào hoặc truyền ra**, nên các dữ liệu cần xử lý nên được lưu vào:

- **Biến toàn cục**
- **Vùng RAM/Buffer chung**
- **Queue hoặc flag xử lý sau**

**Ví dụ dùng flag:**

```c
volatile uint8_t flag_adc_ready = 0;

void ADC1_2_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
        flag_adc_ready = 1;
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    }
}

// Trong vòng lặp chính:
if (flag_adc_ready) {
    flag_adc_ready = 0;
    processADCData();
}
```

---

### 5.5. Một số lưu ý nâng cao

- **Không gọi `printf` trong ISR** (trừ khi bạn hiểu rõ hệ thống delay và đồng bộ)
- **Không xử lý chậm**, nên chỉ đặt flag rồi xử lý sau
- Nếu cần nhiều ISR dùng chung tài nguyên → cần **dùng biến `volatile`** hoặc cơ chế đồng bộ

---

# 6. VÍ DỤ NGẮT ADC HOÀN CHỈNH TRÊN STM32F103

## 6.1. Mục tiêu

- Sử dụng **ADC1** đo giá trị điện áp từ **chân PA0**
- Bật **ngắt EOC (End of Conversion)**
- Khi ngắt xảy ra, CPU sẽ vào ISR `ADC1_2_IRQHandler` để đọc kết quả và lưu vào biến `adcResult`
- Toàn bộ quá trình được minh họa bằng **cấu hình đầy đủ và ISR**

## 6.2. Sơ đồ hệ thống

```
PA0 → [ ADC1 Channel 0 ] → [ Start Conversion ] 
                             ↓
                      [ EOC flag được set ]
                             ↓
                        [ NVIC kiểm tra ]
                             ↓
                CPU nhảy vào ADC1_2_IRQHandler()
                             ↓
        Đọc giá trị ADC, xóa cờ, lưu vào adcResult
```

## 6.3. Mã nguồn ví dụ đầy đủ

```c
#include "stm32f10x.h"

volatile uint16_t adcResult = 0;

void ADC_Config(void);
void GPIO_Config(void);
void NVIC_Config(void);

int main(void)
{
    GPIO_Config();
    ADC_Config();
    NVIC_Config();

    while (1)
    {
        // Bắt đầu một phép đo
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);

        // Giả sử sau đó xử lý adcResult...
    }
}

// Cấu hình chân PA0 làm input analog
void GPIO_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
}

// Cấu hình ADC1
void ADC_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // Giảm tốc độ clock ADC

    ADC_InitTypeDef adc;
    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc);

    // Chọn kênh 0, thời gian lấy mẫu 55.5 cycles
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

    // Bật ngắt EOC
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

    // Bật ADC và reset bộ chuyển đổi
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

// Bật NVIC cho ADC1
void NVIC_Config(void)
{
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = ADC1_2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

// ISR cho ADC1
void ADC1_2_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
        adcResult = ADC_GetConversionValue(ADC1);
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    }
}
```

## 6.4. Lưu ý quan trọng

- Phải bật **cờ ngắt EOC** bằng `ADC_ITConfig`
- Phải **bật NVIC** với đúng IRQChannel (`ADC1_2_IRQn`)
- Trong ISR, phải **xóa cờ ADC_IT_EOC** để tránh lặp
- Không nên xử lý phức tạp trong ISR – chỉ nên lưu giá trị ADC và xử lý ngoài `main`

---

## 7. TỔNG KẾT & SƠ ĐỒ HOẠT ĐỘNG CỦA INTERRUPT TRÊN STM32F103

### 7.1. Tổng kết kiến thức

| Thành phần                   | Vai trò                                                                                  |
| ---------------------------- | ---------------------------------------------------------------------------------------- |
| **Interrupt (Ngắt)**         | Cơ chế giúp vi điều khiển phản ứng tức thời với sự kiện bất ngờ                          |
| **Ngoại vi (Peripheral)**    | Sinh ra sự kiện → set cờ ngắt để báo hiệu                                                |
| **Cờ ngắt (Interrupt Flag)** | Được set khi ngoại vi hoàn tất sự kiện cần xử lý (VD: đo xong, tràn timer, nhận UART...) |
| **NVIC**                     | Kiểm tra cờ, xác định ưu tiên, cấp quyền xử lý, điều hướng CPU vào đúng ISR              |
| **Vector Table**             | Bản đồ ánh xạ IRQ → ISR (Interrupt Service Routine)                                      |
| **ISR**                      | Hàm xử lý ngắt: phải ngắn gọn, hiệu quả, và xóa cờ ngắt sau xử lý                        |

### Những điểm mấu chốt cần ghi nhớ:

- Không có NVIC → CPU **không xử lý ngắt** dù cờ đã set
- Phải **bật cờ ngắt và NVIC** đúng IRQ mới đảm bảo hệ thống hoạt động
- Cần hiểu rõ **Preemption/SubPriority** để xử lý ngắt đồng thời và lồng ngắt
- Trong ISR **phải xóa cờ ngắt**, tránh rơi vào vòng lặp vô hạn
- ISR nên **tránh xử lý nặng** → chỉ set flag hoặc lưu dữ liệu, xử lý ở main

### 7.2. Sơ đồ hoạt động tổng quát của hệ thống ngắt

```mermaid
flowchart TD
    A[Ngoại vi xảy ra sự kiện] --> B[Set cờ ngắt (Interrupt Flag)]
    B --> C{Ngắt đã được bật trong NVIC?}
    C -- Không --> Z[Không có gì xảy ra]
    C -- Có --> D[NVIC đưa ngắt vào hàng chờ (Pending)]
    D --> E[NVIC kiểm tra ưu tiên ngắt]
    E --> F{CPU đang xử lý gì?}
    F -- Đang Idle hoặc chương trình chính --> G[Nhảy vào ISR xử lý]
    F -- Đang xử lý ngắt ưu tiên thấp hơn --> G[Ngắt lồng (nested interrupt)]
    G --> H[Xử lý ISR (đọc dữ liệu, set flag...)]
    H --> I[Xóa cờ ngắt]
    I --> J[Thoát ISR, trở lại main hoặc ISR cũ]
```

### 7.3. Một cách hiểu dễ nhớ theo hướng "thực tế"

> Hãy tưởng tượng **CPU là nhân viên trực tổng đài**, còn **ngoại vi là khách gọi điện thoại**.

- Nếu tổng đài **không mở đường dây (NVIC chưa bật)** → cuộc gọi không tới được
- Nếu có nhiều cuộc gọi → tổng đài **ưu tiên cuộc gọi quan trọng hơn** (Preemption Priority)
- Mỗi cuộc gọi được chuyển đến **bàn làm việc riêng (ISR)** để xử lý nhanh, sau đó quay lại công việc chính
- Nếu khách chưa gác máy (cờ chưa xóa) → cuộc gọi đó sẽ **liên tục lặp lại**