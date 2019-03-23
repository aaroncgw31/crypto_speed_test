#include <condition_variable>
#include <mutex>


class RegUtil{
public:
	static std::mutex mu;
	static std::condition_variable cv;
	static bool breached;
};


class Conditions{
public:
	static float price;
};


class OrderInfo{
public:
	static char uuid[];
};

