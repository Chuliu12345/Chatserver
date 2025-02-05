**version1.0** - chatserver, chatservice, MySQL连接与线程安全, User类设计 <br> 
**version1.1** - add 客户端退出业务:处理客户端的异常退出，添加ChatService::clientCloseException, ChatService::clientCloseException, 以及ChatServer::onConnection中断开连接时的操作 <br>
**version1.2** - add 点对点聊天及点对点聊天中的离线消息处理，添加ChatServer :: oneChat实现点对点聊天，添加OfflineMsgModel类实现离线消息处理(mysql中添加offlinemessage表) <br>
