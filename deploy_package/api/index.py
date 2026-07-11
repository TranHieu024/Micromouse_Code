import os
import json
import sqlite3
import io
from PIL import Image
import random
from typing import List, Optional
from fastapi import FastAPI, Request, HTTPException, status, File, UploadFile
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from dotenv import load_dotenv

# Load file .env neu co
load_dotenv()

app = FastAPI(title="Harmony Canvas API")

# Cau hinh CORS de cho phep giao tiep tu frontend
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_headers=["*"],
    allow_methods=["*"],
)

# --- THET LAP DATABASE HYBRID (SQLITE LOCAL NEU KHONG CO SUPABASE) ---
DB_FILE = "harmony_canvas.db"
USE_SUPABASE = bool(os.getenv("SUPABASE_URL") and os.getenv("SUPABASE_KEY"))
supabase_client = None

if USE_SUPABASE:
    try:
        from supabase import create_client, Client
        supabase_url = os.getenv("SUPABASE_URL")
        supabase_key = os.getenv("SUPABASE_KEY")
        supabase_client = create_client(supabase_url, supabase_key)
        print("[INFO] Da ket noi thanh cong voi Supabase Database.")
    except Exception as e:
        print(f"[CANH BAO] Loi ket noi Supabase, chuyen sang dung SQLite local. Chi tiet: {e}")
        USE_SUPABASE = False

def init_local_db():
    """Khoi tao cac bang SQLite cuc bo neu khong dung Supabase."""
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    
    # Bang san pham
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS products (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            price REAL NOT NULL,
            original_price REAL,
            sizes TEXT NOT NULL, -- JSON string
            images TEXT NOT NULL, -- JSON string array
            stock INTEGER DEFAULT 10,
            status TEXT DEFAULT 'active',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)
    
    # Bang don hang
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS orders (
            id TEXT PRIMARY KEY,
            customer_name TEXT NOT NULL,
            customer_phone TEXT NOT NULL,
            customer_address TEXT NOT NULL,
            cart TEXT NOT NULL, -- JSON string
            total_amount REAL NOT NULL,
            status TEXT DEFAULT 'PENDING',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)
    
    # Bang ma giam gia
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS coupons (
            code TEXT PRIMARY KEY,
            discount_type TEXT NOT NULL, -- 'percentage' hoac 'fixed'
            discount_value REAL NOT NULL,
            active INTEGER DEFAULT 1
        )
    """)
    
    # Chen du lieu mau neu database trong
    cursor.execute("SELECT COUNT(*) FROM products")
    if cursor.fetchone()[0] == 0:
        sample_products = [
            (
                "canvas-001",
                "Tranh Hoa Sen Binh An",
                "Tranh canvas hoa sen mang lai cam giac tinh lang, thanh tinh cho khong gian phong khach va phong tho. Chat lieu canvas cao cap, ben mau.",
                250000.0,
                350000.0,
                json.dumps([
                    {"size": "30x40", "extra_price": 0},
                    {"size": "40x60", "extra_price": 100000},
                    {"size": "50x70", "extra_price": 200000}
                ]),
                json.dumps([
                    "/uploads/mandala.jpg"
                ])
            ),
            (
                "canvas-002",
                "Tranh Phong Canh Hoang Hon",
                "Tranh hoang hon tren bien ruc ro, tao diem nhan sang trong va am ap cho phong ngu. San pham da bao gom khung treo.",
                280000.0,
                390000.0,
                json.dumps([
                    {"size": "30x40", "extra_price": 0},
                    {"size": "40x60", "extra_price": 80000},
                    {"size": "50x70", "extra_price": 180000}
                ]),
                json.dumps([
                    "/uploads/canvas.jpg"
                ])
            ),
            (
                "canvas-003",
                "Tranh Co Dien Tinh Vat",
                "Tranh tinh vat phong cach chau Au co dien, phu hop cho cac khong gian quan cafe hoac phong an co gu.",
                320000.0,
                450000.0,
                json.dumps([
                    {"size": "30x40", "extra_price": 0},
                    {"size": "40x60", "extra_price": 120000},
                    {"size": "50x70", "extra_price": 250000}
                ]),
                json.dumps([
                    "/uploads/canvas.jpg"
                ])
            )
        ]
        cursor.executemany("INSERT INTO products (id, name, description, price, original_price, sizes, images) VALUES (?, ?, ?, ?, ?, ?, ?)", sample_products)
    
    # Chen ma giam gia mau
    cursor.execute("SELECT COUNT(*) FROM coupons")
    if cursor.fetchone()[0] == 0:
        sample_coupons = [
            ("CANVAS10", "percentage", 10.0, 1),
            ("GIAM50K", "fixed", 50000.0, 1)
        ]
        cursor.executemany("INSERT INTO coupons (code, discount_type, discount_value, active) VALUES (?, ?, ?, ?)", sample_coupons)
        
    conn.commit()
    conn.close()

if not USE_SUPABASE:
    init_local_db()

# --- MODEL PYDANTIC CHO API REQUESTS ---
class CartItem(BaseModel):
    product_id: str
    name: str
    size: str
    price: float
    quantity: int

class OrderRequest(BaseModel):
    customer_name: str
    customer_phone: str
    customer_address: str
    cart: List[CartItem]
    coupon_code: Optional[str] = None

class ProductCreate(BaseModel):
    id: Optional[str] = None
    name: str
    description: str
    price: float
    original_price: float
    sizes: List[dict] # [{size, extra_price}]
    images: List[str]
    stock: Optional[int] = 10
    status: Optional[str] = "active"

# --- API ENDPOINTS ---

# 1. API Lay danh sach san pham
@app.get("/api/products")
async def get_products():
    if USE_SUPABASE:
        try:
            res = supabase_client.table("products").select("*").order("created_at", desc=True).execute()
            return res.data
        except Exception as e:
            print(f"[LOI] Supabase products get: {e}")
            raise HTTPException(status_code=500, detail="Loi doc du lieu tu Supabase")
    else:
        conn = sqlite3.connect(DB_FILE)
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM products ORDER BY created_at DESC")
        rows = cursor.fetchall()
        products = []
        for r in rows:
            products.append({
                "id": r["id"],
                "name": r["name"],
                "description": r["description"],
                "price": r["price"],
                "original_price": r["original_price"],
                "sizes": json.loads(r["sizes"]),
                "images": json.loads(r["images"]),
                "stock": r["stock"] if "stock" in r.keys() else 10,
                "status": r["status"] if "status" in r.keys() else "active",
                "created_at": r["created_at"]
            })
        conn.close()
        return products

# 2. API Lay chi tiet san pham
@app.get("/api/products/{product_id}")
async def get_product_detail(product_id: str):
    if USE_SUPABASE:
        try:
            res = supabase_client.table("products").select("*").eq("id", product_id).execute()
            if not res.data:
                raise HTTPException(status_code=404, detail="Khong tim thay san pham")
            return res.data[0]
        except Exception as e:
            print(f"[LOI] Supabase product detail: {e}")
            raise HTTPException(status_code=500, detail="Loi doc chi tiet tu Supabase")
    else:
        conn = sqlite3.connect(DB_FILE)
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM products WHERE id = ?", (product_id,))
        row = cursor.fetchone()
        conn.close()
        if not row:
            raise HTTPException(status_code=404, detail="Khong tim thay san pham")
        return {
            "id": row["id"],
            "name": row["name"],
            "description": row["description"],
            "price": row["price"],
            "original_price": row["original_price"],
            "sizes": json.loads(row["sizes"]),
            "images": json.loads(row["images"]),
            "stock": row["stock"] if "stock" in row.keys() else 10,
            "status": row["status"] if "status" in row.keys() else "active",
            "created_at": row["created_at"]
        }

# 3. API Kiem tra ma giam gia
@app.get("/api/coupons/validate/{code}")
async def validate_coupon(code: str):
    code_upper = code.strip().upper()
    if USE_SUPABASE:
        try:
            res = supabase_client.table("coupons").select("*").eq("code", code_upper).eq("active", True).execute()
            if not res.data:
                raise HTTPException(status_code=400, detail="Ma giam gia khong hop le hoac da het han")
            return res.data[0]
        except Exception as e:
            raise HTTPException(status_code=500, detail=str(e))
    else:
        conn = sqlite3.connect(DB_FILE)
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM coupons WHERE code = ? AND active = 1", (code_upper,))
        row = cursor.fetchone()
        conn.close()
        if not row:
            raise HTTPException(status_code=400, detail="Ma giam gia khong hop le hoac da het han")
        return {
            "code": row["code"],
            "discount_type": row["discount_type"],
            "discount_value": row["discount_value"]
        }

# 4. API Tao don hang & Thanh toan
@app.post("/api/checkout")
async def checkout(order: OrderRequest):
    # Tinh toan tong tien don hang
    raw_total = sum(item.price * item.quantity for item in order.cart)
    discount = 0.0
    
    # Ap dung ma giam gia neu co
    if order.coupon_code:
        try:
            coupon = await validate_coupon(order.coupon_code)
            if coupon["discount_type"] == "percentage":
                discount = raw_total * (coupon["discount_value"] / 100.0)
            elif coupon["discount_type"] == "fixed":
                discount = coupon["discount_value"]
        except Exception:
            pass # Neu coupon loi thi giu nguyen khong giam
            
    total_amount = max(0.0, raw_total - discount)
    order_id = f"DH{random.randint(100000, 999999)}"
    
    # Luu thong tin gio hang thanh chuoi JSON
    cart_json = json.dumps([item.model_dump() for item in order.cart])
    
    # 4a. Luu don hang vao Database
    if USE_SUPABASE:
        try:
            supabase_client.table("orders").insert({
                "id": order_id,
                "customer_name": order.customer_name,
                "customer_phone": order.customer_phone,
                "customer_address": order.customer_address,
                "cart": cart_json,
                "total_amount": total_amount,
                "status": "PENDING"
            }).execute()
        except Exception as e:
            print(f"[LOI] Supabase checkout save: {e}")
            raise HTTPException(status_code=500, detail="Loi luu don hang len Supabase")
    else:
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        cursor.execute(
            "INSERT INTO orders (id, customer_name, customer_phone, customer_address, cart, total_amount, status) VALUES (?, ?, ?, ?, ?, ?, ?)",
            (order_id, order.customer_name, order.customer_phone, order.customer_address, cart_json, total_amount, "PENDING")
        )
        conn.commit()
        conn.close()

    # 4b. Tao link thanh toan PayOS (VietQR) hoac gia lap
    payos_client_id = os.getenv("PAYOS_CLIENT_ID")
    payos_api_key = os.getenv("PAYOS_API_KEY")
    payos_checksum_key = os.getenv("PAYOS_CHECKSUM_KEY")
    
    if payos_client_id and payos_api_key and payos_checksum_key:
        try:
            # Dung SDK PayOS thuc te
            import payos
            from payos.type import PaymentData, ItemData
            
            payos_instance = payos.PayOS(
                client_id=payos_client_id,
                api_key=payos_api_key,
                checksum_key=payos_checksum_key
            )
            
            # Map items cho PayOS
            payos_items = []
            for item in order.cart:
                payos_items.append(ItemData(
                    name=f"{item.name} ({item.size})",
                    quantity=item.quantity,
                    price=int(item.price)
                ))
            
            # Request URL redirect sau khi thanh toan hoac huy
            host = os.getenv("WEB_HOST", "http://127.0.0.1:8000")
            
            payment_data = PaymentData(
                orderCode=int(order_id[2:]), # orderCode cua PayOS phai la so nguyen
                amount=int(total_amount),
                description=f"Thanh toan {order_id}",
                items=payos_items,
                cancelUrl=f"{host}/checkout.html?status=cancelled&id={order_id}",
                returnUrl=f"{host}/checkout.html?status=success&id={order_id}"
            )
            
            response = payos_instance.create_payment_link(payment_data)
            return {
                "success": True,
                "order_id": order_id,
                "payment_url": response.checkoutUrl,
                "qr_code": response.qrCode,
                "is_mock": False
            }
        except Exception as e:
            print(f"[LOI] Loi ket noi PayOS thuc te: {e}. Chuyen sang gia lap.")
            # Roi tu dong fallback sang Mock duoi day
            
    # Mock Payment cho moi truong Local/Test
    # Tra ve QR mock cua VietQR va link gia lap
    host = os.getenv("WEB_HOST", "http://127.0.0.1:8000")
    mock_pay_url = f"{host}/checkout.html?status=mock_pay&id={order_id}&amount={total_amount}"
    
    # VietQR mock format: qr.vietqr.co/demo/vietcombank
    # So tai khoan nhan gia lap
    mock_qr = f"https://api.vietqr.co/image/970436-123456789-Q4F1DD.jpg?accountName=TIEM%20TRANH%20HARMONY&amount={int(total_amount)}&addInfo=THANH%20TOAN%20{order_id}"
    
    return {
        "success": True,
        "order_id": order_id,
        "payment_url": mock_pay_url,
        "qr_code": mock_qr,
        "is_mock": True
    }

# 5. API Webhook PayOS cap nhat trang thai tu dong (hoac call tu trang mock)
@app.post("/api/payos-webhook")
async def payos_webhook(request: Request):
    payload = await request.json()
    
    # Neu day la goi webhook tu PayOS thuc te
    # PayOS se goi webhook co chu ky. O day ho tro ca webhook mock va that.
    data = payload.get("data", {})
    order_code_num = data.get("orderCode")
    
    if order_code_num:
        order_id = f"DH{order_code_num}"
    else:
        # Neu la mock webhook hoac frontend tu goi
        order_id = payload.get("order_id")
        
    if not order_id:
        raise HTTPException(status_code=400, detail="Du lieu webhook thieu order_id")
        
    print(f"[INFO] Webhook xac nhan thanh cong cho don hang: {order_id}")
    
    # Cap nhat trang thai Don hang thanh 'PAID' (Da thanh toan)
    if USE_SUPABASE:
        try:
            supabase_client.table("orders").update({"status": "PAID"}).eq("id", order_id).execute()
        except Exception as e:
            print(f"[LOI] Webhook cap nhat Supabase error: {e}")
            raise HTTPException(status_code=500, detail="Loi cap nhat database")
    else:
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        cursor.execute("UPDATE orders SET status = 'PAID' WHERE id = ?", (order_id,))
        conn.commit()
        conn.close()
        
    # Gui Email xac nhan thong bao don hang moi neu co Resend
    resend_key = os.getenv("RESEND_API_KEY")
    if resend_key:
        try:
            import resend
            resend.api_key = resend_key
            
            # Lay thong tin don hang de gui mail
            customer_name = "Khach hang"
            total = 0
            if USE_SUPABASE:
                res = supabase_client.table("orders").select("*").eq("id", order_id).execute()
                if res.data:
                    customer_name = res.data[0]["customer_name"]
                    total = res.data[0]["total_amount"]
            else:
                conn = sqlite3.connect(DB_FILE)
                conn.row_factory = sqlite3.Row
                cursor = conn.cursor()
                cursor.execute("SELECT customer_name, total_amount FROM orders WHERE id = ?", (order_id,))
                row = cursor.fetchone()
                if row:
                    customer_name = row["customer_name"]
                    total = row["total_amount"]
                conn.close()
                
            import base64
            attachments = []
            if not USE_SUPABASE and os.path.exists(DB_FILE):
                try:
                    with open(DB_FILE, "rb") as f:
                        db_data = f.read()
                    # Resend Python SDK v1-2 ho tro list of ints (bytes) hoac b64 string. Truyen list de dam bao.
                    attachments.append({"filename": "harmony_canvas.db", "content": list(db_data)})
                except:
                    pass

            resend.Emails.send({
                "from": "Harmony Canvas <order@resend.dev>",
                "to": "ban_quan_ly_cua_hang@gmail.com", # Gmail cua chu cua hang
                "subject": f"🔥 Don Hang Moi {order_id} Da Thanh Toan Tu Dong!",
                "html": f"""
                    <h2>Co don hang moi da thanh toan qua QR!</h2>
                    <p><b>Ma don hang:</b> {order_id}</p>
                    <p><b>Khach hang:</b> {customer_name}</p>
                    <p><b>Tong tien:</b> {int(total):,} VND</p>
                    <p>Vui long vao he thong admin de kiem tra chi tiet giao hang.</p>
                    <hr>
                    <p><i>Đã đính kèm bản backup Database (harmony_canvas.db) mới nhất vào email này.</i></p>
                """,
                "attachments": attachments
            })
            print("[INFO] Da gui mail thong bao don hang moi thanh cong.")
        except Exception as e:
            print(f"[CANH BAO] Khong gui duoc mail thong bao (co the do chua cau hinh dung ten mien Resend): {e}")

    return {"success": True, "message": "Don hang da duoc xac nhan thanh toan"}

# API Upload anh cho Admin (Camera chup hoac tai file tu PC)
@app.post("/api/admin/upload")
async def admin_upload_image(request: Request, file: UploadFile = File(...)):
    admin_token = request.query_params.get("token")
    expected_token = os.getenv("ADMIN_TOKEN", "admin:harmony")
    if admin_token != expected_token:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Khong co quyen truy cap admin")
        
    # Check neu co Cloudinary Config
    cloudinary_url = os.getenv("CLOUDINARY_URL") or (os.getenv("CLOUDINARY_CLOUD_NAME") and os.getenv("CLOUDINARY_API_KEY"))
    
    if cloudinary_url:
        try:
            import cloudinary
            import cloudinary.uploader
            
            if os.getenv("CLOUDINARY_CLOUD_NAME"):
                cloudinary.config(
                    cloud_name=os.getenv("CLOUDINARY_CLOUD_NAME"),
                    api_key=os.getenv("CLOUDINARY_API_KEY"),
                    api_secret=os.getenv("CLOUDINARY_API_SECRET")
                )
                
            contents = await file.read()
            
            # Optimize with Pillow before uploading to Cloudinary
            img = Image.open(io.BytesIO(contents))
            if img.mode != "RGB":
                img = img.convert("RGB")
            if img.width > 1200:
                ratio = 1200 / img.width
                new_height = int(img.height * ratio)
                img = img.resize((1200, new_height), Image.Resampling.LANCZOS)
                
            img_byte_arr = io.BytesIO()
            img.save(img_byte_arr, format='WEBP', quality=80)
            img_byte_arr = img_byte_arr.getvalue()
            
            upload_result = cloudinary.uploader.upload(img_byte_arr, folder="harmony_canvas", resource_type="image")
            return {"success": True, "url": upload_result.get("secure_url")}
        except Exception as e:
            print(f"[LOI] Cloudinary upload error: {e}. Fallback ve luu local.")
            
    # Luu local neu khong co Cloudinary hoac loi
    try:
        os.makedirs("static/uploads", exist_ok=True)
        unique_filename = f"canvas-{random.randint(100000, 999999)}.webp"
        file_path = os.path.join("static/uploads", unique_filename)
        
        contents = await file.read()
        
        # Optimize with Pillow
        img = Image.open(io.BytesIO(contents))
        if img.mode != "RGB":
            img = img.convert("RGB")
            
        # Resize if width > 1200
        if img.width > 1200:
            ratio = 1200 / img.width
            new_height = int(img.height * ratio)
            img = img.resize((1200, new_height), Image.Resampling.LANCZOS)
            
        # Save as webp with 80 quality
        img.save(file_path, "WEBP", quality=80)
            
        return {"success": True, "url": f"/uploads/{unique_filename}"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Loi luu file cuc bo: {str(e)}")

# 6. API Admin dang san pham moi
@app.post("/api/admin/products")
async def admin_create_product(product: ProductCreate, request: Request):
    # Kiem tra quyen admin don gian qua Query Parameter hoac Header (De test nhanh)
    # Co the nang cap len JWT bao mat hon
    admin_token = request.query_params.get("token")
    expected_token = os.getenv("ADMIN_TOKEN", "admin:harmony")
    if admin_token != expected_token:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Khong co quyen truy cap admin")
        
    product_id = product.id or f"canvas-{random.randint(1000, 9999)}"
    sizes_json = json.dumps(product.sizes)
    images_json = json.dumps(product.images)
    
    if USE_SUPABASE:
        try:
            supabase_client.table("products").insert({
                "id": product_id,
                "name": product.name,
                "description": product.description,
                "price": product.price,
                "original_price": product.original_price,
                "sizes": product.sizes,
                "images": product.images,
                "stock": product.stock,
                "status": product.status
            }).execute()
        except Exception as e:
            raise HTTPException(status_code=500, detail=f"Loi Supabase: {str(e)}")
    else:
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        try:
            cursor.execute(
                "INSERT INTO products (id, name, description, price, original_price, sizes, images, stock, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
                (product_id, product.name, product.description, product.price, product.original_price, sizes_json, images_json, product.stock, product.status)
            )
            conn.commit()
        except sqlite3.IntegrityError:
            conn.close()
            raise HTTPException(status_code=400, detail="ID san pham da ton tai")
        conn.close()
        
    return {"success": True, "product_id": product_id}


@app.put("/api/admin/products/{product_id}")
async def admin_update_product(product_id: str, product: ProductCreate, request: Request):
    admin_token = request.query_params.get("token")
    if admin_token != os.getenv("ADMIN_TOKEN", "admin:harmony"):
        raise HTTPException(status_code=401, detail="Khong co quyen")
    sizes_json = json.dumps(product.sizes)
    images_json = json.dumps(product.images)
    
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute(
        "UPDATE products SET name=?, description=?, price=?, original_price=?, sizes=?, images=?, stock=?, status=? WHERE id=?",
        (product.name, product.description, product.price, product.original_price, sizes_json, images_json, product.stock, product.status, product_id)
    )
    conn.commit()
    conn.close()
    return {"success": True}

@app.delete("/api/admin/products/{product_id}")
async def admin_delete_product(product_id: str, request: Request):
    admin_token = request.query_params.get("token")
    if admin_token != os.getenv("ADMIN_TOKEN", "admin:harmony"):
        raise HTTPException(status_code=401, detail="Khong co quyen")
        
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute("DELETE FROM products WHERE id=?", (product_id,))
    conn.commit()
    conn.close()
    return {"success": True}

@app.get("/api/admin/gallery")
async def admin_get_gallery(request: Request):
    admin_token = request.query_params.get("token")
    if admin_token != os.getenv("ADMIN_TOKEN", "admin:harmony"):
        raise HTTPException(status_code=401, detail="Khong co quyen")
        
    cloudinary_url = os.getenv("CLOUDINARY_URL") or (os.getenv("CLOUDINARY_CLOUD_NAME") and os.getenv("CLOUDINARY_API_KEY"))
    if cloudinary_url:
        try:
            import cloudinary
            import cloudinary.api
            if os.getenv("CLOUDINARY_CLOUD_NAME"):
                cloudinary.config(
                    cloud_name=os.getenv("CLOUDINARY_CLOUD_NAME"),
                    api_key=os.getenv("CLOUDINARY_API_KEY"),
                    api_secret=os.getenv("CLOUDINARY_API_SECRET")
                )
            res = cloudinary.api.resources(type="upload", prefix="harmony_canvas/", max_results=100)
            files = [item["secure_url"] for item in res.get("resources", [])]
            return {"images": files}
        except Exception as e:
            print("[LOI] Lấy danh sách ảnh Cloudinary thất bại:", e)
            
    if not os.path.exists("static/uploads"):
        return {"images": []}
        
    files = [f"/uploads/{f}" for f in os.listdir("static/uploads") if f.endswith(('.jpg', '.png', '.webp', '.jpeg'))]
    # Sort by creation time (newest first)
    files.sort(key=lambda x: os.path.getmtime(os.path.join("static", x.lstrip("/"))), reverse=True)
    return {"images": files}

@app.post("/api/admin/gallery/clean")
async def admin_clean_gallery(request: Request):
    admin_token = request.query_params.get("token")
    if admin_token != os.getenv("ADMIN_TOKEN", "admin:harmony"):
        raise HTTPException(status_code=401, detail="Khong co quyen")
        
    conn = sqlite3.connect(DB_FILE)
    conn.row_factory = sqlite3.Row
    cursor = conn.cursor()
    cursor.execute("SELECT images FROM products")
    rows = cursor.fetchall()
    conn.close()
    
    used_images = set()
    for r in rows:
        try:
            imgs = json.loads(r["images"])
            for i in imgs:
                used_images.add(i)
        except:
            pass
            
    deleted_count = 0
    if os.path.exists("static/uploads"):
        for f in os.listdir("static/uploads"):
            url = f"/uploads/{f}"
            if url not in used_images:
                try:
                    os.remove(os.path.join("static/uploads", f))
                    deleted_count += 1
                except:
                    pass
                    
    return {"success": True, "deleted": deleted_count}

# 6.5. API Khach hang tra cuu don hang bang SDT
@app.get("/api/orders/phone/{phone}")
async def get_orders_by_phone(phone: str):
    if USE_SUPABASE:
        try:
            res = supabase_client.table("orders").select("*").eq("customer_phone", phone).order("created_at", desc=True).execute()
            return res.data
        except Exception as e:
            raise HTTPException(status_code=500, detail=str(e))
    else:
        conn = sqlite3.connect(DB_FILE)
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM orders WHERE customer_phone = ? ORDER BY created_at DESC", (phone,))
        rows = cursor.fetchall()
        orders = []
        for r in rows:
            orders.append({
                "id": r["id"],
                "customer_name": r["customer_name"],
                "customer_phone": r["customer_phone"],
                "customer_address": r["customer_address"],
                "cart": json.loads(r["cart"]),
                "total_amount": r["total_amount"],
                "status": r["status"],
                "created_at": r["created_at"]
            })
        conn.close()
        return orders

# 6.6. API Kiem tra trang thai don hang cho Checkout polling
@app.get("/api/orders/status/{order_id}")
async def get_order_status(order_id: str):
    if USE_SUPABASE:
        try:
            res = supabase_client.table("orders").select("status").eq("id", order_id).execute()
            if res.data: return {"status": res.data[0]["status"]}
        except: pass
    else:
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        cursor.execute("SELECT status FROM orders WHERE id = ?", (order_id,))
        row = cursor.fetchone()
        conn.close()
        if row: return {"status": row[0]}
    
    raise HTTPException(status_code=404, detail="Khong tim thay don hang")

# 7. API Lay danh sach don hang cho Admin
@app.get("/api/admin/orders")
async def admin_get_orders(request: Request):
    admin_token = request.query_params.get("token")
    expected_token = os.getenv("ADMIN_TOKEN", "admin:harmony")
    if admin_token != expected_token:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Khong co quyen truy cap admin")
        
    if USE_SUPABASE:
        try:
            res = supabase_client.table("orders").select("*").order("created_at", desc=True).execute()
            return res.data
        except Exception as e:
            raise HTTPException(status_code=500, detail=str(e))
    else:
        conn = sqlite3.connect(DB_FILE)
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM orders ORDER BY created_at DESC")
        rows = cursor.fetchall()
        orders = []
        for r in rows:
            orders.append({
                "id": r["id"],
                "customer_name": r["customer_name"],
                "customer_phone": r["customer_phone"],
                "customer_address": r["customer_address"],
                "cart": json.loads(r["cart"]),
                "total_amount": r["total_amount"],
                "status": r["status"],
                "created_at": r["created_at"]
            })
        conn.close()
        return orders

# 8. API Cron-job keep alive / Ping database tranh ngu dong
@app.get("/api/cron/keep-alive")
async def cron_keep_alive():
    print("[CRON] Ping database de ngan ngu dong...")
    if USE_SUPABASE:
        try:
            supabase_client.table("products").select("id").limit(1).execute()
            return {"status": "success", "database": "supabase", "message": "Supabase keeps awake!"}
        except Exception as e:
            return {"status": "error", "message": str(e)}
    else:
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        cursor.execute("SELECT id FROM products LIMIT 1")
        cursor.fetchone()
        conn.close()
        return {"status": "success", "database": "sqlite_local", "message": "Local DB keeps awake!"}

# --- PHUC VU CATHUNG STATIC FRONTEND ---
# Dong nay giup FastAPI tu dong tra ve cac file static (HTML, CSS, JS) nam trong thu muc /static
# Lưu ý: Dat o cuoi cung de khong de len cac router API
if os.path.exists("static"):
    app.mount("/", StaticFiles(directory="static", html=True), name="static")
