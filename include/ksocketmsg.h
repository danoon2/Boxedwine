#ifndef __KSOCKETMSG_H__
#define __KSOCKETMSG_H__

#include "kobject.h"

class KSocketMsgObject {
public:
    std::shared_ptr<KObject> object;
    U32 accessFlags = 0;
};


class KSocketMsg {
public:
    std::vector<KSocketMsgObject> objects;
    std::vector<U8> data;
};

#endif