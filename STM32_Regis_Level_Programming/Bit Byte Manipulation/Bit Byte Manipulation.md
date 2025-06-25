# BIT BYTE MANIPULATION - THAO TÁC BIT BYTE
Chúng ta sẽ cùng tìm hiểu nội dung thao tác với bit byte ở đây. Thứ mà sẽ mang ý nghĩa vô cùng quan trọng, phục vụ các bạn trong quá trình lập trình vi điều khiển.
Nội dung kiến thức:
- Khái niệm bit, byte
- Làm quen với hệ 16, hệ 10, hệ 2
- Cách biểu diễn bit trong vi điều khiển
- Các phép logic với bit

# 1. KHÁI NIỆM
## 1.1 Bit là gì?
- Bit là đơn vị nhỏ nhất để lưu trữ dữ liệu trong máy tính.
- Bit chỉ có 2 giá trị 0 và 1 (Có thể dùng để biểu thị trạng thái: đóng/mở, thấp/cao, sai/đúng,..)
- Tên gọi bit là viết tắt của binary digit (chữ số nhị phân).
- Và nếu mọi người có đọc qua về kiến thức C, thì chắc đã biết được, ngôn ngữ được biên dịch ra cho máy hiểu chính là các chữ số 0 và 1 này.

## 1.2 Byte là gì?
- Byte là tập hợp của 8 bit.
- Một byte có thể biểu diễn 256 giá trị khác nhau (từ 00000000 = 0 đến 11111111 = 255).
- Đây là đơn vị cơ bản để máy tính xử lý dữ liệu (ký tự, màu, số,...).

# 2. LÀM QUEN VỚI CÁC HỆ CHỮ SỐ
Chúng ta sẽ cùng nhau tìm hiểu về cách tính toán cũng như chuyển đổi giữa các hệ với nhau.
## 2.1 Hệ nhị phân
- Hệ nhị phân là hệ chỉ gồm chữ số 0 và 1 như cách đề cập ở trên.
- Lấy ví dụ: 0010 0011 0001 0111. Đây chính là hệ nhị phân.
- Bây giờ cùng tìm hiểu cách tính toán với hệ nhị phân này. Nhưng trước đó, các bạn cần nắm rõ vấn đề này, đó là dù cho là hệ nhị phân, hệ thập lục phân, hệ bát phân, thì chúng đều được quy đổi về một giá trị cố định, độc nhất, chuẩn ở hệ thập phân. Lấy ví dụ:

```cpp
Khi này, có thể thấy rằng đến số 1 ta đã hết số để đi tiếp nên sẽ quay về 0 và tăng chữ số phía trước lên 1, tương tự cho các số phía sau
Nhị phân:   0   1   10  11  100 101
Thập phân:  0   1   3   4   5   6
```

- Cách tính toán để chuyển đổi giữa hệ 2 và hệ 10 đó là:

![alt text](image-2.png)

![alt text](image-3.png)

## 2.2 Hệ thập lục phân (Hệ 16)
- Hệ thập lục phân là hệ gồm 16 kí tự, bao gồm 10 kí tự số và 6 kí tự chữ. Vì độ dài các chữ số chỉ gồm từ 0 đến 9 thôi, nên họ đã chèn thêm các kí tự chữ cái gồm A đến F vào.
- Lấy ví dụ: 7128 AFF8 981D. Đây chính là hệ 16.
- Bây giờ cùng tìm hiểu cách tính toán với hệ 16 này nhé. Tương tự với hệ nhị phân.

```cpp
Thập lục phân:  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   10  11  12
Thập phân:      0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18
```

- Cách tính toán để chuyển đổi giữa hệ 16 và hệ 10 đó là:

![alt text](image.png)

![alt text](image-1.png)

## 2.3 Hệ thập lục phân và Hệ nhị phận
- Có một cách chuyển đổi nhanh giữa hệ 16 và hệ 2. Đó là 1 ký tự trong hệ 16 tương đương với 4 kí tự trong hệ 2.
- Lấy ví dụ: Hệ 16: 3ABF <-> Hệ 2: 0011(3) 1010(A) 1011(B) 1111(F)
- Có một cách tra nhanh là dựa vào bảng sau:

![alt text](image-4.png)

# 3. CÁCH BIỂU DIỄN BIT TRONG VI ĐIỀU KHIỂN
- Trong lập trình thanh ghi chắc hẳn khi mọi người đọc datasheet sẽ không lạ với các địa chỉ thanh ghi hay cách thanh ghi tổ chức dưới dạng các bit như này:

![alt text](image-5.png)

![alt text](image-6.png)

