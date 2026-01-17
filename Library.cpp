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

Library::Library() {}

Library::~Library() {}

void Library::addBook(const Book& book) { 
    books[book.getId()] = book;
}

void Library::listAll() const {
    cout << left
         << setw(5)  << "ID"
         << setw(30) << "Title"
         << setw(25) << "Author"
         << setw(10) << "Qty"
         << setw(12) << "Status"
         << endl;

    cout << string(82, '-') << endl;

    for (const auto& pair : books) {
        const Book& b = pair.second;
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
    if (!it->second.getAvailability()) return false;
    Reader* r = findReaderById(readerId);
    if (!r) return false;
    it->second.setAvailability(false);
    r->borrowBook(bookId);
    return true;
}

bool Library::returnBook(const string& bookId, const string& readerId) {
    auto it = books.find(bookId);
    if (it == books.end()) return false;
    Reader* r = findReaderById(readerId);
    if (!r) return false;
    if (r->returnBook(bookId)) {
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

bool Library::addReader(const Reader& r) { 
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
    books.erase(it);
    return true;
}

bool Library::removeReader(const string& readerId) {
    for (auto it = readers.begin(); it != readers.end(); ++it) {
        if (it->getId() == readerId) {
            readers.erase(it);
            return true;
        }
    }
    return false;
}
