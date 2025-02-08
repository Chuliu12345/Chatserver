#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "group.hpp"
#include <vector>
using std::vector;
class GroupModel {
public:
    bool createGroup(Group& group);
    // 用户加入群
    bool addGroup(int userid, int groupid, string role);
    // 查看用户加入的所有群组
    vector<Group> queryGroups(int userid);
    // 查询同一群组中的成员id
    vector<int> queryGroupUsers(int userid, int groupid);
private:
};

#endif