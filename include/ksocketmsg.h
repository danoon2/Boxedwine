#ifndef __KSOCKETMSG_H__
#define __KSOCKETMSG_H__

#include "kobject.h"

class KSocketMsgObject {
public:
    BoxedPtr<KObject> object;
    U32 accessFlags;
};


class KSocketMsg : public BoxedPtrBase {
public:
    std::vector<KSocketMsgObject> objects;
    std::vector<U8> data;
};

#endif