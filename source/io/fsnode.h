#ifndef __FSNODE_H__
#define __FSNODE_H__

#include "platform.h"
#include "kfilelock.h"

class FsOpenNode;
class KProcess;
class KThread;
class KObject;

class FsNode : public BoxedPtrBase {
public:
    enum Type
    {
        File,
        Zip,
        Virtual,
        Socket,
        Memory
    };
    FsNode(Type type, U32 id, U32 rdev, BString path, BString link, BString nativePath, bool isDirectory, BoxedPtr<FsNode> parent);

    virtual U32 rename(BString path)=0; //return 0 if success, else errno
    virtual bool remove()=0;
    virtual U64 lastModified()=0; // returns ms since 1970
    virtual U64 length()=0;
    virtual FsOpenNode* open(U32 flags)=0;    
    virtual U32 getType(bool checkForLink)=0;
    virtual U32 getMode()=0;
    virtual U32 removeDir()=0;
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano)=0;

    virtual bool canRead();
    virtual bool canWrite();

    virtual BString getLink() {return this->link;}
    virtual bool isLink() { return this->link.length() > 0; }

    U32 getHardLinkCount() {return this->hardLinkCount;}    
    bool isDirectory() {return this->isDir;}
    BoxedPtr<FsNode> getParent() {return this->parent;}

    void removeOpenNode(FsOpenNode* node);
    void removeNodeFromParent();

    BString path; 
    BString nativePath;
    BString name;
    BString link;
    const U32 id;
    const U32 rdev;  
    U32 hardLinkCount;    
    const Type type;
    std::weak_ptr<KObject> kobject;

    BoxedPtr<FsNode> getChildByName(BString name);
    BoxedPtr<FsNode> getChildByNameIgnoreCase(BString name);

    U32 getChildCount();
    void addChild(BoxedPtr<FsNode> node);
    void removeChildByName(BString name);
    void getAllChildren(std::vector<BoxedPtr<FsNode> > & results);

    U32 addLock(KFileLock* lock);
    bool unlock(KFileLock* lock);
    KFileLock* getLock(KFileLock* lock, bool otherProcess);
    U32 addLockAndWait(KFileLock* lock, bool otherProcess);
    bool hasLock(U32 pid);
    void unlockAll(U32 pid);

    void addOpenNode(KListNode<FsOpenNode*>* node);
protected:
    BoxedPtr<FsNode> parent;

    KList<FsOpenNode*> openNodes;
    BOXEDWINE_MUTEX openNodesMutex;

private:
    const bool isDir;
    bool hasLoadedChildrenFromFileSystem;    

    std::unordered_map<BString, BoxedPtr<FsNode> > childrenByName;
    BOXEDWINE_MUTEX childrenByNameMutex;

    std::vector<KFileLock> locks;       
    BOXEDWINE_CONDITION locksCS;    

    void loadChildren();
    KFileLock* internalGetLock(KFileLock* lock, bool otherProcess);
};

#endif
