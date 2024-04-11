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
    Order& order = mOrders[id];
    auto&  queue = *mQueues[ size_t(order.sell) ];

    bool price_is_different = ( price != order.price );
    bool vol_increase       = ( vol   >  order.vol   );

    const bool order_loses_time_priority = (vol_increase || price_is_different);
    if( order_loses_time_priority )     // update internal id of order and reinsert in map to give it lower priority
    {
        // remove order from queue
        auto price_map_it = queue.find(order.price);
        price_map_it->second.erase( order.intId );

        // update order
        order.price = price;
        order.vol   = vol;
        order.intId = mIntId;
        ++mIntId;
        
        // lower the priority by reinserting with new internal id
        queue[order.price].insert( {order.intId, order} );

        if( price_map_it->second.empty() )
        {
            queue.erase( price_map_it );
        }
    }
    else
    {
        // priority stays the same, only update volume
        auto& price_map              = queue[order.price];
        price_map[order.intId].vol   = vol;
        order.vol                    = vol;
    }

    if( price_is_different )
    {
        ExecuteOrders( );
    }
}



void OrderBook::Pull( const size_t id )
{
    if( auto order_it = mOrders.find( id ); order_it != mOrders.end() )
    {
        const Order& order = order_it->second;

        // delete order from sell/buy queue
        auto& queue        = *mQueues[ size_t(order.sell) ];
        auto  price_map_it = queue.find(order.price);
        price_map_it->second.erase( order.intId );

        // remove price map from sell/buy queue if no orders left at that price
        if( price_map_it->second.empty() )
        {
            queue.erase( price_map_it );
        }

        mOrders.erase(order_it);
    }
}


void OrderBook::ExecuteOrders()
{
    while( !mBuyQueue.empty() && !mSellQueue.empty() )
    {
        auto highestBuyPricesIt = mBuyQueue.rbegin(); // reverse iterator because we want highest price first
        auto lowestSellPricesIt = mSellQueue.begin();

        const double buyPrice   = highestBuyPricesIt->first;
        const double sellPrice  = lowestSellPricesIt->first;

        if( buyPrice < sellPrice ) { break; }

        Order& highestBuyOrder  = highestBuyPricesIt->second.begin()->second;
        Order& lowestSellOrder  = lowestSellPricesIt->second.begin()->second;

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
            highestBuyPricesIt->second.erase( highestBuyPricesIt->second.begin() );

            if( highestBuyPricesIt->second.empty() )
            {
                mBuyQueue.erase( std::next(highestBuyPricesIt).base() );
            }
        }

        if( 0 == lowestSellOrder.vol )
        {
            lowestSellPricesIt->second.erase( lowestSellPricesIt->second.begin() );

            if( lowestSellPricesIt->second.empty() )
            {
                mSellQueue.erase( lowestSellPricesIt );
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
