#ifndef __KVULKAN_H__
#define __KVULKAN_H__

class XWindow;

class KVulkan {
public:
    virtual ~KVulkan() {}
    virtual void* createVulkanSurface(const std::shared_ptr<XWindow>& wnd, void* instance) = 0;
};

typedef std::shared_ptr<KVulkan> KVulkanPtr;

#endif
