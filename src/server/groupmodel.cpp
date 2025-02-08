#include "groupmodel.hpp"
#include "db.h"
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO allgroup(groupname,groupdesc) VALUES('%s','%s')", group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            MYSQL *conn = mysql.getConnection();
            // 返回最后一次插入操作生成的id
            int id = mysql_insert_id(conn);
            // create group id 为默认值-1，需要设置值
            group.setId(id);
            return true;
        }
    }
    return false;
}
bool GroupModel::addGroup(int userid, int groupid, string role = "normal")
{
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser (groupid,userid,grouprole) values(%d,%d,'%s')", groupid, userid, role.c_str());
    MySQL mysql;

    if (mysql.connect())
    {
        mysql.update(sql);
        return true;
    }
    return false;
}
// 查看用户加入的所有群组
vector<Group> GroupModel::queryGroups(int userid)
{
    // 联合查询
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a inner join groupuser b\
            on a.id = b.groupid where b.userid = %d",
            userid);
    MySQL mysql;
    vector<Group> group_vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        MYSQL_ROW row;
        if (res != nullptr)
        {
            while (row = mysql_fetch_row(res))
            {
                if (row != nullptr)
                {
                    Group group;
                    group.setId(atoi(row[0]));
                    group.setName(row[1]);
                    group.setDesc(row[2]);
                    group_vec.push_back(group);
                }
            }
        }
        mysql_free_result(res);
    }
    return group_vec;
}
// 查询同一群组中的成员id
vector<int> GroupModel::queryGroupUsers(int userid, int groupid) {
    char sql[1024]  = {0};
    vector<int> groupuser_id_vec;
    sprintf(sql,"select userid from groupuser where groupid = %d and userid != %d",groupid,userid);
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row; 
            while( ( row = mysql_fetch_row(res) ) != nullptr) {
                groupuser_id_vec.push_back(atoi(row[0]));
            }
        }
    }
    return groupuser_id_vec;
}