
#include <iostream>
#include <limits>
#include "include/MatchingEngine.h"

using namespace std;

int main() {
    MatchingEngine engine;
    int choice = 0;

    do {
        cout << "\n========== TRADING ORDER MATCHING SYSTEM ==========" << endl;
        cout << "1. Submit Buy Order" << endl;
        cout << "2. Submit Sell Order" << endl;
        cout << "3. Cancel Order" << endl;
        cout << "4. Display Order Book" << endl;
        cout << "5. Display Trade History" << endl;
        cout << "6. Save Trade History" << endl;
        cout << "7. Exit" << endl;
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            if (cin.eof()) {
                cout << "\nInput ended. Exiting program." << endl;
                return 0;
            }

            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter a number from 1 to 7." << endl;
            continue;
        }

        if (choice == 1 || choice == 2) {
            double price;
            int quantity;

            cout << "Enter price in rupees: ";

            if (!(cin >> price)) {
                if (cin.eof()) {
                    cout << "\nInput ended. Exiting program." << endl;
                    return 0;
                }

                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid price input." << endl;
                continue;
            }

            cout << "Enter quantity: ";

            if (!(cin >> quantity)) {
                if (cin.eof()) {
                    cout << "\nInput ended. Exiting program." << endl;
                    return 0;
                }

                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid quantity input." << endl;
                continue;
            }

            if (price <= 0 || quantity <= 0) {
                cout << "Price and quantity must be positive." << endl;
                continue;
            }

            int priceInPaisa = (int)(price * 100 + 0.5);

            if (choice == 1) {
                engine.submitOrder(BUY, priceInPaisa, quantity);
            } else {
                engine.submitOrder(SELL, priceInPaisa, quantity);
            }

        } else if (choice == 3) {
            int orderID;

            cout << "Enter order ID to cancel: ";

            if (!(cin >> orderID)) {
                if (cin.eof()) {
                    cout << "\nInput ended. Exiting program." << endl;
                    return 0;
                }

                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid order ID." << endl;
                continue;
            }

            engine.cancelOrder(orderID);

        } else if (choice == 4) {
            engine.displayOrderBook();

        } else if (choice == 5) {
            engine.displayTradeHistory();

        } else if (choice == 6) {
            engine.saveTradeHistory();

        } else if (choice == 7) {
            engine.saveTradeHistory();
            cout << "Exiting Trading Order Matching System." << endl;

        } else {
            cout << "Invalid choice. Please enter a number from 1 to 7." << endl;
        }

    } while (choice != 7);

    return 0;
}
