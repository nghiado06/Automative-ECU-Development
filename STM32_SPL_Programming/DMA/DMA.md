# DMA - Direct Memory Access Controller
Khi làm việc với các ngoại vi như ADC, UART, SPI hoặc Timer trên STM32, việc truyền dữ liệu giữa RAM và các thanh ghi ngoại vi là một thao tác lặp đi lặp lại và thường chiếm tài nguyên CPU. Trong những trường hợp như đọc ADC liên tục, gửi chuỗi UART, hay nhận dữ liệu SPI theo khối, việc để CPU "copy tay" từng byte dữ liệu là không hiệu quả.

Đó chính là lúc DMA (Direct Memory Access) phát huy vai trò. DMA cho phép truyền dữ liệu trực tiếp giữa bộ nhớ và ngoại vi mà không cần CPU can thiệp từng bước, giúp:

- Giảm tải cho CPU, nhất là trong các ứng dụng real-time
- Tăng tốc độ truyền, nhờ truyền song song
- Giảm độ trễ giữa các lần lấy/gửi dữ liệu

Trong vi điều khiển STM32F103, DMA không phải là một tính năng "cao siêu", mà là một công cụ rất thực tiễn, đặc biệt trong các tình huống sau:

- Lấy dữ liệu ADC liên tục và lưu vào mảng để xử lý tín hiệu
- Nhận chuỗi UART có độ dài cố định mà không mất byte nào
- Giao tiếp SPI tốc độ cao theo block
- Tự động cập nhật PWM thông qua Timer + DMA

Tuy nhiên, để sử dụng DMA đúng cách, lập trình viên cần nắm:

- Cơ chế hoạt động bên trong của DMA  
- Cách cấu hình đúng thứ tự DMA  
- Hiểu rõ các thanh ghi, bit cấu hình, cờ trạng thái và ISR  

Trong file README này, nội dung sẽ được trình bày theo từng phần, từ khái niệm đến cấu hình, với cách giải thích chi tiết và dễ hiểu, giúp bạn vừa hiểu lý thuyết vừa áp dụng thực tế. File sẽ gồm các nội dung sau:

**1. Khái niệm và flow hoạt động tổng quan của DMA**  
   - DMA là gì, vì sao cần DMA trong vi điều khiển  
   - Sự khác biệt giữa DMA và xử lý truyền thống bằng CPU  
   - Luồng hoạt động cụ thể: từ ngoại vi → bộ nhớ RAM  
   - Khi nào DMA chủ động gửi ngắt (ISR)  
   - So sánh truyền thống – interrupt – DMA  

**2. Cấu trúc phần cứng DMA trong STM32F103**  
   - Có bao nhiêu DMA controller? Mỗi DMA có bao nhiêu channel?  
   - Bảng ánh xạ channel – ngoại vi (ADC, USART, SPI, TIM...)  
   - Cơ chế ưu tiên channel: độ ưu tiên của channel không đơn giản là theo số thứ tự  
   - DMA hoạt động độc lập với CPU – nhưng vẫn phối hợp qua ngắt  

**3. Hệ thống thanh ghi DMA**  
   - Phân tích chi tiết từng thanh ghi:  
     - DMA_CCRx (cấu hình), DMA_CNDTRx (số phần tử), DMA_CPARx (địa chỉ ngoại vi), DMA_CMARx (địa chỉ RAM)  
   - Giải thích kỹ các bit: DIR, MINC, PINC, MSIZE, PSIZE, CIRC, TCIE, EN...  
   - Giải thích ý nghĩa “phần tử truyền”, Word/Half-word alignment  
   - Thanh ghi trạng thái DMA_ISR, thanh ghi xóa cờ DMA_IFCR  

**4. Cấu hình và lập trình DMA thực tế**  
   - Trình tự các bước cấu hình DMA  
   - Cấu hình DMA truyền dữ liệu từ ADC vào RAM (ví dụ dùng SPL)  
   - Quản lý flag truyền xong: dùng polling hoặc ISR  
   - Tắt ngắt, clear cờ, tránh lỗi truyền lặp  
   - Gợi ý tổ chức code thực tế: tách hàm init, start, ISR, clear

## 1. KHÁI NIỆM VÀ FLOW HOẠT ĐỘNG TỔNG QUAN CỦA DMA

### 1.1 DMA là gì?
DMA (Direct Memory Access) là một bộ điều khiển truyền dữ liệu đặc biệt trong vi điều khiển, cho phép truyền dữ liệu giữa RAM và ngoại vi (hoặc giữa 2 vùng bộ nhớ) mà không cần CPU can thiệp vào mỗi bước truyền.

DMA thích hợp với các bài toán truyền dữ liệu có tần số cao hoặc liên tục như:
- Lấy mẫu ADC liên tục
- Nhận/gửi dữ liệu UART, SPI
- Truyền dữ liệu cho PWM hoặc Timer

### 1.2 Khác biệt giữa DMA và truyền dữ liệu qua CPU
Truyền thông thường dùng CPU (polling hoặc interrupt) sẽ gồm các bước:
1. CPU kiểm tra flag ngoại vi
2. CPU đọc/ghi dữ liệu từ thanh ghi ngoại vi
3. CPU chép dữ liệu vào RAM hoặc ngược lại

Với DMA, toàn bộ quy trình trên được tự động thực hiện:
- CPU chỉ cần cấu hình ban đầu, DMA tự lo điều khiển truyền
- DMA truy cập trực tiếp RAM và thanh ghi ngoại vi

Kết quả:
- CPU nhẹ tải hơn, dữ liệu truyền nhanh hơn

### 1.3 Mô hình truyền DMA

Sơ đồ logic:
```
[ Peripheral DR (Data Register) ] <---> [ DMA Controller ] <---> [ RAM buffer ]
```
- DR (Data Register): thanh ghi dữ liệu của ngoại vi (VD: ADC1->DR)
- DMA Controller sẽ tự động truy xuất DR và RAM theo thứ tự

### 1.4 Flow truyền DMA cơ bản (1 lần truyền)

1. CPU cấu hình DMA: gồm địa chỉ nguồn (CPARx), địa chỉ đích (CMARx), số lượng truyền (CNDTRx), kiểu truyền (CCR)
2. Kích hoạt DMA (set EN = 1)
3. DMA đọc dữ liệu từ ngoại vi (CPARx)
4. DMA ghi dữ liệu vào RAM (CMARx), tự động tăng địa chỉ nếu có MINC
5. Giảm CNDTR sau mỗi lần truyền
6. Khi CNDTR = 0:
   - DMA set cờ TCIFx trong thanh ghi DMA_ISR
   - Nếu bật TCIE, ISR sẽ được gọi
   - Nếu CCR.CIRC = 1, DMA tự động truyền tiếp vòng mới

DMA không tự disable sau khi truyền xong. Muốn dừng DMA phải clear EN bằng tay hoặc viết lại cấu hình.

Các bạn có thể theo dõi cấu trúc hệ thống của DMA ở đây: ![alt text](<Documents/DMA Block Diagram.png>)

### 1.5 So sánh: Polling vs Interrupt vs DMA
| Tiêu chí           | Polling      | Interrupt (IRQ) | DMA              |
| ------------------ | ------------ | --------------- | ---------------- |
| Tỷ lệ CPU          | Cao          | Trung bình      | Thấp             |
| Phục tạp ISR       | Không cần    | Cần viết ISR    | Tuỳ chọn có ISR  |
| Dữ liệu tần số cao | Khó theo kịp | Có thể trễn ISR | Theo kịp dễ dàng |
| Hiệu suất          | Thấp         | Trung bình      | Cao              |


Kết luận: DMA giúp tối ưu hóa truyền dữ liệu trong các tình huống truyền nhanh, lặp lại hoặc tự động, nhất là ADC, UART, SPI, Timer.

## 2. CẤU TRÚC PHẦN CỨNG DMA TRONG STM32F103

### 2.1 Có bao nhiêu bộ DMA và channel?

STM32F103 dòng phổ biến (Medium-density) có:

- 1 bộ điều khiển DMA: gọi là DMA1
- Gồm 7 channel đánh số từ 1 đến 7: DMA1\_Channel1 đến DMA1\_Channel7

Mỗi channel có thể được ánh xạ tới các ngoại vi nhất định như ADC, USART, SPI, TIM...

### 2.2 Mỗi channel phục vụ ngoại vi nào?

STM32 không cho phép bất kỳ channel nào cũng dùng được cho bất kỳ ngoại vi. Bảng ánh xạ sau thể hiện mối liên hệ:

| Channel | Ngoại vi sử dụng (ví dụ) |
| ------- | ------------------------ |
| 1       | ADC1                     |
| 2       | SPI1\_TX                 |
| 3       | SPI1\_RX                 |
| 4       | USART1\_TX               |
| 5       | USART1\_RX               |
| 6       | USART2\_RX               |
| 7       | USART2\_TX               |

Ghi chú: Tùy dòng chip (Connectivity Line, XL-density...), số lượng DMA controller và channel có thể khác (VD: có thêm DMA2).

Các bạn có thể theo dõi DMA1 kỹ hơn ở bảng này ![DMA1](<Documents/DMA1 Channels.png>)

Và theo dõi DMA2 ở những dòng cao hơn ở bảng này ![DMA2](<Documents/DMA2 Channels.png>)

### 2.3 Ưu tiên channel hoạt động

Mỗi channel có thể được cấu hình mức độ ưu tiên (low, medium, high, very high) thông qua các bit trong thanh ghi DMA\_CCRx.

Tuy nhiên, **thứ tự xử lý giữa các channel DMA không phụ thuộc vào số thứ tự Channel (1 đến 7)**. DMA xử lý các yêu cầu theo:

- Mức độ ưu tiên (priority) cấu hình trong CCRx
- Nếu cùng mức ưu tiên: channel có số thấp hơn sẽ được ưu tiên

Các bạn có thể theo dõi kỹ hơn ở đây: 

![alt text](<Documents/DMA Request Mapping.png>)

### 2.4 DMA hoạt động độc lập với CPU

- DMA là một khối logic riêng, hoạt động độc lập với pipeline của CPU
- Trong quá trình DMA hoạt động, CPU vẫn tiếp tục thực thi code bình thường
- DMA sử dụng cơ chế bus arbiter để truy cập bộ nhớ RAM và thanh ghi ngoại vi mà không gây xung đột với CPU

Tóm lại, DMA giúp tạo ra luồng truyền dữ liệu song song và tự động, nâng cao hiệu suất hệ thống mà không ảnh hưởng đến luồng điều khiển chính của CPU

## 3. HỆ THỐNG THANH GHI DMA

DMA được điều khiển thông qua các thanh ghi cấu hình riêng cho từng kênh. Mỗi kênh DMAx\_Channely có một tập hợp thanh ghi riêng, nằm tại địa chỉ:

```
DMAx Channel y base = DMAx_BASE + 0x08 + (y - 1) * 0x14
```

Với mỗi kênh DMA (channel y), có 5 thanh ghi chính:

- **DMA\_CCRy**: Thanh ghi điều khiển cấu hình DMA
- **DMA\_CNDTRy**: Số phần tử cần truyền
- **DMA\_CPARy**: Địa chỉ thanh ghi ngoại vi (Peripheral Address)
- **DMA\_CMARy**: Địa chỉ RAM (Memory Address)
- **DMA\_ISR / DMA\_IFCR**: Các thanh ghi trạng thái và xóa cờ chung cho toàn DMA

### Phân tích chi tiết: DMA\_CCRx – Thanh ghi điều khiển kênh DMA

| Bit   | Tên     | Vai trò chính                                     |
| ----- | ------- | ------------------------------------------------- |
| 14    | MEM2MEM | Cho phép truyền giữa 2 vùng RAM (bỏ qua ngoại vi) |
| 13:12 | PL[1:0] | Độ ưu tiên (priority level)                       |
| 11:10 | MSIZE   | Kích thước phần tử từ RAM                         |
| 9:8   | PSIZE   | Kích thước phần tử từ ngoại vi                    |
| 7     | MINC    | Tự động tăng địa chỉ RAM                          |
| 6     | PINC    | Tự động tăng địa chỉ ngoại vi                     |
| 5     | CIRC    | Chế độ vòng lặp liên tục                          |
| 4     | DIR     | Chiều truyền: 0 = P → M, 1 = M → P                |
| 3     | TEIE    | Ngắt khi có lỗi truyền                            |
| 2     | HTIE    | Ngắt khi truyền được nửa                          |
| 1     | TCIE    | Ngắt khi truyền xong                              |
| 0     | EN      | Enable kênh DMA                                   |

---

#### MSIZE và PSIZE – Kích thước phần tử truyền

- **MSIZE**: xác định số byte mỗi lần đọc/ghi từ RAM
- **PSIZE**: xác định số byte mỗi lần đọc/ghi từ thanh ghi ngoại vi

| Giá trị bit | Kích thước | Ý nghĩa               |
| ----------- | ---------- | --------------------- |
| `00`        | 8-bit      | Mỗi lần truyền 1 byte |
| `01`        | 16-bit     | Truyền 2 byte mỗi lần |
| `10`        | 32-bit     | Truyền 4 byte mỗi lần |

Thường chọn giống nhau (MSIZE = PSIZE) để tránh sai lệch bộ căn địa chỉ (alignment).

**Ví dụ chọn cấu hình**:

| Ứng dụng                  | PSIZE   | MSIZE   | Giải thích                     |
| ------------------------- | ------- | ------- | ------------------------------ |
| Đọc ADC (12-bit → 16-bit) | 16bit   | 16bit   | Phù hợp với thanh ghi ADC1->DR |
| Gửi UART (8-bit)          | 8bit    | 8bit    | UART chỉ gửi từng byte         |
| Giao tiếp SPI theo khối   | 8/16/32 | 8/16/32 | Tuỳ cấu hình SPI               |

---

#### MINC – Memory Increment

- Nếu `MINC = 1`: DMA tăng địa chỉ RAM sau mỗi lần truyền → dùng cho mảng buffer[]
- Nếu `MINC = 0`: DMA luôn ghi vào đúng 1 ô RAM → dùng cho biến đơn (myVar)

---

#### PINC – Peripheral Increment

- Nếu `PINC = 1`: DMA tăng địa chỉ ngoại vi mỗi lần truyền (dùng khi nguồn/đích là vùng RAM)
- Nếu `PINC = 0`: DMA luôn truy cập đúng 1 thanh ghi ngoại vi → thường là DR

**Chú ý**: Thanh ghi ngoại vi như `ADC1->DR`, `USART1->DR` đều có địa chỉ cố định → **luôn để PINC = 0**

---

#### DIR – Chiều truyền

| DIR | Hướng truyền              | Dùng khi                 |
| --- | ------------------------- | ------------------------ |
| 0   | Peripheral → Memory (P2M) | Đọc ADC, UART RX, SPI RX |
| 1   | Memory → Peripheral (M2P) | Gửi UART TX, SPI TX      |

---

#### CIRC – Circular Mode

- Khi `CIRC = 1`: DMA tự động truyền lại từ đầu sau khi CNDTR = 0 → dùng cho stream liên tục (ADC liên tục, phát PWM…)
- Khi `CIRC = 0`: DMA sẽ dừng sau khi hoàn thành khối truyền

---

#### MEM2MEM – Memory to Memory Mode

- Khi `MEM2MEM = 1`: DMA dùng để copy RAM → RAM (không cần ngoại vi)
- Bỏ qua cấu hình DIR, PSIZE/PINC (vì không dùng thanh ghi ngoại vi)

---

#### Tổng kết chi tiết CCRx

| Bit   | Tên     | Vai trò chính                | Gợi ý sử dụng                          |
| ----- | ------- | ---------------------------- | -------------------------------------- |
| 14    | MEM2MEM | Copy giữa RAM                | Bật khi copy mảng                      |
| 13:12 | PL      | Priority                     | Dùng khi nhiều DMA hoạt động           |
| 11:10 | MSIZE   | Độ lớn phần tử RAM           | 8/16/32-bit, phụ thuộc dữ liệu         |
| 9:8   | PSIZE   | Độ lớn phần tử ngoại vi      | Phải phù hợp thanh ghi ngoại vi        |
| 7     | MINC    | Tăng địa chỉ RAM             | Bật nếu ghi vào mảng                   |
| 6     | PINC    | Tăng địa chỉ ngoại vi        | Luôn để = 0 nếu truy cập DR            |
| 5     | CIRC    | Truyền vòng                  | Bật khi truyền liên tục                |
| 4     | DIR     | Hướng truyền                 | 0 = ngoại vi → RAM, 1 = RAM → ngoại vi |
| 3     | TEIE    | Bật ngắt khi có lỗi          | Dùng trong debug hoặc xử lý an toàn    |
| 2     | HTIE    | Bật ngắt khi truyền nửa      | Dùng cho stream hoặc audio phân đoạn   |
| 1     | TCIE    | Bật ngắt khi truyền hoàn tất | Dùng phổ biến nhất                     |
| 0     | EN      | Enable channel               | Ghi 1 để bắt đầu DMA                   |

Đây là một trong những thanh ghi quan trọng nhất để cấu hình DMA chính xác. Tiếp theo, ta sẽ tìm hiểu DMA\_CNDTRx, CPARx, CMARx và hệ thống cờ trong DMA.

### 3.2 DMA_CNDTRx – DMA channel number of data register

Thanh ghi `CNDTRx` quy định **số phần tử dữ liệu (data units)** mà DMA sẽ truyền cho mỗi lần hoạt động.

> Đây là **số phần tử**, KHÔNG phải số byte.

---

#### Ý nghĩa

Mỗi khi DMA truyền thành công một phần tử, giá trị trong `CNDTRx` sẽ **tự động giảm 1**.

- Khi `CNDTRx == 0` → DMA tự dừng (nếu không bật chế độ circular `CIRC`)
- DMA sẽ **không truyền quá số phần tử đã định** trong `CNDTRx`

---

#### Sự khác biệt: "Phần tử" vs "Số byte"

| Thuộc tính        | Ý nghĩa                                        |
| ----------------- | ---------------------------------------------- |
| Phần tử (element) | Một đơn vị dữ liệu DMA sẽ truyền trong mỗi lần |
| Byte (byte count) | Kích thước thực tế tính theo đơn vị byte       |

DMA truyền dữ liệu theo **phần tử**, và mỗi phần tử có kích thước phụ thuộc vào `MSIZE` (RAM) và `PSIZE` (ngoại vi).

| MSIZE / PSIZE | Kích thước phần tử | Số byte thực tế (CNDTR = 4) |
| ------------- | ------------------ | --------------------------- |
| 8-bit         | 1 byte             | 4 byte                      |
| 16-bit        | 2 byte             | 8 byte                      |
| 32-bit        | 4 byte             | 16 byte                     |

---

#### Ví dụ minh họa

```c
DMA1_Channel1->CNDTR = 4;
DMA1_Channel1->CCR |= (1 << 10); // MSIZE = 16-bit
```

- CNDTR = 4 nghĩa là sẽ truyền 4 phần tử
- Mỗi phần tử 2 byte → tổng cộng truyền 8 byte
- Sau mỗi lần truyền thành công 1 phần tử → `CNDTR` giảm: 4 → 3 → 2 → 1 → 0 → DMA dừng

---

#### Lưu ý khi dùng

- Luôn đảm bảo rằng `CNDTR` phù hợp với độ dài mảng hoặc số phần tử bạn cần truyền
- Không nhầm lẫn giữa số phần tử và số byte


### 3.3 DMA_CPARx – DMA channel peripheral address register

Thanh ghi này chứa **địa chỉ thanh ghi ngoại vi** nơi DMA sẽ **đọc dữ liệu vào** (nếu là Peripheral → Memory) hoặc **ghi dữ liệu ra** (nếu là Memory → Peripheral).

---

#### Ý nghĩa

DMA không truyền dữ liệu tới toàn bộ ngoại vi, mà chỉ truyền đến **1 thanh ghi dữ liệu duy nhất**, ví dụ:

| Ngoại vi  | Thanh ghi dữ liệu           | DMA_CPARx     |
| --------- | --------------------------- | ------------- |
| ADC1      | `ADC1->DR`                  | `&ADC1->DR`   |
| USART1 TX | `USART1->DR` (thanh ghi TX) | `&USART1->DR` |
| SPI1 RX   | `SPI1->DR` (thanh ghi RX)   | `&SPI1->DR`   |

Vì vậy bạn luôn cần truyền đúng địa chỉ thanh ghi đó vào `CPARx`.

---

#### Căn thẳng địa chỉ – Vì sao phải bỏ qua một số bit địa chỉ?

DMA hỗ trợ truyền dữ liệu theo **3 kích thước cơ bản**:

| PSIZE bit | Kích thước truyền | Đơn vị         |
| --------- | ----------------- | -------------- |
| 00        | 8-bit             | Byte           |
| 01        | 16-bit            | Half-word (2B) |
| 10        | 32-bit            | Word (4B)      |

Mỗi kiểu dữ liệu này yêu cầu **địa chỉ phải được căn chỉnh (aligned)** sao cho:

| Kích thước | Địa chỉ phải chia hết cho | Bắt buộc bỏ qua bit nào |
| ---------- | ------------------------- | ----------------------- |
| 8-bit      | 1 (bất kỳ)                | Không                   |
| 16-bit     | 2                         | Bỏ qua `PA[0]`          |
| 32-bit     | 4                         | Bỏ qua `PA[1:0]`        |

---

#### Phân tích kỹ hơn

- Khi PSIZE = 16-bit, DMA truyền theo từng **half-word** → nếu địa chỉ không chẵn (không chia hết cho 2), việc truy xuất bộ nhớ sẽ bị lỗi (vì truy cập vào địa chỉ không hợp lệ).
- Khi PSIZE = 32-bit, DMA truyền từng **word** → nếu địa chỉ không chia hết cho 4, **bộ xử lý sẽ báo lỗi alignment** hoặc truyền sai dữ liệu.
- Vì vậy, **các bit thấp (PA[0], PA[1]) bị bỏ qua bởi DMA**, nhằm **ép truy cập đúng địa chỉ hợp lệ** (tự căn chỉnh).

---

#### Ví dụ minh họa

```c
DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
```

- Giả sử bạn dùng PSIZE = 16-bit → DMA sẽ **bỏ qua PA[0]**
- Nếu bạn truyền `&ADC1->DR + 1` mà PA không chẵn → lỗi sẽ xảy ra

---

#### Nếu không align đúng

| Tình huống                                   | Hậu quả                                |
| -------------------------------------------- | -------------------------------------- |
| PSIZE = 16-bit, nhưng địa chỉ lẻ             | HardFault / lỗi dữ liệu                |
| PSIZE = 32-bit, địa chỉ không chia hết cho 4 | Không truyền được / crash chương trình |

---

### Tóm lại

| PSIZE  | Cần chia hết cho | DMA bỏ qua bit nào | Vì sao                                   |
| ------ | ---------------- | ------------------ | ---------------------------------------- |
| 8-bit  | Không cần        | Không              | Byte nào cũng đọc được                   |
| 16-bit | 2                | PA[0]              | Truy cập half-word cần địa chỉ chẵn      |
| 32-bit | 4                | PA[1:0]            | Truy cập word cần địa chỉ chia hết cho 4 |

### 3.4 DMA_CMARx – DMA channel memory address register

Đây là thanh ghi chứa **địa chỉ vùng RAM** mà DMA sẽ **đọc dữ liệu từ đó** (nếu truyền từ RAM → ngoại vi), hoặc **ghi dữ liệu vào đó** (nếu truyền từ ngoại vi → RAM).

CMAR là viết tắt của **Channel Memory Address Register**.

---

#### Cách sử dụng

- Khi bạn truyền dữ liệu từ **ngoại vi → RAM** (ví dụ: đọc ADC, USART RX…), CMAR phải trỏ tới **nơi DMA sẽ ghi dữ liệu**
- Khi bạn truyền từ **RAM → ngoại vi** (ví dụ: gửi UART, SPI TX…), CMAR phải trỏ tới **nơi DMA sẽ lấy dữ liệu**

---

#### Ví dụ

```c
uint16_t adc_buffer[10];
DMA1_Channel1->CMAR = (uint32_t)adc_buffer;
```

→ DMA sẽ ghi dữ liệu ADC vào từng ô trong mảng `adc_buffer`.

---

#### Tác động của bit MINC (Memory Increment)

| MINC | Ý nghĩa                                                                  |
| ---- | ------------------------------------------------------------------------ |
| 1    | Sau mỗi lần truyền, DMA **tự động tăng địa chỉ** RAM (CMAR++)            |
| 0    | DMA **luôn dùng đúng 1 địa chỉ RAM** – không thay đổi sau mỗi lần truyền |

---

##### Giải thích kỹ:

- Nếu **MINC = 1**:
  - Dữ liệu sẽ ghi vào `CMAR`, rồi `CMAR + 1`, `CMAR + 2`… cho đến hết `CNDTR`
  - Phù hợp khi dùng mảng: DMA ghi vào các phần tử kế tiếp

- Nếu **MINC = 0**:
  - DMA sẽ ghi **tất cả dữ liệu vào đúng 1 ô nhớ duy nhất**
  - Giống như đè lên một biến cố định nhiều lần

---

##### So sánh bằng hình ảnh (giả định mỗi lần truyền là 1 byte):

| MINC | Ghi dữ liệu tại RAM                             |
| ---- | ----------------------------------------------- |
| 1    | buffer[0], buffer[1], buffer[2], ..., buffer[9] |
| 0    | luôn luôn ghi tại địa chỉ `buffer[0]`           |

---

#### Khi nào dùng MINC = 0?

- Bạn chỉ cần 1 giá trị mới nhất (VD: chỉ quan tâm giá trị ADC hiện tại)
- Muốn tiết kiệm RAM – không cần buffer

---

Tóm lại:

| Trường hợp sử dụng                | CMAR trỏ đến     | MINC |
| --------------------------------- | ---------------- | ---- |
| Lưu ADC vào mảng                  | `&buffer[0]`     | 1    |
| Lưu ADC vào 1 biến duy nhất       | `&adc_value`     | 0    |
| Gửi SPI từ mảng data              | `&tx_data[0]`    | 1    |
| Gửi SPI từ 1 byte cố định lặp lại | `&constant_byte` | 0    |

### 3.5 DMA_ISR và DMA_IFCR

Khi DMA truyền dữ liệu, trạng thái hoạt động của từng channel sẽ được phản ánh qua thanh ghi `DMA_ISR` (Interrupt Status Register). Đây là **thanh ghi chung cho toàn bộ DMA1**, không riêng cho channel nào.

Ngoài ra, STM32 cung cấp thanh ghi `DMA_IFCR` (Interrupt Flag Clear Register) để **xoá các cờ** trong `DMA_ISR`.

---

#### Cấu trúc các cờ trong `DMA_ISR`

| Cờ trong ISR | Ý nghĩa                         | Khi nào xảy ra                           |
| ------------ | ------------------------------- | ---------------------------------------- |
| `TCIFx`      | Transfer Complete (Truyền xong) | DMA hoàn thành toàn bộ số phần tử truyền |
| `HTIFx`      | Half Transfer (Nửa chừng)       | DMA truyền được một nửa số phần tử       |
| `TEIFx`      | Transfer Error (Lỗi)            | Có lỗi trong quá trình truyền            |

> Với `x` là số channel, ví dụ `TCIF1` là cờ truyền xong của DMA_Channel1

Các cờ này được set **tự động bởi phần cứng DMA**.

---

#### Vai trò của `DMA_IFCR`

`DMA_IFCR` dùng để **xoá (clear)** các cờ trạng thái trong `DMA_ISR`. Bạn không thể xóa cờ bằng cách ghi 0 vào `DMA_ISR`. Thay vào đó, cần:

```c
DMA1->IFCR |= DMA_IFCR_CTCIF1; // Xoá cờ truyền xong channel 1
```

Các bit trong `DMA_IFCR` có dạng `CTCIFx`, `CHTIFx`, `CTEIFx` tương ứng với cờ `TCIFx`, `HTIFx`, `TEIFx`

---

### Vì sao phải xoá cờ bằng tay?

1. **DMA không tự xoá cờ sau khi ISR được gọi**.
   - Cờ vẫn giữ nguyên giá trị `1` sau khi ngắt xảy ra.
   - Nếu bạn bật lại ngắt `TCIE = 1` mà không xoá cờ `TCIFx`, thì ISR sẽ **bị gọi lại ngay lập tức** → gây lỗi logic, gọi nhầm ISR.

2. **Không xoá cờ = không tái sử dụng ISR đúng cách**
   - ISR sẽ tiếp tục bị kích hoạt mỗi lần bạn bật lại `TCIE`, vì phần cứng thấy `TCIFx = 1`

---

### Quy trình chuẩn khi xử lý ngắt DMA

1. Trong hàm ISR, đọc dữ liệu hoặc xử lý logic cần thiết
2. Xoá cờ tương ứng trong `DMA_IFCR`
3. Nếu cần cấu hình lại DMA:
   - Tắt `EN = 0` trong CCR để dừng DMA
   - Viết lại các thanh ghi (CPAR, CMAR, CNDTR...)
   - Bật lại `EN = 1`

---

### Tổng kết

| Mục            | Vai trò                                      |
| -------------- | -------------------------------------------- |
| `DMA_ISR`      | Thanh ghi chứa cờ trạng thái cho tất cả kênh |
| `DMA_IFCR`     | Cho phép xoá các cờ trong `DMA_ISR`          |
| Phải xoá cờ?   | Có, nếu không ISR sẽ tiếp tục bị gọi lại     |
| ISR tự xoá cờ? | Không, phải xoá thủ công trong chương trình  |

## 4. Cấu hình và lập trình DMA thực tế

Đây là phần hướng dẫn chi tiết quy trình cấu hình và sử dụng DMA để truyền dữ liệu từ ADC vào RAM bằng thư viện SPL trên STM32F1. Cách trình bày được chia rõ theo từng bước và có ví dụ cụ thể giúp dễ áp dụng thực tế.

---

### Trình tự các bước cấu hình DMA

1. **Cấp clock cho DMA**
2. **Cấu hình DMA_Channelx** với:
   - Địa chỉ nguồn (CPARx): ADC1->DR
   - Địa chỉ đích (CMARx): buffer RAM
   - Số phần tử truyền (CNDTRx)
   - Các chế độ truyền (PSIZE, MSIZE, MINC...)
3. **Bật DMA Channel**
4. **Cấu hình ADC để kết hợp với DMA**
5. (Optional) Cấu hình NVIC nếu dùng ISR

---

### Ví dụ: Dùng DMA truyền dữ liệu ADC vào RAM

#### Buffer RAM nhận dữ liệu:
```c
uint16_t adcBuffer[8];
```

#### 1. Cấp clock cho DMA và ADC:
```c
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
```

#### 2. Cấu hình DMA:
```c
DMA_InitTypeDef DMA_InitStructure;
DMA_DeInit(DMA1_Channel1);  // Reset DMA1 Channel1

DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adcBuffer;
DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
DMA_InitStructure.DMA_BufferSize = 8;
DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
DMA_InitStructure.DMA_Priority = DMA_Priority_High;
DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
DMA_Init(DMA1_Channel1, &DMA_InitStructure);
```

#### Bảng giải thích từng thành phần trong DMA\_InitStructure

| Trường cấu hình          | Giá trị khả dụng                                  | Ý nghĩa và ứng dụng                                                               |
| ------------------------ | ------------------------------------------------- | --------------------------------------------------------------------------------- |
| `DMA_PeripheralBaseAddr` | `(uint32_t)&ADC1->DR`                             | Địa chỉ thanh ghi nguồn – thường là thanh ghi DR của ADC, USART, SPI, v.v.        |
| `DMA_MemoryBaseAddr`     | `(uint32_t)adcBuffer`                             | Địa chỉ RAM nơi dữ liệu sẽ được ghi vào (hoặc lấy ra nếu là chiều RAM → ngoại vi) |
| `DMA_DIR`                | `DMA_DIR_PeripheralSRC` / `DMA_DIR_PeripheralDST` | Hướng truyền: từ ngoại vi → RAM hoặc RAM → ngoại vi                               |
| `DMA_BufferSize`         | 1 – 65535                                         | Số phần tử DMA sẽ truyền (phần tử chứ không phải byte)                            |
| `DMA_PeripheralInc`      | `Enable` / `Disable`                              | Tăng địa chỉ thanh ghi ngoại vi sau mỗi lần truyền – luôn đặt là Disable          |
| `DMA_MemoryInc`          | `Enable` / `Disable`                              | Tăng địa chỉ RAM – bật khi ghi mảng, tắt khi ghi biến đơn                         |
| `DMA_PeripheralDataSize` | `Byte` / `HalfWord` / `Word`                      | Kích thước 1 phần tử tại thanh ghi ngoại vi                                       |
| `DMA_MemoryDataSize`     | `Byte` / `HalfWord` / `Word`                      | Kích thước phần tử trong RAM                                                      |
| `DMA_Mode`               | `DMA_Mode_Normal` / `DMA_Mode_Circular`           | Normal: truyền 1 lần rồi dừng. Circular: lặp lại liên tục                         |
| `DMA_Priority`           | `Low` / `Medium` / `High` / `VeryHigh`            | Ưu tiên khi có nhiều kênh DMA chạy cùng lúc                                       |
| `DMA_M2M`                | `Enable` / `Disable`                              | Bật khi truyền từ RAM đến RAM – không qua ngoại vi                                |

#### 3. Bật DMA channel:
```c
DMA_Cmd(DMA1_Channel1, ENABLE);
```

#### 4. Cấu hình ADC kết hợp DMA:
```c
ADC_InitTypeDef ADC_InitStructure;
ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
ADC_InitStructure.ADC_ScanConvMode = ENABLE;
ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
ADC_InitStructure.ADC_NbrOfChannel = 1;
ADC_Init(ADC1, &ADC_InitStructure);

ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
ADC_DMACmd(ADC1, ENABLE);
ADC_Cmd(ADC1, ENABLE);

ADC_ResetCalibration(ADC1);
while(ADC_GetResetCalibrationStatus(ADC1));
ADC_StartCalibration(ADC1);
while(ADC_GetCalibrationStatus(ADC1));
```

#### 5. Bắt đầu convert ADC:
```c
ADC_SoftwareStartConvCmd(ADC1, ENABLE);
```

---

### Quản lý flag truyền xong

#### Cách 1: Dùng polling trong vòng lặp
```c
while (!DMA_GetFlagStatus(DMA1_FLAG_TC1));
DMA_ClearFlag(DMA1_FLAG_TC1); // Clear cờ sau khi xong
```

#### Cách 2: Dùng ISR (ngắt DMA)

1. **Bật ngắt DMA**:
```c
DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
```

2. **Khai báo ISR**:
```c
void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC1); // Xoá cờ
        // Xử lý dữ liệu xong
    }
}
```

3. **Cấu hình NVIC**:
```c
NVIC_InitTypeDef NVIC_InitStructure;
NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);
```

---

### Tắt DMA và clear flag đúng cách

- **Tắt DMA** để tránh xung đột cấu hình:
```c
DMA_Cmd(DMA1_Channel1, DISABLE);
```

- **Xoá cờ truyền xong (TCIF)**:
```c
DMA_ClearFlag(DMA1_FLAG_TC1);
```

- **Tránh lỗi ISR bị gọi lại ngay lập tức:**
  - Nếu không clear flag mà bật lại `TCIE`, ISR sẽ bị gọi ngay vì `TCIF1 = 1`

---

### Gợi ý tổ chức code thực tế

| Hàm                          | Chức năng                          |
| ---------------------------- | ---------------------------------- |
| `DMA_ADC_Init()`             | Cấu hình DMA để truyền dữ liệu ADC |
| `ADC_Start()`                | Bật ADC và bắt đầu convert         |
| `DMA_ClearFlag()`            | Hàm clear cờ truyền xong           |
| `DMA1_Channel1_IRQHandler()` | ISR xử lý khi DMA truyền xong      |

Tách code theo module giúp dễ tái sử dụng, test và mở rộng hệ thống.

