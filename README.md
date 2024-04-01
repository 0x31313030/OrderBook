A minimal C++ order book library showing how a simple 'central limit order book' (CLOB) works.


Order Book
==========

An order book is used by exchanges to enable trading by keeping track of all buy and sell orders for a particular asset (such as stocks or cryptocurrencies). Orders are organized by price, with the highest bid prices (buy orders) and the lowest ask prices (sell orders) listed at the top. A matching engine then executes trades by matching buy and sell orders from the top of the order book. A trade occurs when a 'buy order' is equal or higher than a 'sell order'.

A CLOB has two sides, the buy and sell side. An order contains an id, limit price, volume and side. The limit price is the price someone is willing to buy an asset at a given price or lower, or in the case of a sell order, the price to sell at a given price or higher. When the matching engine receives a new order, it checks if it can match it with an existing order on the opposite side of the order book and if so will exhaust the volume until there are no more matching orders, at which point it will save the order in the CLOB with the remaining volume. The received order which caused the trade to trigger is called the 'aggressive order' and the matched order is called the 'passive order'. If multiple orders match with the same price, the oldest order gets matched first (called 'time priority'). The price of the trade is therefore determined by the 'passive order'.

CLOBs support three main operations: 'Insert' adds a new order, 'Amend' modifies either the price or volume of a previous order, and 'Pull' removes a previously added order. An 'Amend' operation results in the loss of the orders 'time priority' unless the only change is a decrease in the volume.


Example
=======

```cpp

UniqueIDGenerator id_gen;        // An ID generator which is used for trades when orders are matched. Can be shared between multiple order books to have unique IDs across order books if needed.
OrderBook goog("GOOG", id_gen);
OrderBook tsla("TSLA", id_gen);

/*
sym,  op,   id,     buy/sell side,     price,  vol */
goog.Insert( 1, OrderBook::Side::BUY,  145.3,  17 );
tsla.Insert( 2, OrderBook::Side::SELL, 201.2, 121 );
tsla.Insert( 3, OrderBook::Side::SELL, 205.5,  68 );
goog.Insert( 4, OrderBook::Side::BUY,  136.1,  12 );
tsla.Insert( 5, OrderBook::Side::SELL, 205.5, 205 );  // Same price as order with ID '3', which has price priority.
tsla.Insert( 6, OrderBook::Side::SELL, 206.9,  41 );
goog.Insert( 7, OrderBook::Side::SELL, 146.2, 130 );
goog.Amend ( 4, 147.0, 50);                           // Change order with ID '4' to a price of 147, causing it to match with order '7'.
tsla.Pull  ( 6 );                                     // Remove order with ID '6'.
tsla.Amend ( 3, 205.5, 75);                           // Change order with ID '3' causing it to loose price priority with order '5'.
tsla.Insert( 8, OrderBook::Side::BUY,  209.8, 300 );  // Matches with order IDs '3' and '5', exhausting all of '3' volume and leaving '5' with a volume of 26.


// get list of exectuted trades for 'GOOG' and 'TSLA'                                                            price | vol | agg. order | pass. order | trade ID
const std::vector< OrderBook::ExecutedTrade >& goog_trades = goog.GetListOfTrades(); // returns a single trade { 146.2,   50,       4,           7,          0     }.
const std::vector< OrderBook::ExecutedTrade >& tsla_trades = tsla.GetListOfTrades(); // returns two trades     { 201.6,  121,       8,           2,          1     } and 
                                                                                     //                        { 205.5,  179,       8,           5,          2     }.


// get price levels for 'GOOG' and 'TSLA'                                                               buy price | buy vol | sell price | sell vol
std::vector<OrderBook::PriceLevel> goog_levels { goog.GetPriceLevels() }; // returns one price level: {   145.3,       17,      146.2,        80    }
std::vector<OrderBook::PriceLevel> tsla_levels { tsla.GetPriceLevels() }; // returns one price level: {       0,        0,      205.5,       100    }

```

Note that order IDs are assigned by the caller and trade IDs by the callee. IDs do not necessarily have to be contiguous but must be unique.



Testing
=======

OrderBook includes unit tests located in the **unit_tests** folder. They can be run like so:

```bash

git clone https://github.com/0x31313030/OrderBook
cd OrderBook
mkdir _build && cd _build
cmake ..
make
./unit_tests/OrderBookTests

```