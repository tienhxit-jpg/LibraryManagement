# Hệ Thống Quản Lý Thư Viện - Phân tích Cấu Trúc Dữ Liệu & Giải Thuật

## 📚 Mục lục

1. [Tổng quan kiến trúc](#tổng-quan-kiến-trúc)
2. [Cấu trúc dữ liệu chính](#cấu-trúc-dữ-liệu-chính)
3. [Phân tích từng thao tác](#phân-tích-từng-thao-tác)
4. [Độ phức tạp thời gian & không gian](#độ-phức-tạp-thời-gian--không-gian)
5. [Chiến lược tối ưu](#chiến-lược-tối-ưu)
6. [Bảo vệ dữ liệu](#bảo-vệ-dữ-liệu)

---

## 🏗️ Tổng quan kiến trúc

```
┌────────────────────────────────────────────────┐
│           Library (Quản lý tổng hợp)           │
├────────────────────────────────────────────────┤
│                                                │
│  ┌──────────────────────────────────────────┐ │
│  │  unordered_map<string, Book> books       │ │
│  │  (Bảng hash - tìm kiếm O(1))             │ │
│  └──────────────────────────────────────────┘ │
│                                                │
│  ┌──────────────────────────────────────────┐ │
│  │  vector<Reader> readers                  │ │
│  │  (Mảng động - duyệt O(n))                │ │
│  └──────────────────────────────────────────┘ │
│                                                │
└────────────────────────────────────────────────┘
```

---

## 📋 Cấu trúc dữ liệu chính

### 1. **unordered_map<string, Book> books**

**Lý do chọn:**
- ✅ Tìm kiếm theo ID: **O(1)** (trung bình)
- ✅ Thêm/Xóa sách: **O(1)** (trung bình)
- ✅ Phù hợp với truy vấn thường xuyên: `findBookById()`, `bookExists()`

**Hoạt động:**
```cpp
// Tìm sách theo ID
Book* findBookById(const string& id) {
    auto it = books.find(id);           // O(1)
    if (it != books.end()) {
        return &(it->second);
    }
    return nullptr;
}

// Thêm sách mới
bool addBook(const Book& book) {
    if (bookExists(book.getId())) return false;  // O(1)
    books[book.getId()] = book;                  // O(1)
    return true;
}
```

**Hash Function:**
- Sử dụng default hash của `std::string`
- Phân phối tốt cho mã sách như "B001", "B002", ...

**Collision Handling:**
- Sử dụng chaining (mặc định của unordered_map)

---

### 2. **vector<Reader> readers**

**Lý do chọn:**
- ✅ Quản lý độc giả theo danh sách tuần tự
- ✅ Lặp duyệt hiệu quả: **O(n)**
- ✅ Tìm kiếm thường không quá tần xuyên

**Hoạt động:**
```cpp
// Tìm độc giả theo ID
Reader* findReaderById(const string& id) {
    for (auto &r : readers) {           // O(n)
        if (r.getId() == id) return &r;
    }
    return nullptr;
}

// Thêm độc giả mới
bool addReader(const Reader& r) {
    if (readerExists(r.getId())) return false;  // O(n)
    readers.push_back(r);                        // O(1) amortized
    return true;
}
```

**Tại sao không dùng unordered_map:**
- Độc giả ít thay đổi
- Thường cần duyệt toàn bộ danh sách
- Vector có cache locality tốt hơn

---

### 3. **vector<string> (trong Reader)**

**Danh sách sách đã mượn:**
```cpp
vector<string> borrowedBookIds;  // IDs of borrowed books
```

**Lý do:**
- Mỗi độc giả mượn ít sách (thường < 10)
- Tìm kiếm trong danh sách nhỏ → O(n) chấp nhận được
- Thêm/xóa sách dễ dàng

---

## 🔍 Phân tích từng thao tác

### 1. **Load Books từ File**

```cpp
bool Library::loadBooks(const string& filename) {
    ifstream ifs(filename);
    string line;
    while (getline(ifs, line)) {  // O(m) - m dòng file
        // Parse: id|title|author|quantity|available
        stringstream ss(line);
        string id, title, author, qty, avail;
        
        getline(ss, id, '|');        // O(1)
        // ... other fields ...
        
        int quantity = stoi(qty);     // O(1)
        Book b(id, title, author, qty);
        
        books[b.getId()] = b;         // O(1) hash insert
    }
    return true;
}
```

**Độ phức tạp:** `O(m)` - m là số sách  
**Bộ nhớ:** `O(m)` - lưu m cuốn sách

---

### 2. **Tìm kiếm theo ID**

```cpp
Book* Library::findBookById(const string& id) {
    auto it = books.find(id);    // O(1) hash lookup
    if (it != books.end()) {
        return &(it->second);
    }
    return nullptr;
}
```

**Độ phức tạp:** 
- **Trung bình:** `O(1)` ✅
- **Tệ nhất:** `O(m)` (khi có hash collision)

---

### 3. **Tìm kiếm theo Tiêu đề**

```cpp
vector<Book> Library::searchByTitle(const string& keyword) const {
    string k = toLower(keyword);
    vector<Book> res;
    
    for (const auto& pair : books) {     // O(m) duyệt tất cả sách
        const Book& b = pair.second;
        if (toLower(b.getTitle()).find(k) != string::npos) {  // O(t)
            res.push_back(b);            // O(1) amortized
        }
    }
    return res;
}
```

**Độ phức tạp:** `O(m * t)` 
- m = số sách
- t = độ dài tiêu đề trung bình

**Tối ưu:** Có thể dùng **trie** hoặc **suffix tree** cho tìm kiếm nhanh hơn

---

### 4. **Mượn Sách**

```cpp
bool Library::borrowBook(const string& bookId, const string& readerId) {
    auto it = books.find(bookId);        // O(1) tìm sách
    if (it == books.end()) return false;
    if (it->second.getQuantity() <= 0) return false;
    
    Reader* r = findReaderById(readerId);  // O(n) tìm độc giả
    if (!r) return false;
    
    it->second.decreaseQuantity(1);      // O(1) cập nhật số lượng
    r->borrowBook(bookId);               // O(1) thêm vào vector
    return true;
}
```

**Độ phức tạp:** `O(n)` - do tìm độc giả O(n)

**Cải thiện:** Có thể dùng unordered_map cho readers → O(1)

---

### 5. **Liệt kê Sách (với sắp xếp)**

```cpp
void Library::listAll() const {
    vector<const Book*> items;
    items.reserve(books.size());
    
    for (const auto& kv : books)           // O(m) duyệt
        items.push_back(&kv.second);
    
    sort(items.begin(), items.end(),       // O(m log m) sắp xếp
         [](const Book* a, const Book* b){ 
             return a->getId() < b->getId(); 
         });
    
    for (const Book* bp : items) {         // O(m) in
        // ... display ...
    }
}
```

**Độ phức tạp:** `O(m log m)` - do sắp xếp

**Bộ nhớ thêm:** `O(m)` - vector pointers

---

### 6. **Xóa Sách (với kiểm tra ràng buộc)**

```cpp
bool Library::removeBook(const string& bookId) {
    auto it = books.find(bookId);          // O(1) tìm sách
    if (it == books.end()) return false;
    
    // Kiểm tra không ai đang mượn
    for (const auto& r : readers) {        // O(n) duyệt độc giả
        const auto& borrowed = r.getBorrowed();
        for (const auto& bid : borrowed) { // O(k) duyệt sách mượn/người
            if (bid == bookId) return false;
        }
    }
    
    books.erase(it);                       // O(1) xóa
    return true;
}
```

**Độ phức tạp:** `O(n * k)`
- n = số độc giả
- k = sách mượn trung bình/người

---

## 📊 Độ phức tạp thời gian & không gian

### Bảng Độ phức tạp Thời gian

| Thao tác | Độ phức tạp | Ghi chú |
|---------|------------|--------|
| **findBookById()** | O(1) | Hash lookup |
| **addBook()** | O(1) | Hash insert |
| **removeBook()** | O(n*k) | Kiểm tra ràng buộc |
| **borrowBook()** | O(n) | Tìm độc giả O(n) |
| **returnBook()** | O(n) | Tìm độc giả O(n) |
| **searchByTitle()** | O(m*t) | m=sách, t=độ dài |
| **searchByAuthor()** | O(m*a) | m=sách, a=độ dài tác giả |
| **listAll()** | O(m log m) | Do sắp xếp |
| **loadBooks()** | O(m) | m=sách trong file |
| **saveBooks()** | O(m log m) | m=sách (sắp xếp) |

### Bảng Độ phức tạp Không gian

| Cấu trúc | Không gian | Ghi chú |
|---------|-----------|--------|
| **books** | O(m) | m cuốn sách |
| **readers** | O(n) | n độc giả |
| **borrowedBookIds** | O(k) | k=sách mượn/người |
| **Tổng** | **O(m + n*k)** | Thường k < 10 |

---

## ⚡ Chiến lược tối ưu

### 1. **Sorting Strategy**

**Vấn đề:** unordered_map không giữ thứ tự

**Giải pháp:**
```cpp
// Tạo vector pointers, sắp xếp theo ID
vector<const Book*> items;
items.reserve(books.size());
for (const auto& kv : books) 
    items.push_back(&kv.second);
sort(items.begin(), items.end(), 
     [](const Book* a, const Book* b){ 
         return a->getId() < b->getId(); 
     });
```

**Lợi ích:**
- ✅ Hiển thị luôn theo thứ tự ID
- ✅ File lưu ổn định
- ✅ Cache locality tốt hơn

---

### 2. **String Formatting - Case Insensitive**

```cpp
static inline string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), 
              [](unsigned char c){ return tolower(c); });
    return s;
}

// Sử dụng khi tìm kiếm
vector<Book> Library::searchByTitle(const string& keyword) const {
    string k = toLower(keyword);
    for (const auto& pair : books) {
        if (toLower(pair.second.getTitle()).find(k) != string::npos) {
            // ... match ...
        }
    }
}
```

**Lợi ích:** Tìm kiếm không phân biệt chữ hoa/thường

---

### 3. **Error Handling trong Load**

```cpp
bool Library::loadBooks(const string& filename) {
    // ...
    try {
        int quantity = stoi(qty);
        if (quantity < 0) {
            cout << "Canh bao: So luong am, dung 0\n";
            quantity = 0;
        }
    } catch (const exception& e) {
        cout << "Loi parse, dung 0\n";
        quantity = 0;
    }
    // ...
}
```

---

### 4. **Whitespace Trimming**

```cpp
// Trim leading/trailing whitespace
size_t start = tok.find_first_not_of(" \t\r\n");
size_t end = tok.find_last_not_of(" \t\r\n");
if (start != string::npos) {
    tok = tok.substr(start, end - start + 1);
}
```

---

## 🔒 Bảo vệ dữ liệu

### 1. **Kiểm tra trùng lặp**

```cpp
// Không cho thêm sách trùng ID
bool addBook(const Book& book) { 
    if (bookExists(book.getId())) return false;  // O(1)
    // ...
}

// Không cho thêm độc giả trùng ID
bool addReader(const Reader& r) {
    if (readerExists(r.getId())) return false;   // O(n)
    // ...
}
```

---

### 2. **Ràng buộc Tham chiếu (Referential Integrity)**

```cpp
// Không cho xóa sách đang được mượn
bool removeBook(const string& bookId) {
    for (const auto& r : readers) {        // O(n*k)
        if (r.borrowedBookIds.find(bookId) != npos) {
            return false;  // Sách đang mượn
        }
    }
    // ...
}

// Không cho xóa độc giả đang mượn sách
bool removeReader(const string& readerId) {
    Reader* r = findReaderById(readerId);
    if (!r->getBorrowed().empty()) {
        return false;  // Độc giả còn nợ sách
    }
    // ...
}
```

---

### 3. **Logging & Audit Trail**

```cpp
// Ghi lại mọi thao tác thêm/xóa
void writeFile(const string& filename, const string& content) {
    ofstream ofs(filename, ios::app);
    if (ofs) {
        ofs << content << "\n";
    }
}

// Khi thêm sách
bool addBook(const Book& book) {
    books[book.getId()] = book;
    stringstream ss;
    ss << book.getId() << "|" << book.getTitle() << ...
    writeFile("books_them.txt", ss.str());  // Log
    return true;
}
```

**Log files:**
- `books_them.txt` - sách thêm mới
- `books_xoa.txt` - sách bị xóa
- `readers_them.txt` - độc giả thêm mới
- `readers_xoa.txt` - độc giả bị xóa

---

## 📈 Đánh giá Hiệu suất

### Trường hợp sử dụng thực tế

Giả sử:
- **m = 10,000** cuốn sách
- **n = 5,000** độc giả
- **k = 5** sách/người mượn trung bình

| Thao tác | Thời gian | Ghi chú |
|---------|----------|--------|
| Tìm sách theo ID | ~1μs | O(1) hash |
| Thêm sách | ~1μs | O(1) hash |
| Tìm độc giả | ~10μs | O(n) |
| Mượn sách | ~10μs | O(n) |
| Liệt kê sách | ~150ms | O(m log m) sắp xếp |
| Xóa sách | ~100ms | O(n*k) kiểm tra |

---

## 🎓 Bài học rút ra

1. **Hash Table (unordered_map):** Tối ưu cho tìm kiếm O(1)
2. **Vector:** Phù hợp cho danh sách tuần tự, duyệt nhanh
3. **Sắp xếp:** Cần O(m log m) nhưng giúp hiển thị ổn định
4. **Ràng buộc dữ liệu:** Phòng chống inconsistency, mất dữ liệu
5. **Logging:** Theo dõi tất cả thay đổi cho audit

---

**Ngày cập nhật:** 18/01/2026  
**Phiên bản:** 1.0
