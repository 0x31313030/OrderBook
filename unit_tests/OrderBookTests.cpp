#include <gtest/gtest.h>
#include <random>

#include <chrono>

#include "OrderBook.h"

TEST(OrderBookTests, SimpleInsert)
{
    OrderBook book("MSFT");

    book.Insert( 1, OrderBook::Side::BUY, 12.2, 5 );

    auto trades = book.GetListOfTrades();

    ASSERT_EQ( 0, trades.size() );

    std::vector<OrderBook::PriceLevel> levels  { book.GetPriceLevels() };

    ASSERT_EQ( 1, levels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(12.2, 5, 0, 0), levels[0] );
}

TEST(OrderBookTests, SimpleMatchWithAggressiveSell)
{
    OrderBook book("MSFT");

    book.Insert( 1, OrderBook::Side::BUY,  12.2, 5 ); //TODO: why is this not detected!?
    book.Insert( 1, OrderBook::Side::SELL, 12.1, 8 );

    auto trades = book.GetListOfTrades();

    ASSERT_EQ( 1, trades.size() );

    std::vector<OrderBook::PriceLevel> levels  { book.GetPriceLevels() };

    ASSERT_EQ( 1, levels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(0, 0, 12.1, 3), levels[0] );
}


TEST(OrderBookTests, SimpleMatchWithAggressiveBuy)
{
    OrderBook book("MSFT");

    book.Insert( 1, OrderBook::Side::SELL, 12.1, 8 );
    book.Insert( 2, OrderBook::Side::BUY,  12.2, 5 );

    std::vector<OrderBook::PriceLevel> levels  { book.GetPriceLevels() };

    ASSERT_EQ( 1, levels.size() );
    ASSERT_EQ( OrderBook::PriceLevel(0, 0, 12.1, 3), levels[0] );
}

