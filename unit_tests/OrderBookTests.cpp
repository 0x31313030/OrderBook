#include <gtest/gtest.h>

#include "OrderBook.h"
#include "UniqueIDGenerator.h"

TEST(OrderBookTests, SimpleInsert)
{
    UniqueIDGenerator id_gen;
    OrderBook book("MSFT", id_gen);

    /*
    sym,  op,   id,     buy/sell side,     price,  vol */
    book.Insert( 1, OrderBook::Side::BUY,  12.2,    5 );

    // check trades
    std::vector< OrderBook::ExecutedTrade > trades { book.GetListOfTrades() };
    ASSERT_EQ( 0, trades.size() );

    // check price levels
    std::vector<OrderBook::PriceLevel> levels { book.GetPriceLevels() };
    ASSERT_EQ( 1, levels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(12.2, 5, 0, 0), levels[0] );
}


TEST(OrderBookTests, SimpleMatchWithAggressiveSell)
{
    UniqueIDGenerator id_gen;
    OrderBook book("MSFT", id_gen);

    /*
    sym,  op,   id,     buy/sell side,     price,  vol */
    book.Insert( 1, OrderBook::Side::BUY,  12.2,    5 );
    book.Insert( 2, OrderBook::Side::SELL, 12.1,    8 );

    // check trades
    std::vector< OrderBook::ExecutedTrade > trades = book.GetListOfTrades();
    ASSERT_EQ( 1, trades.size() );
    ASSERT_EQ( OrderBook::ExecutedTrade(12.2, 5, 2, 1, 0), trades[0] );

    // check price levels
    std::vector<OrderBook::PriceLevel> levels { book.GetPriceLevels() };
    ASSERT_EQ( 1, levels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(0, 0, 12.1, 3), levels[0] );
}


TEST(OrderBookTests, SimpleMatchWithAggressiveBuy)
{
    UniqueIDGenerator id_gen;
    OrderBook book("MSFT", id_gen);

    /*
    sym,  op,   id,     buy/sell side,     price,  vol */
    book.Insert( 1, OrderBook::Side::SELL, 12.1,    8 );
    book.Insert( 2, OrderBook::Side::BUY,  12.2,    5 );

    // check trades
    std::vector< OrderBook::ExecutedTrade > trades = book.GetListOfTrades();
    ASSERT_EQ( 1, trades.size() );
    ASSERT_EQ( OrderBook::ExecutedTrade(12.1, 5, 2, 1, 0), trades[0] );

    // check price levels
    std::vector<OrderBook::PriceLevel> levels { book.GetPriceLevels() };
    ASSERT_EQ( 1, levels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(0, 0, 12.1, 3), levels[0] );
}


TEST(OrderBookTests, SingleSymbolMultiInsertAndMultiMatch)
{
    UniqueIDGenerator id_gen;
    OrderBook book("MSFT", id_gen);

    /*
    sym,  op,   id,     buy/sell side,     price,  vol */
    book.Insert(8, OrderBook::Side::BUY,  14.235,  5 );
    book.Insert(6, OrderBook::Side::BUY,  14.235,  6 );
    book.Insert(7, OrderBook::Side::BUY,  14.235, 12 );
    book.Insert(2, OrderBook::Side::BUY,  14.234,  5 );
    book.Insert(1, OrderBook::Side::BUY,  14.23,   3 );
    book.Insert(5, OrderBook::Side::SELL, 14.237,  8 );
    book.Insert(3, OrderBook::Side::SELL, 14.24,   9 );
    book.Pull(8);
    book.Insert(4, OrderBook::Side::SELL, 14.234, 25 );

    // check trades
    std::vector< OrderBook::ExecutedTrade > trades = book.GetListOfTrades();
    ASSERT_EQ( 3, trades.size() );
    ASSERT_EQ( OrderBook::ExecutedTrade(14.235,  6, 4, 6, 0), trades[0] );
    ASSERT_EQ( OrderBook::ExecutedTrade(14.235, 12, 4, 7, 1), trades[1] );
    ASSERT_EQ( OrderBook::ExecutedTrade(14.234,  5, 4, 2, 2), trades[2] );

    // check price levels
    std::vector<OrderBook::PriceLevel> levels { book.GetPriceLevels() };
    ASSERT_EQ( 3, levels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(14.23, 3, 14.234, 2), levels[0] );
    ASSERT_EQ( OrderBook::PriceLevel(    0, 0, 14.237, 8), levels[1] );
    ASSERT_EQ( OrderBook::PriceLevel(    0, 0, 14.24,  9), levels[2] );
}

TEST(OrderBookTests, MultiSymbolMultiInsertAndMultiMatch)
{
    UniqueIDGenerator id_gen;
    OrderBook msft("MSFT", id_gen);
    OrderBook nvda("NVDA", id_gen);
    OrderBook goog("GOOG", id_gen);
    OrderBook tsla("TSLA", id_gen);
    
    /*
    sym,  op,   id,     buy/sell side,        price,  vol */
    msft.Insert(  1, OrderBook::Side::BUY,    0.3854,  5 );
    nvda.Insert(  2, OrderBook::Side::BUY,  412,      31 );
    nvda.Insert(  3, OrderBook::Side::BUY,  410.5,    27 );
    goog.Insert(  4, OrderBook::Side::SELL,  21,       8 );
    tsla.Insert(  6, OrderBook::Side::SELL,  15,       5 );
    msft.Insert( 11, OrderBook::Side::SELL,   0.3854,  4 );
    msft.Insert( 13, OrderBook::Side::SELL,   0.3853,  6 );
    tsla.Insert(  7, OrderBook::Side::BUY,   18,       5 );


    // check trades
    const std::vector< OrderBook::ExecutedTrade >& msftTrades = msft.GetListOfTrades();
    const std::vector< OrderBook::ExecutedTrade >& nvdaTrades = nvda.GetListOfTrades();
    const std::vector< OrderBook::ExecutedTrade >& googTrades = goog.GetListOfTrades();
    const std::vector< OrderBook::ExecutedTrade >& tslaTrades = tsla.GetListOfTrades();

    ASSERT_EQ( 2, msftTrades.size() );
    ASSERT_EQ( 0, nvdaTrades.size() );
    ASSERT_EQ( 0, googTrades.size() );
    ASSERT_EQ( 1, tslaTrades.size() );

    ASSERT_EQ( OrderBook::ExecutedTrade(0.3854, 4, 11, 1, 0), msftTrades[0] );
    ASSERT_EQ( OrderBook::ExecutedTrade(0.3854, 1, 13, 1, 1), msftTrades[1] );
    ASSERT_EQ( OrderBook::ExecutedTrade(15,     5,  7, 6, 2), tslaTrades[0] );

    // check price levels
    std::vector<OrderBook::PriceLevel> msftLevels { msft.GetPriceLevels() };
    std::vector<OrderBook::PriceLevel> nvdaLevels { nvda.GetPriceLevels() };
    std::vector<OrderBook::PriceLevel> googLevels { goog.GetPriceLevels() };

    ASSERT_EQ( 1, msftLevels.size() );
    ASSERT_EQ( 2, nvdaLevels.size() );
    ASSERT_EQ( 1, googLevels.size() );

    ASSERT_EQ( OrderBook::PriceLevel(0,0,  21,8), googLevels[0] );
    ASSERT_EQ( OrderBook::PriceLevel(412,31,  0,0), nvdaLevels[0] );
    ASSERT_EQ( OrderBook::PriceLevel(410.5,27,  0,0), nvdaLevels[1] );
    ASSERT_EQ( OrderBook::PriceLevel(0,0,  0.3853,5), msftLevels[0] );
}

TEST(OrderBookTests, InsertAndAmend)
{
    UniqueIDGenerator id_gen;
    OrderBook msft("MSFT", id_gen);

    /*
    sym,  op,   id,     buy/sell side,       price,  vol */
    msft.Insert(  1, OrderBook::Side::BUY,    45.95,  5 );
    msft.Insert(  2, OrderBook::Side::BUY,    45.95,  6 );
    msft.Insert(  3, OrderBook::Side::BUY,    45.95,  12 );
    msft.Insert(  4, OrderBook::Side::SELL,   46,     8 );
    msft.Amend (  2,                          46,     3 );
    msft.Insert(  5, OrderBook::Side::SELL,   45.95,  1 );
    msft.Amend (  1,                          45.95,  1 );
    msft.Insert(  6, OrderBook::Side::SELL,   45.95,  1 );
    msft.Amend (  1,                          45.95,  5 );
    msft.Insert(  7, OrderBook::Side::SELL,   45.95,  1 );

    // check trades
    std::vector< OrderBook::ExecutedTrade > trades = msft.GetListOfTrades();
    ASSERT_EQ( 4, trades.size() );
    ASSERT_EQ( OrderBook::ExecutedTrade(46,3,2,4,0),    trades[0] );
    ASSERT_EQ( OrderBook::ExecutedTrade(45.95,1,5,1,1), trades[1] );
    ASSERT_EQ( OrderBook::ExecutedTrade(45.95,1,6,1,2), trades[2] );
    ASSERT_EQ( OrderBook::ExecutedTrade(45.95,1,7,3,3), trades[3] );


    // check price levels
    std::vector<OrderBook::PriceLevel> levels { msft.GetPriceLevels() };
    ASSERT_EQ( 1, levels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(45.95,16,  46,5), levels[0] );

}


TEST(OrderBookTests, TradeCorrectlyLoosesPricePriority)
{
    UniqueIDGenerator id_gen;        // An ID generator which is used for trades when orders are matched. Can be shared between multiple order books to have unique IDs across order books if needed.
    OrderBook goog("GOOG", id_gen);
    OrderBook tsla("TSLA", id_gen);

    /*
    sym,  op,   id,     buy/sell side,     price,  vol */
    goog.Insert( 1, OrderBook::Side::BUY,  145.3,  17 );
    tsla.Insert( 2, OrderBook::Side::SELL, 201.2, 121 );
    tsla.Insert( 3, OrderBook::Side::SELL, 205.5,  68 );
    goog.Insert( 4, OrderBook::Side::BUY,  136.1,  12 );
    tsla.Insert( 5, OrderBook::Side::SELL, 205.5, 204 );  // Same price as order with ID '3', which has price priority.
    tsla.Insert( 6, OrderBook::Side::SELL, 206.9,  41 );
    goog.Insert( 7, OrderBook::Side::SELL, 146.2, 130 );
    goog.Amend ( 4, 147.0, 50);                           // Change order with ID '4', to a price of 147, causing it to match with order '7'.
    tsla.Pull  ( 6 );                                     // Remove order with ID '6'
    tsla.Amend ( 3, 205.5, 75);                           // Change order with ID '3' causing it to loose its price priority with order '5'
    tsla.Insert( 8, OrderBook::Side::BUY, 209.8,  300 );  // Matches with order IDs '2' and '5', exhausting all of '2' volume and leaving '5' with a volume of 25.


    // get list of exectuted trades for 'GOOG'
    const std::vector< OrderBook::ExecutedTrade >& googTrades = goog.GetListOfTrades();
    ASSERT_EQ( 1, googTrades.size() );
    ASSERT_EQ( OrderBook::ExecutedTrade(146.2, 50, 4, 7, 0), googTrades[0] );


    // get list of exectuted trades for 'TSLA'
    const std::vector< OrderBook::ExecutedTrade >& tslaTrades = tsla.GetListOfTrades(); // returns two trades { 201.6, 121, 8, 2, 1 } and { 205.5, 179, 8, 5, 2 }
    ASSERT_EQ( 2, tslaTrades.size() );
    ASSERT_EQ( OrderBook::ExecutedTrade(201.2, 121, 8, 2, 1), tslaTrades[0] );
    ASSERT_EQ( OrderBook::ExecutedTrade(205.5, 179, 8, 5, 2), tslaTrades[1] );


    // get price levels for 'GOOG'
    std::vector<OrderBook::PriceLevel> googLevels { goog.GetPriceLevels() };
    ASSERT_EQ( 1, googLevels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(145.3, 17, 146.2, 80), googLevels[0] );


    // get price levels for 'TSLA'
    std::vector<OrderBook::PriceLevel> tslaLevels { tsla.GetPriceLevels() };
    ASSERT_EQ( 1, tslaLevels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(0,0, 205.5, 100), tslaLevels[0] );
}