#include "chatservice.hpp"
#include "public.hpp"
#include "user.hpp"
#include "usermodel.hpp"
#include <muduo/base/Logging.h>
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    // std::function<void(const TcpConnectionPtr&,json&,Timestamp)> loginFun = std::bind(&ChatService::login,this,_1,_2,_3);
    auto loginFun = std::bind(&ChatService::login, this, _1, _2, _3);
    msgHandlerMap.insert({LOGIN_MSG, loginFun});
    auto regFun = std::bind(&ChatService::reg, this, _1, _2, _3);
    msgHandlerMap.insert({REG_MSG, regFun});
    auto oneChatFun = std::bind(&ChatService::oneChat, this, _1, _2, _3);
    msgHandlerMap.insert({ONE_CHAT_MSG,oneChatFun});
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "do login service!!";
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = userModel.query(id);
    if (user.getId() != -1 && user.getPwd() == pwd) {
        // 考虑用户已经登陆情况
        if (user.getState() == "online") {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errorno"] = 2;
            response["errmsg"] = "该账号已登录，请重新输入新账号";
            conn->send(response.dump());
        }
        else {
            // 登录成功
            {
            std::lock_guard <std::mutex> lock(connMutex);
            // 记录用户链接
            userConnMap.insert({id,conn});
            }
            // 更新用户状态
            user.setState("online");
            userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errorno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询用户是否有离线消息
            vector<string> vec = offlineMsgModel.query(id);
            if (!vec.empty()) {
                response["offlinemsg"] = vec;
                // 注意释放资源
                offlineMsgModel.remove(id);
            }

            conn->send(response.dump());
        }
    }
    else {
        // 登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errorno"] = 1;
        response["errormsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}

// 注册业务 ： name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    LOG_INFO << "do reg service!!";
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = userModel.insert(user);
    if (state) {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; // 响应成功
        response["id"] = user.getId();
        conn->send(response.dump()); // 发送response json

    }
    else {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1; // 响应成功
        response["id"] = user.getId();
        conn->send(response.dump()); // 发送response json
    }

}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int toid = js["to"].get<int>();
    {
    std::lock_guard<std::mutex> lock(connMutex);
    auto it = userConnMap.find(toid);
    if (it != userConnMap.end()) {
        // 用户在线，转发消息
        it->second->send(js.dump());
        return;

    }
    else {
        // 用户下线,存储离线消息
        offlineMsgModel.insert(toid,js.dump());

    }

    }

}

MsgHandler ChatService::getHandler(int msgid)
{
    // 考虑报错,利用muduo库打印错误信息
    auto it = msgHandlerMap.find(msgid);
    if (it == msgHandlerMap.end())
    {
        // #TODO 返回空操作,这个lambda表达式对应的函数对象和MsgHandler同类型
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        std::cout << "msgid" << msgid << std::endl;
        return msgHandlerMap[msgid];
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr& conn) {
    User user;
    {    
    std::lock_guard<std::mutex> lock(connMutex);
    for (auto it = userConnMap.begin(); it != userConnMap.end(); it++) {
        if (it->second == conn) {
            // 由于conn为指针，可以直接进行比较
            user.setId(it->first);
            userConnMap.erase(it);
            break;
        }
    }
    }
    // 更新用户状态
    if (user.getId() != -1) {
        user.setState("offline");
        userModel.updateState(user); // 在updataState中，需要向mysql传入的参数只有id和state，因此需要对这两个成员赋值
    }

}

void ChatService::reset() {
    // 把online状态的用户设置为offline
    userModel.resetState();
}

