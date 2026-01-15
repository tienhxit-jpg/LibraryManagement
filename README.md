# Library Management System

Hệ thống quản lý thư viện số được xây dựng bằng C++ với các cấu trúc dữ liệu nâng cao.

## 📋 Mục lục

- [Tổng quan](#tổng-quan)
- [Tính năng](#tính-năng)
- [Cấu trúc dữ liệu & Thuật toán](#cấu-trúc-dữ-liệu--thuật-toán)
- [Cài đặt & Chạy](#cài-đặt--chạy)
- [Cách sử dụng](#cách-sử-dụng)
- [Cấu trúc file](#cấu-trúc-file)
- [Chi tiết kỹ thuật](#chi-tiết-kỹ-thuật)

## 🎯 Tổng quan

Đây là một hệ thống quản lý thư viện hoàn chỉnh cho phép quản lý sách và độc giả, bao gồm:
- Quản lý danh sách sách (thêm, xóa, tìm kiếm)
- Quản lý danh sách độc giả
- Chức năng mượn/trả sách
- Tìm kiếm theo tiêu đề, tác giả
- Theo dõi số lượng sách có sẵn

## ✨ Tính năng

| Tính năng | Mô tả |
|----------|-------|
| **Tải dữ liệu** | Đọc danh sách sách và độc giả từ file |
| **Liệt kê sách** | Hiển thị tất cả sách với thông tin chi tiết |
| **Tìm kiếm tiêu đề** | Tìm sách theo từ khóa trong tiêu đề |
| **Tìm kiếm tác giả** | Tìm sách theo từ khóa trong tên tác giả |
| **Mượn sách** | Cập nhật trạng thái sách khi độc giả mượn |
| **Trả sách** | Cập nhật trạng thái sách khi độc giả trả |
| **Quản lý độc giả** | Thêm/xóa độc giả, liệt kê thông tin |
| **Quản lý sách** | Thêm/xóa sách khỏi hệ thống |

## 📊 Cấu trúc dữ liệu & Thuật toán

### Cấu trúc dữ liệu sử dụng:

```
┌─────────────────────────────────┐
│  Library (Quản lý thư viện)     │
├─────────────────────────────────┤
│ • Hash Table (unordered_map)    │  O(1) tìm kiếm theo ID
│   → lưu trữ sách               │
│                                 │
│ • Vector<Reader>               │  O(n) duyệt danh sách
│   → lưu trữ độc giả            │
│                                 │
│ • Mỗi Reader có Vector<string> │  O(n) duyệt sách đã mượn
│   → lưu sách đã mượn           │
└─────────────────────────────────┘
```

### Thuật toán tìm kiếm:

| Loại | Thuật toán | Độ phức tạp | Chi tiết |
|------|-----------|-----------|---------|
| **Theo ID** | Hash Table Lookup | $O(1)$ | Tìm trực tiếp trong hash table |
| **Theo tiêu đề** | Linear Search + String Matching | $O(n)$ | Duyệt tất cả sách, so sánh chuỗi |
| **Theo tác giả** | Linear Search + String Matching | $O(n)$ | Duyệt tất cả sách, so sánh chuỗi |

### Các phép toán chính:

- **Thêm sách**: $O(1)$ - Hash table insertion
- **Xóa sách**: $O(1)$ - Hash table deletion  
- **Mượn sách**: $O(1)$ - Cập nhật trạng thái
- **Trả sách**: $O(1)$ - Cập nhật trạng thái
- **Tìm kiếm**: $O(n)$ - Duyệt toàn bộ hash table

## 🚀 Cài đặt & Chạy

### Yêu cầu hệ thống

- **Compiler**: g++ (C++11 hoặc cao hơn)
- **OS**: Windows, Linux, macOS
- **IDE**: Visual Studio Code (tùy chọn)

### Compile

```bash
g++ -Wall -Wextra -std=c++11 -o output main.cpp Book.cpp Reader.cpp Library.cpp
```

### Chạy chương trình

**Windows:**
```bash
.\output.exe
```

**Linux/macOS:**
```bash
./output
```

## 💻 Cách sử dụng

### Menu chính

Khi chạy chương trình, bạn sẽ thấy menu sau:

```
--- HE THONG QUAN LY THU VIEN ---
1. Tai du lieu tu file
2. Liet ke toan bo sach
3. Tim theo tieu de
4. Tim theo tac gia
5. Muon sach
6. Tra sach
7. Liet ke doc gia
8. Them doc gia
9. Xoa sach
10. Xoa doc gia
0. Thoat
```

### Các bước sử dụng:

**Bước 1**: Chọn **1** để tải dữ liệu từ file
```
Dang tai du lieu tu books.txt va readers.txt...
```

**Bước 2**: Chọn **2** để xem danh sách sách
```
The C++ Programming Language | Bjarne Stroustrup | B001 | Quantity: 5 | Available
Clean Code | Robert C. Martin | B002 | Quantity: 3 | Available
...
```

**Bước 3**: Chọn **3** hoặc **4** để tìm kiếm
```
Nhap tu khoa tieu de: C++
Effective Modern C++ | Scott Meyers | B005 | Quantity: 6 | Available
The C++ Programming Language | Bjarne Stroustrup | B001 | Quantity: 5 | Available
```

**Bước 4**: Chọn **5** để mượn sách
```
Ma doc gia: R001
Ma sach: B001
Muon sach thanh cong
```

**Bước 5**: Chọn **6** để trả sách
```
Ma doc gia: R001
Ma sach: B001
Tra sach thanh cong
```

## 📁 Cấu trúc file

```
LibraryManagement/
├── main.cpp              # Chương trình chính
├── Book.h / Book.cpp     # Lớp Book (sách)
├── Reader.h / Reader.cpp # Lớp Reader (độc giả)
├── Library.h / Library.cpp # Lớp Library (quản lý thư viện)
├── books.txt             # Dữ liệu sách
├── readers.txt           # Dữ liệu độc giả
└── README.md             # File này
```

### Định dạng dữ liệu

**books.txt** - Định dạng: `ID|Tiêu đề|Tác giả|Số lượng|Có sẵn`
```
B001|The C++ Programming Language|Bjarne Stroustrup|5|1
B002|Clean Code|Robert C. Martin|3|1
B003|Introduction to Algorithms|Thomas H. Cormen|2|1
```

**readers.txt** - Định dạng: `ID|Tên|Sách đã mượn`
```
R001|Nguyen Van A|
R002|Tran Thi B|B001,B002
R003|Le Van C|B003
```

## 🔧 Chi tiết kỹ thuật

### Lớp Book

```cpp
class Book {
    private:
        string id;              // Mã sách
        string title;           // Tiêu đề
        string author;          // Tác giả
        bool isAvailable;       // Có sẵn?
        int quantity;           // Số lượng

    public:
        // Getters
        string getId() const;
        string getTitle() const;
        string getAuthor() const;
        bool getAvailability() const;
        int getQuantity() const;
        
        // Setters
        void setAvailability(bool available);
        void setQuantity(int qty);
        void increaseQuantity(int amount);
        void decreaseQuantity(int amount);
};
```

### Lớp Reader

```cpp
class Reader {
    private:
        string id;                    // Mã độc giả
        string name;                  // Tên độc giả
        vector<string> borrowedBookIds; // Sách đã mượn

    public:
        void borrowBook(const string& bookId);
        bool returnBook(const string& bookId);
        // ... getter/setter khác
};
```

### Lớp Library

```cpp
class Library {
    private:
        unordered_map<string, Book> books;  // Hash table lưu sách
        vector<Reader> readers;              // Vector lưu độc giả

    public:
        void addBook(const Book& book);
        bool removeBook(const string& bookId);
        bool borrowBook(const string& bookId, const string& readerId);
        bool returnBook(const string& bookId, const string& readerId);
        void searchByTitle(const string& keyword);
        void searchByAuthor(const string& keyword);
        bool loadBooks(const string& filename);
        bool loadReaders(const string& filename);
        // ... các method khác
};
```

## 📝 Ví dụ sử dụng

### Ví dụ 1: Tìm kiếm sách theo tiêu đề
```
Chon: 3
Nhap tu khoa tieu de: Algorithm
Introduction to Algorithms | Thomas H. Cormen | B003 | Quantity: 2 | Available
```

### Ví dụ 2: Mượn sách
```
Chon: 5
Ma doc gia: R001
Ma sach: B001
Muon sach thanh cong
```

### Ví dụ 3: Xem danh sách độc giả
```
Chon: 7
R001 | Nguyen Van A | borrowed: B001,B002
R002 | Tran Thi B | borrowed: B003
```

## 🎓 Kiến thức áp dụng

- **Cấu trúc dữ liệu**: Hash Table, Vector, String
- **Thuật toán**: Linear Search, Hash Table Lookup, String Matching
- **OOP**: Class, Encapsulation, Inheritance
- **File I/O**: Đọc/ghi file text
- **Dynamic Memory**: Con trỏ (trước đây dùng BST)

## 📌 Lưu ý

- Đảm bảo file `books.txt` và `readers.txt` nằm cùng thư mục với file thực thi
- Dữ liệu chỉ được lưu trong bộ nhớ, không tự động lưu lại file sau khi chỉnh sửa
- ID sách và ID độc giả không được trùng lặp
- Có thể mượn nhiều sách, nhưng cần trả từng cuốn một

## 👨‍💻 Tác giả

**Mục đích**: Bài tập case study - Hệ thống quản lý thư viện số  
**Ngôn ngữ**: C++ 11  
**Năm**: 2026

---

**Cập nhật lần cuối**: January 15, 2026
