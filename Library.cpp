#include "Library.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

static inline string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

void Library::writeFile(const string &filename, const string &content)
{
    ofstream ofs(filename, ios::app);
    if (ofs) {
        ofs << content << "|";
        ofs.close();
    }
}

Library::Library() {}

Library::~Library() {}

void Library::addBook(const Book& book) { 
    books[book.getId()] = book;
}

void Library::listAll() const {
    for (const auto& pair : books) {
        const Book& b = pair.second;
        cout << b.getTitle() << " | " << b.getAuthor() << " | " << b.getId()
             << " | So luong: " << b.getQuantity()
             << " | Trang thai: " << (b.getQuantity() > 0 ? "Con sach" : "Het sach") << "\n";
    }
}

vector<Book> Library::searchByTitle(const string& keyword) const {
    string k = toLower(keyword);
    vector<Book> res;
    for (const auto& pair : books) {
        const Book& b = pair.second;
        if (toLower(b.getTitle()).find(k) != string::npos) {
            res.push_back(b);
        }
    }
    return res;
}

vector<Book> Library::searchByAuthor(const string& keyword) const {
    string k = toLower(keyword);
    vector<Book> res;
    for (const auto& pair : books) {
        const Book& b = pair.second;
        if (toLower(b.getAuthor()).find(k) != string::npos) {
            res.push_back(b);
        }
    }
    return res;
}

vector<Book> Library::suggest(const string& keyword) const { 
    return searchByTitle(keyword); 
}

bool Library::borrowBook(const string& bookId, const string& readerId) {
    auto it = books.find(bookId);
    if (it == books.end()) return false;
    if (it->second.getQuantity() <= 0) return false;  // Kiem tra so luong
    Reader* r = findReaderById(readerId);
    if (!r) return false;
    
    // Giam so luong sach
    it->second.decreaseQuantity(1);
    
    // Neu het sach thi set availability = false
    if (it->second.getQuantity() == 0) {
        it->second.setAvailability(false);
    }
    
    r->borrowBook(bookId);
    return true;
}

bool Library::returnBook(const string& bookId, const string& readerId) {
    auto it = books.find(bookId);
    if (it == books.end()) return false;
    Reader* r = findReaderById(readerId);
    if (!r) return false;
    
    // Kiem tra doc gia co muon sach nay khong
    if (r->returnBook(bookId)) {
        // Tang so luong sach
        it->second.increaseQuantity(1);
        
        // Set availability = true vi da co sach
        it->second.setAvailability(true);
        return true;
    }
    return false;
}

bool Library::loadBooks(const string& filename) {
    ifstream ifs(filename);
    if (!ifs) return false;
    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        // format: id|title|author|quantity|available
        stringstream ss(line);
        string id, title, author, qty, avail;
        if (!getline(ss, id, '|')) continue;
        if (!getline(ss, title, '|')) continue;
        if (!getline(ss, author, '|')) continue;
        if (!getline(ss, qty, '|')) qty = "0";
        if (!getline(ss, avail)) avail = "1";
        int quantity = stoi(qty);
        Book b(id, title, author, quantity, (avail != "0"));
        addBook(b);
    }
    return true;
}

bool Library::loadReaders(const string& filename) {
    ifstream ifs(filename);
    if (!ifs) return false;
    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        // format: id|name|isbn1,isbn2
        stringstream ss(line);
        string id, name, list;
        if (!getline(ss, id, '|')) continue;
        if (!getline(ss, name, '|')) continue;
        if (!getline(ss, list)) list = "";
        Reader r(id, name);
        if (!list.empty()) {
            stringstream ss2(list);
            string tok;
            while (getline(ss2, tok, ',')) {
                if (!tok.empty()) r.borrowBook(tok);
            }
        }
        readers.push_back(r);
    }
    return true;
}

bool Library::saveBooks(const string &filename)
{
        ofstream ofs(filename);
    if (!ofs) return false;
    for (const auto& pair : books) {
        const Book& b = pair.second;
        ofs << b.getId() << "|" << b.getTitle() << "|" << b.getAuthor() 
            << "|" << b.getQuantity() << "|" << (b.getAvailability() ? "1" : "0") << "\n";
    }
    ofs.close();
    return true;
}

bool Library::saveReaders(const string &filename)
{
    ofstream ofs(filename);
    if (!ofs) return false;
    for (const auto& r : readers) {
        ofs << r.getId() << "|" << r.getName() << "|";
        const auto& borrowed = r.getBorrowed();
        for (size_t i = 0; i < borrowed.size(); ++i) {
            ofs << borrowed[i];
            if (i + 1 < borrowed.size()) ofs << ",";
        }
        ofs << "\n";
    }
    ofs.close();
    return true;
}

bool Library::addReader(const Reader& r) {
    // Kiem tra trung ID
    if (readerExists(r.getId())) return false;
    readers.push_back(r); 
    return true; 
}

void Library::listReaders() const {
    for (const auto &r : readers) {
        cout << r.getId() << " | " << r.getName() << " | Da muon: ";
        const auto &b = r.getBorrowed();
        for (size_t i = 0; i < b.size(); ++i) {
            cout << b[i];
            if (i + 1 < b.size()) cout << ",";
        }
        cout << "\n";
    }
}

Reader* Library::findReaderById(const string& id) {
    for (auto &r : readers) if (r.getId() == id) return &r;
    return nullptr;
}

bool Library::removeBook(const string& bookId) {
    auto it = books.find(bookId);
    if (it == books.end()) return false;
    
    // Kiem tra xem co doc gia nao dang muon sach nay khong
    for (const auto& r : readers) {
        const auto& borrowed = r.getBorrowed();
        for (const auto& bid : borrowed) {
            if (bid == bookId) return false;  // Sach dang duoc muon, khong the xoa
        }
    }
    
    books.erase(it);
    return true;
}

bool Library::removeReader(const string& readerId) {
    for (auto it = readers.begin(); it != readers.end(); ++it) {
        if (it->getId() == readerId) {
            // Kiem tra xem doc gia co dang muon sach nao khong
            if (!it->getBorrowed().empty()) {
                return false;  // Doc gia dang muon sach, khong the xoa
            }
            readers.erase(it);
            return true;
        }
    }
    return false;
}

Book* Library::findBookById(const string& id) {
    auto it = books.find(id);
    if (it != books.end()) return &(it->second);
    return nullptr;
}

bool Library::bookExists(const string& id) const {
    return books.find(id) != books.end();
}

bool Library::readerExists(const string& id) const {
    for (const auto& r : readers) {
        if (r.getId() == id) return true;
    }
    return false;
}
