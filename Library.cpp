#include "Library.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iomanip>

static inline string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

void Library::writeFile(const string &filename, const string &content)
{
    ofstream ofs(filename, ios::app);
    if (ofs) {
        ofs << content << "\n";
    }
}

Library::Library() {}

Library::~Library() {}

bool Library::addBook(const Book& book) { 
    // Kiem tra trung ID
    if (bookExists(book.getId())) return false;
    
    books[book.getId()] = book;

    stringstream ss;
    ss << book.getId() << "|" << book.getTitle() << "|" << book.getAuthor()
       << "|" << book.getQuantity();
    writeFile("books_them.txt", ss.str());
    return true;
}

void Library::listAll() const {
    vector<string> ids;
    for (const auto& kv : books) ids.push_back(kv.first);
    sort(ids.begin(), ids.end());
    cout << left
        << setw(5)  << "Ma"
        << setw(30) << "Ten"
        << setw(25) << "Tac gia"
        << setw(10) << "So luong"
        << setw(12) << "Tinh trang"
        << endl;
    for (const auto& id : ids) {
        auto it = books.find(id);
        if (it == books.end()) continue;
        const Book& b = it->second;
        cout << left
            << setw(5)  << b.getId()
            << setw(30) << b.getTitle()
            << setw(25) << b.getAuthor()
            << setw(10) << b.getQuantity()
            << setw(12) << (b.getAvailability() ? "Available" : "Borrowed")
            << endl;
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
        it->second.increaseQuantity(1);
        
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
        if (id.empty()) continue;  // Kiem tra ID trong
        if (!getline(ss, title, '|')) continue;
        if (!getline(ss, author, '|')) continue;
        if (!getline(ss, qty, '|')) qty = "0";
        if (!getline(ss, avail)) avail = "1";
        
        // Kiem tra va convert quantity
        int quantity = 0;
        try {
            quantity = stoi(qty);
            if (quantity < 0) {
                cout << "Canh bao: So luong am cho sach " << id << ", dung 0\n";
                quantity = 0;
            }
        } catch (const exception& e) {
            cout << "Canh bao: Khong the parse so luong cho sach " << id << ", dung 0\n";
            quantity = 0;
        }
        
        // Kiem tra ID trung lap
        if (bookExists(id)) {
            cout << "Canh bao: Ma sach " << id << " da ton tai, bo qua\n";
            continue;
        }
        
        Book b(id, title, author, quantity, (avail != "0"));
        books[b.getId()] = b;  // Dung truc tiep thay vi addBook() de tranh log duplicate
    }
    ifs.close();
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
        if (id.empty()) continue;  // Kiem tra ID trong
        if (!getline(ss, name, '|')) continue;
        if (name.empty()) {
            cout << "Canh bao: Dang ki ten trong cho doc gia " << id << ", bo qua\n";
            continue;
        }
        if (!getline(ss, list)) list = "";
        
        // Kiem tra ID trung lap
        if (readerExists(id)) {
            cout << "Canh bao: Ma doc gia " << id << " da ton tai, bo qua\n";
            continue;
        }
        
        Reader r(id, name);
        if (!list.empty()) {
            stringstream ss2(list);
            string tok;
            while (getline(ss2, tok, ',')) {
                // Trim whitespace from token
                size_t start = tok.find_first_not_of(" \t\r\n");
                size_t end = tok.find_last_not_of(" \t\r\n");
                if (start != string::npos) {
                    tok = tok.substr(start, end - start + 1);
                } else {
                    continue;  // Skip empty tokens
                }
                if (!tok.empty()) {
                    // Kiem tra sach co ton tai khong
                    if (!bookExists(tok)) {
                        cout << "Canh bao: Sach " << tok << " khong ton tai cho doc gia " << id << ", bo qua\n";
                    } else {
                        r.borrowBook(tok);
                    }
                }
            }
        }
        readers.push_back(r);
    }
    ifs.close();
    return true;
}

bool Library::saveBooks(const string &filename)
{
        ofstream ofs(filename);
    if (!ofs) return false;
    vector<string> ids;
    ids.reserve(books.size());
    for (const auto& kv : books) ids.push_back(kv.first);
    sort(ids.begin(), ids.end());
    for (const auto& id : ids) {
        auto it = books.find(id);
        if (it == books.end()) continue;
        const Book& b = it->second;
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
            if (i + 1 < borrowed.size()) ofs << ", ";
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

    stringstream ss;
    ss << r.getId() << "|" << r.getName();
    writeFile("readers_them.txt", ss.str()); 
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
    //Luu thong tin sach bi xoa
    stringstream ss;
    ss << it->second.getId() << "|" << it->second.getTitle() << "|" << it->second.getAuthor() << "|" << it->second.getQuantity() << "|" << (it->second.getAvailability() ? "1" : "0");
    writeFile("books_xoa.txt", ss.str());
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
            // Luu thong tin doc gia bi xoa
            stringstream ss;
            ss << it->getId() << "|" << it->getName();
            writeFile("readers_xoa.txt", ss.str());
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
