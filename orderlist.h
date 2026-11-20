
#ifndef ORDERLIST_H
#define ORDERLIST_H

#include <iostream>
#include "Order.h"

using namespace std;

class OrderNode {
public:
    Order order;
    OrderNode* next;
    OrderNode* prev;

    OrderNode(Order newOrder) {
        order = newOrder;
        next = NULL;
        prev = NULL;
    }
};

class OrderList {
private:
    OrderNode* head;
    OrderNode* tail;
    int count;

public:
    OrderList() {
        head = NULL;
        tail = NULL;
        count = 0;
    }

    ~OrderList() {
        OrderNode* current = head;

        while (current != NULL) {
            OrderNode* temp = current;
            current = current->next;
            delete temp;
        }
    }

    void addOrder(Order newOrder) {
        OrderNode* newNode = new OrderNode(newOrder);

        if (head == NULL) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }

        count++;
    }

    void displayOrders() {
        OrderNode* current = head;

        if (head == NULL) {
            cout << "No orders in this list." << endl;
            return;
        }

        while (current != NULL) {
            cout << "Order ID: " << current->order.id
                 << " | Price: Rs. "
                 << current->order.price / 100.0
                 << " | Remaining Quantity: "
                 << current->order.remainingQuantity
                 << endl;

            current = current->next;
        }
    }
    
    OrderNode* findOrder(int orderID) {
        OrderNode* current = head;

        while (current != NULL) {
            if (current->order.id == orderID) {
                return current;
            }

            current = current->next;
        }

        return NULL;
    }

    bool removeOrder(int orderID) {
        OrderNode* target = findOrder(orderID);

        if (target == NULL) {
            return false;
        }

        if (target == head) {
            head = target->next;
        }

        if (target == tail) {
            tail = target->prev;
        }

        if (target->prev != NULL) {
            target->prev->next = target->next;
        }

        if (target->next != NULL) {
            target->next->prev = target->prev;
        }

        delete target;
        count--;

        return true;
    }
    OrderNode* getHead() {
    return head;
}

bool removeFirst() {
    if (head == NULL) {
        return false;
    }

    return removeOrder(head->order.id);
}


    int getCount() {
        return count;
    }

    bool isEmpty() {
        return head == NULL;
    }
};

#endif
