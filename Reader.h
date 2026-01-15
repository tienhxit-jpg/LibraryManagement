#pragma once
#include <string>
#include <vector>

using namespace std;

class Reader {
private:
    string id;
    string name;
    vector<string> borrowedBookIds;

public:
    Reader() {}
    Reader(const string& id, const string& name);

    string getId() const;
    string getName() const;

    void borrowBook(const string& bookId);
    bool returnBook(const string& bookId);
    const vector<string>& getBorrowed() const;
};
