#pragma once
#include <unordered_map>
#include <queue>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "UniqueIDGenerator.h"

class OrderBook
{
public:

    enum class Side
    {
        BUY = 0,
        SELL
    };


    /**
     * @brief Construct a new Order Book
     * 
     * @param symbol_name The name of the symbol the order book will be handling buy and sell orders for.
     */
    OrderBook( const std::string& symbol_name, UniqueIDGenerator& id_gen );


    /**
     * @brief Insert either a buy or sell order in the order book.
     * 
     * @param id     Unique id for order. Id may not be the same as a previously inserted order. Must be handleded by caller!
     * @param side   Buy or sell side.
     * @param price  Price to sell for or buy at.
     * @param vol    Number of units.
     */
    void Insert( const size_t id, const Side side, const double price, const size_t vol );


    /**
     * @brief Updates an existing order in the order book.
     * 
     * @param id     The id of an existing order to change.
     * @param price  The new price.
     * @param vol    The new volume.
     */
    void Amend( const size_t id, const double price, const size_t vol );


    /**
     * @brief Remove an existing order from the order book.
     * 
     * @param id  The id of the order to remove from the order book.
     */
    void Pull( const size_t id );


    /**
     * @brief Buy/Sell orders which have been matched by order book, resulting in a trade.
     */
    struct ExecutedTrade
    {
        double price;                // Price which resulted in trade.
        size_t volume;               // Number of units traded.
        size_t aggressive_order_id;  // The id of the order which triggered the trade. Can be sell or buy order type.
        size_t passive_order_id;     // The id of the order which matched with the above order. Opposite order type of the above order.
        size_t trade_id;             // The id of the trade. Can be used for ordering trades accross multiple order books depending on 'id_gen'.

        auto operator<=>(const ExecutedTrade&) const = default;
    };

    /**
     * @brief Returns the list of executed trades
     */
    const std::vector< OrderBook::ExecutedTrade >& GetListOfTrades();


    /**
     * @brief The closest sell and buy prices pair
     */
    struct PriceLevel
    {
        // The total volume for a specific buy price
        double buy_price = 0.0;
        size_t buy_vol   = 0;

        // The total volume for the sell price closest to the above buy price
        double sell_price = 0.0;
        size_t sell_vol   = 0;

        auto operator<=>(const PriceLevel&) const = default;
    };


    /**
     * @brief Returns the price levels for all buy and sell prices in the order book.
     *        If for a level, only one side has a bid, then the non bid side will be 
     *        set to 0 volume and price. Sorted in increasing sell price.
     */
    std::vector< PriceLevel > GetPriceLevels();
    
        void PrintOrderBook();


private:

    /**
     * @brief Goes through the sell and buy orders and matches orders
     */
    void ExecuteOrders();


    /**
     * @brief Active order which is waiting to be matched with another order
     */
    struct Order
    {
        size_t id;     // global order id (the one from 'Insert()' method)
        size_t intId;  // internal order book id (used for prioritization)
        double price;  // price which will trigger a trade
        size_t vol;    // number of units which will be traded
        bool   sell;   // sell or buy order

        void print()
        {
            std::cout << "[" << id << "|" << price << "|" << vol << "|" << intId << "] x ";
        }
    };


    const std::string mSymbol;                               // Symbol of order book.
    UniqueIDGenerator& mIdGen;                               // used for generating IDs for executed trades.
    size_t mIntId { 0 };                                     // Used for giving orders 'time priority'.
    std::unordered_map<size_t, Order> mOrders;               // Contains all sell and buy orders. Maps 'order id' -> 'order'. Used for looking up orders when doing 'Amend()' operations.
    std::vector<ExecutedTrade> mExecutedTrades;              // Contains all matched orders which resulted in a trade.

    std::map< double, std::map<size_t, Order> > mSellQueue;  // All sell orders. Given a sell price, will return all active sell orders sorted after 'time priority'.
    std::map< double, std::map<size_t, Order> > mBuyQueue;   // Same as 'mSellQueue' but for buy orders.

    std::map< double, std::map<size_t, Order> >* mQueues[2] { &mBuyQueue, &mSellQueue };
};