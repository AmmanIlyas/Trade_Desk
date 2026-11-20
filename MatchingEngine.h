
#ifndef MATCHINGENGINE_H
#define MATCHINGENGINE_H

#include <iostream>
#include "AVLTree.h"
#include "DynamicArray.h"
#include "OrderHashTable.h"
#include "FileManager.h"
using namespace std;

class MatchingEngine {
private:

    AVLTree buyTree;
    AVLTree sellTree;
    DynamicArray trades;
    OrderHashTable orderTable;
    FileManager fileManager;

    int nextOrderID;
    int nextTradeID;
    int nextSequence;

    void matchOrders() {
        while (true) {
            PriceNode* buyLevel = buyTree.getBestLevel(true);
            PriceNode* sellLevel = sellTree.getBestLevel(false);

            if (buyLevel == NULL || sellLevel == NULL) {
                break;
            }

            if (buyLevel->price < sellLevel->price) {
                break;
            }

            OrderNode* buyNode = buyLevel->orders.getHead();
            OrderNode* sellNode = sellLevel->orders.getHead();

            int tradeQuantity;

            if (buyNode->order.remainingQuantity <
                sellNode->order.remainingQuantity) {
                tradeQuantity = buyNode->order.remainingQuantity;
            } else {
                tradeQuantity = sellNode->order.remainingQuantity;
            }

            int tradePrice;

            if (buyNode->order.sequence < sellNode->order.sequence) {
                tradePrice = buyNode->order.price;
            } else {
                tradePrice = sellNode->order.price;
            }

            trades.addTrade(Trade(
                nextTradeID,
                buyNode->order.id,
                sellNode->order.id,
                tradePrice,
                tradeQuantity,
                nextSequence
            ));

            nextTradeID++;
            nextSequence++;

           
buyNode->order.remainingQuantity -= tradeQuantity;
sellNode->order.remainingQuantity -= tradeQuantity;
if (buyNode->order.remainingQuantity == 0)
    buyNode->order.status = FILLED;
else
    buyNode->order.status = PARTIALLY_FILLED;

if (sellNode->order.remainingQuantity == 0)
    sellNode->order.status = FILLED;
else
    sellNode->order.status = PARTIALLY_FILLED;

int buyOrderID = buyNode->order.id;
int sellOrderID = sellNode->order.id;

int buyRemaining = buyNode->order.remainingQuantity;
int sellRemaining = sellNode->order.remainingQuantity;

if (buyRemaining == 0) {
    buyTree.removeOrder(buyOrderID);
    orderTable.removeOrder(buyOrderID);
} else {
    orderTable.updateOrderQuantity(buyOrderID, buyRemaining);
}

if (sellRemaining == 0) {
    sellTree.removeOrder(sellOrderID);
    orderTable.removeOrder(sellOrderID);
} else {
    orderTable.updateOrderQuantity(sellOrderID, sellRemaining);
}


        }
    }

public:
   MatchingEngine() {
    nextOrderID = 1;
    nextTradeID = 1;
    nextSequence = 1;

    fileManager.loadTrades(trades, nextTradeID, nextOrderID, nextSequence);
    fileManager.loadOrders(buyTree, sellTree, orderTable, nextOrderID, nextSequence);
    fileManager.loadCounters(nextOrderID, nextTradeID, nextSequence);
}
    void saveTradeHistory() {
    fileManager.saveTrades(trades);
    fileManager.saveOrders(buyTree, sellTree);
    fileManager.saveCounters(nextOrderID, nextTradeID, nextSequence);
}
    void submitOrder(OrderSide side, int price, int quantity) {
        if (price <= 0 || quantity <= 0) {
            cout << "Invalid price or quantity." << endl;
            return;
        }

        Order newOrder(
            nextOrderID,
            nextSequence,
            side,
            price,
            quantity
        );

        nextOrderID++;
        nextSequence++;

       
if (side == BUY) {
    buyTree.addOrder(newOrder);
    orderTable.insertOrder(newOrder.id, newOrder.price,
                           newOrder.remainingQuantity, newOrder.side);
    cout << "Buy order submitted. ID: " << newOrder.id << endl;
} else {
    sellTree.addOrder(newOrder);
    orderTable.insertOrder(newOrder.id, newOrder.price,
                           newOrder.remainingQuantity, newOrder.side);
    cout << "Sell order submitted. ID: " << newOrder.id << endl;
}


        matchOrders();
        saveTradeHistory();
    }

    
bool cancelOrder(int orderID) {
    int price;
    int quantity;
    OrderSide side;

    if (!orderTable.getOrderInfo(orderID, price, quantity, side)) {
        cout << "Order " << orderID
             << " not found or already filled." << endl;
        return false;
    }

    bool removed = false;

    if (side == BUY) {
        removed = buyTree.removeOrder(orderID);
    } else {
        removed = sellTree.removeOrder(orderID);
    }

    if (removed) {
        orderTable.removeOrder(orderID);
        saveTradeHistory();

        if (side == BUY) {
            cout << "Buy order " << orderID
                 << " cancelled successfully." << endl;
        } else {
            cout << "Sell order " << orderID
                 << " cancelled successfully." << endl;
        }

        return true;
    }

    cout << "Order " << orderID
         << " could not be cancelled." << endl;
    return false;
}


   void displayOrderBook() {
    cout << "\n===== BUY ORDERS =====" << endl;
    buyTree.displayAllOrders(BUY);

    cout << "\n===== SELL ORDERS =====" << endl;
    sellTree.displayAllOrders(SELL);
}
    void displayTradeHistory() {
        cout << "\n===== TRADE HISTORY =====" << endl;
        trades.displayTrades();
    }
};

#endif
