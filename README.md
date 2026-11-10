# HỆ THỐNG ĐÈN GIAO THÔNG 2 TUYẾN ĐƯỜNG TRÊN MẠCH XNUCLEO-F103RB

**Bài tập lớn môn Vi Xử Lý – Vi Điều Khiển (CO3009)**  
Khoa Khoa Học và Kỹ Thuật Máy Tính – Trường Đại học Bách Khoa, ĐHQG-HCM  
Giảng viên hướng dẫn: **TS. Lê Trọng Nhân**

---

## 👥 Thành viên nhóm thực hiện

| STT | Họ và tên | MSSV | Tài khoản GitHub | Nhiệm vụ chính |
| :---: | :--- | :---: | :---: | :--- |
| 1 | Trịnh Thanh Quang (Nhóm trưởng) | [MSSV 1] | `@quangtrinhthanhcse` | Scheduler O(1), Software Timer, Tích hợp hệ thống |
| 2 | [Họ và tên thành viên 2] | [MSSV 2] | `@github_username2` | Drivers (Button debounce, Traffic Light, DHT20, LCD I2C) |
| 3 | [Họ và tên thành viên 3] | [MSSV 3] | `@github_username3` | Máy trạng thái (FSM Auto, Manual, Tuning), Báo cáo |

---

## 📌 Tổng quan dự án

Hệ thống điều khiển đèn giao thông thông minh cho ngã tư 2 tuyến đường hoạt động trên nền vi điều khiển **STM32F103RB** (mạch XNUCLEO-F103RB) với các tính năng cốt lõi:
1. **Chế độ tự động (Automatic):** Hai tuyến đèn Đỏ – Xanh – Vàng hoạt động luân phiên theo chu kỳ định trước, đảm bảo an toàn giao thông.
2. **Chế độ bằng tay (Manual):** Cho phép lực lượng điều phối chuyển pha đèn khẩn cấp bằng nút bấm.
3. **Chế độ chỉnh thời gian (Tuning):** Cài đặt thời gian Đỏ/Xanh/Vàng trực tiếp bằng nút bấm, hiển thị trực quan qua màn hình LCD I2C với điều kiện ràng buộc hợp lệ: $t_{\text{Đỏ}} = t_{\text{Xanh}} + t_{\text{Vàng}}$.
4. **Hiển thị môi trường:** Đọc cảm biến nhiệt độ & độ ẩm DHT20 qua giao tiếp I2C và cập nhật định kỳ lên LCD.
5. **Scheduler $O(1)$:** Toàn bộ hệ thống chạy trên bộ điều phối tác vụ (Cooperative Scheduler) đạt độ phức tạp $O(1)$ cho hàm `SCH_Update()`, hỗ trợ one-shot task và không sử dụng hàm trễ chặn (`HAL_Delay`).

---

## 🔌 Sơ đồ kết nối phần cứng (Pinout)

| Thiết bị ngoại vi | Chân Arduino (Shield Grove) | Chân STM32 | Chế độ GPIO / Ngoại vi | Chức năng |
| :--- | :---: | :---: | :--- | :--- |
| **Đèn giao thông tuyến 1** | D3, D4 | `PB3`, `PB5` *(tra cứu .ioc)* | Output Push-Pull | 2 chân tạo 4 trạng thái đèn |
| **Đèn giao thông tuyến 2** | D5, D6 | `PB4`, `PB10` *(tra cứu .ioc)* | Output Push-Pull | 2 chân tạo 4 trạng thái đèn |
| **Module Nút nhấn 1 & 2** | D7, D8 | `PA8`, `PA9` *(tra cứu .ioc)* | Input Pull-up | Nút 1 (Đổi Mode), Nút 2 (Select) |
| **Module Nút nhấn 3 & 4** | D9, D10 | `PC7`, `PB6` *(tra cứu .ioc)* | Input Pull-up | Nút 3 (Tăng +1s), Nút 4 (Giảm -1s) |
| **Cảm biến DHT20 & LCD** | Khe I2C (màu đỏ) | `PB6` (SCL), `PB7` (SDA) | I2C1 (Standard Mode 100kHz) | Đọc DHT20 & hiển thị LCD |
| **LED tích hợp trên bo** | – | `PA5` | Output Push-Pull | Đèn báo nhịp tim hệ thống / Debug |
| **Nút nhấn USER trên bo** | – | `PC13` | Input | Nút phụ / Debug |

---

## 📂 Cấu trúc mã nguồn

```text
├── Core/
│   ├── Inc/
│   │   ├── global.h              # Biến toàn cục, cấu hình hệ thống, enum trạng thái
│   │   ├── scheduler.h           # Bộ điều phối tác vụ O(1)
│   │   ├── software_timer.h      # Timer phần mềm đếm thời gian đèn
│   │   ├── button.h              # Xử lý nút nhấn & chống dội (debounce)
│   │   ├── traffic_light.h       # Driver điều khiển đèn 2 tuyến độc lập
│   │   ├── dht20.h               # Driver giao tiếp cảm biến DHT20 (I2C)
│   │   ├── lcd.h                 # Driver hiển thị màn hình LCD (I2C)
│   │   ├── fsm_automatic.h       # Máy trạng thái Chế độ Tự động
│   │   ├── fsm_manual.h          # Máy trạng thái Chế độ Bằng tay
│   │   └── fsm_tuning.h          # Máy trạng thái Chế độ Cài đặt thời gian
│   └── Src/
│       ├── main.c                # Khởi tạo phần cứng HAL, khởi động Scheduler
│       └── [các file .c tương ứng]
├── .gitignore                    # Bỏ qua các file sinh ra khi build
├── README.md                     # Hướng dẫn dự án
└── *.ioc                         # Cấu hình phần cứng STM32CubeMX
```

---

## 🛠️ Hướng dẫn Build và Nạp chương trình

1. **Yêu cầu phần mềm:** Cài đặt **STM32CubeIDE** (phiên bản $\ge$ 1.14.0).
2. **Mở dự án:**
   - Mở STM32CubeIDE $\rightarrow$ `File` $\rightarrow$ `Open Projects from File System...`
   - Trỏ đường dẫn đến thư mục dự án và nhấn `Finish`.
3. **Lưu ý cấu hình phần cứng (BẮT BUỘC):**
   - Mở file cấu hình `.ioc`.
   - Vào mục **System Core** $\rightarrow$ **SYS** $\rightarrow$ Kiểm tra mục **Debug** đã chọn **Serial Wire**. *(Tuyệt đối không để No Debug để tránh lỗi chip nạp 1 lần!)*.
4. **Biên dịch & Nạp:**
   - Nhấn tổ hợp `Ctrl + B` để biên dịch mã nguồn (yêu cầu không có lỗi).
   - Kết nối mạch XNUCLEO-F103RB với máy tính qua cổng cáp Micro-USB.
   - Nhấn nút **Run** (biểu tượng Play màu xanh) để nạp chương trình xuống vi điều khiển.

---

## 🎮 Hướng dẫn Vận hành

* **Khi vừa khởi động / Reset:** Hệ thống chạy ở **Chế độ Tự động (Automatic)**, luân phiên pha đèn Tuyến 1 và Tuyến 2. LCD hiển thị nhiệt độ/độ ẩm đọc từ DHT20.
* **Nút 1 (Chuyển chế độ):** Nhấn để chuyển vòng tròn: `Auto` $\rightarrow$ `Manual` $\rightarrow$ `Tuning` $\rightarrow$ `Auto`.
* **Trong Chế độ Manual (Bằng tay):**
  - Nhấn **Nút 2** để chuyển pha đèn tiếp theo một cách an toàn.
* **Trong Chế độ Tuning (Cài đặt):**
  - Nhấn **Nút 2** để chọn loại đèn cần chỉnh (`RED` $\rightarrow$ `GREEN` $\rightarrow$ `AMBER`).
  - Nhấn **Nút 3** để tăng thời gian (+1 giây).
  - Nhấn **Nút 4** để giảm thời gian (-1 giây).
  - Nhấn **Nút 1** để xác nhận & thoát về chế độ Auto (hệ thống tự kiểm tra ràng buộc $t_{\text{Đỏ}} = t_{\text{Xanh}} + t_{\text{Vàng}}$, nếu sai sẽ báo lỗi và khôi phục giá trị cũ).
