#include "redis.hpp"
#include <iostream>

Redis::Redis() :
    publish_context(nullptr),subscribe_context(nullptr) {};

// 析构函数中回收资源
Redis::~Redis() {
    if (publish_context != nullptr) {
        redisFree(publish_context);
    }
    if (subscribe_context != nullptr) {
        redisFree(subscribe_context);
    }
}

// redis连接，端口号通过netstap -tnap查看
bool Redis::connect()
{
    // 负责publish发布消息的上下文连接
    publish_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == publish_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 负责subscribe订阅消息的上下文连接
    subscribe_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == subscribe_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    cout << "connect redis-server success!" << endl;

    return true;
}

// publish为即时操作
bool Redis::publish(int channel, string message)
{
    redisReply *reply = (redisReply *)redisCommand(publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}


// subscribe会造成线程阻塞，因此需要区分订阅操作和接收消息的操作
// 本函数中只进行接收操作
bool Redis::subscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(this->subscribe_context,"SUBSCRIBE %d",channel) ) {
        cerr << "error! Subscribe command fail " << endl;
        return false;
    }
    // done用于指示缓冲区发送状态，当缓冲区发送完毕时退出循环
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->subscribe_context,&done)) {
            cerr << "error! Subscribe command fail" << endl;
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(this->subscribe_context,"UNSUBSCRIBE %d",channel) ) {
        cerr << "error! Unsubscribe command fail " << endl;
        return false;
    }
    // done用于指示缓冲区发送状态，当缓冲区发送完毕时退出循环
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->subscribe_context,&done)) {
            cerr << "error! Unsubscribe command fail" << endl;
            return false;
        }
    }
    return true;
}

void Redis::observer_channel_message() {
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(this->subscribe_context,(void**)&reply)) {
        // 要求收到一个数组，数组中含有三元素
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            // notify
            notify_message_handler(atoi(reply->element[1]->str) , reply->element[2]->str);
        }

        freeReplyObject(reply);
    }
    cerr << "observer_channel_message quit" << endl;
}

void Redis::init_notify_handler(function<void(int,string)> fn)
{
    this->notify_message_handler = fn;
}



