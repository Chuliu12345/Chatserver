#include "chatserver.hpp"
#include "chatservice.hpp"
#include <muduo/base/Logging.h>
#include <iostream>
#include <signal.h>
using namespace std;

void resetHandler(int) {
    LOG_INFO << "server interrupted!";
    ChatService::instance()->reset();
    exit(0);
}

int main() {
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server (&loop, addr, "ChatServer");

    server.start();
    loop.loop();
    return 0;
}