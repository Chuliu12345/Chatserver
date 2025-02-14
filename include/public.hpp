#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType {
    LOGIN_MSG = 1,
    LOGIN_MSG_ACK,
    LOGINOUT_MSG,
    REG_MSG ,
    REG_MSG_ACK ,
    ONE_CHAT_MSG ,
    ADD_FRIEND_MSG ,
    // 群组相关
    CREATE_GROUP_MSG ,
    ADD_GROUP_MSG ,
    GROUP_CHAT_MSG ,

    ADD_GROUP_MSG_ACK,
    CREATE_GROUP_MSG_ACK
    
};

#endif