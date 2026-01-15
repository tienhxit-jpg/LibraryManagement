#include <string>
#include "Book.h"

using namespace std;

Book::Book() : id(""), title(""), author(""), isAvailable(true), quantity(0)
{
}

Book::Book(const std::string& id, const std::string& title, const std::string& author, int qty, bool available)
	: id(id), title(title), author(author), isAvailable(available), quantity(qty)
{
}

Book::~Book() {}

std::string Book::getId() const { return id; }
std::string Book::getTitle() const { return title; }
std::string Book::getAuthor() const { return author; }
bool Book::getAvailability() const { return isAvailable; }
int Book::getQuantity() const { return quantity; }

void Book::setAvailability(bool available) { isAvailable = available; }
void Book::setQuantity(int qty) { 
	if (qty >= 0) quantity = qty; 
}
void Book::increaseQuantity(int amount) { 
	if (amount > 0) quantity += amount; 
}
void Book::decreaseQuantity(int amount) { 
	if (amount > 0 && quantity >= amount) quantity -= amount; 
}