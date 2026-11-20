
#ifndef TRADE_H
#define TRADE_H

struct Trade {
    int tradeID;
    int buyOrderID;
    int sellOrderID;
    int price;
    int quantity;
    int sequence;

    Trade() {
        tradeID = 0;
        buyOrderID = 0;
        sellOrderID = 0;
        price = 0;
        quantity = 0;
        sequence = 0;
    }

    Trade(int id, int buyID, int sellID, int tradePrice,
          int tradeQuantity, int tradeSequence) {
        tradeID = id;
        buyOrderID = buyID;
        sellOrderID = sellID;
        price = tradePrice;
        quantity = tradeQuantity;
        sequence = tradeSequence;
    }
};

#endif
