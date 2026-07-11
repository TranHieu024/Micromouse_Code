# HƯỚNG DẪN TRIỂN KHAI CHO TEAM KỸ THUẬT
*(Bản quyền: Harmony Canvas - Cấu trúc 100% Cloud Miễn Phí)*

Thư mục `deploy_package` này chứa phiên bản mã nguồn đã được tối ưu hóa cực nhẹ để đẩy thẳng lên nền tảng đám mây (Cloud). Không lưu bất kỳ dữ liệu nào ở ổ cứng nội bộ, đảm bảo Web sống vĩnh viễn không bao giờ sập.

Hệ sinh thái bao gồm:
1. **Render.com** (Chạy code Python FastAPI)
2. **Supabase** (Lưu trữ Text/Database)
3. **Cloudinary** (Lưu trữ Hình Ảnh)

---

## BƯỚC 1: CHUẨN BỊ 3 TÀI KHOẢN CLOUD

### 1. Tạo Database trên Supabase (Miễn phí)
- Truy cập `supabase.com`, đăng ký bằng Github/Gmail.
- Bấm **New Project**, đặt tên là `harmony-db`, chọn khu vực `Singapore` cho mạng nhanh. Mật khẩu tự đặt (nhớ lưu lại).
- Chờ khoảng 1-2 phút để Database khởi tạo.
- Vào phần **Settings > API**, copy 2 mã:
  - `Project URL` (Đây là SUPABASE_URL)
  - `Project API keys (anon, public)` (Đây là SUPABASE_KEY)
- Vào mục **SQL Editor**, mở file `harmony_canvas.db` bằng phần mềm SQLite (trên máy tính) và Export ra file SQL, sau đó chạy đoạn SQL đó vào Supabase để tạo 3 bảng (`products`, `orders`, `coupons`).

### 2. Tạo Kho Ảnh trên Cloudinary (Miễn phí)
- Truy cập `cloudinary.com`, đăng ký tài khoản.
- Ngay ở màn hình chính (Dashboard), bạn sẽ thấy phần **API Environment variable**.
- Dòng chữ bắt đầu bằng `cloudinary://...` chính là biến `CLOUDINARY_URL`. Hãy copy lại.
- *Lưu ý: Mọi ảnh tải lên từ Web của khách sẽ tự động nén còn 150KB/ảnh và đẩy thẳng vào đây.*

### 3. Tạo cổng thanh toán PayOS (Đã có sẵn)
- Đăng nhập `payos.vn`.
- Lấy `Client ID`, `API Key`, `Checksum Key`.

---

## BƯỚC 2: UP CODE LÊN GITHUB

Render.com bắt buộc phải kéo code từ Github. Vì vậy team hãy đẩy nguyên cái thư mục `deploy_package` này lên Github (dưới dạng Private Repo).

```bash
git init
git add .
git commit -m "Bản chuẩn Cloud Deploy"
git branch -M main
git remote add origin https://github.com/TenCuaBan/RepoCuaBan.git
git push -u origin main
```

---

## BƯỚC 3: ĐẨY LÊN RENDER.COM CHỈ VỚI 1 CLICK

Vì trong code đã có sẵn file `render.yaml`, việc deploy lên Render sẽ được **tự động hóa 100%**.

1. Truy cập `render.com`, đăng nhập bằng Github.
2. Bấm nút **New +**, chọn **Blueprint**.
3. Render sẽ quét danh sách Github của bạn, hãy bấm chọn vào Repo bạn vừa đẩy lên ở Bước 2.
4. Render sẽ tự động đọc file `render.yaml` và hỏi bạn muốn khởi tạo `harmony-canvas-api` không. Bấm **Apply Blueprint**.
5. Giai đoạn quan trọng nhất: **Nhập Environment Variables (Biến môi trường)**
   - Ngay trong phần Dashboard của dịch vụ vừa tạo trên Render, chọn mục **Environment**.
   - Bấm **Add Environment Variable** và nhập đủ các cặp Key - Value (như trong file `.env.example`).
   - Ví dụ:
     - Tên: `SUPABASE_URL` | Giá trị: `https://xxx.supabase.co`
     - Tên: `CLOUDINARY_URL` | Giá trị: `cloudinary://123:abc@cloudname`
     - ...
6. Lưu lại. Render sẽ tự khởi động lại Web.

Xong! Giờ web đã hoạt động hoàn chỉnh và chạy vĩnh viễn trên Cloud. Khách hàng xem ảnh sẽ lấy ảnh trực tiếp từ Cloudinary siêu tốc độ, đơn hàng bắn thẳng về Supabase và Gửi Email!
