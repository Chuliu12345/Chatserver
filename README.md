**version1.0**  -   chatserver, chatservice, MySQL连接与线程安全, User类设计 <br> 
**version1.1**  -   add 客户端退出业务:处理客户端的异常退出，添加ChatService::clientCloseException, ChatService::clientCloseException, 以及ChatServer::onConnection中断开连接时的操作 <br>
**version1.2**  -   add 点对点聊天及点对点聊天中的离线消息处理，添加ChatServer :: oneChat实现点对点聊天，添加OfflineMsgModel类实现离线消息处理(mysql中添加offlinemessage表) <br>
**version1.3**  -   服务器异常处理,在ctrl+C中止服务器后，重置user的状态 : ChatService::reset,UserModel::userModel.resetState <br>

**version2.1**  -   
add 好友业务 : ChatService::addFriend, <br>
fix 修复登录时用户id显示错误 (in ChatService::login)<br>
add 群组业务：群创建，成员加入，显示群组信息
