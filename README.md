# 📚 HỆ THỐNG QUẢN LÝ THƯ VIỆN
## Phân Tích Chi Tiết Cấu Trúc Dữ Liệu & Giải Thuật

---

## 📑 MỤC LỤC

1. [Tổng Quan Dự Án](#1-tổng-quan-dự-án)
2. [Kiến Trúc Hệ Thống](#2-kiến-trúc-hệ-thống)
3. [Cấu Trúc Dữ Liệu](#3-cấu-trúc-dữ-liệu)
4. [Thuật Toán & Phân Tích Độ Phức Tạp](#4-thuật-toán--phân-tích-độ-phức-tạp)
5. [Chi Tiết Implementation](#5-chi-tiết-implementation)
6. [Quản Lý File & Persistence](#6-quản-lý-file--persistence)
7. [Tối Ưu Hóa & Trade-offs](#7-tối-ưu-hóa--trade-offs)
8. [Hướng Phát Triển](#8-hướng-phát-triển)

---

## 1. TỔNG QUAN DỰ ÁN

### 1.1. Mô Tả
Hệ thống quản lý thư viện số với các chức năng:
- ✅ Quản lý sách (thêm, xóa, tìm kiếm, cập nhật số lượng)
- ✅ Quản lý độc giả (thêm, xóa, theo dõi lịch sử mượn)
- ✅ Xử lý mượn/trả sách với kiểm soát tồn kho
- ✅ Tìm kiếm theo tên sách, tác giả
- ✅ Lưu trữ dữ liệu persistent qua file

### 1.2. Cấu Trúc Files
```
LibraryManagement/
├── Book.h / Book.cpp          # Class Sách
├── Reader.h / Reader.cpp      # Class Độc giả
├── Library.h / Library.cpp    # Class Thư viện (core)
├── main.cpp                   # UI Console
├── books.txt                  # Database sách
├── readers.txt                # Database độc giả
├── books_them.txt             # Log thêm sách
├── books_xoa.txt              # Log xóa sách
├── readers_them.txt           # Log thêm độc giả
└── readers_xoa.txt            # Log xóa độc giả
```

---

## 2. KIẾN TRÚC HỆ THỐNG

### 2.1. Class Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    Library (Core)                       │
├─────────────────────────────────────────────────────────┤
│ - unordered_map<string, Book> books                     │
│ - vector<Reader> readers                                │
├─────────────────────────────────────────────────────────┤
│ + addBook(Book) : bool                                  │
│ + removeBook(string) : bool                             │
│ + borrowBook(string, string) : bool                     │
│ + returnBook(string, string) : bool                     │
│ + searchByTitle(string) : vector<Book>                  │
│ + searchByAuthor(string) : vector<Book>                 │
│ + loadBooks/saveBooks(string) : bool                    │
└─────────────────────────────────────────────────────────┘
           ▲                           ▲
           │ contains                  │ contains
           │                           │
┌──────────┴──────────┐    ┌──────────┴──────────────┐
│       Book          │    │       Reader            │
├─────────────────────┤    ├─────────────────────────┤
│ - id: string        │    │ - id: string            │
│ - title: string     │    │ - name: string          │
│ - author: string    │    │ - borrowedBookIds:      │
│ - quantity: int     │    │   vector<string>        │
│ - isAvailable: bool │    └─────────────────────────┘
└─────────────────────┘
```

### 2.2. Luồng Hoạt Động

```
main.cpp (UI)
     │
     ├──> Khởi tạo Library
     │
     ├──> Load dữ liệu: loadBooks(), loadReaders()
     │         │
     │         └──> Parse từ file → Validate → Insert vào structures
     │
     ├──> Xử lý thao tác người dùng
     │    │
     │    ├──> Add/Remove/Search → Update in-memory structures
     │    │
     │    ├──> Borrow/Return → Update Book quantity & Reader borrowed list
     │    │
     │    └──> Log operations → Append to log files
     │
     └──> Save dữ liệu: saveBooks(), saveReaders()
               │
               └──> Sort by ID → Write to file
```

---

## 3. CẤU TRÚC DỮ LIỆU

### 3.1. unordered_map<string, Book> books

#### **Đặc Điểm Kỹ Thuật**

**Cấu trúc:**
```cpp
// Hash Table với separate chaining
unordered_map<string, Book> books;

// Internal structure (conceptual):
┌───────────────────────────────────┐
│  Buckets (Hash Table)             │
├───────────────────────────────────┤
│ [0] → "B001" → Book{...}          │
│ [1] → nullptr                     │
│ [2] → "B042" → Book{...}          │
│ [3] → "B015" → Book{...}          │
│      ...                          │
└───────────────────────────────────┘
```

**Lý do lựa chọn:**
1. **Access Time O(1)**: Tìm kiếm nhanh theo mã sách
2. **Insert/Delete O(1)**: Thêm/xóa hiệu quả
3. **Key-Based Access**: Mã sách là unique identifier tự nhiên

**Hash Function:**
```cpp
// Default std::hash<string> sử dụng thuật toán:
// - Combine character codes với prime numbers
// - FNV-1a hoặc MurmurHash (tùy implementation)
// 
// Ví dụ simplified:
size_t hash = 0;
for (char c : bookId) {
    hash = hash * 31 + c;
}
return hash % bucket_count;
```

**Collision Resolution:**
- **Method**: Separate Chaining (linked lists tại mỗi bucket)
- **Load Factor**: α ≈ 1.0 (rehash khi α > max_load_factor)
- **Performance**: O(1 + α) = O(1) average case

#### **Operations Analysis**

```cpp
// 1. Tìm kiếm - O(1) average
Book* findBookById(const string& id) {
    auto it = books.find(id);        // Hash lookup: O(1)
    return (it != books.end()) ? &(it->second) : nullptr;
}

// 2. Thêm sách - O(1) average
bool addBook(const Book& book) {
    if (bookExists(book.getId())) return false;  // O(1) check
    books[book.getId()] = book;                  // O(1) insert
    // Log to file: O(k) where k = string length
    return true;
}

// 3. Xóa sách - O(1) average + O(n*m) validation
bool removeBook(const string& bookId) {
    auto it = books.find(bookId);    // O(1)
    if (it == books.end()) return false;
    
    // Kiểm tra ràng buộc: sách không đang được mượn
    // O(n*m): n readers, m borrowed books per reader
    for (const auto& r : readers) {
        for (const auto& bid : r.getBorrowed()) {
            if (bid == bookId) return false;
        }
    }
    
    books.erase(it);                 // O(1)
    return true;
}
```

---

### 3.2. vector<Reader> readers

#### **Đặc Điểm Kỹ Thuật**

**Cấu trúc:**
```cpp
vector<Reader> readers;

// Internal structure:
┌────────────────────────────────────┐
│  Dynamic Array                     │
├────────────────────────────────────┤
│ [0] Reader{id="R001", ...}         │
│ [1] Reader{id="R002", ...}         │
│ [2] Reader{id="R003", ...}         │
│ ...                                │
│ [capacity-1] (unused)              │
└────────────────────────────────────┘
```

**Lý do lựa chọn:**
1. **Sequential Access**: Thường cần duyệt toàn bộ danh sách
2. **Cache Locality**: Dữ liệu liên tục trong bộ nhớ
3. **Low Overhead**: Không tốn memory cho hash table
4. **Infrequent Search**: Tìm kiếm độc giả ít khi thực hiện

**Trade-off Analysis:**
- **Ưu điểm**: Iteration O(n) nhanh, memory efficient
- **Nhược điểm**: Search O(n) chậm hơn hash table
- **Kết luận**: Phù hợp vì số lượng readers thường nhỏ (<1000)

#### **Operations Analysis**

```cpp
// 1. Tìm kiếm - O(n) linear
Reader* findReaderById(const string& id) {
    for (auto &r : readers) {        // Linear scan: O(n)
        if (r.getId() == id) return &r;
    }
    return nullptr;
}

// 2. Thêm độc giả - O(n) check + O(1) insert
bool addReader(const Reader& r) {
    if (readerExists(r.getId())) return false;  // O(n)
    readers.push_back(r);                       // O(1) amortized
    return true;
}

// 3. Xóa độc giả - O(n) search + O(n) erase
bool removeReader(const string& readerId) {
    for (auto it = readers.begin(); it != readers.end(); ++it) {
        if (it->getId() == readerId) {
            if (!it->getBorrowed().empty()) return false;  // O(1)
            readers.erase(it);    // O(n) - shift elements
            return true;
        }
    }
    return false;
}
```

**Vector Dynamic Resizing:**
```cpp
// Capacity growth strategy:
// - Initial capacity: 0
// - Growth factor: 2x (MSVC) hoặc 1.5x (GCC/Clang)
// - Amortized O(1) insertion:
//   * n inserts → log(n) resizes
//   * Total copies: n + n/2 + n/4 + ... = 2n
//   * Average per insert: 2n/n = O(1)
```

---

### 3.3. vector<string> borrowedBookIds (trong Reader)

#### **Đặc Điểm**

```cpp
class Reader {
    vector<string> borrowedBookIds;  // Danh sách mã sách đã mượn
};
```

**Lý do lựa chọn:**
- ✅ Số sách mượn nhỏ (thường <10)
- ✅ Thao tác chính: append (mượn) và linear search (trả)
- ✅ Không cần truy cập ngẫu nhiên nhanh

**Operations:**
```cpp
// Mượn sách - O(1)
void borrowBook(const string& bookId) {
    borrowedBookIds.push_back(bookId);  // Append: O(1) amortized
}

// Trả sách - O(m) where m = số sách đã mượn
bool returnBook(const string& bookId) {
    for (auto it = borrowedBookIds.begin(); it != borrowedBookIds.end(); ++it) {
        if (*it == bookId) {
            borrowedBookIds.erase(it);   // O(m) shift
            return true;
        }
    }
    return false;
}
```

---

## 4. THUẬT TOÁN & PHÂN TÍCH ĐỘ PHỨC TẠP

### 4.1. Tìm Kiếm (Search Algorithms)

#### **4.1.1. Tìm theo ID (Hash-based)**

```cpp
Book* findBookById(const string& id) {
    auto it = books.find(id);
    return (it != books.end()) ? &(it->second) : nullptr;
}
```

**Phân tích:**
- **Time Complexity**: O(1) average, O(n) worst case
- **Space Complexity**: O(1)
- **Best Case**: Direct hit trong hash bucket
- **Worst Case**: Tất cả keys hash vào cùng bucket (collision chain)
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
