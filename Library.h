#pragma once
#include "Book.h"
#include "Reader.h"
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class Library {
private:
    unordered_map<string, Book> books;  // Hash table: key = book ID, value = Book object
    vector<Reader> readers;

public:
    Library();
    ~Library();

    void addBook(const Book& book);
    bool borrowBook(const string& bookId, const string& readerId);
    bool returnBook(const string& bookId, const string& readerId);
    vector<Book> searchByTitle(const string& keyword) const;
    vector<Book> searchByAuthor(const string& keyword) const;
    vector<Book> suggest(const string& keyword) const;
    void listAll() const;

    // File I/O
    bool loadBooks(const string& filename);
    bool loadReaders(const string& filename);
    bool addReader(const Reader& r);
    void listReaders() const;
    bool removeBook(const string& bookId);
    bool removeReader(const string& readerId);
    Reader* findReaderById(const string& id);
};
