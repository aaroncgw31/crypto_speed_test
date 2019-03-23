#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <chrono>


#include "condition_var.h"
#include "WebsocketFeed.h"
#include "fixengine/fix_client.h"

std::mutex RegUtil::mu;
std::condition_variable RegUtil::cv;
bool RegUtil::breached = false;
float Conditions::price = 6000;
char OrderInfo::uuid[] = "22ee5f42-287c-11e8-b467-0ed5f89f718b";

int main(){
	std::cout << "FIX channel connecting..." << std::endl;
	std::thread th1(fix_client_run);
	std::this_thread::sleep_for(std::chrono::milliseconds(30000));

	WebsocketFeed *feed = new WebsocketFeed();
	std::cout << "Websocket connecting..." << std::endl;
	//std::thread th2(&WebsocketFeed::Run, feed, "ws-feed.gdax.com" ,443);
	std::thread th2(&WebsocketFeed::Run, feed, "ws-feed-public.sandbox.gdax.com" ,443);			

	std::unique_lock<std::mutex> lk(RegUtil::mu);
	RegUtil::cv.wait(lk, []{return RegUtil::breached;});
	
	std::cout << "Placing Order" << std::endl;
	fix_new_order_single();
	
	th1.join();
	th2.join();
	return 0;
}

