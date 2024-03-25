#pragma once
#include <string>
#include <unordered_map>
#include <queue>
#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>



/*
struct FullOrder
{
    size_t      id;
    Side        side;
    double      price;
    size_t      vol;
    
    void print()
    {
        std::cout << id << " | " << (int) side << " | " << price << " | " << vol << "\n";
    }
};

struct AmendOrder
{
    size_t id;
    double price;
    size_t vol;
    
    void print()
    {
        std::cout << id << " | " << price << " | " << vol << "\n";
    }
};
*/


class OrderBook
{
public:
    enum class Side
    {
        BUY = 0,
        SELL
    };

    /**
     * @brief Construct a new Order Book object 
     * 
     * @param symbol_name The name of the order book
     */
    OrderBook( const std::string& symbol_name );


    /**
     * @brief Insert either a buy or sell order in the order book.
     * 
     * @param id     Unique id for this trade.
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
    void Amend ( const size_t id, const double price, const size_t vol );


    /**
     * @brief Remove an existing order from the order book.
     * 
     * @param id  The id of the order to remove from the order book.
     */
    void Pull( const size_t id );
    

    /** TODO: return data structure instead of tuple and string
     * @brief Gives the list matched trades
     * 
     * @return tuple with trade id and the trade as a comma seperated string (symbol, price, volume, aggressive order id, passive order id).
     */
    std::vector< std::pair<size_t, std::string> > GetListOfTrades();


    struct PriceLevel
    {
        // The total volume for a specific buy price
        double buy_price = 0.0;
        size_t buy_vol   = 0;

        // The total volume for a specific sell price
        double sell_price = 0.0;
        size_t sell_vol   = 0;

        auto operator<=>(const PriceLevel&) const = default;
    };

    /**
     * @brief Get the Price Levels object
     * 
     * @return std::vector< std::string > 
     */
    std::vector< PriceLevel > GetPriceLevels();
    
        void PrintOrderBook();


private:

    struct Order
    {
        size_t id;     // global id
        size_t intId;  // internal order book id
        double price;
        size_t vol;
        bool   sell;

        void print()
        {
            std::cout << "[" << id << "|" << price << "|" << vol << "|" << intId << "] x ";
        }
    };

    struct ExecutedTrade
    {
        double price;
        size_t volume;
        size_t aggressive_order_id;
        size_t passive_order_id;
        size_t gId;
    };

    void ExecuteOrders();

    std::unordered_map<size_t, Order> mTrades;
    std::vector<ExecutedTrade> mExecutedTrades;

    std::map< double, std::map<size_t, Order>                       > mSellQueue;
    std::map< double, std::map<size_t, Order>, std::greater<double> > mBuyQueue;

    const std::string mSymbol;
    static size_t mTradeId;
    size_t mIntId { 0 };
};


