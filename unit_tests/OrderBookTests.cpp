#include <gtest/gtest.h>
#include <random>

#include <chrono>

#include "OrderBook.h"

TEST(OrderBookTests, SimpleInsert)
{
    OrderBook book("AAPL");

    book.Insert( 1, OrderBook::Side::BUY, 12.2, 5 );
    book.PrintOrderBook();

    std::vector<std::string> levels = book.GetPriceLevels();

    ASSERT_EQ( 1, levels.size() );
    ASSERT_EQ( "12.2,5,,", levels[0] );
}