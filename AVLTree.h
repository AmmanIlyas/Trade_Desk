
#ifndef AVLTREE_H
#define AVLTREE_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include "OrderList.h"

using namespace std;

class PriceNode {
public:
    int price;
    int height;
    OrderList orders;
    PriceNode* left;
    PriceNode* right;

    PriceNode(int newPrice) {
        price = newPrice;
        height = 1;
        left = NULL;
        right = NULL;
    }
};

class AVLTree {
private:
    PriceNode* root;

    int getHeight(PriceNode* node) {
        if (node == NULL) {
            return 0;
        }

        return node->height;
    }

    int getBalance(PriceNode* node) {
        if (node == NULL) {
            return 0;
        }

        return getHeight(node->left) - getHeight(node->right);
    }

    void updateHeight(PriceNode* node) {
        int leftHeight = getHeight(node->left);
        int rightHeight = getHeight(node->right);

        if (leftHeight > rightHeight) {
            node->height = leftHeight + 1;
        } else {
            node->height = rightHeight + 1;
        }
    }

    PriceNode* rotateRight(PriceNode* y) {
        PriceNode* x = y->left;
        PriceNode* temp = x->right;

        x->right = y;
        y->left = temp;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    PriceNode* rotateLeft(PriceNode* x) {
        PriceNode* y = x->right;
        PriceNode* temp = y->left;

        y->left = x;
        x->right = temp;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    PriceNode* insert(PriceNode* node, int price) {
        if (node == NULL) {
            return new PriceNode(price);
        }

        if (price < node->price) {
            node->left = insert(node->left, price);
        } else if (price > node->price) {
            node->right = insert(node->right, price);
        } else {
            return node;
        }

        updateHeight(node);

        int balance = getBalance(node);

        if (balance > 1 && price < node->left->price) {
            return rotateRight(node);
        }

        if (balance < -1 && price > node->right->price) {
            return rotateLeft(node);
        }

        if (balance > 1 && price > node->left->price) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }

        if (balance < -1 && price < node->right->price) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    PriceNode* search(PriceNode* node, int price) {
        if (node == NULL || node->price == price) {
            return node;
        }

        if (price < node->price) {
            return search(node->left, price);
        }

        return search(node->right, price);
    }

    void displayInOrder(PriceNode* node) {
        if (node == NULL) {
            return;
        }

        displayInOrder(node->left);

        cout << "Price: Rs. " << fixed << setprecision(2)
             << node->price / 100.0
             << " | Height: " << node->height
             << " | Orders: " << node->orders.getCount()
             << endl;

        displayInOrder(node->right);
    }

    void writeOrders(PriceNode* node, ofstream& file, OrderSide side) {
        if (node == NULL) {
            return;
        }

        writeOrders(node->left, file, side);

        OrderNode* current = node->orders.getHead();

        while (current != NULL) {
            Order order = current->order;

            file << order.id << " "
                 << order.sequence << " "
                 << (int)side << " "
                 << order.price << " "
                 << order.originalQuantity << " "
                 << order.remainingQuantity << " "
                 << (int)order.status << "\n";

            current = current->next;
        }

        writeOrders(node->right, file, side);
    }

    void displayOrdersInOrder(PriceNode* node, OrderSide side) {
        if (node == NULL) {
            return;
        }

        displayOrdersInOrder(node->left, side);

        OrderNode* current = node->orders.getHead();

        while (current != NULL) {
            Order order = current->order;

            cout << "Order ID: " << order.id
                 << " | Side: " << (side == BUY ? "BUY" : "SELL")
                 << " | Price: Rs. " << fixed << setprecision(2)
                 << order.price / 100.0
                 << " | Original Quantity: " << order.originalQuantity
                 << " | Remaining Quantity: " << order.remainingQuantity
                 << " | Status: ";

            if (order.status == OPEN) {
                cout << "OPEN";
            } else if (order.status == PARTIALLY_FILLED) {
                cout << "PARTIALLY FILLED";
            } else if (order.status == FILLED) {
                cout << "FILLED";
            } else {
                cout << "CANCELLED";
            }

            cout << endl;
            current = current->next;
        }

        displayOrdersInOrder(node->right, side);
    }

    PriceNode* findBestLevel(PriceNode* node, bool isBuyTree) {
        if (node == NULL) {
            return NULL;
        }

        PriceNode* result;

        if (isBuyTree) {
            result = findBestLevel(node->right, isBuyTree);
        } else {
            result = findBestLevel(node->left, isBuyTree);
        }

        if (result != NULL) {
            return result;
        }

        if (!node->orders.isEmpty()) {
            return node;
        }

        if (isBuyTree) {
            return findBestLevel(node->left, isBuyTree);
        }

        return findBestLevel(node->right, isBuyTree);
    }

    PriceNode* findOrderLevel(PriceNode* node, int orderID) {
        if (node == NULL) {
            return NULL;
        }

        if (node->orders.findOrder(orderID) != NULL) {
            return node;
        }

        PriceNode* result = findOrderLevel(node->left, orderID);

        if (result != NULL) {
            return result;
        }

        return findOrderLevel(node->right, orderID);
    }

    PriceNode* rebalance(PriceNode* node) {
        if (node == NULL) {
            return NULL;
        }

        updateHeight(node);

        int balance = getBalance(node);

        if (balance > 1) {
            if (getBalance(node->left) < 0) {
                node->left = rotateLeft(node->left);
            }

            return rotateRight(node);
        }

        if (balance < -1) {
            if (getBalance(node->right) > 0) {
                node->right = rotateRight(node->right);
            }

            return rotateLeft(node);
        }

        return node;
    }

    PriceNode* detachMin(PriceNode* node, PriceNode*& minNode) {
        if (node->left == NULL) {
            minNode = node;

            PriceNode* rightSubtree = node->right;
            minNode->right = NULL;

            return rightSubtree;
        }

        node->left = detachMin(node->left, minNode);

        return rebalance(node);
    }

    PriceNode* removePriceNode(PriceNode* node, int price) {
        if (node == NULL) {
            return NULL;
        }

        if (price < node->price) {
            node->left = removePriceNode(node->left, price);
        } else if (price > node->price) {
            node->right = removePriceNode(node->right, price);
        } else {
            if (node->left == NULL) {
                PriceNode* temp = node->right;
                node->right = NULL;
                delete node;
                return temp;
            }

            if (node->right == NULL) {
                PriceNode* temp = node->left;
                node->left = NULL;
                delete node;
                return temp;
            }

            PriceNode* successor = NULL;
            PriceNode* newRight = detachMin(node->right, successor);

            successor->left = node->left;
            successor->right = newRight;

            node->left = NULL;
            node->right = NULL;

            delete node;

            return rebalance(successor);
        }

        return rebalance(node);
    }

    void destroyTree(PriceNode* node) {
        if (node == NULL) {
            return;
        }

        destroyTree(node->left);
        destroyTree(node->right);

        delete node;
    }

public:
    AVLTree() {
        root = NULL;
    }

    ~AVLTree() {
        destroyTree(root);
    }

    void insertPrice(int price) {
        root = insert(root, price);
    }

    bool addOrder(Order newOrder) {
        insertPrice(newOrder.price);

        PriceNode* level = findPrice(newOrder.price);

        if (level == NULL) {
            return false;
        }

        level->orders.addOrder(newOrder);
        return true;
    }

    PriceNode* findPrice(int price) {
        return search(root, price);
    }

    PriceNode* getBestLevel(bool isBuyTree) {
        return findBestLevel(root, isBuyTree);
    }

    bool removeOrder(int orderID) {
        PriceNode* level = findOrderLevel(root, orderID);

        if (level == NULL) {
            return false;
        }

        int price = level->price;

        if (!level->orders.removeOrder(orderID)) {
            return false;
        }

        if (level->orders.isEmpty()) {
            root = removePriceNode(root, price);
        }

        return true;
    }

    int getBestPrice(bool isBuyTree) {
        if (root == NULL) {
            return -1;
        }

        PriceNode* current = root;

        if (isBuyTree) {
            while (current->right != NULL) {
                current = current->right;
            }
        } else {
            while (current->left != NULL) {
                current = current->left;
            }
        }

        return current->price;
    }

    void saveOrdersToFile(ofstream& file, OrderSide side) {
        writeOrders(root, file, side);
    }

    void displayPrices() {
        if (root == NULL) {
            cout << "The price tree is empty." << endl;
            return;
        }

        displayInOrder(root);
    }

    void displayAllOrders(OrderSide side) {
        if (root == NULL) {
            cout << "No outstanding orders." << endl;
            return;
        }

        displayOrdersInOrder(root, side);
    }
};

#endif
