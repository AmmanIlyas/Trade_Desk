
#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include <iostream>
#include "Trade.h"

using namespace std;

class DynamicArray {
private:
    Trade* arr;
    int capacity;
    int size;

    void resize() {
        int newCapacity = capacity * 2;

        Trade* newArr = new Trade[newCapacity];

        for (int i = 0; i < size; i++) {
            newArr[i] = arr[i];
        }

        delete[] arr;

        arr = newArr;
        capacity = newCapacity;
    }

public:
    DynamicArray() {
        capacity = 2;
        size = 0;
        arr = new Trade[capacity];
    }

    ~DynamicArray() {
        delete[] arr;
    }

    void addTrade(Trade newTrade) {
        if (size == capacity) {
            resize();
        }

        arr[size] = newTrade;
        size++;
    }

    int getSize() {
        return size;
    }

    Trade* getTrade(int index) {
        if (index < 0 || index >= size) {
            return nullptr;
        }

        return &arr[index];
    }

    void displayTrades() {
        if (size == 0) {
            cout << "No trades recorded." << endl;
            return;
        }

        for (int i = 0; i < size; i++) {
            cout << "Trade ID: " << arr[i].tradeID
                 << " | Buy Order: " << arr[i].buyOrderID
                 << " | Sell Order: " << arr[i].sellOrderID
                 << " | Price: Rs. " << arr[i].price / 100.0
                 << " | Quantity: " << arr[i].quantity
                 << endl;
        }
    }
};

#endif
