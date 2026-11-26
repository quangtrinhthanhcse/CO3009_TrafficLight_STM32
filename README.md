# HỆ THỐNG ĐÈN GIAO THÔNG 2 TUYẾN ĐƯỜNG TRÊN MẠCH XNUCLEO-F103RB

**Bài tập lớn môn Vi Xử Lý – Vi Điều Khiển (CO3009)**  
Khoa Khoa Học và Kỹ Thuật Máy Tính – Trường Đại học Bách Khoa, ĐHQG-HCM  
Giảng viên hướng dẫn: **TS. Lê Trọng Nhân**

---

## 👥 Thành viên nhóm thực hiện

| STT | Họ và tên | MSSV | Nhiệm vụ chính |
| :---: | :--- | :---: | :--- |
| 1 | Trịnh Thanh Quang (Nhóm trưởng) | 2412862 | Scheduler O(1), Software Timer, Lên ý tưởng logic điều khiển & tính năng mở rộng, Tích hợp hệ thống, Demo sản phẩm |
| 2 | Nguyễn Thành Phát | 2412595 | Drivers (Button debounce, Traffic Light, DHT20, LCD I2C), Tổng hợp báo cáo |
| 3 | Đỗ Lê Nhật Minh | 2412040 | Máy trạng thái (FSM Auto, Manual, Tuning), Tổng hợp báo cáo |

---

## 📌 Tổng quan dự án

Hệ thống điều khiển đèn giao thông thông minh cho ngã tư 2 tuyến đường hoạt động trên nền vi điều khiển **STM32F103RB** (mạch XNUCLEO-F103RB) với các tính năng cốt lõi:
1. **Chế độ tự động (Automatic):** Hai tuyến đèn Đỏ – Xanh – Vàng hoạt động luân phiên theo chu kỳ định trước, đảm bảo an toàn giao thông.
2. **Chế độ bằng tay (Manual):** Cho phép lực lượng điều phối chuyển pha đèn khẩn cấp bằng nút bấm.
3. **Chế độ chỉnh thời gian (Tuning):** Cài đặt thời gian Đỏ/Xanh trực tiếp bằng nút bấm, hiển thị trực quan giá trị Vàng tạm tính qua màn hình LCD I2C với điều kiện hợp lệ: $t_{\text{Vàng}} = t_{\text{Đỏ}} - t_{\text{Xanh}} \ge 3\text{s}$ và $t_{\text{Xanh}} \ge t_{\text{Vàng}}$.
4. **Hiển thị môi trường:** Đọc cảm biến nhiệt độ & độ ẩm DHT20 qua giao tiếp I2C và cập nhật định kỳ lên LCD.
5. **Scheduler $O(1)$:** Toàn bộ hệ thống chạy trên bộ điều phối tác vụ (Cooperative Scheduler) đạt độ phức tạp $O(1)$ cho hàm `SCH_Update()`, hỗ trợ one-shot task và không sử dụng hàm trễ chặn (`HAL_Delay`).
6. **Chế độ Ban đêm (Night Mode - Tính năng mở rộng):** Nhấn giữ **Nút 1 (`BTN_1`)** $\ge 1\text{s}$ để cài đặt bộ hẹn giờ đếm ngược cho Night Mode (`BTN_3`/`BTN_4` chỉnh thời gian, `BTN_2` xác nhận bắt đầu chạy); khi chạy, LCD tắt hiển thị nhiệt độ/độ ẩm để hiển thị đếm ngược `NIGHT_MODE: XXh` và tự động trở về Chế độ Tự động khi hết giờ.

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
│   │   ├── i2c-lcd.h             # Driver hiển thị màn hình LCD (I2C PCF8574)
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

* **Khi vừa khởi động / Reset:** Hệ thống chạy ở **Chế độ Tự động (Automatic)**, luân phiên pha đèn Tuyến 1 và Tuyến 2. LCD mặc định hiển thị nhiệt độ/độ ẩm đọc từ DHT20 ở dòng 1 và đếm ngược pha đèn ở dòng 2.
* **Trong Chế độ Auto (Tự động):** Nhấn **Nút 4 (`BTN_4`)** để chuyển đổi qua lại hiển thị dòng 1 LCD giữa thông số môi trường (**DHT20**) và thời gian cấu hình đèn (**`R:XX  G:XX  A:XX`**) mà không ảnh hưởng tới chu kỳ đèn đang chạy.
* **Nút 1 (Chuyển chế độ):** Nhấn ngắn để chuyển vòng tròn: `Auto` $\rightarrow$ `Manual` *(giữ nguyên/đóng băng pha đèn đang chạy)* $\rightarrow$ `Tuning` $\rightarrow$ `Auto`.
* **Trong Chế độ Manual (Bằng tay):**
  - Khi vừa chuyển từ `Auto` sang, đèn vẫn giữ nguyên đúng pha trước đó để đảm bảo an toàn giao thông.
  - Nhấn **Nút 2** để chuyển pha đèn tiếp theo một cách an toàn.
  - Nhấn **Nút 3** để quay khẩn cấp về chế độ **Auto** tại pha liền kề kế tiếp và bắt đầu đếm ngược.
* **Trong Chế độ Tuning (Cài đặt):**
  - Đèn giao thông ngoài đường **vẫn tiếp tục chạy chu kỳ Auto ở nền** để không làm gián đoạn giao thông; trên dòng 2 LCD, cụm thông số đang chỉnh (`R:XXs` hoặc `G:XXs`) nhấp nháy chu kỳ 500 ms (250 ms sáng / 250 ms tắt).
  - Nhấn **Nút 2** để chuyển qua lại giữa chỉnh **Đỏ (`RED`)** và **Xanh (`GREEN`)** (LCD tự hiển thị kèm giá trị **Vàng (`A = R - G`)** tạm tính).
  - Nhấn **Nút 3** để tăng thời gian (+1 giây, hỗ trợ nhấn giữ tăng nhanh).
  - Nhấn **Nút 4** để giảm thời gian (-1 giây, hỗ trợ nhấn giữ giảm nhanh).
  - Nhấn **Nút 1** để xác nhận & thoát về chế độ Auto (yêu cầu $t_{\text{Vàng}} = t_{\text{Đỏ}} - t_{\text{Xanh}} \ge 3\text{s}$ và $t_{\text{Xanh}} \ge t_{\text{Vàng}}$; hiện `SAVED!` hoặc `ERR` trong 2.5s rồi nối tiếp mượt mà pha Auto đang chạy nền, thời gian mới tự áp dụng từ pha tiếp theo).
* **Nút 1 (`BTN_1`) - Chế độ Ban đêm có Hẹn giờ (Night Mode):**
  - Nhấn giữ **Nút 1** ($\ge 1\text{s}$) khi đang ở Chế độ Auto để vào màn hình hẹn giờ Night Mode (LCD ẩn nhiệt độ/độ ẩm, dòng 1 hiển thị `NIGHT_MODE: XXh`, đèn Vàng nháy báo hiệu mỗi 500 ms).
  - Dùng **Nút 3** (`+`) / **Nút 4** (`-`) để chỉnh thời gian đếm ngược (`01..99`), nhấn thả **Nút 2** để xác nhận bắt đầu chạy đếm lùi (đèn Vàng sáng 2s / tắt 1s).
  - Khi đếm ngược về `0`, hệ thống **tự động chuyển về Chế độ Auto** (hoặc bấm **Nút 1** để hủy/thoát sớm về Auto bất kỳ lúc nào).
