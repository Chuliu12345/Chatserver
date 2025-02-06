#include "friendmodel.hpp"
#include "db.h"
#include "user.hpp"
// 添加好友关系
void FriendModel::insertFriend(int userid, int friendid) {
    char sql[1024] = {0};
    sprintf(sql,"INSERT INTO friend values(%d,%d)",userid,friendid);
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }                                                                                                                                                                                                                                                                                                       
}
// 返回用户好友列表
std::vector<User> FriendModel::query(int userid) {
    using std::vector;
    char sql[1024] = {0};
    sprintf(sql,"SELECT a.id,a.name,a.state FROM user a INNER JOIN friend b ON a.id = b.friendid WHERE b.userid =  %d",userid);
    MySQL mysql;
    vector<User> vec;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                User user;
                user.setId(atoi(row[0])); //row[0]指第一个字段的值
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);

            }
            // res资源释放
            mysql_free_result(res);
        } 
    }     
    return vec;
}