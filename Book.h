#pragma once
#include <string>

using namespace std;
class Book
{
    private:
        string id;
        string title;
        string author;
        bool isAvailable;
        int quantity;

    public:
        Book();
        Book(const string& id, const string& title, const string& author, int qty = 0, bool available = true);
        ~Book();

        // Getters
        string getId() const;
        string getTitle() const;
        string getAuthor() const;
        // old ISBN removed - use `getId()`
        bool getAvailability() const;
        int getQuantity() const;

        // Setters
        void setAvailability(bool available);
        void setQuantity(int qty);
        void increaseQuantity(int amount);
        void decreaseQuantity(int amount);
};