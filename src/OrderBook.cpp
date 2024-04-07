#include "OrderBook.h"


void OrderBook::PrintOrderBook()
{
    printf("orderbook ----------------------------------------- \n");
    printf("SELL:\n");
    for( auto [price, orders] : mSellQueue )
    {
        printf( "%f: ", price );

        for( auto [intId, order] : orders )
        {
            order.print();
        }

        printf("\n");
    }

    printf("BUY:\n");
    for( auto [price, orders] : mBuyQueue )
    {
        printf( "%f: ", price );

        for( auto [intId, order] : orders )
        {
            order.print();
        }

        printf("\n");
    }
}


OrderBook::OrderBook(const std::string& symbol, UniqueIDGenerator& id_gen) 
: mSymbol { symbol },
  mIdGen  { id_gen }
{

}

void OrderBook::Insert( const size_t id, const Side side, const double price, const size_t vol )
{
    Order order { id, mIntId, price, vol, Side::SELL == side };
    ++mIntId;

    mOrders.insert( {order.id, order} );

    auto& queue = *mQueues[size_t(order.sell)];
    queue[order.price].insert( {order.intId, order} );

    ExecuteOrders( );
}



void OrderBook::Amend( const size_t id, const double price, const size_t vol )
{
    // lambda helper function
    auto UpdateQueue = [this](auto& queue, Order& order, const double price, const size_t vol )
    {
        bool price_is_different = ( price != order.price );
        bool vol_increase       = ( vol   >  order.vol   );

        const bool order_loses_time_priority = vol_increase || price_is_different;
        if( order_loses_time_priority )
        {
            // update internal id of order and reinsert in map to give it lower priority
            const double old_price = order.price;
            auto& order_map = queue[old_price];
            order_map.erase( order.intId );
            order.intId = mIntId;
            ++mIntId;

            order.price = price;
            order.vol   = vol;
            
            queue[order.price].insert( {order.intId, order} );

            if( order_map.empty() )
            {
                queue.erase( old_price );
            }
        }
        else
        {
            // priority stays the same, only update volume
            auto& order_map              = queue[order.price];
            order_map[order.intId].vol   = vol;
            order.vol                    = vol;
        }

        if( price_is_different )
        {
            ExecuteOrders( );
        }
    };

    Order& order = mOrders[id];
    UpdateQueue( *mQueues[ size_t(order.sell) ], order, price, vol );
}



void OrderBook::Pull( const size_t id )
{
    if( auto it = mOrders.find( id ); it != mOrders.end() )
    {
        const Order& order = it->second;

        auto EraseOrderFromQueue = [](auto& queue, const Order& order)
        {
            auto& order_map = queue[order.price];
            order_map.erase( order.intId );

            if( order_map.empty() )
            {
                queue.erase( order.price );
            }
        };

        EraseOrderFromQueue( *mQueues[ size_t(order.sell) ], order );
        mOrders.erase(it);
    }
}


void OrderBook::ExecuteOrders()
{
    while( !mBuyQueue.empty() && !mSellQueue.empty() )
    {
        auto itHighestBuyPrices = mBuyQueue.rbegin(); // reverse iterator because we want highest price first
        auto itLowestSellPrices = mSellQueue.begin();

        const double buyPrice   = itHighestBuyPrices->first;
        const double sellPrice  = itLowestSellPrices->first;

        if( buyPrice < sellPrice ) { break; }

        Order& highestBuyOrder  = itHighestBuyPrices->second.begin()->second;
        Order& lowestSellOrder  = itLowestSellPrices->second.begin()->second;

        size_t tradeVol = std::min( highestBuyOrder.vol, lowestSellOrder.vol );

        bool buySideIsPassive = highestBuyOrder.intId < lowestSellOrder.intId;

        const auto& [passiveOrder, aggressiveOrder] = buySideIsPassive ? std::tuple{ highestBuyOrder, lowestSellOrder } : std::tuple{ lowestSellOrder, highestBuyOrder };

        // store trade for later
        ExecutedTrade trade { passiveOrder.price, tradeVol, aggressiveOrder.id, passiveOrder.id, mIdGen.GenerateID() };
        mExecutedTrades.push_back( trade );

        highestBuyOrder.vol -= tradeVol;
        lowestSellOrder.vol -= tradeVol;

        // remove order if no more volume
        if( 0 == highestBuyOrder.vol )
        {
            itHighestBuyPrices->second.erase( itHighestBuyPrices->second.begin() );

            if( itHighestBuyPrices->second.empty() )
            {
                mBuyQueue.erase( std::next(itHighestBuyPrices).base() );
            }
        }

        if( 0 == lowestSellOrder.vol )
        {
            itLowestSellPrices->second.erase( itLowestSellPrices->second.begin() );

            if( itLowestSellPrices->second.empty() )
            {
                mSellQueue.erase( itLowestSellPrices );
            }
        }
    }
}

const std::vector< OrderBook::ExecutedTrade >& OrderBook::GetListOfTrades()
{
    return mExecutedTrades;
}


std::vector< OrderBook::PriceLevel > OrderBook::GetPriceLevels()
{
    std::vector< PriceLevel > result;

    auto itBuy  = mBuyQueue.crbegin();
    auto itSell = mSellQueue.cbegin();

    while( itBuy != mBuyQueue.crend() || itSell != mSellQueue.cend() )
    {
        PriceLevel level;

        if( itBuy != mBuyQueue.crend() )
        {
            size_t total_vol = 0;
            for( auto [intId, order] : itBuy->second )
            {
                total_vol += order.vol;
            }

            level.buy_price = itBuy->first;
            level.buy_vol   = total_vol;

            ++itBuy;
        }

        if( itSell != mSellQueue.cend() )
        {
            size_t total_vol = 0;
            for( auto [intId, order] : itSell->second )
            {
                total_vol += order.vol;
            }

            level.sell_price = itSell->first;
            level.sell_vol   = total_vol;

            ++itSell;
        }

        result.push_back(level);
    }
   
    return result;
}
