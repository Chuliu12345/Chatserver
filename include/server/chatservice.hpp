#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <mutex>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace muduo;
using namespace muduo::net;
using json =  nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp )>;

// 单例设计
class ChatService {
public:
    static ChatService* instance();
    // 登陆业务
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 注册业务
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 根据msgid获取对应的业务回调
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    // 处理用户注销
    void loginout(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 服务器异常退出后的状态重置
    void reset();

private:

    ChatService();
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void handleRedisSubscribeMessage(int userid, string msg);
    std::unordered_map<int,MsgHandler> msgHandlerMap;

    // 存储在线用户的通信连接 -- 使用unordered_mat
    std::unordered_map<int,TcpConnectionPtr> userConnMap;

    std::mutex connMutex;

    // 数据对象
    UserModel userModel;
    OfflineMsgModel offlineMsgModel;
    FriendModel friendModel;
    GroupModel groupModel;
    Redis redis;

};

#endif