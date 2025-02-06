#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <vector>
#include "user.hpp"


class FriendModel {
public:
    // 添加好友关系
    void insertFriend(int userid, int friendid);

    // 返回用户好友列表
    std::vector<User> query(int userid);

private:

};

#endif