/*
muduo 为用户提供两个类
TcpServer : 服务器程序
TcpClient : 客户端程序
epoll + 线程池，
将网络I/O代码和业务代码区分
*/
/*
在知道需要使用TcpServer类之后，可以通过构造函数，ChatServer类的实现
*/
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <iostream>
using namespace std;
using namespace muduo::net;
using namespace muduo;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &,
                                           Buffer *, Timestamp)>;

class ChatServer
{
public:
    // ** In TcpServere.h :
    //   TcpServer(EventLoop* loop,
    //         const InetAddress& listenAddr,
    //         const string& nameArg,
    //         Option option = kNoReusePort);
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    ~ChatServer() = default;
    void start();

private:
    TcpServer server; // 服务器对象
    EventLoop *loop;  // 事件循环对象指针

    // ** In Callbacks.h :
    // typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
    // typedef std::function<void (const TcpConnectionPtr&,
    //                             Buffer*,
    //                             Timestamp)> MessageCallback;

    // 建立和销毁连接时，调用
    void onConnection(const TcpConnectionPtr &conn);

    // 接收信息时，调用
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buffer,
                   Timestamp time);
};

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg) : server(loop, listenAddr, nameArg)
{
    this->loop = loop;
    ConnectionCallback connectionCallback = std::bind(&ChatServer::onConnection, this, _1);
    server.setConnectionCallback(connectionCallback);

    MessageCallback messageCallbask = std::bind(&ChatServer::onMessage, this, _1, _2, _3);
    server.setMessageCallback(messageCallbask);

    server.setThreadNum(4);
}

void ChatServer::start()
{
    server.start();
}

void ChatServer::onMessage(const TcpConnectionPtr &conn,
               Buffer *buffer,
               Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    cout << "recv data" << buf << "time:" << time.toFormattedString() << endl;
    conn->send(buf);
}
void ChatServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:online" << endl;
    }
    else
    {
        cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:offline" << endl;
        conn->shutdown(); // close(fd)
        // _loop->quit();
    }
};

int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");

    server.start();
    loop.loop();
    return 0;

}