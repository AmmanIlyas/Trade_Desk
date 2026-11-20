
#ifndef ORDERHASHTABLE_H
#define ORDERHASHTABLE_H
#include <cstddef>
#include "Order.h"

class OrderHashTable {
private:
    static const int TABLE_SIZE = 1009;

    struct HashNode {
        int orderID;
        int price;
        int remainingQuantity;
        OrderSide side;
        HashNode* next;

        HashNode(int id, int orderPrice, int quantity, OrderSide orderSide) {
            orderID = id;
            price = orderPrice;
            remainingQuantity = quantity;
            side = orderSide;
            next = NULL;
        }
    };

    HashNode* table[TABLE_SIZE];

    int hashFunction(int orderID) {
        if (orderID < 0) {
            orderID = -orderID;
        }

        return orderID % TABLE_SIZE;
    }

public:
    OrderHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = NULL;
        }
    }

    ~OrderHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashNode* current = table[i];

            while (current != NULL) {
                HashNode* temp = current;
                current = current->next;
                delete temp;
            }
        }
    }

    bool insertOrder(int orderID, int price, int quantity, OrderSide side) {
        int index = hashFunction(orderID);
        HashNode* current = table[index];

        while (current != NULL) {
            if (current->orderID == orderID) {
                return false;
            }

            current = current->next;
        }

        HashNode* newNode = new HashNode(orderID, price, quantity, side);
        newNode->next = table[index];
        table[index] = newNode;

        return true;
    }

    bool findOrder(int orderID) {
        int index = hashFunction(orderID);
        HashNode* current = table[index];

        while (current != NULL) {
            if (current->orderID == orderID) {
                return true;
            }

            current = current->next;
        }

        return false;
    }
    
    bool getOrderInfo(int orderID, int &price, int &quantity, OrderSide &side) {
        int index = hashFunction(orderID);
        HashNode* current = table[index];

        while (current != NULL) {
            if (current->orderID == orderID) {
                price = current->price;
                quantity = current->remainingQuantity;
                side = current->side;
                return true;
            }

            current = current->next;
        }

        return false;
    }

    bool updateOrderQuantity(int orderID, int newQuantity) {
        int index = hashFunction(orderID);
        HashNode* current = table[index];

        while (current != NULL) {
            if (current->orderID == orderID) {
                current->remainingQuantity = newQuantity;
                return true;
            }

            current = current->next;
        }

        return false;
    }


    bool removeOrder(int orderID) {
        int index = hashFunction(orderID);
        HashNode* current = table[index];
        HashNode* previous = NULL;

        while (current != NULL) {
            if (current->orderID == orderID) {
                if (previous == NULL) {
                    table[index] = current->next;
                } else {
                    previous->next = current->next;
                }

                delete current;
                return true;
            }

            previous = current;
            current = current->next;
        }

        return false;
    }

    int getTableSize() {
        return TABLE_SIZE;
    }
};

#endif
