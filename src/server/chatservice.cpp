#include "chatservice.hpp"
#include <muduo/base/Logging.h>
#include "public.hpp"
#include "friendmodel.hpp"
#include "redis.hpp"
// #include "chatserver.hpp"
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
    auto loginoutFun = std::bind(&ChatService::login, this, _1, _2, _3);
    msgHandlerMap.insert({LOGINOUT_MSG, loginoutFun});
    auto regFun = std::bind(&ChatService::reg, this, _1, _2, _3);
    msgHandlerMap.insert({REG_MSG, regFun});
    auto oneChatFun = std::bind(&ChatService::oneChat, this, _1, _2, _3);
    msgHandlerMap.insert({ONE_CHAT_MSG, oneChatFun});
    auto addFriendFun = std::bind(&ChatService::addFriend, this, _1, _2, _3);
    msgHandlerMap.insert({ADD_FRIEND_MSG, addFriendFun});
    // add v2.1 群组相关业务
    auto createGroupFun = std::bind(&ChatService::createGroup, this, _1, _2, _3);
    msgHandlerMap.insert({CREATE_GROUP_MSG, createGroupFun});
    auto addGroupFun = std::bind(&ChatService::addGroup, this, _1, _2, _3);
    msgHandlerMap.insert({ADD_GROUP_MSG, addGroupFun});
    auto groupChatFun = std::bind(&ChatService::groupChat, this, _1, _2, _3);
    msgHandlerMap.insert({GROUP_CHAT_MSG, groupChatFun});
    // auto  = std::bind(&ChatService::, this, _1, _2, _3);
    // msgHandlerMap.insert({, });

    // 连接redis服务器
    if (redis.connect()) {
        redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];
    std::cout << "id = " << id << std::endl;
    std::cout << "password = " << pwd << std::endl;
    User user = userModel.query(id);
    if (user.getId() != -1 && user.getPwd() == pwd)
    {
        // 考虑用户已经登陆情况
        if (user.getState() == "online")
        {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已登录，请重新输入新账号";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功
            {
                std::lock_guard<std::mutex> lock(connMutex);
                // 记录用户链接
                userConnMap.insert({id, conn});
            }
            // 用户登录成功后向redis订阅channel
            redis.subscribe(id);
            // 更新用户状态
            user.setState("online");
            userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询用户是否有离线消息
            vector<string> offlinemsg_vec = offlineMsgModel.query(id);
            if (!offlinemsg_vec.empty())
            {
                response["offlinemsg"] = offlinemsg_vec;
                // 注意释放资源
                offlineMsgModel.remove(id);
            }

            // 查询好友列表
            vector<User> friend_vec = friendModel.query(id);
            if (!friend_vec.empty())
            {
                vector<string> friendinfo_vec;
                for (User &user : friend_vec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    friendinfo_vec.push_back(js.dump());
                }
                response["friends"] = friendinfo_vec;
            }

            // v2.1 查询用户群组消息
            vector<Group> usergroup_vec = groupModel.queryGroups(id);
            if (!usergroup_vec.empty())
            {
                vector<string> groups;
                for (Group &group : usergroup_vec)
                {
                    json groupjson;
                    groupjson["id"] = group.getId();
                    groupjson["groupname"] = group.getName();
                    groupjson["groupdesc"] = group.getDesc();
                    groups.push_back(groupjson.dump());
                }
                response["groups"] = groups;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errormsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}

// 注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(connMutex);
        auto it = userConnMap.find(userid);
        if (it != userConnMap.end())
        {
            userConnMap.erase(it);
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    redis.unsubscribe(userid); 

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    userModel.updateState(user);
}
// 注册业务 ： name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; // 响应成功
        response["id"] = user.getId();
        conn->send(response.dump()); // 发送response json
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1; // 响应成功
        response["id"] = user.getId();
        conn->send(response.dump()); // 发送response json
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connMutex);
        auto it = userConnMap.find(toid);
        if (it != userConnMap.end())
        {
            // 用户在线(从集群服务器的角度，是用户在本机上)，转发消息
            it->second->send(js.dump());
            return;
        }
        else
        {
            // @redis
            /*
              用户不在本机的两种可能：1. 用户在其他服务器登录 2. 用户下线
              利用state = offline or online区分  
            */
            // 用户下线,存储离线消息
            User user = userModel.query(toid);
            if (user.getState() == "online") {
                redis.publish(toid,js.dump());
                return ;
            }
            
        }
        offlineMsgModel.insert(toid, js.dump());
    }
}

// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息
    FriendModel friendModel;
    friendModel.insertFriend(userid, friendid);
}

//------v2.1-----------
// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string groupname = js["groupname"];
    string groupdesc = js["groupdesc"];

    Group group(-1, groupname, groupdesc);
    bool state = groupModel.createGroup(group);
    if (state)
    {
        json response;
        response["msgid"] = CREATE_GROUP_MSG;
        response["errno"] = 0;
        response["errormsg"] = "群组创建成功!";

        // 存储创建人信息
        bool addRes = groupModel.addGroup(userid, group.getId(), "creator");
        if (addRes)
        {
            response["create_errmsg"] = "创建者成功加入群组";
            response["create_userid"] = userid;
        }
        else
        {
            response["create_errmsg"] = "群组创建失败：创建者加入群组失败";
        }
        conn->send(response.dump());
    }
    else
    {
        // 群组创建失败
        json response;
        response["msgid"] = CREATE_GROUP_MSG_ACK;
        response["errno"] = 1;
        response["errormsg"] = "群组创建失败";
        conn->send(response.dump());
    }
}

// ------add v2.1---------
// 用户加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    bool addRes = groupModel.addGroup(userid, groupid, "normal");
    if (addRes)
    {
        json response;
        response["msgid"] = ADD_GROUP_MSG_ACK;
        response["error"] = 0;
        response["errmsg"] = "成功加入群组!";
        response["userid"] = userid;
        response["groupid"] = groupid;
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = ADD_GROUP_MSG_ACK;
        response["error"] = 1;
        response["errmsg"] = "加入群组失败!";
        conn->send(response.dump());
    }
}

//-----add v2.1-----------
// 群聊业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> userid_vec = groupModel.queryGroupUsers(userid, groupid);
    std::lock_guard<std::mutex> lock(connMutex);
    // 遍历用户，判断是否在线
    // @redis 同onechat理，还需要检测状态
    for (int id : userid_vec)
    {
        auto it = userConnMap.find(id);
        if (it != userConnMap.end())
        {
            it->second->send(js.dump()); // 在线转发消息
        }
        else
        {
            User user = userModel.query(id);
            if (user.getState() == "online") {
                redis.publish(id,js.dump());
            }
            else {
                offlineMsgModel.insert(id, js.dump()); // 不在线插入离线消息表中
            }
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

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(connMutex);
        for (auto it = userConnMap.begin(); it != userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                // 由于conn为指针，可以直接进行比较
                user.setId(it->first);
                userConnMap.erase(it);
                break;
            }
        }
    }

    // @redis:用户注销,取消订阅
    redis.unsubscribe(user.getId());

    // 更新用户状态
    if (user.getId() != -1)
    {
        user.setState("offline");
        userModel.updateState(user); // 在updataState中，需要向mysql传入的参数只有id和state，因此需要对这两个成员赋值
    }
}

void ChatService::reset()
{
    // 把online状态的用户设置为offline
    userModel.resetState();
}

void ChatService::handleRedisSubscribeMessage(int userid, string msg) {
    lock_guard<mutex> lock(connMutex);
    auto it = userConnMap.find(userid);
    if (it != userConnMap.end()) {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    offlineMsgModel.insert(userid, msg);
}
