#ifndef USERMODEL_H
#define USERMODEL_H
/*User相关业务*/
#include "user.hpp"

class UserModel {
public:
    // 增加用户表
bool insert(User& User);
User query(int id);
bool updateState(User user);
void resetState();
    private:
};
#endif