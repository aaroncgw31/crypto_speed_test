#pragma once

#ifndef _MARKET_DATA_MESSAGE_H_
#define _MARKET_DATA_MESSAGE_H_

#include <stdint.h>
#include <string>

#ifndef PACKED
#define PACKED __aligned__((packed))
#endif

#include "WebsocketFeed.h"

//int signal = 0;
void copy_order_id(char* to, const char* from);

struct OrderId
{
    static constexpr const int ORDER_ID_LEN = 32;
    char order_id[ORDER_ID_LEN];

    void Set(const char* guid);
    std::string AsGuid() const;
};

struct OrderMessage
{
    static constexpr const int SYMBOL_LEN = 16;
    enum class Type : char
    {
        RECEIVED = 'R'
    ,   OPEN = 'O'
    ,   DONE = 'D'
    ,   MATCH = 'M'
    ,   CHANGE = 'C'
    ,   ACTIVATE = 'A'
    };

    enum class Side : char
    {
        BUY = 'B'
    ,   SELL = 'S'
    ,   UNKNOWN = 'U'
    };

    enum class Reason : char
    {
        FILLED = 'F'
    ,   CANCELED = 'C'
    ,   UNKNOWN = 'U'
    };

    enum class OrderType : char
    {
        LIMIT = 'L'
    ,   STOP_LIMIT = 'S'
    ,   MARKET = 'M'
    ,   UNKNOWN = 'U'
    };

    int64_t receive_time;
    int64_t exchange_time;

    char order_id[32];
    char taker_id[32];
    char client_oid[32];    

    Type type;
    Side side;
    OrderType order_type;
    Reason reason;

    uint64_t sequence_number;

    char symbol[SYMBOL_LEN];
    double size;
    double price;
    double stop_price;

    double remaining_size;

    bool FromJson(const ValueType& value);
} __attribute__((packed));

#endif // _MARKET_DATA_MESSAGE_H_
