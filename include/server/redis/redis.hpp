#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
#include <string>
using namespace std;
const string ip = "127.0.0.1";
const int port = 6379;

class Redis {
public:
    Redis();
    ~Redis();
    bool connect();
    /*
    订阅/取消订阅
    publish
    notify
    */
    bool publish(int channel, string message);
    bool subscribe(int channel);
    bool unsubscribe(int channel);
    void observer_channel_message();
    void init_notify_handler(function<void(int,string)> fn);

private:
    redisContext* subscribe_context;
    redisContext* publish_context;
    function<void(int,string)> notify_message_handler;
};

#endif