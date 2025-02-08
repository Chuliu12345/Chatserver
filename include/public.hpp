#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType {
    LOGIN_MSG = 1,
    LOGIN_MSG_ACK = 2 , 
    REG_MSG = 3 ,
    REG_MSG_ACK = 4 ,
    ONE_CHAT_MSG = 5,
    ADD_FRIEND_MSG = 6,
    ADD_FRIEND_MSG_ACK  = 7,

    // 群组相关
    CREATE_GROUP_MSG = 8,
    CREATE_GROUP_MSG_ACK = 9,
    ADD_GROUP_MSG = 10,
    ADD_GROUP_MSG_ACK = 11,
    GROUP_CHAT_MSG = 12
    
};

#endif