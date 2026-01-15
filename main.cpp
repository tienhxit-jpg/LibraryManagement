/* Case study 4: Hệ thống quản lý thư viện số.
   Simple OOP C++ implementation using a BST to store books.
*/
#include <iostream>
#include <string>
#include <limits>
#include "Library.h"

using namespace std;

void showMenu() {
	cout << "\n--- HE THONG QUAN LY THU VIEN ---\n";
	cout << "1. Tai du lieu tu file\n";
	cout << "2. Liet ke toan bo sach\n";
	cout << "3. Tim theo tieu de\n";
	cout << "4. Tim theo tac gia\n";
	cout << "5. Muon sach\n";
	cout << "6. Tra sach\n";
	cout << "7. Liet ke doc gia\n";
	cout << "8. Them doc gia\n";
	cout << "9. Xoa sach\n";
	cout << "10. Xoa doc gia\n";
	cout << "0. Thoat\n";
}

int main()
{
	Library lib;
	bool running = true;
	while (running) {
		showMenu();
		cout << "Chon: ";
		int choice; if (!(cin >> choice)) break;
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		if (choice == 1) {
			cout << "Dang tai du lieu tu books.txt va readers.txt...\n";
			if (!lib.loadBooks("books.txt")) cout << "Khong the tai books.txt\n";
			if (!lib.loadReaders("readers.txt")) cout << "Khong the tai readers.txt\n";
		} else if (choice == 2) {
			lib.listAll();
		} else if (choice == 3) {
			cout << "Nhap tu khoa tieu de: "; string kw; getline(cin, kw);
			auto res = lib.searchByTitle(kw);
			for (auto &b : res) cout << b.getTitle() << " | " << b.getAuthor() << " | " << b.getId() << '\n';
		} else if (choice == 4) {
			cout << "Nhap tu khoa tac gia: "; string kw; getline(cin, kw);
			auto res = lib.searchByAuthor(kw);
			for (auto &b : res) cout << b.getTitle() << " | " << b.getAuthor() << " | " << b.getId() << '\n';
		} else if (choice == 5) {
			cout << "Ma doc gia: "; string id; getline(cin, id);
			cout << "Ma sach: "; string bookId; getline(cin, bookId);
			if (lib.borrowBook(bookId, id)) cout << "Muon sach thanh cong\n"; else cout << "Muon that bai\n";
		} else if (choice == 6) {
			cout << "Ma doc gia: "; string id; getline(cin, id);
			cout << "Ma sach: "; string bookId; getline(cin, bookId);
			if (lib.returnBook(bookId, id)) cout << "Tra sach thanh cong\n"; else cout << "Tra that bai\n";
		} else if (choice == 7) {
			lib.listReaders();
		} else if (choice == 8) {
			cout << "Ma doc gia moi: "; string id; getline(cin, id);
			cout << "Ho ten: "; string name; getline(cin, name);
			Reader r(id, name);
			lib.addReader(r);
			cout << "Da them doc gia.\n";
		} else if (choice == 9) {
			cout << "Ma sach can xoa: "; string bid; getline(cin, bid);
			if (lib.removeBook(bid)) cout << "Da xoa sach.\n"; else cout << "Xoa that bai (khong tim thay hoac dang duoc muon).\n";
		} else if (choice == 10) {
			cout << "Ma doc gia can xoa: "; string rid; getline(cin, rid);
			if (lib.removeReader(rid)) cout << "Da xoa doc gia.\n"; else cout << "Xoa that bai (khong tim thay).\n";
		} else if (choice == 0) running = false;
	}
	cout << "Tam biet.\n";
	return 0;
}