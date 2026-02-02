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

#### **4.1.2. Tìm theo Title/Author (Linear Search)**

```cpp
vector<Book> searchByTitle(const string& keyword) const {
    string k = toLower(keyword);
    vector<Book> res;
    
    for (const auto& pair : books) {              // O(B) - duyệt B sách
        const Book& b = pair.second;
        if (toLower(b.getTitle()).find(k) != string::npos) {  // O(T)
            res.push_back(b);
        }
    }
    return res;
}
```

**Phân tích:**
- **Time Complexity**: O(B × T)
  - B = số sách
  - T = độ dài title trung bình
- **Space Complexity**: O(R) - R = số kết quả
- **Algorithm**: Substring matching với case-insensitive

**String Matching Detail:**
```cpp
// find() sử dụng Boyer-Moore hoặc naive matching
// Worst case: O(|text| × |pattern|)
// Average: O(|text| + |pattern|)
```

**Optimization Ideas:**
1. **Inverted Index**: Tạo map từ keyword → list of book IDs
   - Space: O(V × B) với V = vocabulary size
   - Query: O(1) + O(R)
2. **Trie/Suffix Tree**: Cho prefix/substring search nhanh
   - Query: O(|pattern| + R)

---

### 4.2. Thuật Toán Mượn/Trả Sách

#### **4.2.1. Borrow Book Flow**

```cpp
bool borrowBook(const string& bookId, const string& readerId) {
    // Step 1: Tìm sách - O(1) hash lookup
    auto it = books.find(bookId);
    if (it == books.end()) return false;
    
    // Step 2: Kiểm tra tồn kho - O(1)
    if (it->second.getQuantity() <= 0) return false;
    
    // Step 3: Tìm độc giả - O(N) linear search
    Reader* r = findReaderById(readerId);
    if (!r) return false;
    
    // Step 4: Cập nhật inventory - O(1)
    it->second.decreaseQuantity(1);
    if (it->second.getQuantity() == 0) {
        it->second.setAvailability(false);
    }
    
    // Step 5: Thêm vào borrowed list - O(1) amortized
    r->borrowBook(bookId);
    
    return true;
}
```

**Complexity Analysis:**
- **Time**: O(N) dominated by findReaderById
- **Space**: O(1) auxiliary
- **Critical Path**: Reader lookup

**Transaction Atomicity:**
- Không có rollback mechanism
- Nếu crash giữa decreaseQuantity và borrowBook → inconsistent state
- **Fix**: Implement transaction log hoặc savepoint

#### **4.2.2. Return Book Flow**

```cpp
bool returnBook(const string& bookId, const string& readerId) {
    auto it = books.find(bookId);          // O(1)
    if (it == books.end()) return false;
    
    Reader* r = findReaderById(readerId);  // O(N)
    if (!r) return false;
    
    // Xóa khỏi borrowed list - O(M) where M = books borrowed by reader
    if (r->returnBook(bookId)) {
        it->second.increaseQuantity(1);    // O(1)
        it->second.setAvailability(true);  // O(1)
        return true;
    }
    return false;
}

// Trong Reader class:
bool returnBook(const string& bookId) {
    for (auto it = borrowedBookIds.begin(); it != borrowedBookIds.end(); ++it) {
        if (*it == bookId) {
            borrowedBookIds.erase(it);     // O(M) - shift elements
            return true;
        }
    }
    return false;
}
```

**Complexity:**
- **Time**: O(N + M)
  - N = tìm reader
  - M = xóa khỏi borrowed list
- **Space**: O(1)

**Optimization:**
```cpp
// Alternative: Dùng unordered_set<string> cho borrowedBookIds
// returnBook sẽ trở thành O(1) thay vì O(M)
unordered_set<string> borrowedBookIds;

bool returnBook(const string& bookId) {
    return borrowedBookIds.erase(bookId) > 0;  // O(1) average
}
```

---

### 4.3. Thuật Toán Sorting & Display

#### **4.3.1. List All Books (Sorted)**

```cpp
void listAll() const {
    // Step 1: Collect all IDs - O(B)
    vector<string> ids;
    for (const auto& kv : books) 
        ids.push_back(kv.first);
    
    // Step 2: Sort IDs - O(B log B)
    sort(ids.begin(), ids.end());
    
    // Step 3: Display sorted - O(B)
    for (const auto& id : ids) {
        auto it = books.find(id);    // O(1) per lookup
        // Print book info...
    }
}
```

**Complexity Analysis:**
- **Time**: O(B log B) - dominated by sorting
- **Space**: O(B) - temporary vector of IDs

**Sorting Algorithm (std::sort):**
- **Implementation**: Introsort (hybrid)
  - QuickSort: O(n log n) average
  - HeapSort: O(n log n) worst case fallback
  - InsertionSort: O(n²) for small subarrays (<16 elements)
- **Stable**: NO (use std::stable_sort if needed)

**Alternative Approaches:**
1. **Maintain sorted order**: Use `map<string, Book>` thay vì `unordered_map`
   - Pro: Always sorted, listAll becomes O(B)
   - Con: Insert/Find O(log B) thay vì O(1)
2. **Cache sorted list**: Invalidate khi có thay đổi
   - Pro: Amortized O(1) if read >> write
   - Con: Memory overhead + invalidation logic

---

### 4.4. Thuật Toán Delete Operations

#### **4.4.1. Remove Book with Constraint Checking**

```cpp
bool removeBook(const string& bookId) {
    // Step 1: Tìm sách - O(1)
    auto it = books.find(bookId);
    if (it == books.end()) return false;
    
    // Step 2: Kiểm tra ràng buộc - O(N × M)
    for (const auto& r : readers) {               // O(N) readers
        const auto& borrowed = r.getBorrowed();
        for (const auto& bid : borrowed) {        // O(M) borrowed per reader
            if (bid == bookId) return false;      // Sách đang được mượn
        }
    }
    
    // Step 3: Log deletion - O(k) string concatenation
    stringstream ss;
    ss << it->second.getId() << "|" << ...;
    writeFile("books_xoa.txt", ss.str());
    
    // Step 4: Delete - O(1)
    books.erase(it);
    return true;
}
```

**Complexity:**
- **Time**: O(N × M) worst case
  - N = số readers
  - M = trung bình sách mượn/reader
- **Space**: O(k) where k = log entry size

**Bottleneck**: Nested loop để check constraint

**Optimization Strategy:**
```cpp
// Maintain reverse index: bookId → set of readerIds who borrowed it
unordered_map<string, unordered_set<string>> bookBorrowers;

// Check constraint becomes O(1):
bool canRemoveBook(const string& bookId) {
    auto it = bookBorrowers.find(bookId);
    return (it == bookBorrowers.end() || it->second.empty());
}

// Trade-off: Extra O(B) space + maintain consistency on borrow/return
```

#### **4.4.2. Remove Reader**

```cpp
bool removeReader(const string& readerId) {
    for (auto it = readers.begin(); it != readers.end(); ++it) {
        if (it->getId() == readerId) {
            // Check constraint - O(1)
            if (!it->getBorrowed().empty()) {
                return false;  // Đang mượn sách
            }
            
            // Log & erase - O(N) vector erase
            writeFile("readers_xoa.txt", ...);
            readers.erase(it);  // Shift remaining elements
            return true;
        }
    }
    return false;
}
```

**Complexity:**
- **Time**: O(N) - linear search + vector erase
- **Space**: O(1)

**Vector Erase Mechanism:**
```cpp
// erase(iterator) shifts all subsequent elements left
// Example: [A, B, C, D] → erase B → [A, C, D]
// Copies: n - index - 1 elements

// Optimization: Swap with last element if order không quan trọng
readers[index] = readers.back();
readers.pop_back();  // O(1) instead of O(n)
```

---

### 4.5. File I/O Algorithms

#### **4.5.1. Load Books from File**

```cpp
bool loadBooks(const string& filename) {
    ifstream ifs(filename);
    string line;
    
    while (getline(ifs, line)) {              // O(L) - L lines
        stringstream ss(line);
        string id, title, author, qty, avail;
        
        // Parsing - O(k) per line, k = line length
        getline(ss, id, '|');
        getline(ss, title, '|');
        getline(ss, author, '|');
        getline(ss, qty, '|');
        getline(ss, avail);
        
        // Validation
        if (bookExists(id)) {                 // O(1) hash check
            cout << "Canh bao: Ma sach da ton tai\n";
            continue;
        }
        
        // Parse quantity - O(d) where d = number of digits
        int quantity = stoi(qty);
        
        // Insert - O(1) average
        Book b(id, title, author, quantity);
        books[id] = b;
    }
    return true;
}
```

**Complexity:**
- **Time**: O(L × k) 
  - L = số dòng
  - k = độ dài dòng trung bình
- **Space**: O(B) - store B books
- **I/O**: Buffered read → O(file_size)

**Error Handling:**
- ✅ Skip empty lines
- ✅ Handle missing fields với defaults
- ✅ Validate quantity (không âm)
- ✅ Duplicate ID detection
- ✅ Exception handling cho stoi()

#### **4.5.2. Save Books to File**

```cpp
bool saveBooks(const string& filename) {
    ofstream ofs(filename);
    
    // Step 1: Collect & sort IDs - O(B log B)
    vector<string> ids;
    ids.reserve(books.size());
    for (const auto& kv : books) ids.push_back(kv.first);
    sort(ids.begin(), ids.end());
    
    // Step 2: Write sorted - O(B × k)
    for (const auto& id : ids) {
        auto it = books.find(id);
        const Book& b = it->second;
        ofs << b.getId() << "|" << b.getTitle() << "|" 
            << b.getAuthor() << "|" << b.getQuantity() 
            << "|" << (b.getAvailability() ? "1" : "0") << "\n";
    }
    
    return true;
}
```

**Complexity:**
- **Time**: O(B log B + B × k) = O(B log B)
- **Space**: O(B) - temporary ID vector
- **I/O**: Buffered write

**Design Choice**: Lưu sorted để dễ diff & human-readable

---

## 5. CHI TIẾT IMPLEMENTATION

### 5.1. Class Book

```cpp
class Book {
private:
    string id;              // Primary key
    string title;           // Searchable
    string author;          // Searchable
    bool isAvailable;       // Computed from quantity
    int quantity;           // Inventory count
    
public:
    // Constructors
    Book();
    Book(const string& id, const string& title, 
         const string& author, int qty = 0, bool available = true);
    
    // Getters - O(1)
    string getId() const;
    string getTitle() const;
    string getAuthor() const;
    bool getAvailability() const;
    int getQuantity() const;
    
    // Setters with validation - O(1)
    void setAvailability(bool available);
    void setQuantity(int qty);           // Check qty >= 0
    void increaseQuantity(int amount);   // Check amount > 0
    void decreaseQuantity(int amount);   // Check sufficient qty
};
```

**Design Decisions:**
1. **String for ID**: Flexible format (B001, ISBN, etc.)
2. **Redundant isAvailable**: Cache để tránh check quantity nhiều lần
3. **Validation in setters**: Đảm bảo invariants
4. **Const correctness**: Getters are const methods

**Memory Layout:**
```
sizeof(Book) ≈ 
  sizeof(string) * 3    // id, title, author (~72 bytes)
  + sizeof(bool)        // isAvailable (1 byte + padding)
  + sizeof(int)         // quantity (4 bytes)
  ≈ 80-100 bytes per book
```

---

### 5.2. Class Reader

```cpp
class Reader {
private:
    string id;                      // Primary key
    string name;                    // Display name
    vector<string> borrowedBookIds; // Foreign keys
    
public:
    Reader(const string& id, const string& name);
    
    // Getters - O(1)
    string getId() const;
    string getName() const;
    const vector<string>& getBorrowed() const;
    
    // Operations
    void borrowBook(const string& bookId);       // O(1) amortized
    bool returnBook(const string& bookId);       // O(M)
};
```

**Key Points:**
- **borrowedBookIds**: Vector thay vì set
  - Pro: Simple, low overhead for small M
  - Con: O(M) return operation
- **Return by const reference**: Avoid copying vector

---

### 5.3. Class Library

```cpp
class Library {
private:
    unordered_map<string, Book> books;   // Main book database
    vector<Reader> readers;               // Reader database
    
    // Helper
    void writeFile(const string& filename, const string& content);
    
public:
    // CRUD for Books
    bool addBook(const Book& book);                    // O(1)
    bool removeBook(const string& bookId);             // O(N×M)
    Book* findBookById(const string& id);              // O(1)
    bool bookExists(const string& id) const;           // O(1)
    
    // CRUD for Readers
    bool addReader(const Reader& r);                   // O(N)
    bool removeReader(const string& readerId);         // O(N)
    Reader* findReaderById(const string& id);          // O(N)
    bool readerExists(const string& id) const;         // O(N)
    
    // Search
    vector<Book> searchByTitle(const string& kw) const;   // O(B×T)
    vector<Book> searchByAuthor(const string& kw) const;  // O(B×A)
    vector<Book> suggest(const string& keyword) const;    // Alias
    
    // Transactions
    bool borrowBook(const string& bookId, 
                    const string& readerId);           // O(N)
    bool returnBook(const string& bookId,
                    const string& readerId);           // O(N+M)
    
    // Display
    void listAll() const;                              // O(B log B)
    void listReaders() const;                          // O(N)
    
    // Persistence
    bool loadBooks(const string& filename);            // O(L×k)
    bool saveBooks(const string& filename);            // O(B log B)
    bool loadReaders(const string& filename);          // O(L×k)
    bool saveReaders(const string& filename);          // O(N)
};
```

**Architecture Pattern**: Facade Pattern
- Library class encapsulates all operations
- Client (main.cpp) không trực tiếp access Book/Reader

---

## 6. QUẢN LÝ FILE & PERSISTENCE

### 6.1. File Format

#### **books.txt**
```
Format: id|title|author|quantity|available
Example:
B001|Introduction to Algorithms|Cormen|5|1
B002|Clean Code|Robert Martin|0|0
B003|Design Patterns|Gang of Four|3|1
```

#### **readers.txt**
```
Format: id|name|borrowedBook1,borrowedBook2,...
Example:
R001|Nguyen Van A|B001,B005
R002|Tran Thi B|
R003|Le Van C|B002,B003,B007
```

### 6.2. Log Files

**Purpose**: Audit trail + debugging

**books_them.txt**: Log additions
```
B001|Introduction to Algorithms|Cormen|5
B004|The Pragmatic Programmer|Hunt|2
```

**books_xoa.txt**: Log deletions
```
B010|Old Book|Unknown|0|0
```

**Design Trade-off:**
- ✅ Append-only → Fast write
- ✅ Không mất dữ liệu historical
- ❌ Không có timestamp
- ❌ Không có rollback mechanism

---

## 7. TỐI ƯU HÓA & TRADE-OFFS

### 7.1. Space-Time Trade-offs

| Optimization | Time Benefit | Space Cost | Worth It? |
|---|---|---|---|
| **books: map → unordered_map** | O(log B) → O(1) | +20% overhead | ✅ YES |
| **readers: vector → unordered_map** | O(N) → O(1) | +O(N) overhead | ⚠️ If N > 1000 |
| **borrowedBookIds: vector → unordered_set** | O(M) → O(1) return | +O(M) overhead | ⚠️ If M > 10 |
| **Inverted index for search** | O(B×T) → O(1) | +O(V×B) memory | ❌ NO for small B |
| **Cache sorted book list** | O(B log B) → O(1) | +O(B) memory | ⚠️ If reads >> writes |

### 7.2. Current Bottlenecks

1. **removeBook() - O(N×M)**
   ```cpp
   // Problem: Nested loop check
   for (const auto& r : readers) {
       for (const auto& bid : r.getBorrowed()) {
           if (bid == bookId) return false;
       }
   }
   
   // Solution: Maintain reverse index
   unordered_map<string, unordered_set<string>> bookToBorrowers;
   ```

2. **searchByTitle() - O(B×T)**
   ```cpp
   // Problem: Linear scan với substring match
   
   // Solution 1: Inverted index
   unordered_map<string, vector<string>> wordToBookIds;
   
   // Solution 2: Trie for prefix search
   class Trie {
       struct Node {
           unordered_map<char, Node*> children;
           vector<string> bookIds;
       };
   };
   ```

3. **findReaderById() - O(N)**
   ```cpp
   // Problem: Linear search
   
   // Solution: Change to unordered_map
   unordered_map<string, Reader> readers;
   ```

### 7.3. Recommended Improvements

#### **Phase 1: Immediate Wins**
```cpp
// 1. Đổi readers thành hash map
unordered_map<string, Reader> readers;
// Impact: borrowBook, returnBook: O(N) → O(1)

// 2. Dùng unordered_set cho borrowedBookIds
unordered_set<string> borrowedBookIds;
// Impact: returnBook: O(M) → O(1)

// 3. Reserve capacity cho vectors
books.reserve(1000);
ids.reserve(books.size());
```

**Performance Impact:**
- borrowBook: O(N) → O(1) ⚡ **100x faster for N=100**
- returnBook: O(N+M) → O(1) ⚡ **10x faster**

#### **Phase 2: Advanced Optimizations**
```cpp
// 1. Maintain bookToBorrowers index
unordered_map<string, unordered_set<string>> bookToBorrowers;

// Update on borrow:
void borrowBook(bookId, readerId) {
    // ... existing code ...
    bookToBorrowers[bookId].insert(readerId);
}

// Update on return:
void returnBook(bookId, readerId) {
    // ... existing code ...
    bookToBorrowers[bookId].erase(readerId);
}

// Fast removeBook:
bool removeBook(const string& bookId) {
    if (!bookToBorrowers[bookId].empty()) return false;  // O(1)
    // ... rest of code ...
}
```

**Performance:**
- removeBook: O(N×M) → O(1) ⚡ **1000x faster**

---

## 8. HƯỚNG PHÁT TRIỂN

### 8.1. Feature Enhancements

1. **Advanced Search**
   ```cpp
   // Multi-field search
   vector<Book> search(const SearchCriteria& criteria);
   
   // Fuzzy matching (Levenshtein distance)
   vector<Book> fuzzySearch(const string& keyword, int maxDistance = 2);
   
   // Autocomplete
   vector<string> autocomplete(const string& prefix, int limit = 10);
   ```

2. **Transaction Management**
   ```cpp
   class Transaction {
       enum Type { BORROW, RETURN, ADD_BOOK, REMOVE_BOOK };
       Type type;
       string timestamp;
       string userId;
       string bookId;
   };
   
   vector<Transaction> transactionHistory;
   void rollback(const Transaction& t);
   ```

3. **Caching Layer**
   ```cpp
   class LRUCache<K, V> {
       list<pair<K, V>> items;
       unordered_map<K, list<pair<K, V>>::iterator> cache;
       size_t capacity;
   public:
       V* get(const K& key);        // O(1)
       void put(const K& key, const V& val);  // O(1)
   };
   
   LRUCache<string, Book> recentBooks;
   ```

### 8.2. Data Structure Upgrades

1. **B+ Tree for Books**
   - Hỗ trợ range queries: "Tìm sách ID từ B100-B200"
   - Disk-friendly cho large datasets
   
2. **Trie for Title Search**
   ```
   Root
   ├─ I → n → t → r → o → ... [B001]
   ├─ C → l → e → a → n → ... [B002]
   └─ D → e → s → i → g → n → ... [B003]
   ```

3. **Bloom Filter for Existence Check**
   ```cpp
   BloomFilter bookFilter(10000, 0.01);  // 10k items, 1% false positive
   
   bool mayExist(const string& id) {
       return bookFilter.contains(id);  // O(k) with k hash functions
   }
   ```

### 8.3. Scalability

**Current Limits:**
- Books: ~10,000 (memory constraint)
- Readers: ~1,000 (O(N) search acceptable)
- File I/O: Single-threaded, blocking

**Scaling Strategy:**
1. **Database Migration**: SQLite/MySQL
2. **Indexing**: B-tree indices on ID, title, author
3. **Concurrent Access**: Mutex/RW locks
4. **Pagination**: Không load toàn bộ vào memory

### 8.4. Code Quality

1. **Unit Testing**
   ```cpp
   TEST(LibraryTest, BorrowBookDecreasesQuantity) {
       Library lib;
       Book b("B001", "Test", "Author", 5);
       lib.addBook(b);
       Reader r("R001", "User");
       lib.addReader(r);
       
       ASSERT_TRUE(lib.borrowBook("B001", "R001"));
       ASSERT_EQ(lib.findBookById("B001")->getQuantity(), 4);
   }
   ```

2. **Error Handling**
   ```cpp
   enum class LibraryError {
       BOOK_NOT_FOUND,
       READER_NOT_FOUND,
       INSUFFICIENT_QUANTITY,
       BOOK_ALREADY_BORROWED,
       DUPLICATE_ID
   };
   
   Result<bool, LibraryError> borrowBook(...);
   ```

3. **Logging**
   ```cpp
   class Logger {
   public:
       static void info(const string& msg);
       static void error(const string& msg);
       static void debug(const string& msg);
   };
   ```

---

## 📊 BẢNG TỔNG HỢP ĐỘ PHỨC TẠP

| Operation | Time Complexity | Space Complexity | Notes |
|-----------|----------------|------------------|-------|
| **Load Books** | O(L × k) | O(B) | L lines, k chars/line |
| **Save Books** | O(B log B) | O(B) | Sorting overhead |
| **Add Book** | O(1) avg | O(1) | Hash insert |
| **Remove Book** | O(N × M) | O(1) | Constraint check bottleneck |
| **Find Book by ID** | O(1) avg | O(1) | Hash lookup |
| **Search by Title** | O(B × T) | O(R) | Linear scan, T = title length |
| **Search by Author** | O(B × A) | O(R) | Linear scan, A = author length |
| **List All (sorted)** | O(B log B) | O(B) | Sorting IDs |
| **Add Reader** | O(N) | O(1) | Duplicate check |
| **Remove Reader** | O(N) | O(1) | Linear search + erase |
| **Find Reader by ID** | O(N) | O(1) | Linear search |
| **Borrow Book** | O(N) | O(1) | Find reader dominates |
| **Return Book** | O(N + M) | O(1) | Find reader + linear erase |
| **List Readers** | O(N) | O(1) | Simple iteration |

**Legend:**
- B = số sách
- N = số độc giả
- M = số sách mượn trung bình/người
- L = số dòng file
- k = độ dài dòng trung bình
- R = số kết quả search
- T = độ dài title
- A = độ dài author name

---

## 🎯 KẾT LUẬN

### Điểm Mạnh
1. ✅ **Hash table cho Books**: O(1) access rất hiệu quả
2. ✅ **Simple architecture**: Dễ hiểu, dễ maintain
3. ✅ **File persistence**: Data không mất khi restart
4. ✅ **Validation**: Error checking tốt
5. ✅ **Audit trail**: Log files for debugging

### Điểm Cần Cải Thiện
1. ❌ **Reader lookup O(N)**: Nên dùng hash map
2. ❌ **removeBook O(N×M)**: Cần reverse index
3. ❌ **Search O(B×T)**: Có thể optimize với inverted index
4. ❌ **No transaction support**: Risk của data inconsistency
5. ❌ **Single-threaded**: Không handle concurrent access

### Phù Hợp Cho
- ✅ Small-medium scale: <10,000 books, <1,000 readers
- ✅ Learning DSA: Demonstration của hash table, vector, string algorithms
- ✅ Single-user application: Console-based library management

### Không Phù Hợp Cho
- ❌ Large-scale: >100,000 books (cần database)
- ❌ Multi-user: Concurrent access (cần locks)
- ❌ Real-time: Search latency có thể cao
- ❌ Production: Thiếu error recovery, backup mechanisms

---

**Tác giả**: Library Management System  
**Ngôn ngữ**: C++17  
**Mục đích**: Educational project - Data Structures & Algorithms demonstration  
**License**: MIT

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
