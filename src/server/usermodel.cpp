#include "usermodel.hpp"
#include "db.h"
#include <iostream>
bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    /*向sql写入时，需要将string转为c类型字符串*/
    // sprintf(sql, "insert into user(name, password, state) values('%s', '%s',' %s')",
    //         user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    sprintf(sql, "insert into user(name, password) values('%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            // 获取插入成功的用户的主键，作为id号
            int id = mysql_insert_id(mysql.getConnection());
            user.setId(id);
            return true;
        }
    } 
    return false;       
    
}

User UserModel::query(int id) {
    char sql[1024] = {0};
    sprintf(sql,"select * from user where id = %d",id);

    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr) {
                // 从mysql数据库中获取用户信息，并形成user对象
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                /*使用完 MYSQL_RES 后，需要调用 mysql_free_result 函数释放其占用的内存*/
                mysql_free_result(res);
                return user;
            }

        }
    }
    return User();

}

bool UserModel::updateState(User user) {
    char sql[1024] = {0};
    sprintf(sql,"update user set state = '%s' where id = %d",user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    } 
    return false;

}