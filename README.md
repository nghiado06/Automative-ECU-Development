# 🚗 Automotive ECU Development with STM32F1

## 📌 Giới thiệu

Xin chào! Đây là nơi mình chia sẻ toàn bộ kiến thức đã học và tổng hợp được trong quá trình **lập trình trên dòng vi điều khiển STM32F1x**, đặc biệt trong bối cảnh **phát triển ECU (Electronic Control Unit)** cho các ứng dụng automotive.

Mục tiêu của repo là giúp sinh viên và người mới bắt đầu hiểu rõ cách làm việc với **các tầng thấp (bare-metal)** cũng như **chuẩn hóa phần mềm theo AUTOSAR**.

---

## 📚 Nội dung chính

### 1. 🧱 Lập trình thanh ghi (Register-Level Programming)
- Hiểu cấu trúc bộ nhớ, thanh ghi ngoại vi
- Bitwise manipulation – thao tác bit/byte hiệu quả
- Thực hành bật/tắt LED, đọc button bằng thanh ghi trực tiếp

### 2. 🔧 Sử dụng SPL (Standard Peripheral Library)
- Cách dùng thư viện chính thức của ST cho STM32F1
- So sánh giữa SPL và thanh ghi thuần
- Viết code theo chuẩn dễ bảo trì và mở rộng

### 3. 🧩 AUTOSAR Classic Platform
- Kiến trúc phần mềm AUTOSAR và khái niệm tầng mềm ECU
- Các module cơ bản như Port, Dio, Pwm, Adc, Gpt
- Cách viết code theo guideline AUTOSAR (naming, layered structure,...)
- Tạo các driver đơn giản theo chuẩn AUTOSAR

---

## 🛠️ Yêu cầu phần mềm

- **Toolchain**: ARM GCC / Keil uVision / STM32CubeIDE
- **Board**: STM32F103C8T6 (hoặc tương đương dòng F1)
- **Thư viện**: SPL (Standard Peripheral Library)

---

