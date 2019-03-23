#pragma once

#ifndef _WEBSOCKET_FEED_H_
#define _WEBSOCKET_FEED_H_

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <stdint.h>
#include <libwebsockets.h>

#include <vector>
#include <string>
#include <map>

typedef rapidjson::Document ValueType;

class WebsocketFeedClient
{
public:

    virtual void OnMessage(int64_t ts, const ValueType& msg) = 0;
};

class WebsocketFeed
{
public:

    void AddClient(const std::string& symbol, WebsocketFeedClient* client);
    void Run(const std::string& host, int port = 443);

private:

    typedef std::vector<WebsocketFeedClient*> Clients;
    typedef std::map<std::string, Clients> ClientMap;

    void SubscribeSymbols(lws* wsi);
    void OnMessage(int64_t now, const char* message, int length);

    static int WebsocketCallback(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t length);

    void StartFeed(const std::string& host, int port);

    //ClientMap _clients;

    bool _connected;
    bool _running;
};

#endif // _WEBSOCKET_FEED_H_
