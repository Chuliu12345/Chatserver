#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"
#include <functional>
#include <muduo/base/Logging.h>

using json = nlohmann::json;
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg) : server(loop,listenAddr,nameArg),loop(loop)
                       {
                            // 注册连接回调
                            ConnectionCallback connectionCallback = std::bind(&ChatServer::onConnection, this, _1);
                            server.setConnectionCallback(connectionCallback);

                            // 注册消息回调，接收消息时调用
                            MessageCallback messageCallbask = std::bind(&ChatServer::onMessage, this, _1, _2, _3);
                            server.setMessageCallback(messageCallbask);

                            server.setThreadNum(4); // 1个主Reactor,3个次Reactor

                            start();

                       }
void ChatServer::start() {
    server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr& conn) {
    // 断开连接时释放资源
    if (!conn->connected()) {
        // 断开时需要处理
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
    else {
    }

}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer,Timestamp time) {
    string buf = buffer->retrieveAllAsString();
        // json数据解码，数据反序列化
    json js = json::parse(buf);

    // 解耦网络和业务模块,在server模块中，只有网络相关的代码
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn,js,time);

}