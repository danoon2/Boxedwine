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
        Socket
    };
    FsNode(Type type, U32 id, U32 rdev, const std::string& path, const std::string& link, bool isDirectory, BoxedPtr<FsNode> parent);

    virtual U32 rename(const std::string& path)=0; //return 0 if success, else errno
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

    U32 getHardLinkCount() {return this->hardLinkCount;}
    bool isLink() {return this->link.size()>0;}
    bool isDirectory() {return this->isDir;}
    BoxedPtr<FsNode> getParent() {return this->parent;}

    void removeOpenNode(FsOpenNode* node);
    void removeNodeFromParent();

    std::string path; 
    std::string name;
    const std::string link;
    const U32 id;
    const U32 rdev;  
    U32 hardLinkCount;
    std::vector<KFileLock> locks;        
    KList<FsOpenNode*> openNodes;
    const Type type;
    BoxedPtr<KObject> kobject;

    BoxedPtr<FsNode> getChildByName(const std::string& name);
    U32 getChildCount();
    void addChild(BoxedPtr<FsNode> node);
    void removeChildByName(const std::string& name);
    void getAllChildren(std::vector<BoxedPtr<FsNode> > & results);

protected:
    BoxedPtr<FsNode> parent;

private:
    const bool isDir;
    bool hasLoadedChildrenFromFileSystem;    
    std::unordered_map<std::string, BoxedPtr<FsNode> > childrenByName;
    void loadChildren();
};

#endif
