#include "Reader.h"

Reader::Reader(const string& id, const string& name)
    : id(id), name(name) {}

string Reader::getId() const { return id; }
string Reader::getName() const { return name; }

void Reader::borrowBook(const string& bookId) { borrowedBookIds.push_back(bookId); }

bool Reader::returnBook(const string& bookId)
{
    for (auto it = borrowedBookIds.begin(); it != borrowedBookIds.end(); ++it) {
        if (*it == bookId) {
            borrowedBookIds.erase(it);
            return true;
        }
    }
    return false;
}

const vector<string>& Reader::getBorrowed() const { return borrowedBookIds; }
