#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>
using std::string;
using std::vector;

class OfflineMsgModel {
public:
    // 存储用户离线消息
    void insert(int userid, string msg);
    // 删除已发送用户消息
    void remove(int userid);
    // 查询用户离线消息
    vector<string> query(int userid);
private:
};

#endif