#ifndef GROUP_H
#define GROUP_H
#include <string>
#include <vector>
#include "groupuser.hpp"
using std::vector;
using std::string;
class Group {
public:
    Group(int id = -1,string name= "",string desc = "") {
        this->id = id;
        this->name = name;
        this->desc = desc;
    } 
    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setDesc(string desc) { this->desc = desc; }
 
    int getId() { return this->id; }
    string getName() { return this->name; }
    string getDesc() { return this->desc; }
    // 获取群组中的用户
    vector<GroupUser>&  getUsers() {
        return this->users;
    }

private:
    int id;
    std::string name;
    std::string desc; 
    vector<GroupUser> users;

};

#endif