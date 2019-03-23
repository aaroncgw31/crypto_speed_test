#include "MarketDataMessage.h"

#include <time.h>
#include <limits>
#include <iostream>

using namespace std;

int64_t parse_time(const char* ts)
{
    static const char* FORMAT = "%Y-%m-%dT%H:%M:%S";
    int64_t microseconds = atoi(ts + strlen("xxxx-xx-xxTxx:xx:xx."));
    struct tm t;
    strptime(ts, FORMAT, &t);
    return static_cast<int64_t>(timegm(&t)) * 1000000000LL + (microseconds * 1000LL);
}

void copy_order_id(char* to, const char* from)
{
    for(int i = 0; from[i]; ++i)
    {
        if( from[i] != '-' )
            *to++ = from[i];
    }
}

bool OrderMessage::FromJson(const ValueType& value)
{
    if( !value.HasMember("type") )
        return false;

    if( !strcmp(value["type"].GetString(), "margin_profile_update") )
        return false;

    type = (Type)toupper(value["type"].GetString()[0]);
    switch(type)
    {
    case Type::RECEIVED:
    case Type::OPEN:
    case Type::DONE:
    case Type::CHANGE:
    case Type::ACTIVATE:
    case Type::MATCH:
        break;
    default:
        return false;
    }

    exchange_time = parse_time(value["time"].GetString());

    order_type = OrderType::UNKNOWN;
    if( value.HasMember("order_type") )
        order_type = (OrderType)toupper(value["order_type"].GetString()[0]);

    if( value.HasMember("reason") )
        reason = (Reason)toupper(value["reason"].GetString()[0]);
    else
        reason = Reason::UNKNOWN;

    if( value.HasMember("side") )
        side = (Side)toupper(value["side"].GetString()[0]);
    else
        side = Side::UNKNOWN;

    if( value.HasMember("order_id") )
        copy_order_id(order_id, value["order_id"].GetString());
    else
        memset(order_id, ' ', sizeof(order_id));

    if( value.HasMember("client_oid") )
        copy_order_id(client_oid, value["client_oid"].GetString());
	//std::cout << value["client_oid"].GetString() << std::endl;
    else
        memset(client_oid, ' ', sizeof(client_oid));

    if( value.HasMember("order_id") )
        copy_order_id(order_id, value["order_id"].GetString());
    else
    {
        if( value.HasMember("maker_id") )
            copy_order_id(order_id, value["maker_id"].GetString());
        else
            memset(order_id, 0, sizeof(order_id));
    }

    if( value.HasMember("taker_id") )
        copy_order_id(taker_id, value["taker_id"].GetString());
    else
        memset(taker_id, 0, sizeof(taker_id));

    if( value.HasMember("size") )
        size = atof(value["size"].GetString());
    else
        size = std::numeric_limits<double>::quiet_NaN();


    if( value.HasMember("price") )
        price = atof(value["price"].GetString());
    else
        price = std::numeric_limits<double>::quiet_NaN();

    if( value.HasMember("stop_price") )
        stop_price = atof(value["stop_price"].GetString());
    else
        stop_price = std::numeric_limits<double>::quiet_NaN();

    if( value.HasMember("remaining_size") )
        remaining_size = atof(value["remaining_size"].GetString());
    else
        remaining_size = std::numeric_limits<double>::quiet_NaN();

    if( value.HasMember("sequence") )
        sequence_number = value["sequence"].GetUint64();
    else
        sequence_number = 0;

    if( value.HasMember("product_id") )
        strncpy(symbol, value["product_id"].GetString(), sizeof(symbol));
    else
        memset(symbol, 0, sizeof(symbol));

    return true;
}

