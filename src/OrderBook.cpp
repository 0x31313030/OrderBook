#include "OrderBook.h"

//TODO: use dependency injection for this!! create two classes, one thread safe (maybe atomics? look into it!!) and a normal one.
size_t OrderBook::mTradeId { 0 }; // NOTE: could overflow so depending on use, it might have to be done another way or reset in a controlled way.

std::string remove_zeros(std::string& numberstring);
std::string fts(double number);


std::string remove_zeros(std::string& numberstring)
{
    auto it = numberstring.end() - 1;
    while(*it == '0') {
        numberstring.erase(it);
        it = numberstring.end() - 1;
    }
    if(*it == '.') numberstring.erase(it);
    return numberstring;
}


std::string fts(double number) // float to string
{
    std::stringstream ss{};
    ss << std::setprecision(4) << std::fixed << std::showpoint << number;
    std::string numberstring{ss.str()};
    return remove_zeros(numberstring);
}


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


OrderBook::OrderBook(const std::string& symbol) 
: mSymbol { symbol }
{

}

void OrderBook::Insert( const size_t id, const Side side, const double price, const size_t vol )
{
    Order order { id, mIntId, price, vol, Side::SELL == side };
    ++mIntId;

    mTrades.insert( {order.id, order} );

    if( order.sell )
    {
        mSellQueue[ order.price ].insert( {order.intId, order} );
    }
    else
    {
        mBuyQueue[ order.price ].insert( {order.intId, order} );
    }

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
            // priority stays the same, only update vol
            auto& order_map              = queue[order.price];
            order_map[order.intId].vol   = vol;
            order.vol                    = vol;
        }

        //PrintOrderBook();
        if( price_is_different )
        {
            ExecuteOrders( );
        }
    };

    Order& order = mTrades[id];

    //TODO: remove this 'if' by just assigning a pointer to the correct queue? retard


    if( order.sell )
    {
        UpdateQueue( mSellQueue, order, price, vol );
    }
    else
    {
        UpdateQueue( mBuyQueue, order, price, vol );
    }
}



void OrderBook::Pull( const size_t id )
{
    if( auto it = mTrades.find( id ); it != mTrades.end() )
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


        if( order.sell )
        {
            EraseOrderFromQueue(mSellQueue, order);
        }
        else
        {
            EraseOrderFromQueue(mBuyQueue, order);
        }

        mTrades.erase(it);
    }
}


void OrderBook::ExecuteOrders()
{
    while( !mBuyQueue.empty() && !mSellQueue.empty() )
    {
        auto itHighestBuyPrices = mBuyQueue.begin();
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
        ExecutedTrade trade { passiveOrder.price, tradeVol, aggressiveOrder.id, passiveOrder.id, mTradeId };
        mExecutedTrades.push_back( trade );
        ++mTradeId;

        highestBuyOrder.vol -= tradeVol;
        lowestSellOrder.vol -= tradeVol;

        // remove order if no more volume
        if( 0 == highestBuyOrder.vol )
        {
            itHighestBuyPrices->second.erase( itHighestBuyPrices->second.begin() );

            if( itHighestBuyPrices->second.empty() )
            {
                mBuyQueue.erase( itHighestBuyPrices );
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

std::vector< std::pair<size_t, std::string> > OrderBook::GetListOfTrades()
{
    std::vector< std::pair<size_t, std::string> > result;

    for( ExecutedTrade trade : mExecutedTrades )
    {
        result.emplace_back( trade.gId, mSymbol+","+
                                        fts(trade.price)+","+
                                        std::to_string(trade.volume)+","+
                                        std::to_string(trade.aggressive_order_id)+","+
                                        std::to_string(trade.passive_order_id) );
    }

    return result;
}


std::vector< std::string > OrderBook::GetPriceLevels()
{
    std::vector< std::string > result;

    auto itBuy  = mBuyQueue.begin();
    auto itSell = mSellQueue.begin();

    while( itBuy != mBuyQueue.end() || itSell != mSellQueue.end() )
    {
        std::string level;

        if( itBuy != mBuyQueue.end() )
        {
            size_t total_vol = 0;
            for( auto [intId, order] : itBuy->second )
            {
                total_vol += order.vol;
            }

            level += fts(itBuy->first) + "," + std::to_string(total_vol) + ",";

            ++itBuy;
        }
        else
        {
            level += ",,";
        }

        if( itSell != mSellQueue.end() )
        {
            size_t total_vol = 0;
            for( auto [intId, order] : itSell->second )
            {
                total_vol += order.vol;
            }
            
            level += fts(itSell->first) + "," + std::to_string(total_vol);

            ++itSell;
        }
        else
        {
            level += ",";
        }

        result.push_back(level);
    }
   
    return result;
}
