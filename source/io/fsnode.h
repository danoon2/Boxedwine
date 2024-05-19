#ifndef __FSNODE_H__
#define __FSNODE_H__

#include "platform.h"
#include "kfilelock.h"

// on windows one.txt and One.txt are the same file but on Linux they are different
#define EXT_MIXED ".mixed"
#define EXT_DOSATTRIB ".user.DOSATTRIB"
#define EXT_LINK ".link"

class FsOpenNode;
class KProcess;
class KThread;
class KObject;

class FsNode : public std::enable_shared_from_this<FsNode> {
public:
    enum class Type
    {
        File,
        Zip,
        Virtual,
        Socket,
        Memory,
        Timer
    };
    FsNode(Type type, U32 id, U32 rdev, BString path, BString link, BString nativePath, bool isDirectory, std::shared_ptr<FsNode> parent);

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
    std::weak_ptr<FsNode> getParent() {return this->parent;}

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

    std::shared_ptr<FsNode> getChildByName(BString name);
    std::shared_ptr<FsNode> getChildByNameIgnoreCase(BString name);

    U32 getChildCount();
    void addChild(std::shared_ptr<FsNode> node);
    void removeChildByName(BString name);
    void getAllChildren(std::vector<std::shared_ptr<FsNode> > & results);

    U32 addLock(KFileLock* lock);
    bool unlock(KFileLock* lock);
    KFileLock* getLock(KFileLock* lock, bool otherProcess);
    U32 addLockAndWait(KFileLock* lock, bool otherProcess);
    bool hasLock(U32 pid);
    void unlockAll(U32 pid);

    void addOpenNode(KListNode<FsOpenNode*>* node);
protected:
    std::weak_ptr<FsNode> parent; // the parent holds a strong reference to the children

    KList<FsOpenNode*> openNodes;
    BOXEDWINE_MUTEX openNodesMutex;

private:
    const bool isDir;
    bool hasLoadedChildrenFromFileSystem;    

    BHashTable<BString, std::shared_ptr<FsNode> > childrenByName;
    BOXEDWINE_MUTEX childrenByNameMutex;

    std::vector<KFileLock> locks;       
    BOXEDWINE_CONDITION locksCS;    

    void loadChildren();
    KFileLock* internalGetLock(KFileLock* lock, bool otherProcess);
};

#endif
