
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include "DynamicArray.h"
#include "AVLTree.h"
#include "OrderHashTable.h"

using namespace std;

class FileManager {
public:
    void saveTrades(DynamicArray &trades) {
        ofstream report("data/trades.txt");
        ofstream dataFile("data/trades_data.txt");

        if (!report.is_open() || !dataFile.is_open()) {
            cout << "Error: Could not save trade files. Check the data folder."
                 << endl;
            return;
        }

        report << "========================================\n";
        report << "       TRADING SYSTEM - TRADE REPORT\n";
        report << "========================================\n\n";
        report << "Total Trades: " << trades.getSize() << "\n\n";

        for (int i = 0; i < trades.getSize(); i++) {
            Trade* trade = trades.getTrade(i);

            if (trade != NULL) {
                report << "----------------------------------------\n";
                report << "Trade ID:       " << trade->tradeID << "\n";
                report << "Buy Order ID:   " << trade->buyOrderID << "\n";
                report << "Sell Order ID:  " << trade->sellOrderID << "\n";
                report << "Trade Price:    Rs. "
                       << fixed << setprecision(2)
                       << trade->price / 100.0 << "\n";
                report << "Quantity:       " << trade->quantity << " units\n";
                report << "Trade Sequence: " << trade->sequence << "\n";
                report << "Total Value:    Rs. "
                       << (trade->price / 100.0) * trade->quantity
                       << "\n\n";

                dataFile << trade->tradeID << " "
                         << trade->buyOrderID << " "
                         << trade->sellOrderID << " "
                         << trade->price << " "
                         << trade->quantity << " "
                         << trade->sequence << "\n";
            }
        }

        report << "----------------------------------------\n";
        report << "End of Trade Report\n";

        report.close();
        dataFile.close();

        cout << "Trade report saved to data/trades.txt." << endl;
        cout << "Trade data saved to data/trades_data.txt." << endl;
    }

    void loadTrades(DynamicArray &trades,
                int &nextTradeID,
                int &nextOrderID,
                int &nextSequence) {
        ifstream dataFile("data/trades_data.txt");

        if (!dataFile.is_open()) {
            cout << "No previous trade data found. Starting fresh."
                 << endl;
            return;
        }

        int tradeID;
        int buyOrderID;
        int sellOrderID;
        int price;
        int quantity;
        int sequence;
        int count = 0;

        while (dataFile >> tradeID
                        >> buyOrderID
                        >> sellOrderID
                        >> price
                        >> quantity
                        >> sequence) {
            Trade trade(
                tradeID,
                buyOrderID,
                sellOrderID,
                price,
                quantity,
                sequence
            );

            trades.addTrade(trade);
            count++;

            if (tradeID >= nextTradeID) {
                nextTradeID = tradeID + 1;
            }
            if (buyOrderID >= nextOrderID) {
    nextOrderID = buyOrderID + 1;
}

if (sellOrderID >= nextOrderID) {
    nextOrderID = sellOrderID + 1;
}

            if (sequence >= nextSequence) {
                nextSequence = sequence + 1;
            }
        }

        dataFile.close();

        cout << count << " previous trade(s) loaded." << endl;
    }
    
void saveOrders(AVLTree& buyTree, AVLTree& sellTree) {
    ofstream file("data/orders_data.txt");

    if (!file.is_open()) {
        cout << "Error: Could not save outstanding orders."
             << endl;
        return;
    }

    buyTree.saveOrdersToFile(file, BUY);
    sellTree.saveOrdersToFile(file, SELL);

    file.close();

    cout << "Outstanding orders saved to data/orders_data.txt."
         << endl;
}

void loadOrders(AVLTree& buyTree, AVLTree& sellTree,OrderHashTable& orderTable,
                int& nextOrderID, int& nextSequence) {
    ifstream file("data/orders_data.txt");

    if (!file.is_open()) {
        cout << "No previous outstanding orders found."
             << endl;
        return;
    }

    int id, sequence, sideValue, price;
    int originalQuantity, remainingQuantity, statusValue;
    int count = 0;

    while (file >> id >> sequence >> sideValue >> price
                >> originalQuantity >> remainingQuantity >> statusValue) {
        if ((sideValue != (int)BUY && sideValue != (int)SELL) ||
            price <= 0 || originalQuantity <= 0 ||
            remainingQuantity <= 0 ||
            remainingQuantity > originalQuantity ||
            statusValue < (int)OPEN ||
            statusValue > (int)CANCELLED) {
            cout << "Skipping invalid saved order record." << endl;
            continue;
        }

        OrderSide side = (OrderSide)sideValue;
        OrderStatus status = (OrderStatus)statusValue;

        if (status == FILLED || status == CANCELLED) {
            continue;
        }

        Order order;
        order.id = id;
        order.sequence = sequence;
        order.side = side;
        order.price = price;
        order.originalQuantity = originalQuantity;
        order.remainingQuantity = remainingQuantity;
        order.status = status;

        bool added;

if (side == BUY) {
    added = buyTree.addOrder(order);
} else {
    added = sellTree.addOrder(order);
}

if (!added) {
    continue;
}

orderTable.insertOrder(
    order.id,
    order.price,
    order.remainingQuantity,
    order.side
);

        if (id >= nextOrderID) {
            nextOrderID = id + 1;
        }

        if (sequence >= nextSequence) {
            nextSequence = sequence + 1;
        }

        count++;
    }

    file.close();

    cout << count << " outstanding order(s) loaded." << endl;
}
void saveCounters(int nextOrderID, int nextTradeID, int nextSequence) {
    ofstream file("data/counters.txt");

    if (file.is_open()) {
        file << nextOrderID << endl;
        file << nextTradeID << endl;
        file << nextSequence << endl;
        file.close();
    }
}

void loadCounters(int &nextOrderID, int &nextTradeID, int &nextSequence) {
    ifstream file("data/counters.txt");

    if (file.is_open()) {
        int orderID, tradeID, sequence;

        if (file >> orderID >> tradeID >> sequence) {
            nextOrderID = orderID;
            nextTradeID = tradeID;
            nextSequence = sequence;
        }

        file.close();
    }
}
};

#endif
