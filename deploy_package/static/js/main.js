// ==========================================================================
// HARMONY CANVAS - CORE FRONTEND JAVASCRIPT
// ==========================================================================

document.addEventListener('DOMContentLoaded', () => {
    initApp();
});

// State toàn cục của Frontend
const state = {
    products: [],
    cart: JSON.parse(localStorage.getItem('harmony_cart')) || [],
    currentCategory: 'Tất cả',
    searchQuery: '',
    currentCoupon: JSON.parse(localStorage.getItem('harmony_coupon')) || null
};

async function initApp() {
    updateCartBadge();
    setupMobileDrawers();
    setupSearchAndCategoryFilters();
    setupCouponLogic();
    
    // Check xem đang ở trang chủ hay trang chi tiết
    const productsContainer = document.getElementById('products-container');
    if (productsContainer) {
        await loadProductsList();
    }
    
    const detailContainer = document.getElementById('product-detail-container');
    if (detailContainer) {
        await loadProductDetail();
    }

    // Khởi chạy đồng hồ đếm ngược 4 tiếng ảo
    startGlobalCountdown();
}

// --- PARSE JSON DESCRIPTION UTILS ---
function parseProductDetails(product) {
    let descText = product.description;
    let storyText = "Bức họa mang biểu tượng tâm linh và triết lý thiền định sâu sắc. Từng đường nét đều được chau chuốt kỹ lưỡng để mang lại sự cân bằng, tài lộc và an yên cho ngôi nhà của bạn.";
    let category = "Tranh Trí Tuệ";
    let contextImages = {
        living_room: "/uploads/canvas.jpg",
        kitchen: "/images/harmony.jpg",
        lobby: "/uploads/mandala.jpg"
    };

    try {
        const parsed = JSON.parse(product.description);
        descText = parsed.description || product.description;
        storyText = parsed.story || storyText;
        category = parsed.category || category;
        if (parsed.context_images) {
            contextImages = { ...contextImages, ...parsed.context_images };
        }
    } catch (e) {
        // Fallback cho mô tả dạng text cũ
    }

    return { descText, storyText, category, contextImages };
}

// --- MOBILE DRAWER TOGGLES ---
function setupMobileDrawers() {
    const menuTrigger = document.getElementById('mobile-menu-trigger');
    const cartTrigger = document.getElementById('mobile-cart-trigger');
    const sidebarLeft = document.getElementById('sidebar-left');
    const sidebarRight = document.getElementById('sidebar-right');
    
    if (menuTrigger && sidebarLeft) {
        menuTrigger.addEventListener('click', (e) => {
            e.stopPropagation();
            sidebarLeft.classList.toggle('active');
            sidebarRight.classList.remove('active');
        });
    }
    
    if (cartTrigger && sidebarRight) {
        cartTrigger.addEventListener('click', (e) => {
            e.stopPropagation();
            sidebarRight.classList.toggle('active');
            sidebarLeft.classList.remove('active');
        });
    }
    
    // Close drawers when clicking outside
    document.addEventListener('click', (e) => {
        if (sidebarLeft && !sidebarLeft.contains(e.target) && e.target !== menuTrigger) {
            sidebarLeft.classList.remove('active');
        }
        if (sidebarRight && !sidebarRight.contains(e.target) && e.target !== cartTrigger) {
            sidebarRight.classList.remove('active');
        }
    });
}

// --- SEARCH & CATEGORY FILTERS ---
function setupSearchAndCategoryFilters() {
    const searchInput = document.getElementById('search-input');
    if (searchInput) {
        searchInput.addEventListener('input', (e) => {
            state.searchQuery = e.target.value.toLowerCase().trim();
            filterAndRenderProducts();
        });
    }

    const menuItems = document.querySelectorAll('#category-menu .menu-item');
    menuItems.forEach(item => {
        item.addEventListener('click', () => {
            menuItems.forEach(btn => btn.classList.remove('active'));
            item.classList.add('active');
            state.currentCategory = item.getAttribute('data-category');
            filterAndRenderProducts();
            
            // Close mobile menu sidebar if open
            const sidebarLeft = document.getElementById('sidebar-left');
            if (sidebarLeft) sidebarLeft.classList.remove('active');
        });
    });
}

function filterAndRenderProducts() {
    const container = document.getElementById('products-container');
    if (!container) return;

    const filtered = state.products.filter(product => {
        const details = parseProductDetails(product);
        const matchCategory = state.currentCategory === 'Tất cả' || details.category === state.currentCategory;
        const matchSearch = product.name.toLowerCase().includes(state.searchQuery) || 
                            details.descText.toLowerCase().includes(state.searchQuery) ||
                            product.id.toLowerCase().includes(state.searchQuery);
        return matchCategory && matchSearch;
    });

    if (filtered.length === 0) {
        container.innerHTML = `<div style="grid-column: 1/-1; text-align: center; color: var(--text-secondary); padding: 40px 0;">Không tìm thấy bức tranh nào phù hợp.</div>`;
    } else {
        container.innerHTML = filtered.map(product => renderProductCard(product)).join('');
    }
}

// --- GIO HANG SYSTEM ---
function updateCartBadge() {
    const badge = document.getElementById('cart-badge');
    const mobileBadge = document.getElementById('mobile-cart-badge');
    const totalItems = state.cart.reduce((sum, item) => sum + item.quantity, 0);
    
    if (badge) badge.innerText = totalItems;
    if (mobileBadge) mobileBadge.innerText = totalItems;
    
    renderCartSidebar();
}

function renderCartSidebar() {
    const container = document.getElementById('cart-items-container');
    if (!container) return;
    
    if (state.cart.length === 0) {
        container.innerHTML = `
            <div style="text-align: center; padding: 60px 20px; color: var(--text-secondary);">
                <div style="font-size: 40px; margin-bottom: 12px;">🛒</div>
                <p style="font-size: 13px;">Giỏ hàng đang trống.<br>Hãy chọn cho mình một tác phẩm bình an nhé!</p>
            </div>
        `;
        document.getElementById('cart-subtotal').innerText = '0đ';
        document.getElementById('cart-total').innerText = '0đ';
        document.getElementById('discount-row').style.display = 'none';
        return;
    }
    
    container.innerHTML = state.cart.map((item, idx) => `
        <div class="cart-item-card">
            <img src="${item.image || 'https://images.unsplash.com/photo-1579783902614-a3fb3927b675?w=200&auto=format&fit=crop'}" alt="${item.name}">
            <button class="cart-item-remove" onclick="removeCartItem(${idx})">×</button>
            <div class="cart-item-details">
                <div>
                    <div class="cart-item-name">${item.name}</div>
                    <div class="cart-item-meta">Khung ${item.size} cm | Mã: ${item.product_id}</div>
                </div>
                <div class="cart-item-bottom">
                    <span class="cart-item-price">${formatVND(item.price * item.quantity)}</span>
                    <div class="cart-item-qty">
                        <button onclick="changeCartQty(${idx}, -1)">-</button>
                        <span>${item.quantity}</span>
                        <button onclick="changeCartQty(${idx}, 1)">+</button>
                    </div>
                </div>
            </div>
        </div>
    `).join('');
    
    // Calculate total prices
    const subtotal = state.cart.reduce((sum, item) => sum + (item.price * item.quantity), 0);
    let discount = 0.0;
    
    if (state.currentCoupon) {
        if (state.currentCoupon.discount_type === 'percentage') {
            discount = subtotal * (state.currentCoupon.discount_value / 100);
        } else if (state.currentCoupon.discount_type === 'fixed') {
            discount = state.currentCoupon.discount_value;
        }
        document.getElementById('discount-row').style.display = 'flex';
        document.getElementById('cart-discount').innerText = `-${formatVND(discount)}`;
    } else {
        document.getElementById('discount-row').style.display = 'none';
    }
    
    const total = Math.max(0, subtotal - discount);
    document.getElementById('cart-subtotal').innerText = formatVND(subtotal);
    document.getElementById('cart-total').innerText = formatVND(total);
}

function changeCartQty(index, delta) {
    state.cart[index].quantity += delta;
    if (state.cart[index].quantity <= 0) {
        state.cart.splice(index, 1);
    }
    localStorage.setItem('harmony_cart', JSON.stringify(state.cart));
    updateCartBadge();
}

function removeCartItem(index) {
    state.cart.splice(index, 1);
    localStorage.setItem('harmony_cart', JSON.stringify(state.cart));
    updateCartBadge();
}

// --- AP APPLY COUPON ---
function setupCouponLogic() {
    const couponInput = document.getElementById('coupon-input');
    const applyBtn = document.getElementById('apply-coupon-btn');
    const msgEl = document.getElementById('coupon-msg');
    
    if (state.currentCoupon && couponInput) {
        couponInput.value = state.currentCoupon.code;
    }
    
    if (applyBtn) {
        applyBtn.addEventListener('click', async () => {
            const code = couponInput.value.trim().toUpperCase();
            if (!code) {
                state.currentCoupon = null;
                localStorage.removeItem('harmony_coupon');
                msgEl.innerText = '';
                updateCartBadge();
                return;
            }
            
            try {
                const response = await fetch(`/api/coupons/validate/${code}`);
                if (!response.ok) {
                    const err = await response.json();
                    throw new Error(err.detail || 'Mã giảm giá không hợp lệ');
                }
                
                state.currentCoupon = await response.json();
                localStorage.setItem('harmony_coupon', JSON.stringify(state.currentCoupon));
                msgEl.innerText = '✓ Áp dụng mã giảm giá thành công!';
                msgEl.style.color = 'var(--success)';
                updateCartBadge();
            } catch (error) {
                msgEl.innerText = error.message;
                msgEl.style.color = 'var(--danger)';
                state.currentCoupon = null;
                localStorage.removeItem('harmony_coupon');
                updateCartBadge();
            }
        });
    }

    const checkoutBtn = document.getElementById('checkout-submit-btn');
    if (checkoutBtn) {
        checkoutBtn.addEventListener('click', () => {
            if (state.cart.length === 0) return;
            window.location.href = 'checkout.html';
        });
    }
}

// --- LOGIC LAY DANH SACH SAN PHAM ---
async function loadProductsList() {
    const container = document.getElementById('products-container');
    const placeholder = document.getElementById('loading-placeholder');
    
    try {
        const response = await fetch('/api/products');
        if (!response.ok) throw new Error('Không thể kết nối thư viện sản phẩm');
        
        state.products = await response.json();
        
        placeholder.style.display = 'none';
        container.style.display = 'grid';
        filterAndRenderProducts();
        
    } catch (error) {
        console.error('Loi load products:', error);
        placeholder.innerText = 'Có lỗi xảy ra khi tải danh sách tranh. Vui lòng thử lại sau.';
    }
}

// --- RENDER CARD SAN PHAM VOI LOGIC TAM LY ---
function renderProductCard(product) {
    const originalPrice = product.original_price || (product.price * 1.4);
    const discountPercent = Math.round(((originalPrice - product.price) / originalPrice) * 100);
    const details = parseProductDetails(product);
    
    return `
        <div class="product-card" onclick="openQuickView('${product.id}')" style="cursor: pointer;">
            ${discountPercent > 20 ? `<div class="bestseller-ribbon">SALE ${discountPercent}%</div>` : ''}
            
            <div class="product-image-wrapper">
                <img src="${product.images[0] || 'https://images.unsplash.com/photo-1579783902614-a3fb3927b675?w=800&auto=format&fit=crop'}" alt="${product.name}" class="product-image" loading="lazy">
                <div class="card-countdown">⚡ Deal kết thúc sau: <span class="countdown-timer">--:--:--</span></div>
            </div>
            
            <div class="product-info" style="text-align: left; padding: 16px;">
                <h3 class="product-name" style="font-size: 14px; margin-bottom: 4px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;">${product.name}</h3>
                <div style="font-size: 11px; color: var(--text-secondary); margin-bottom: 8px; font-weight: 500;">Mã SP: ${product.id}</div>
                
                <div class="product-price-wrapper" style="justify-content: flex-start; gap: 8px;">
                    <span class="price-current" style="font-size: 15px;">${formatVND(product.price)}</span>
                    <span class="price-original" style="font-size: 11px;">${formatVND(originalPrice)}</span>
                </div>
            </div>
        </div>
    `;
}

// --- QUICK VIEW MODAL LOGIC (CHI TIẾT & CÂU CHUYỆN) ---
let qvCurrentProduct = null;
let qvSelectedSizeIndex = 0;

function openQuickView(productId) {
    const product = state.products.find(p => p.id === productId);
    if (!product) return;
    
    qvCurrentProduct = product;
    qvSelectedSizeIndex = 0;
    
    const modal = document.getElementById('quick-view-modal');
    if (!modal) return;
    
    // Fill basic details
    document.getElementById('qv-image').src = product.images[0] || 'https://images.unsplash.com/photo-1579783902614-a3fb3927b675?w=800&auto=format&fit=crop';
    document.getElementById('qv-name').innerText = product.name;
    document.getElementById('qv-sku').innerText = `Mã SP: ${product.id}`;
    
    // Parse JSON details (story and context room images)
    const details = parseProductDetails(product);
    document.getElementById('qv-description').innerText = details.descText;
    document.getElementById('qv-story').innerText = details.storyText;
    
    // Original vs new price calculation
    const originalPrice = product.original_price || (product.price * 1.4);
    const discountPercent = Math.round(((originalPrice - product.price) / originalPrice) * 100);
    const badge = document.getElementById('qv-sale-badge');
    if (discountPercent > 0) {
        badge.innerText = `GIẢM ${discountPercent}%`;
        badge.style.display = 'inline-block';
    } else {
        badge.style.display = 'none';
    }
    
    // Render sizes option cards
    renderQvSizes();
    
    // Reset tabs
    document.querySelectorAll('.qv-tab-trigger').forEach(tab => tab.classList.remove('active'));
    document.querySelectorAll('.qv-tab-content').forEach(content => content.classList.remove('active'));
    document.querySelector('.qv-tab-trigger[data-tab="tab-desc"]').classList.add('active');
    document.getElementById('tab-desc').classList.add('active');

    // Reset room selector
    document.querySelectorAll('.room-btn').forEach(btn => btn.classList.remove('active'));
    document.querySelector('.room-btn[data-room="default"]').classList.add('active');
    
    // Setup Context Room Switcher listeners
    const roomButtons = document.querySelectorAll('.room-btn');
    roomButtons.forEach(btn => {
        // Clone to remove old event listeners
        const newBtn = btn.cloneNode(true);
        btn.parentNode.replaceChild(newBtn, btn);
        
        newBtn.addEventListener('click', () => {
            document.querySelectorAll('.room-btn').forEach(b => b.classList.remove('active'));
            newBtn.classList.add('active');
            
            const roomType = newBtn.getAttribute('data-room');
            if (roomType === 'default') {
                document.getElementById('qv-image').src = product.images[0];
            } else if (details.context_images && details.context_images[roomType]) {
                document.getElementById('qv-image').src = details.context_images[roomType];
            }
        });
    });

    // Setup tab events
    const tabTriggers = document.querySelectorAll('.qv-tab-trigger');
    tabTriggers.forEach(trigger => {
        const newTrigger = trigger.cloneNode(true);
        trigger.parentNode.replaceChild(newTrigger, trigger);
        
        newTrigger.addEventListener('click', () => {
            document.querySelectorAll('.qv-tab-trigger').forEach(t => t.classList.remove('active'));
            document.querySelectorAll('.qv-tab-content').forEach(c => c.classList.remove('active'));
            
            newTrigger.classList.add('active');
            const targetTab = newTrigger.getAttribute('data-tab');
            document.getElementById(targetTab).classList.add('active');
        });
    });
    
    document.getElementById('quick-view-loading').style.display = 'none';
    document.getElementById('quick-view-details').style.display = 'grid';
    modal.classList.add('active');
}

function renderQvSizes() {
    if (!qvCurrentProduct) return;
    const sizesContainer = document.getElementById('qv-size-options');
    sizesContainer.innerHTML = qvCurrentProduct.sizes.map((sz, idx) => {
        const priceDiffText = sz.extra_price > 0 ? `(+${formatVND(sz.extra_price)})` : 'Giá gốc';
        return `
            <div class="size-option-card ${idx === qvSelectedSizeIndex ? 'selected' : ''}" onclick="selectQvSize(${idx}); event.stopPropagation();">
                <span class="size-name">Khung ${sz.size} cm</span>
                <span class="size-price-diff">${priceDiffText}</span>
            </div>
        `;
    }).join('');
    updateQvPrice();
}

function selectQvSize(idx) {
    qvSelectedSizeIndex = idx;
    renderQvSizes();
}

function updateQvPrice() {
    if (!qvCurrentProduct) return;
    const sizeOption = qvCurrentProduct.sizes[qvSelectedSizeIndex];
    const currentPrice = qvCurrentProduct.price + sizeOption.extra_price;
    const originalPrice = (qvCurrentProduct.original_price || (qvCurrentProduct.price * 1.4)) + sizeOption.extra_price;

    document.getElementById('qv-price').innerText = formatVND(currentPrice);
    document.getElementById('qv-original-price').innerText = formatVND(originalPrice);
}

// Add to cart from Quick View
function qvAddToCart(isBuyNow = false) {
    if (!qvCurrentProduct) return;
    const sizeOption = qvCurrentProduct.sizes[qvSelectedSizeIndex];
    const price = qvCurrentProduct.price + sizeOption.extra_price;
    
    const cartItem = {
        product_id: qvCurrentProduct.id,
        name: qvCurrentProduct.name,
        size: sizeOption.size,
        price: price,
        quantity: 1, // Quick view adds 1 by default
        image: qvCurrentProduct.images[0]
    };

    const existingIdx = state.cart.findIndex(item => item.product_id === cartItem.product_id && item.size === cartItem.size);
    if (existingIdx > -1) {
        state.cart[existingIdx].quantity += 1;
    } else {
        state.cart.push(cartItem);
    }

    localStorage.setItem('harmony_cart', JSON.stringify(state.cart));
    updateCartBadge();
    
    if (isBuyNow) {
        window.location.href = 'checkout.html';
    } else {
        // Feedback
        const btn = document.getElementById('qv-add-to-cart');
        const origText = btn.innerText;
        btn.innerText = '✓ ĐÃ THÊM THÀNH CÔNG';
        btn.style.backgroundColor = 'var(--success)';
        btn.style.color = 'white';
        setTimeout(() => {
            btn.innerText = origText;
            btn.style.backgroundColor = '#fff';
            btn.style.color = 'var(--text-primary)';
        }, 1500);
    }
}

// Event Listeners for Quick View Modal Close
document.addEventListener('DOMContentLoaded', () => {
    const modal = document.getElementById('quick-view-modal');
    if (modal) {
        document.getElementById('quick-view-close').addEventListener('click', () => modal.classList.remove('active'));
        modal.addEventListener('click', (e) => {
            if (e.target === modal) modal.classList.remove('active');
        });
        
        document.getElementById('qv-add-to-cart').addEventListener('click', (e) => {
            e.stopPropagation();
            qvAddToCart(false);
        });
        document.getElementById('qv-buy-now').addEventListener('click', (e) => {
            e.stopPropagation();
            qvAddToCart(true);
        });
    }
});

// --- DONG HO DEM NGUOC 4 TIENG AO (DONG BO THEO GIO HE THONG) ---
function startGlobalCountdown() {
    function updateCountdown() {
        const now = new Date();
        const currentHour = now.getHours();
        
        let nextMilestoneHour = Math.ceil((currentHour + 0.1) / 4) * 4;
        
        const target = new Date(now);
        target.setHours(nextMilestoneHour, 0, 0, 0);
        
        const diff = target - now;
        
        const hours = Math.floor(diff / (1000 * 60 * 60));
        const minutes = Math.floor((diff % (1000 * 60 * 60)) / (1000 * 60));
        const seconds = Math.floor((diff % (1000 * 60)) / 1000);
        
        const timeStr = [
            hours.toString().padStart(2, '0'),
            minutes.toString().padStart(2, '0'),
            seconds.toString().padStart(2, '0')
        ].join(':');
        
        document.querySelectorAll('.countdown-timer').forEach(el => {
            el.innerText = timeStr;
        });
    }
    
    updateCountdown();
    setInterval(updateCountdown, 1000);
}

// --- THUAT TOAN RANDOM CO DINH THEO NGAY & MA SAN PHAM (SOCIAL PROOF) ---
function getSocialProofData(productId) {
    const today = new Date();
    const dateSeed = today.getFullYear() * 10000 + (today.getMonth() + 1) * 100 + today.getDate();
    
    let productHash = 0;
    for (let i = 0; i < productId.length; i++) {
        productHash = (productHash << 5) - productHash + productId.charCodeAt(i);
        productHash |= 0; 
    }
    
    const seed = Math.abs(productHash + dateSeed);
    
    const viewers = 12 + (seed % 16);
    const soldCount = 45 + (seed % 66);
    const stockLeft = 2 + (seed % 6);
    
    return { viewers, soldCount, stockLeft };
}

// --- UTILS FORMAT TIEN VIET NAM ---
function formatVND(amount) {
    return new Intl.NumberFormat('vi-VN', { style: 'currency', currency: 'VND' }).format(amount);
}
