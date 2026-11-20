
#ifndef ORDER_H
#define ORDER_H

enum OrderSide {
    BUY,
    SELL
};

enum OrderStatus {
    OPEN,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED
};

struct Order {
    int id;
    int sequence;
    OrderSide side;
    int price;
    int originalQuantity;
    int remainingQuantity;
    OrderStatus status;

    Order() {
        id = 0;
        sequence = 0;
        side = BUY;
        price = 0;
        originalQuantity = 0;
        remainingQuantity = 0;
        status = OPEN;
    }

    Order(int orderID, int orderSequence, OrderSide orderSide,
          int orderPrice, int quantity) {
        id = orderID;
        sequence = orderSequence;
        side = orderSide;
        price = orderPrice;
        originalQuantity = quantity;
        remainingQuantity = quantity;
        status = OPEN;
    }
};

#endif
