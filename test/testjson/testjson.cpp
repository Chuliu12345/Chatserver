#include <iostream>
#include "json.hpp"
using json = nlohmann::json;
#include <map>
using namespace std;

void func1() {
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    cout << js << endl;
};

/*容器序列化*/
void func2() {
    json js;
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    js["list"] = vec;
    map<int, string> m;
    m.insert({1,"黄山"});
    js["path"] = m;
    cout<< js << endl;
}

/*Json序列反序列化*/
void func3() {
    string jsonstr = js.dump();
    
}


int main() {
    func2();
    return 0;
}