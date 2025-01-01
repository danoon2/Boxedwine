struct MarshalFloat {
    union {
        float f;
        U32   i;
    };
};
class MarshalCallbackData {
public:
    U32 callbackAddress;
    U32 userData;
};
VkBool32 VKAPI_PTR boxed_vkDebugReportCallbackEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
VkBool32 VKAPI_PTR boxed_vkDebugUtilsMessengerCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
class MarshalVkBaseOutStructure {
public:
    MarshalVkBaseOutStructure() {}
    VkBaseOutStructure s;
    MarshalVkBaseOutStructure(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBaseOutStructure* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBaseOutStructure* s);
    ~MarshalVkBaseOutStructure();
};

class MarshalVkOffset2D {
public:
    MarshalVkOffset2D() {}
    VkOffset2D s;
    MarshalVkOffset2D(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOffset2D* s);
};

class MarshalVkOffset3D {
public:
    MarshalVkOffset3D() {}
    VkOffset3D s;
    MarshalVkOffset3D(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOffset3D* s);
};

class MarshalVkExtent2D {
public:
    MarshalVkExtent2D() {}
    VkExtent2D s;
    MarshalVkExtent2D(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExtent2D* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExtent2D* s);
};

class MarshalVkExtent3D {
public:
    MarshalVkExtent3D() {}
    VkExtent3D s;
    MarshalVkExtent3D(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExtent3D* s);
};

class MarshalVkViewport {
public:
    MarshalVkViewport() {}
    VkViewport s;
    MarshalVkViewport(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkViewport* s);
};

class MarshalVkRect2D {
public:
    MarshalVkRect2D() {}
    VkRect2D s;
    MarshalVkRect2D(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRect2D* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRect2D* s);
};

class MarshalVkClearRect {
public:
    MarshalVkClearRect() {}
    VkClearRect s;
    MarshalVkClearRect(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkClearRect* s);
};

class MarshalVkComponentMapping {
public:
    MarshalVkComponentMapping() {}
    VkComponentMapping s;
    MarshalVkComponentMapping(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkComponentMapping* s);
};

class MarshalVkPhysicalDeviceProperties {
public:
    MarshalVkPhysicalDeviceProperties() {}
    VkPhysicalDeviceProperties s;
    MarshalVkPhysicalDeviceProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProperties* s);
};

class MarshalVkExtensionProperties {
public:
    MarshalVkExtensionProperties() {}
    VkExtensionProperties s;
    MarshalVkExtensionProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExtensionProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExtensionProperties* s);
};

class MarshalVkLayerProperties {
public:
    MarshalVkLayerProperties() {}
    VkLayerProperties s;
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLayerProperties* s);
};

class MarshalVkApplicationInfo {
public:
    MarshalVkApplicationInfo() {}
    VkApplicationInfo s;
    MarshalVkApplicationInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkApplicationInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkApplicationInfo* s);
    ~MarshalVkApplicationInfo();
};

class MarshalVkDeviceQueueCreateInfo {
public:
    MarshalVkDeviceQueueCreateInfo() {}
    VkDeviceQueueCreateInfo s;
    MarshalVkDeviceQueueCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceQueueCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceQueueCreateInfo* s);
    ~MarshalVkDeviceQueueCreateInfo();
};

class MarshalVkDeviceCreateInfo {
public:
    MarshalVkDeviceCreateInfo() {}
    VkDeviceCreateInfo s;
    MarshalVkDeviceCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceCreateInfo* s);
    ~MarshalVkDeviceCreateInfo();
};

class MarshalVkInstanceCreateInfo {
public:
    MarshalVkInstanceCreateInfo() {}
    VkInstanceCreateInfo s;
    MarshalVkInstanceCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkInstanceCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkInstanceCreateInfo* s);
    ~MarshalVkInstanceCreateInfo();
};

class MarshalVkQueueFamilyProperties {
public:
    MarshalVkQueueFamilyProperties() {}
    VkQueueFamilyProperties s;
    MarshalVkQueueFamilyProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyProperties* s);
};

class MarshalVkPhysicalDeviceMemoryProperties {
public:
    MarshalVkPhysicalDeviceMemoryProperties() {}
    VkPhysicalDeviceMemoryProperties s;
    MarshalVkPhysicalDeviceMemoryProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryProperties* s);
};

class MarshalVkMemoryAllocateInfo {
public:
    MarshalVkMemoryAllocateInfo() {}
    VkMemoryAllocateInfo s;
    MarshalVkMemoryAllocateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryAllocateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryAllocateInfo* s);
    ~MarshalVkMemoryAllocateInfo();
};

class MarshalVkMemoryRequirements {
public:
    MarshalVkMemoryRequirements() {}
    VkMemoryRequirements s;
    MarshalVkMemoryRequirements(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryRequirements* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryRequirements* s);
};

class MarshalVkSparseImageFormatProperties {
public:
    MarshalVkSparseImageFormatProperties() {}
    VkSparseImageFormatProperties s;
    MarshalVkSparseImageFormatProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageFormatProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageFormatProperties* s);
};

class MarshalVkSparseImageMemoryRequirements {
public:
    MarshalVkSparseImageMemoryRequirements() {}
    VkSparseImageMemoryRequirements s;
    MarshalVkSparseImageMemoryRequirements(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageMemoryRequirements* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageMemoryRequirements* s);
};

class MarshalVkMemoryType {
public:
    MarshalVkMemoryType() {}
    VkMemoryType s;
    MarshalVkMemoryType(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryType* s);
};

class MarshalVkMemoryHeap {
public:
    MarshalVkMemoryHeap() {}
    VkMemoryHeap s;
    MarshalVkMemoryHeap(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryHeap* s);
};

class MarshalVkMappedMemoryRange {
public:
    MarshalVkMappedMemoryRange() {}
    VkMappedMemoryRange s;
    MarshalVkMappedMemoryRange(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMappedMemoryRange* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMappedMemoryRange* s);
    ~MarshalVkMappedMemoryRange();
};

class MarshalVkFormatProperties {
public:
    MarshalVkFormatProperties() {}
    VkFormatProperties s;
    MarshalVkFormatProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFormatProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFormatProperties* s);
};

class MarshalVkImageFormatProperties {
public:
    MarshalVkImageFormatProperties() {}
    VkImageFormatProperties s;
    MarshalVkImageFormatProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageFormatProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageFormatProperties* s);
};

class MarshalVkDescriptorBufferInfo {
public:
    MarshalVkDescriptorBufferInfo() {}
    VkDescriptorBufferInfo s;
    MarshalVkDescriptorBufferInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorBufferInfo* s);
};

class MarshalVkDescriptorImageInfo {
public:
    MarshalVkDescriptorImageInfo() {}
    VkDescriptorImageInfo s;
    MarshalVkDescriptorImageInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorImageInfo* s);
};

class MarshalVkWriteDescriptorSet {
public:
    MarshalVkWriteDescriptorSet() {}
    VkWriteDescriptorSet s;
    MarshalVkWriteDescriptorSet(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteDescriptorSet* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteDescriptorSet* s);
    ~MarshalVkWriteDescriptorSet();
};

class MarshalVkCopyDescriptorSet {
public:
    MarshalVkCopyDescriptorSet() {}
    VkCopyDescriptorSet s;
    MarshalVkCopyDescriptorSet(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyDescriptorSet* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyDescriptorSet* s);
    ~MarshalVkCopyDescriptorSet();
};

class MarshalVkBufferUsageFlags2CreateInfo {
public:
    MarshalVkBufferUsageFlags2CreateInfo() {}
    VkBufferUsageFlags2CreateInfo s;
    MarshalVkBufferUsageFlags2CreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferUsageFlags2CreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferUsageFlags2CreateInfo* s);
    ~MarshalVkBufferUsageFlags2CreateInfo();
};

class MarshalVkBufferCreateInfo {
public:
    MarshalVkBufferCreateInfo() {}
    VkBufferCreateInfo s;
    MarshalVkBufferCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferCreateInfo* s);
    ~MarshalVkBufferCreateInfo();
};

class MarshalVkBufferViewCreateInfo {
public:
    MarshalVkBufferViewCreateInfo() {}
    VkBufferViewCreateInfo s;
    MarshalVkBufferViewCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferViewCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferViewCreateInfo* s);
    ~MarshalVkBufferViewCreateInfo();
};

class MarshalVkImageSubresource {
public:
    MarshalVkImageSubresource() {}
    VkImageSubresource s;
    MarshalVkImageSubresource(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageSubresource* s);
};

class MarshalVkImageSubresourceLayers {
public:
    MarshalVkImageSubresourceLayers() {}
    VkImageSubresourceLayers s;
    MarshalVkImageSubresourceLayers(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageSubresourceLayers* s);
};

class MarshalVkImageSubresourceRange {
public:
    MarshalVkImageSubresourceRange() {}
    VkImageSubresourceRange s;
    MarshalVkImageSubresourceRange(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageSubresourceRange* s);
};

class MarshalVkMemoryBarrier {
public:
    MarshalVkMemoryBarrier() {}
    VkMemoryBarrier s;
    MarshalVkMemoryBarrier(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryBarrier* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryBarrier* s);
    ~MarshalVkMemoryBarrier();
};

class MarshalVkBufferMemoryBarrier {
public:
    MarshalVkBufferMemoryBarrier() {}
    VkBufferMemoryBarrier s;
    MarshalVkBufferMemoryBarrier(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferMemoryBarrier* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferMemoryBarrier* s);
    ~MarshalVkBufferMemoryBarrier();
};

class MarshalVkImageMemoryBarrier {
public:
    MarshalVkImageMemoryBarrier() {}
    VkImageMemoryBarrier s;
    MarshalVkImageMemoryBarrier(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageMemoryBarrier* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageMemoryBarrier* s);
    ~MarshalVkImageMemoryBarrier();
};

class MarshalVkImageCreateInfo {
public:
    MarshalVkImageCreateInfo() {}
    VkImageCreateInfo s;
    MarshalVkImageCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCreateInfo* s);
    ~MarshalVkImageCreateInfo();
};

class MarshalVkSubresourceLayout {
public:
    MarshalVkSubresourceLayout() {}
    VkSubresourceLayout s;
    MarshalVkSubresourceLayout(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubresourceLayout* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubresourceLayout* s);
};

class MarshalVkImageViewCreateInfo {
public:
    MarshalVkImageViewCreateInfo() {}
    VkImageViewCreateInfo s;
    MarshalVkImageViewCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewCreateInfo* s);
    ~MarshalVkImageViewCreateInfo();
};

class MarshalVkBufferCopy {
public:
    MarshalVkBufferCopy() {}
    VkBufferCopy s;
    MarshalVkBufferCopy(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferCopy* s);
};

class MarshalVkSparseMemoryBind {
public:
    MarshalVkSparseMemoryBind() {}
    VkSparseMemoryBind s;
    MarshalVkSparseMemoryBind(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseMemoryBind* s);
};

class MarshalVkSparseImageMemoryBind {
public:
    MarshalVkSparseImageMemoryBind() {}
    VkSparseImageMemoryBind s;
    MarshalVkSparseImageMemoryBind(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageMemoryBind* s);
};

class MarshalVkSparseBufferMemoryBindInfo {
public:
    MarshalVkSparseBufferMemoryBindInfo() {}
    VkSparseBufferMemoryBindInfo s;
    MarshalVkSparseBufferMemoryBindInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseBufferMemoryBindInfo* s);
    ~MarshalVkSparseBufferMemoryBindInfo();
};

class MarshalVkSparseImageOpaqueMemoryBindInfo {
public:
    MarshalVkSparseImageOpaqueMemoryBindInfo() {}
    VkSparseImageOpaqueMemoryBindInfo s;
    MarshalVkSparseImageOpaqueMemoryBindInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageOpaqueMemoryBindInfo* s);
    ~MarshalVkSparseImageOpaqueMemoryBindInfo();
};

class MarshalVkSparseImageMemoryBindInfo {
public:
    MarshalVkSparseImageMemoryBindInfo() {}
    VkSparseImageMemoryBindInfo s;
    MarshalVkSparseImageMemoryBindInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageMemoryBindInfo* s);
    ~MarshalVkSparseImageMemoryBindInfo();
};

class MarshalVkBindSparseInfo {
public:
    MarshalVkBindSparseInfo() {}
    VkBindSparseInfo s;
    MarshalVkBindSparseInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindSparseInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindSparseInfo* s);
    ~MarshalVkBindSparseInfo();
};

class MarshalVkImageCopy {
public:
    MarshalVkImageCopy() {}
    VkImageCopy s;
    MarshalVkImageCopy(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCopy* s);
};

class MarshalVkImageBlit {
public:
    MarshalVkImageBlit() {}
    VkImageBlit s;
    MarshalVkImageBlit(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageBlit* s);
};

class MarshalVkBufferImageCopy {
public:
    MarshalVkBufferImageCopy() {}
    VkBufferImageCopy s;
    MarshalVkBufferImageCopy(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferImageCopy* s);
};

class MarshalVkImageResolve {
public:
    MarshalVkImageResolve() {}
    VkImageResolve s;
    MarshalVkImageResolve(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageResolve* s);
};

class MarshalVkShaderModuleCreateInfo {
public:
    MarshalVkShaderModuleCreateInfo() {}
    VkShaderModuleCreateInfo s;
    MarshalVkShaderModuleCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkShaderModuleCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkShaderModuleCreateInfo* s);
    ~MarshalVkShaderModuleCreateInfo();
};

class MarshalVkDescriptorSetLayoutBinding {
public:
    MarshalVkDescriptorSetLayoutBinding() {}
    VkDescriptorSetLayoutBinding s;
    MarshalVkDescriptorSetLayoutBinding(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetLayoutBinding* s);
    ~MarshalVkDescriptorSetLayoutBinding();
};

class MarshalVkDescriptorSetLayoutCreateInfo {
public:
    MarshalVkDescriptorSetLayoutCreateInfo() {}
    VkDescriptorSetLayoutCreateInfo s;
    MarshalVkDescriptorSetLayoutCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetLayoutCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetLayoutCreateInfo* s);
    ~MarshalVkDescriptorSetLayoutCreateInfo();
};

class MarshalVkDescriptorPoolSize {
public:
    MarshalVkDescriptorPoolSize() {}
    VkDescriptorPoolSize s;
    MarshalVkDescriptorPoolSize(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorPoolSize* s);
};

class MarshalVkDescriptorPoolCreateInfo {
public:
    MarshalVkDescriptorPoolCreateInfo() {}
    VkDescriptorPoolCreateInfo s;
    MarshalVkDescriptorPoolCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorPoolCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorPoolCreateInfo* s);
    ~MarshalVkDescriptorPoolCreateInfo();
};

class MarshalVkDescriptorSetAllocateInfo {
public:
    MarshalVkDescriptorSetAllocateInfo() {}
    VkDescriptorSetAllocateInfo s;
    MarshalVkDescriptorSetAllocateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetAllocateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetAllocateInfo* s);
    ~MarshalVkDescriptorSetAllocateInfo();
};

class MarshalVkSpecializationMapEntry {
public:
    MarshalVkSpecializationMapEntry() {}
    VkSpecializationMapEntry s;
    MarshalVkSpecializationMapEntry(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSpecializationMapEntry* s);
};

class MarshalVkSpecializationInfo {
public:
    MarshalVkSpecializationInfo() {}
    VkSpecializationInfo s;
    MarshalVkSpecializationInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSpecializationInfo* s);
    ~MarshalVkSpecializationInfo();
};

class MarshalVkPipelineShaderStageCreateInfo {
public:
    MarshalVkPipelineShaderStageCreateInfo() {}
    VkPipelineShaderStageCreateInfo s;
    MarshalVkPipelineShaderStageCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineShaderStageCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineShaderStageCreateInfo* s);
    ~MarshalVkPipelineShaderStageCreateInfo();
};

class MarshalVkComputePipelineCreateInfo {
public:
    MarshalVkComputePipelineCreateInfo() {}
    VkComputePipelineCreateInfo s;
    MarshalVkComputePipelineCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkComputePipelineCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkComputePipelineCreateInfo* s);
    ~MarshalVkComputePipelineCreateInfo();
};

class MarshalVkComputePipelineIndirectBufferInfoNV {
public:
    MarshalVkComputePipelineIndirectBufferInfoNV() {}
    VkComputePipelineIndirectBufferInfoNV s;
    MarshalVkComputePipelineIndirectBufferInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkComputePipelineIndirectBufferInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkComputePipelineIndirectBufferInfoNV* s);
    ~MarshalVkComputePipelineIndirectBufferInfoNV();
};

class MarshalVkPipelineCreateFlags2CreateInfo {
public:
    MarshalVkPipelineCreateFlags2CreateInfo() {}
    VkPipelineCreateFlags2CreateInfo s;
    MarshalVkPipelineCreateFlags2CreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCreateFlags2CreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCreateFlags2CreateInfo* s);
    ~MarshalVkPipelineCreateFlags2CreateInfo();
};

class MarshalVkVertexInputBindingDescription {
public:
    MarshalVkVertexInputBindingDescription() {}
    VkVertexInputBindingDescription s;
    MarshalVkVertexInputBindingDescription(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVertexInputBindingDescription* s);
};

class MarshalVkVertexInputAttributeDescription {
public:
    MarshalVkVertexInputAttributeDescription() {}
    VkVertexInputAttributeDescription s;
    MarshalVkVertexInputAttributeDescription(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVertexInputAttributeDescription* s);
};

class MarshalVkPipelineVertexInputStateCreateInfo {
public:
    MarshalVkPipelineVertexInputStateCreateInfo() {}
    VkPipelineVertexInputStateCreateInfo s;
    MarshalVkPipelineVertexInputStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineVertexInputStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineVertexInputStateCreateInfo* s);
    ~MarshalVkPipelineVertexInputStateCreateInfo();
};

class MarshalVkPipelineInputAssemblyStateCreateInfo {
public:
    MarshalVkPipelineInputAssemblyStateCreateInfo() {}
    VkPipelineInputAssemblyStateCreateInfo s;
    MarshalVkPipelineInputAssemblyStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineInputAssemblyStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineInputAssemblyStateCreateInfo* s);
    ~MarshalVkPipelineInputAssemblyStateCreateInfo();
};

class MarshalVkPipelineTessellationStateCreateInfo {
public:
    MarshalVkPipelineTessellationStateCreateInfo() {}
    VkPipelineTessellationStateCreateInfo s;
    MarshalVkPipelineTessellationStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineTessellationStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineTessellationStateCreateInfo* s);
    ~MarshalVkPipelineTessellationStateCreateInfo();
};

class MarshalVkPipelineViewportStateCreateInfo {
public:
    MarshalVkPipelineViewportStateCreateInfo() {}
    VkPipelineViewportStateCreateInfo s;
    MarshalVkPipelineViewportStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportStateCreateInfo* s);
    ~MarshalVkPipelineViewportStateCreateInfo();
};

class MarshalVkPipelineRasterizationStateCreateInfo {
public:
    MarshalVkPipelineRasterizationStateCreateInfo() {}
    VkPipelineRasterizationStateCreateInfo s;
    MarshalVkPipelineRasterizationStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationStateCreateInfo* s);
    ~MarshalVkPipelineRasterizationStateCreateInfo();
};

class MarshalVkPipelineMultisampleStateCreateInfo {
public:
    MarshalVkPipelineMultisampleStateCreateInfo() {}
    VkPipelineMultisampleStateCreateInfo s;
    MarshalVkPipelineMultisampleStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineMultisampleStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineMultisampleStateCreateInfo* s);
    ~MarshalVkPipelineMultisampleStateCreateInfo();
};

class MarshalVkPipelineColorBlendAttachmentState {
public:
    MarshalVkPipelineColorBlendAttachmentState() {}
    VkPipelineColorBlendAttachmentState s;
    MarshalVkPipelineColorBlendAttachmentState(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineColorBlendAttachmentState* s);
};

class MarshalVkPipelineColorBlendStateCreateInfo {
public:
    MarshalVkPipelineColorBlendStateCreateInfo() {}
    VkPipelineColorBlendStateCreateInfo s;
    MarshalVkPipelineColorBlendStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineColorBlendStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineColorBlendStateCreateInfo* s);
    ~MarshalVkPipelineColorBlendStateCreateInfo();
};

class MarshalVkPipelineDynamicStateCreateInfo {
public:
    MarshalVkPipelineDynamicStateCreateInfo() {}
    VkPipelineDynamicStateCreateInfo s;
    MarshalVkPipelineDynamicStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineDynamicStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineDynamicStateCreateInfo* s);
    ~MarshalVkPipelineDynamicStateCreateInfo();
};

class MarshalVkStencilOpState {
public:
    MarshalVkStencilOpState() {}
    VkStencilOpState s;
    MarshalVkStencilOpState(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkStencilOpState* s);
};

class MarshalVkPipelineDepthStencilStateCreateInfo {
public:
    MarshalVkPipelineDepthStencilStateCreateInfo() {}
    VkPipelineDepthStencilStateCreateInfo s;
    MarshalVkPipelineDepthStencilStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineDepthStencilStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineDepthStencilStateCreateInfo* s);
    ~MarshalVkPipelineDepthStencilStateCreateInfo();
};

class MarshalVkGraphicsPipelineCreateInfo {
public:
    MarshalVkGraphicsPipelineCreateInfo() {}
    VkGraphicsPipelineCreateInfo s;
    MarshalVkGraphicsPipelineCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGraphicsPipelineCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGraphicsPipelineCreateInfo* s);
    ~MarshalVkGraphicsPipelineCreateInfo();
};

class MarshalVkPipelineCacheCreateInfo {
public:
    MarshalVkPipelineCacheCreateInfo() {}
    VkPipelineCacheCreateInfo s;
    MarshalVkPipelineCacheCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCacheCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCacheCreateInfo* s);
    ~MarshalVkPipelineCacheCreateInfo();
};

class MarshalVkPushConstantRange {
public:
    MarshalVkPushConstantRange() {}
    VkPushConstantRange s;
    MarshalVkPushConstantRange(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPushConstantRange* s);
};

class MarshalVkPipelineBinaryCreateInfoKHR {
public:
    MarshalVkPipelineBinaryCreateInfoKHR() {}
    VkPipelineBinaryCreateInfoKHR s;
    MarshalVkPipelineBinaryCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryCreateInfoKHR* s);
    ~MarshalVkPipelineBinaryCreateInfoKHR();
};

class MarshalVkPipelineBinaryHandlesInfoKHR {
public:
    MarshalVkPipelineBinaryHandlesInfoKHR() {}
    VkPipelineBinaryHandlesInfoKHR s;
    MarshalVkPipelineBinaryHandlesInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryHandlesInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryHandlesInfoKHR* s);
    ~MarshalVkPipelineBinaryHandlesInfoKHR();
};

class MarshalVkPipelineBinaryDataKHR {
public:
    MarshalVkPipelineBinaryDataKHR() {}
    VkPipelineBinaryDataKHR s;
    MarshalVkPipelineBinaryDataKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryDataKHR* s);
    ~MarshalVkPipelineBinaryDataKHR();
};

class MarshalVkPipelineBinaryKeysAndDataKHR {
public:
    MarshalVkPipelineBinaryKeysAndDataKHR() {}
    VkPipelineBinaryKeysAndDataKHR s;
    MarshalVkPipelineBinaryKeysAndDataKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryKeysAndDataKHR* s);
    ~MarshalVkPipelineBinaryKeysAndDataKHR();
};

class MarshalVkPipelineBinaryKeyKHR {
public:
    MarshalVkPipelineBinaryKeyKHR() {}
    VkPipelineBinaryKeyKHR s;
    MarshalVkPipelineBinaryKeyKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryKeyKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryKeyKHR* s);
    ~MarshalVkPipelineBinaryKeyKHR();
};

class MarshalVkPipelineBinaryInfoKHR {
public:
    MarshalVkPipelineBinaryInfoKHR() {}
    VkPipelineBinaryInfoKHR s;
    MarshalVkPipelineBinaryInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryInfoKHR* s);
    ~MarshalVkPipelineBinaryInfoKHR();
};

class MarshalVkReleaseCapturedPipelineDataInfoKHR {
public:
    MarshalVkReleaseCapturedPipelineDataInfoKHR() {}
    VkReleaseCapturedPipelineDataInfoKHR s;
    MarshalVkReleaseCapturedPipelineDataInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkReleaseCapturedPipelineDataInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkReleaseCapturedPipelineDataInfoKHR* s);
    ~MarshalVkReleaseCapturedPipelineDataInfoKHR();
};

class MarshalVkPipelineBinaryDataInfoKHR {
public:
    MarshalVkPipelineBinaryDataInfoKHR() {}
    VkPipelineBinaryDataInfoKHR s;
    MarshalVkPipelineBinaryDataInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryDataInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineBinaryDataInfoKHR* s);
    ~MarshalVkPipelineBinaryDataInfoKHR();
};

class MarshalVkPipelineCreateInfoKHR {
public:
    MarshalVkPipelineCreateInfoKHR() {}
    VkPipelineCreateInfoKHR s;
    MarshalVkPipelineCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCreateInfoKHR* s);
    ~MarshalVkPipelineCreateInfoKHR();
};

class MarshalVkPipelineLayoutCreateInfo {
public:
    MarshalVkPipelineLayoutCreateInfo() {}
    VkPipelineLayoutCreateInfo s;
    MarshalVkPipelineLayoutCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineLayoutCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineLayoutCreateInfo* s);
    ~MarshalVkPipelineLayoutCreateInfo();
};

class MarshalVkSamplerCreateInfo {
public:
    MarshalVkSamplerCreateInfo() {}
    VkSamplerCreateInfo s;
    MarshalVkSamplerCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerCreateInfo* s);
    ~MarshalVkSamplerCreateInfo();
};

class MarshalVkCommandPoolCreateInfo {
public:
    MarshalVkCommandPoolCreateInfo() {}
    VkCommandPoolCreateInfo s;
    MarshalVkCommandPoolCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandPoolCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandPoolCreateInfo* s);
    ~MarshalVkCommandPoolCreateInfo();
};

class MarshalVkCommandBufferAllocateInfo {
public:
    MarshalVkCommandBufferAllocateInfo() {}
    VkCommandBufferAllocateInfo s;
    MarshalVkCommandBufferAllocateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferAllocateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferAllocateInfo* s);
    ~MarshalVkCommandBufferAllocateInfo();
};

class MarshalVkCommandBufferInheritanceInfo {
public:
    MarshalVkCommandBufferInheritanceInfo() {}
    VkCommandBufferInheritanceInfo s;
    MarshalVkCommandBufferInheritanceInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceInfo* s);
    ~MarshalVkCommandBufferInheritanceInfo();
};

class MarshalVkCommandBufferBeginInfo {
public:
    MarshalVkCommandBufferBeginInfo() {}
    VkCommandBufferBeginInfo s;
    MarshalVkCommandBufferBeginInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferBeginInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferBeginInfo* s);
    ~MarshalVkCommandBufferBeginInfo();
};

class MarshalVkRenderPassBeginInfo {
public:
    MarshalVkRenderPassBeginInfo() {}
    VkRenderPassBeginInfo s;
    MarshalVkRenderPassBeginInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassBeginInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassBeginInfo* s);
    ~MarshalVkRenderPassBeginInfo();
};

class MarshalVkClearColorValue {
public:
    MarshalVkClearColorValue() {}
    VkClearColorValue s;
    MarshalVkClearColorValue(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkClearColorValue* s);
};

class MarshalVkClearDepthStencilValue {
public:
    MarshalVkClearDepthStencilValue() {}
    VkClearDepthStencilValue s;
    MarshalVkClearDepthStencilValue(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkClearDepthStencilValue* s);
};

class MarshalVkClearAttachment {
public:
    MarshalVkClearAttachment() {}
    VkClearAttachment s;
    MarshalVkClearAttachment(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkClearAttachment* s);
};

class MarshalVkAttachmentDescription {
public:
    MarshalVkAttachmentDescription() {}
    VkAttachmentDescription s;
    MarshalVkAttachmentDescription(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentDescription* s);
};

class MarshalVkAttachmentReference {
public:
    MarshalVkAttachmentReference() {}
    VkAttachmentReference s;
    MarshalVkAttachmentReference(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentReference* s);
};

class MarshalVkSubpassDescription {
public:
    MarshalVkSubpassDescription() {}
    VkSubpassDescription s;
    MarshalVkSubpassDescription(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassDescription* s);
    ~MarshalVkSubpassDescription();
};

class MarshalVkSubpassDependency {
public:
    MarshalVkSubpassDependency() {}
    VkSubpassDependency s;
    MarshalVkSubpassDependency(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassDependency* s);
};

class MarshalVkRenderPassCreateInfo {
public:
    MarshalVkRenderPassCreateInfo() {}
    VkRenderPassCreateInfo s;
    MarshalVkRenderPassCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassCreateInfo* s);
    ~MarshalVkRenderPassCreateInfo();
};

class MarshalVkEventCreateInfo {
public:
    MarshalVkEventCreateInfo() {}
    VkEventCreateInfo s;
    MarshalVkEventCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkEventCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkEventCreateInfo* s);
    ~MarshalVkEventCreateInfo();
};

class MarshalVkFenceCreateInfo {
public:
    MarshalVkFenceCreateInfo() {}
    VkFenceCreateInfo s;
    MarshalVkFenceCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFenceCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFenceCreateInfo* s);
    ~MarshalVkFenceCreateInfo();
};

class MarshalVkPhysicalDeviceFeatures {
public:
    MarshalVkPhysicalDeviceFeatures() {}
    VkPhysicalDeviceFeatures s;
    MarshalVkPhysicalDeviceFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFeatures* s);
};

class MarshalVkPhysicalDeviceSparseProperties {
public:
    MarshalVkPhysicalDeviceSparseProperties() {}
    VkPhysicalDeviceSparseProperties s;
    MarshalVkPhysicalDeviceSparseProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSparseProperties* s);
};

class MarshalVkPhysicalDeviceLimits {
public:
    MarshalVkPhysicalDeviceLimits() {}
    VkPhysicalDeviceLimits s;
    MarshalVkPhysicalDeviceLimits(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLimits* s);
};

class MarshalVkSemaphoreCreateInfo {
public:
    MarshalVkSemaphoreCreateInfo() {}
    VkSemaphoreCreateInfo s;
    MarshalVkSemaphoreCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreCreateInfo* s);
    ~MarshalVkSemaphoreCreateInfo();
};

class MarshalVkQueryPoolCreateInfo {
public:
    MarshalVkQueryPoolCreateInfo() {}
    VkQueryPoolCreateInfo s;
    MarshalVkQueryPoolCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryPoolCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryPoolCreateInfo* s);
    ~MarshalVkQueryPoolCreateInfo();
};

class MarshalVkFramebufferCreateInfo {
public:
    MarshalVkFramebufferCreateInfo() {}
    VkFramebufferCreateInfo s;
    MarshalVkFramebufferCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFramebufferCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFramebufferCreateInfo* s);
    ~MarshalVkFramebufferCreateInfo();
};

class MarshalVkMultiDrawInfoEXT {
public:
    MarshalVkMultiDrawInfoEXT() {}
    VkMultiDrawInfoEXT s;
    MarshalVkMultiDrawInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultiDrawInfoEXT* s);
};

class MarshalVkMultiDrawIndexedInfoEXT {
public:
    MarshalVkMultiDrawIndexedInfoEXT() {}
    VkMultiDrawIndexedInfoEXT s;
    MarshalVkMultiDrawIndexedInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultiDrawIndexedInfoEXT* s);
};

class MarshalVkSubmitInfo {
public:
    MarshalVkSubmitInfo() {}
    VkSubmitInfo s;
    MarshalVkSubmitInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubmitInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubmitInfo* s);
    ~MarshalVkSubmitInfo();
};

class MarshalVkDisplaySurfaceStereoCreateInfoNV {
public:
    MarshalVkDisplaySurfaceStereoCreateInfoNV() {}
    VkDisplaySurfaceStereoCreateInfoNV s;
    MarshalVkDisplaySurfaceStereoCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplaySurfaceStereoCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplaySurfaceStereoCreateInfoNV* s);
    ~MarshalVkDisplaySurfaceStereoCreateInfoNV();
};

class MarshalVkDisplayPresentInfoKHR {
public:
    MarshalVkDisplayPresentInfoKHR() {}
    VkDisplayPresentInfoKHR s;
    MarshalVkDisplayPresentInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPresentInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPresentInfoKHR* s);
    ~MarshalVkDisplayPresentInfoKHR();
};

class MarshalVkSurfaceCapabilitiesKHR {
public:
    MarshalVkSurfaceCapabilitiesKHR() {}
    VkSurfaceCapabilitiesKHR s;
    MarshalVkSurfaceCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceCapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceCapabilitiesKHR* s);
};

class MarshalVkSurfaceFormatKHR {
public:
    MarshalVkSurfaceFormatKHR() {}
    VkSurfaceFormatKHR s;
    MarshalVkSurfaceFormatKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceFormatKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceFormatKHR* s);
};

class MarshalVkSwapchainCreateInfoKHR {
public:
    MarshalVkSwapchainCreateInfoKHR() {}
    VkSwapchainCreateInfoKHR s;
    MarshalVkSwapchainCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainCreateInfoKHR* s);
    ~MarshalVkSwapchainCreateInfoKHR();
};

class MarshalVkPresentInfoKHR {
public:
    MarshalVkPresentInfoKHR() {}
    VkPresentInfoKHR s;
    MarshalVkPresentInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPresentInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPresentInfoKHR* s);
    ~MarshalVkPresentInfoKHR();
};

class MarshalVkDebugReportCallbackCreateInfoEXT {
public:
    MarshalVkDebugReportCallbackCreateInfoEXT() {}
    VkDebugReportCallbackCreateInfoEXT s;
    MarshalVkDebugReportCallbackCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugReportCallbackCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugReportCallbackCreateInfoEXT* s);
    ~MarshalVkDebugReportCallbackCreateInfoEXT();
};

class MarshalVkValidationFlagsEXT {
public:
    MarshalVkValidationFlagsEXT() {}
    VkValidationFlagsEXT s;
    MarshalVkValidationFlagsEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkValidationFlagsEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkValidationFlagsEXT* s);
    ~MarshalVkValidationFlagsEXT();
};

class MarshalVkValidationFeaturesEXT {
public:
    MarshalVkValidationFeaturesEXT() {}
    VkValidationFeaturesEXT s;
    MarshalVkValidationFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkValidationFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkValidationFeaturesEXT* s);
    ~MarshalVkValidationFeaturesEXT();
};

class MarshalVkLayerSettingsCreateInfoEXT {
public:
    MarshalVkLayerSettingsCreateInfoEXT() {}
    VkLayerSettingsCreateInfoEXT s;
    MarshalVkLayerSettingsCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLayerSettingsCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLayerSettingsCreateInfoEXT* s);
    ~MarshalVkLayerSettingsCreateInfoEXT();
};

class MarshalVkLayerSettingEXT {
public:
    MarshalVkLayerSettingEXT() {}
    VkLayerSettingEXT s;
    MarshalVkLayerSettingEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLayerSettingEXT* s);
    ~MarshalVkLayerSettingEXT();
};

class MarshalVkPipelineRasterizationStateRasterizationOrderAMD {
public:
    MarshalVkPipelineRasterizationStateRasterizationOrderAMD() {}
    VkPipelineRasterizationStateRasterizationOrderAMD s;
    MarshalVkPipelineRasterizationStateRasterizationOrderAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationStateRasterizationOrderAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationStateRasterizationOrderAMD* s);
    ~MarshalVkPipelineRasterizationStateRasterizationOrderAMD();
};

class MarshalVkDebugMarkerObjectNameInfoEXT {
public:
    MarshalVkDebugMarkerObjectNameInfoEXT() {}
    VkDebugMarkerObjectNameInfoEXT s;
    MarshalVkDebugMarkerObjectNameInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugMarkerObjectNameInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugMarkerObjectNameInfoEXT* s);
    ~MarshalVkDebugMarkerObjectNameInfoEXT();
};

class MarshalVkDebugMarkerObjectTagInfoEXT {
public:
    MarshalVkDebugMarkerObjectTagInfoEXT() {}
    VkDebugMarkerObjectTagInfoEXT s;
    MarshalVkDebugMarkerObjectTagInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugMarkerObjectTagInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugMarkerObjectTagInfoEXT* s);
    ~MarshalVkDebugMarkerObjectTagInfoEXT();
};

class MarshalVkDebugMarkerMarkerInfoEXT {
public:
    MarshalVkDebugMarkerMarkerInfoEXT() {}
    VkDebugMarkerMarkerInfoEXT s;
    MarshalVkDebugMarkerMarkerInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugMarkerMarkerInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugMarkerMarkerInfoEXT* s);
    ~MarshalVkDebugMarkerMarkerInfoEXT();
};

class MarshalVkDedicatedAllocationImageCreateInfoNV {
public:
    MarshalVkDedicatedAllocationImageCreateInfoNV() {}
    VkDedicatedAllocationImageCreateInfoNV s;
    MarshalVkDedicatedAllocationImageCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDedicatedAllocationImageCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDedicatedAllocationImageCreateInfoNV* s);
    ~MarshalVkDedicatedAllocationImageCreateInfoNV();
};

class MarshalVkDedicatedAllocationBufferCreateInfoNV {
public:
    MarshalVkDedicatedAllocationBufferCreateInfoNV() {}
    VkDedicatedAllocationBufferCreateInfoNV s;
    MarshalVkDedicatedAllocationBufferCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDedicatedAllocationBufferCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDedicatedAllocationBufferCreateInfoNV* s);
    ~MarshalVkDedicatedAllocationBufferCreateInfoNV();
};

class MarshalVkDedicatedAllocationMemoryAllocateInfoNV {
public:
    MarshalVkDedicatedAllocationMemoryAllocateInfoNV() {}
    VkDedicatedAllocationMemoryAllocateInfoNV s;
    MarshalVkDedicatedAllocationMemoryAllocateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDedicatedAllocationMemoryAllocateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDedicatedAllocationMemoryAllocateInfoNV* s);
    ~MarshalVkDedicatedAllocationMemoryAllocateInfoNV();
};

class MarshalVkExternalMemoryImageCreateInfoNV {
public:
    MarshalVkExternalMemoryImageCreateInfoNV() {}
    VkExternalMemoryImageCreateInfoNV s;
    MarshalVkExternalMemoryImageCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalMemoryImageCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalMemoryImageCreateInfoNV* s);
    ~MarshalVkExternalMemoryImageCreateInfoNV();
};

class MarshalVkExportMemoryAllocateInfoNV {
public:
    MarshalVkExportMemoryAllocateInfoNV() {}
    VkExportMemoryAllocateInfoNV s;
    MarshalVkExportMemoryAllocateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExportMemoryAllocateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExportMemoryAllocateInfoNV* s);
    ~MarshalVkExportMemoryAllocateInfoNV();
};

class MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV {
public:
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV() {}
    VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV s;
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV* s);
    ~MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV();
};

class MarshalVkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV {
public:
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV() {}
    VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV s;
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV* s);
    ~MarshalVkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV();
};

class MarshalVkDevicePrivateDataCreateInfo {
public:
    MarshalVkDevicePrivateDataCreateInfo() {}
    VkDevicePrivateDataCreateInfo s;
    MarshalVkDevicePrivateDataCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDevicePrivateDataCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDevicePrivateDataCreateInfo* s);
    ~MarshalVkDevicePrivateDataCreateInfo();
};

class MarshalVkPrivateDataSlotCreateInfo {
public:
    MarshalVkPrivateDataSlotCreateInfo() {}
    VkPrivateDataSlotCreateInfo s;
    MarshalVkPrivateDataSlotCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPrivateDataSlotCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPrivateDataSlotCreateInfo* s);
    ~MarshalVkPrivateDataSlotCreateInfo();
};

class MarshalVkPhysicalDevicePrivateDataFeatures {
public:
    MarshalVkPhysicalDevicePrivateDataFeatures() {}
    VkPhysicalDevicePrivateDataFeatures s;
    MarshalVkPhysicalDevicePrivateDataFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePrivateDataFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePrivateDataFeatures* s);
    ~MarshalVkPhysicalDevicePrivateDataFeatures();
};

class MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV {
public:
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV() {}
    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV s;
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV* s);
    ~MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV();
};

class MarshalVkPhysicalDeviceMultiDrawPropertiesEXT {
public:
    MarshalVkPhysicalDeviceMultiDrawPropertiesEXT() {}
    VkPhysicalDeviceMultiDrawPropertiesEXT s;
    MarshalVkPhysicalDeviceMultiDrawPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiDrawPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiDrawPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceMultiDrawPropertiesEXT();
};

class MarshalVkGraphicsShaderGroupCreateInfoNV {
public:
    MarshalVkGraphicsShaderGroupCreateInfoNV() {}
    VkGraphicsShaderGroupCreateInfoNV s;
    MarshalVkGraphicsShaderGroupCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGraphicsShaderGroupCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGraphicsShaderGroupCreateInfoNV* s);
    ~MarshalVkGraphicsShaderGroupCreateInfoNV();
};

class MarshalVkGraphicsPipelineShaderGroupsCreateInfoNV {
public:
    MarshalVkGraphicsPipelineShaderGroupsCreateInfoNV() {}
    VkGraphicsPipelineShaderGroupsCreateInfoNV s;
    MarshalVkGraphicsPipelineShaderGroupsCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGraphicsPipelineShaderGroupsCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGraphicsPipelineShaderGroupsCreateInfoNV* s);
    ~MarshalVkGraphicsPipelineShaderGroupsCreateInfoNV();
};

class MarshalVkIndirectCommandsStreamNV {
public:
    MarshalVkIndirectCommandsStreamNV() {}
    VkIndirectCommandsStreamNV s;
    MarshalVkIndirectCommandsStreamNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectCommandsStreamNV* s);
};

class MarshalVkIndirectCommandsLayoutTokenNV {
public:
    MarshalVkIndirectCommandsLayoutTokenNV() {}
    VkIndirectCommandsLayoutTokenNV s;
    MarshalVkIndirectCommandsLayoutTokenNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectCommandsLayoutTokenNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectCommandsLayoutTokenNV* s);
    ~MarshalVkIndirectCommandsLayoutTokenNV();
};

class MarshalVkIndirectCommandsLayoutCreateInfoNV {
public:
    MarshalVkIndirectCommandsLayoutCreateInfoNV() {}
    VkIndirectCommandsLayoutCreateInfoNV s;
    MarshalVkIndirectCommandsLayoutCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectCommandsLayoutCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectCommandsLayoutCreateInfoNV* s);
    ~MarshalVkIndirectCommandsLayoutCreateInfoNV();
};

class MarshalVkGeneratedCommandsInfoNV {
public:
    MarshalVkGeneratedCommandsInfoNV() {}
    VkGeneratedCommandsInfoNV s;
    MarshalVkGeneratedCommandsInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsInfoNV* s);
    ~MarshalVkGeneratedCommandsInfoNV();
};

class MarshalVkGeneratedCommandsMemoryRequirementsInfoNV {
public:
    MarshalVkGeneratedCommandsMemoryRequirementsInfoNV() {}
    VkGeneratedCommandsMemoryRequirementsInfoNV s;
    MarshalVkGeneratedCommandsMemoryRequirementsInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsMemoryRequirementsInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsMemoryRequirementsInfoNV* s);
    ~MarshalVkGeneratedCommandsMemoryRequirementsInfoNV();
};

class MarshalVkPipelineIndirectDeviceAddressInfoNV {
public:
    MarshalVkPipelineIndirectDeviceAddressInfoNV() {}
    VkPipelineIndirectDeviceAddressInfoNV s;
    MarshalVkPipelineIndirectDeviceAddressInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineIndirectDeviceAddressInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineIndirectDeviceAddressInfoNV* s);
    ~MarshalVkPipelineIndirectDeviceAddressInfoNV();
};

class MarshalVkPhysicalDeviceFeatures2 {
public:
    MarshalVkPhysicalDeviceFeatures2() {}
    VkPhysicalDeviceFeatures2 s;
    MarshalVkPhysicalDeviceFeatures2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFeatures2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFeatures2* s);
    ~MarshalVkPhysicalDeviceFeatures2();
};

class MarshalVkPhysicalDeviceProperties2 {
public:
    MarshalVkPhysicalDeviceProperties2() {}
    VkPhysicalDeviceProperties2 s;
    MarshalVkPhysicalDeviceProperties2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProperties2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProperties2* s);
    ~MarshalVkPhysicalDeviceProperties2();
};

class MarshalVkFormatProperties2 {
public:
    MarshalVkFormatProperties2() {}
    VkFormatProperties2 s;
    MarshalVkFormatProperties2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFormatProperties2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFormatProperties2* s);
    ~MarshalVkFormatProperties2();
};

class MarshalVkImageFormatProperties2 {
public:
    MarshalVkImageFormatProperties2() {}
    VkImageFormatProperties2 s;
    MarshalVkImageFormatProperties2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageFormatProperties2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageFormatProperties2* s);
    ~MarshalVkImageFormatProperties2();
};

class MarshalVkPhysicalDeviceImageFormatInfo2 {
public:
    MarshalVkPhysicalDeviceImageFormatInfo2() {}
    VkPhysicalDeviceImageFormatInfo2 s;
    MarshalVkPhysicalDeviceImageFormatInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageFormatInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageFormatInfo2* s);
    ~MarshalVkPhysicalDeviceImageFormatInfo2();
};

class MarshalVkQueueFamilyProperties2 {
public:
    MarshalVkQueueFamilyProperties2() {}
    VkQueueFamilyProperties2 s;
    MarshalVkQueueFamilyProperties2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyProperties2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyProperties2* s);
    ~MarshalVkQueueFamilyProperties2();
};

class MarshalVkPhysicalDeviceMemoryProperties2 {
public:
    MarshalVkPhysicalDeviceMemoryProperties2() {}
    VkPhysicalDeviceMemoryProperties2 s;
    MarshalVkPhysicalDeviceMemoryProperties2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryProperties2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryProperties2* s);
    ~MarshalVkPhysicalDeviceMemoryProperties2();
};

class MarshalVkSparseImageFormatProperties2 {
public:
    MarshalVkSparseImageFormatProperties2() {}
    VkSparseImageFormatProperties2 s;
    MarshalVkSparseImageFormatProperties2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageFormatProperties2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageFormatProperties2* s);
    ~MarshalVkSparseImageFormatProperties2();
};

class MarshalVkPhysicalDeviceSparseImageFormatInfo2 {
public:
    MarshalVkPhysicalDeviceSparseImageFormatInfo2() {}
    VkPhysicalDeviceSparseImageFormatInfo2 s;
    MarshalVkPhysicalDeviceSparseImageFormatInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSparseImageFormatInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSparseImageFormatInfo2* s);
    ~MarshalVkPhysicalDeviceSparseImageFormatInfo2();
};

class MarshalVkPhysicalDevicePushDescriptorProperties {
public:
    MarshalVkPhysicalDevicePushDescriptorProperties() {}
    VkPhysicalDevicePushDescriptorProperties s;
    MarshalVkPhysicalDevicePushDescriptorProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePushDescriptorProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePushDescriptorProperties* s);
    ~MarshalVkPhysicalDevicePushDescriptorProperties();
};

class MarshalVkConformanceVersion {
public:
    MarshalVkConformanceVersion() {}
    VkConformanceVersion s;
    MarshalVkConformanceVersion(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkConformanceVersion* s);
};

class MarshalVkPhysicalDeviceDriverProperties {
public:
    MarshalVkPhysicalDeviceDriverProperties() {}
    VkPhysicalDeviceDriverProperties s;
    MarshalVkPhysicalDeviceDriverProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDriverProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDriverProperties* s);
    ~MarshalVkPhysicalDeviceDriverProperties();
};

class MarshalVkPresentRegionsKHR {
public:
    MarshalVkPresentRegionsKHR() {}
    VkPresentRegionsKHR s;
    MarshalVkPresentRegionsKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPresentRegionsKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPresentRegionsKHR* s);
    ~MarshalVkPresentRegionsKHR();
};

class MarshalVkPresentRegionKHR {
public:
    MarshalVkPresentRegionKHR() {}
    VkPresentRegionKHR s;
    MarshalVkPresentRegionKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPresentRegionKHR* s);
    ~MarshalVkPresentRegionKHR();
};

class MarshalVkRectLayerKHR {
public:
    MarshalVkRectLayerKHR() {}
    VkRectLayerKHR s;
    MarshalVkRectLayerKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRectLayerKHR* s);
};

class MarshalVkPhysicalDeviceVariablePointersFeatures {
public:
    MarshalVkPhysicalDeviceVariablePointersFeatures() {}
    VkPhysicalDeviceVariablePointersFeatures s;
    MarshalVkPhysicalDeviceVariablePointersFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVariablePointersFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVariablePointersFeatures* s);
    ~MarshalVkPhysicalDeviceVariablePointersFeatures();
};

class MarshalVkExternalMemoryProperties {
public:
    MarshalVkExternalMemoryProperties() {}
    VkExternalMemoryProperties s;
    MarshalVkExternalMemoryProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalMemoryProperties* s);
};

class MarshalVkPhysicalDeviceExternalImageFormatInfo {
public:
    MarshalVkPhysicalDeviceExternalImageFormatInfo() {}
    VkPhysicalDeviceExternalImageFormatInfo s;
    MarshalVkPhysicalDeviceExternalImageFormatInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalImageFormatInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalImageFormatInfo* s);
    ~MarshalVkPhysicalDeviceExternalImageFormatInfo();
};

class MarshalVkExternalImageFormatProperties {
public:
    MarshalVkExternalImageFormatProperties() {}
    VkExternalImageFormatProperties s;
    MarshalVkExternalImageFormatProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalImageFormatProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalImageFormatProperties* s);
    ~MarshalVkExternalImageFormatProperties();
};

class MarshalVkPhysicalDeviceExternalBufferInfo {
public:
    MarshalVkPhysicalDeviceExternalBufferInfo() {}
    VkPhysicalDeviceExternalBufferInfo s;
    MarshalVkPhysicalDeviceExternalBufferInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalBufferInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalBufferInfo* s);
    ~MarshalVkPhysicalDeviceExternalBufferInfo();
};

class MarshalVkExternalBufferProperties {
public:
    MarshalVkExternalBufferProperties() {}
    VkExternalBufferProperties s;
    MarshalVkExternalBufferProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalBufferProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalBufferProperties* s);
    ~MarshalVkExternalBufferProperties();
};

class MarshalVkPhysicalDeviceIDProperties {
public:
    MarshalVkPhysicalDeviceIDProperties() {}
    VkPhysicalDeviceIDProperties s;
    MarshalVkPhysicalDeviceIDProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceIDProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceIDProperties* s);
    ~MarshalVkPhysicalDeviceIDProperties();
};

class MarshalVkExternalMemoryImageCreateInfo {
public:
    MarshalVkExternalMemoryImageCreateInfo() {}
    VkExternalMemoryImageCreateInfo s;
    MarshalVkExternalMemoryImageCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalMemoryImageCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalMemoryImageCreateInfo* s);
    ~MarshalVkExternalMemoryImageCreateInfo();
};

class MarshalVkExternalMemoryBufferCreateInfo {
public:
    MarshalVkExternalMemoryBufferCreateInfo() {}
    VkExternalMemoryBufferCreateInfo s;
    MarshalVkExternalMemoryBufferCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalMemoryBufferCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalMemoryBufferCreateInfo* s);
    ~MarshalVkExternalMemoryBufferCreateInfo();
};

class MarshalVkExportMemoryAllocateInfo {
public:
    MarshalVkExportMemoryAllocateInfo() {}
    VkExportMemoryAllocateInfo s;
    MarshalVkExportMemoryAllocateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExportMemoryAllocateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExportMemoryAllocateInfo* s);
    ~MarshalVkExportMemoryAllocateInfo();
};

class MarshalVkPhysicalDeviceExternalSemaphoreInfo {
public:
    MarshalVkPhysicalDeviceExternalSemaphoreInfo() {}
    VkPhysicalDeviceExternalSemaphoreInfo s;
    MarshalVkPhysicalDeviceExternalSemaphoreInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalSemaphoreInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalSemaphoreInfo* s);
    ~MarshalVkPhysicalDeviceExternalSemaphoreInfo();
};

class MarshalVkExternalSemaphoreProperties {
public:
    MarshalVkExternalSemaphoreProperties() {}
    VkExternalSemaphoreProperties s;
    MarshalVkExternalSemaphoreProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalSemaphoreProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalSemaphoreProperties* s);
    ~MarshalVkExternalSemaphoreProperties();
};

class MarshalVkExportSemaphoreCreateInfo {
public:
    MarshalVkExportSemaphoreCreateInfo() {}
    VkExportSemaphoreCreateInfo s;
    MarshalVkExportSemaphoreCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExportSemaphoreCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExportSemaphoreCreateInfo* s);
    ~MarshalVkExportSemaphoreCreateInfo();
};

class MarshalVkPhysicalDeviceExternalFenceInfo {
public:
    MarshalVkPhysicalDeviceExternalFenceInfo() {}
    VkPhysicalDeviceExternalFenceInfo s;
    MarshalVkPhysicalDeviceExternalFenceInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalFenceInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalFenceInfo* s);
    ~MarshalVkPhysicalDeviceExternalFenceInfo();
};

class MarshalVkExternalFenceProperties {
public:
    MarshalVkExternalFenceProperties() {}
    VkExternalFenceProperties s;
    MarshalVkExternalFenceProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalFenceProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalFenceProperties* s);
    ~MarshalVkExternalFenceProperties();
};

class MarshalVkExportFenceCreateInfo {
public:
    MarshalVkExportFenceCreateInfo() {}
    VkExportFenceCreateInfo s;
    MarshalVkExportFenceCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExportFenceCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExportFenceCreateInfo* s);
    ~MarshalVkExportFenceCreateInfo();
};

class MarshalVkPhysicalDeviceMultiviewFeatures {
public:
    MarshalVkPhysicalDeviceMultiviewFeatures() {}
    VkPhysicalDeviceMultiviewFeatures s;
    MarshalVkPhysicalDeviceMultiviewFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewFeatures* s);
    ~MarshalVkPhysicalDeviceMultiviewFeatures();
};

class MarshalVkPhysicalDeviceMultiviewProperties {
public:
    MarshalVkPhysicalDeviceMultiviewProperties() {}
    VkPhysicalDeviceMultiviewProperties s;
    MarshalVkPhysicalDeviceMultiviewProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewProperties* s);
    ~MarshalVkPhysicalDeviceMultiviewProperties();
};

class MarshalVkRenderPassMultiviewCreateInfo {
public:
    MarshalVkRenderPassMultiviewCreateInfo() {}
    VkRenderPassMultiviewCreateInfo s;
    MarshalVkRenderPassMultiviewCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassMultiviewCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassMultiviewCreateInfo* s);
    ~MarshalVkRenderPassMultiviewCreateInfo();
};

class MarshalVkSurfaceCapabilities2EXT {
public:
    MarshalVkSurfaceCapabilities2EXT() {}
    VkSurfaceCapabilities2EXT s;
    MarshalVkSurfaceCapabilities2EXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceCapabilities2EXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceCapabilities2EXT* s);
    ~MarshalVkSurfaceCapabilities2EXT();
};

class MarshalVkDisplayPowerInfoEXT {
public:
    MarshalVkDisplayPowerInfoEXT() {}
    VkDisplayPowerInfoEXT s;
    MarshalVkDisplayPowerInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPowerInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPowerInfoEXT* s);
    ~MarshalVkDisplayPowerInfoEXT();
};

class MarshalVkDeviceEventInfoEXT {
public:
    MarshalVkDeviceEventInfoEXT() {}
    VkDeviceEventInfoEXT s;
    MarshalVkDeviceEventInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceEventInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceEventInfoEXT* s);
    ~MarshalVkDeviceEventInfoEXT();
};

class MarshalVkDisplayEventInfoEXT {
public:
    MarshalVkDisplayEventInfoEXT() {}
    VkDisplayEventInfoEXT s;
    MarshalVkDisplayEventInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayEventInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayEventInfoEXT* s);
    ~MarshalVkDisplayEventInfoEXT();
};

class MarshalVkSwapchainCounterCreateInfoEXT {
public:
    MarshalVkSwapchainCounterCreateInfoEXT() {}
    VkSwapchainCounterCreateInfoEXT s;
    MarshalVkSwapchainCounterCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainCounterCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainCounterCreateInfoEXT* s);
    ~MarshalVkSwapchainCounterCreateInfoEXT();
};

class MarshalVkPhysicalDeviceGroupProperties {
public:
    MarshalVkPhysicalDeviceGroupProperties() {}
    VkPhysicalDeviceGroupProperties s;
    MarshalVkPhysicalDeviceGroupProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceGroupProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceGroupProperties* s);
    ~MarshalVkPhysicalDeviceGroupProperties();
};

class MarshalVkMemoryAllocateFlagsInfo {
public:
    MarshalVkMemoryAllocateFlagsInfo() {}
    VkMemoryAllocateFlagsInfo s;
    MarshalVkMemoryAllocateFlagsInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryAllocateFlagsInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryAllocateFlagsInfo* s);
    ~MarshalVkMemoryAllocateFlagsInfo();
};

class MarshalVkBindBufferMemoryInfo {
public:
    MarshalVkBindBufferMemoryInfo() {}
    VkBindBufferMemoryInfo s;
    MarshalVkBindBufferMemoryInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindBufferMemoryInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindBufferMemoryInfo* s);
    ~MarshalVkBindBufferMemoryInfo();
};

class MarshalVkBindBufferMemoryDeviceGroupInfo {
public:
    MarshalVkBindBufferMemoryDeviceGroupInfo() {}
    VkBindBufferMemoryDeviceGroupInfo s;
    MarshalVkBindBufferMemoryDeviceGroupInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindBufferMemoryDeviceGroupInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindBufferMemoryDeviceGroupInfo* s);
    ~MarshalVkBindBufferMemoryDeviceGroupInfo();
};

class MarshalVkBindImageMemoryInfo {
public:
    MarshalVkBindImageMemoryInfo() {}
    VkBindImageMemoryInfo s;
    MarshalVkBindImageMemoryInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindImageMemoryInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindImageMemoryInfo* s);
    ~MarshalVkBindImageMemoryInfo();
};

class MarshalVkBindImageMemoryDeviceGroupInfo {
public:
    MarshalVkBindImageMemoryDeviceGroupInfo() {}
    VkBindImageMemoryDeviceGroupInfo s;
    MarshalVkBindImageMemoryDeviceGroupInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindImageMemoryDeviceGroupInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindImageMemoryDeviceGroupInfo* s);
    ~MarshalVkBindImageMemoryDeviceGroupInfo();
};

class MarshalVkDeviceGroupRenderPassBeginInfo {
public:
    MarshalVkDeviceGroupRenderPassBeginInfo() {}
    VkDeviceGroupRenderPassBeginInfo s;
    MarshalVkDeviceGroupRenderPassBeginInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupRenderPassBeginInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupRenderPassBeginInfo* s);
    ~MarshalVkDeviceGroupRenderPassBeginInfo();
};

class MarshalVkDeviceGroupCommandBufferBeginInfo {
public:
    MarshalVkDeviceGroupCommandBufferBeginInfo() {}
    VkDeviceGroupCommandBufferBeginInfo s;
    MarshalVkDeviceGroupCommandBufferBeginInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupCommandBufferBeginInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupCommandBufferBeginInfo* s);
    ~MarshalVkDeviceGroupCommandBufferBeginInfo();
};

class MarshalVkDeviceGroupSubmitInfo {
public:
    MarshalVkDeviceGroupSubmitInfo() {}
    VkDeviceGroupSubmitInfo s;
    MarshalVkDeviceGroupSubmitInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupSubmitInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupSubmitInfo* s);
    ~MarshalVkDeviceGroupSubmitInfo();
};

class MarshalVkDeviceGroupBindSparseInfo {
public:
    MarshalVkDeviceGroupBindSparseInfo() {}
    VkDeviceGroupBindSparseInfo s;
    MarshalVkDeviceGroupBindSparseInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupBindSparseInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupBindSparseInfo* s);
    ~MarshalVkDeviceGroupBindSparseInfo();
};

class MarshalVkDeviceGroupPresentCapabilitiesKHR {
public:
    MarshalVkDeviceGroupPresentCapabilitiesKHR() {}
    VkDeviceGroupPresentCapabilitiesKHR s;
    MarshalVkDeviceGroupPresentCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupPresentCapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupPresentCapabilitiesKHR* s);
    ~MarshalVkDeviceGroupPresentCapabilitiesKHR();
};

class MarshalVkImageSwapchainCreateInfoKHR {
public:
    MarshalVkImageSwapchainCreateInfoKHR() {}
    VkImageSwapchainCreateInfoKHR s;
    MarshalVkImageSwapchainCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageSwapchainCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageSwapchainCreateInfoKHR* s);
    ~MarshalVkImageSwapchainCreateInfoKHR();
};

class MarshalVkBindImageMemorySwapchainInfoKHR {
public:
    MarshalVkBindImageMemorySwapchainInfoKHR() {}
    VkBindImageMemorySwapchainInfoKHR s;
    MarshalVkBindImageMemorySwapchainInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindImageMemorySwapchainInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindImageMemorySwapchainInfoKHR* s);
    ~MarshalVkBindImageMemorySwapchainInfoKHR();
};

class MarshalVkAcquireNextImageInfoKHR {
public:
    MarshalVkAcquireNextImageInfoKHR() {}
    VkAcquireNextImageInfoKHR s;
    MarshalVkAcquireNextImageInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAcquireNextImageInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAcquireNextImageInfoKHR* s);
    ~MarshalVkAcquireNextImageInfoKHR();
};

class MarshalVkDeviceGroupPresentInfoKHR {
public:
    MarshalVkDeviceGroupPresentInfoKHR() {}
    VkDeviceGroupPresentInfoKHR s;
    MarshalVkDeviceGroupPresentInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupPresentInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupPresentInfoKHR* s);
    ~MarshalVkDeviceGroupPresentInfoKHR();
};

class MarshalVkDeviceGroupDeviceCreateInfo {
public:
    MarshalVkDeviceGroupDeviceCreateInfo() {}
    VkDeviceGroupDeviceCreateInfo s;
    MarshalVkDeviceGroupDeviceCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupDeviceCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupDeviceCreateInfo* s);
    ~MarshalVkDeviceGroupDeviceCreateInfo();
};

class MarshalVkDeviceGroupSwapchainCreateInfoKHR {
public:
    MarshalVkDeviceGroupSwapchainCreateInfoKHR() {}
    VkDeviceGroupSwapchainCreateInfoKHR s;
    MarshalVkDeviceGroupSwapchainCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupSwapchainCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceGroupSwapchainCreateInfoKHR* s);
    ~MarshalVkDeviceGroupSwapchainCreateInfoKHR();
};

class MarshalVkDescriptorUpdateTemplateEntry {
public:
    MarshalVkDescriptorUpdateTemplateEntry() {}
    VkDescriptorUpdateTemplateEntry s;
    MarshalVkDescriptorUpdateTemplateEntry(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorUpdateTemplateEntry* s);
};

class MarshalVkDescriptorUpdateTemplateCreateInfo {
public:
    MarshalVkDescriptorUpdateTemplateCreateInfo() {}
    VkDescriptorUpdateTemplateCreateInfo s;
    MarshalVkDescriptorUpdateTemplateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorUpdateTemplateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorUpdateTemplateCreateInfo* s);
    ~MarshalVkDescriptorUpdateTemplateCreateInfo();
};

class MarshalVkXYColorEXT {
public:
    MarshalVkXYColorEXT() {}
    VkXYColorEXT s;
    MarshalVkXYColorEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkXYColorEXT* s);
};

class MarshalVkPhysicalDevicePresentIdFeaturesKHR {
public:
    MarshalVkPhysicalDevicePresentIdFeaturesKHR() {}
    VkPhysicalDevicePresentIdFeaturesKHR s;
    MarshalVkPhysicalDevicePresentIdFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePresentIdFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePresentIdFeaturesKHR* s);
    ~MarshalVkPhysicalDevicePresentIdFeaturesKHR();
};

class MarshalVkPresentIdKHR {
public:
    MarshalVkPresentIdKHR() {}
    VkPresentIdKHR s;
    MarshalVkPresentIdKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPresentIdKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPresentIdKHR* s);
    ~MarshalVkPresentIdKHR();
};

class MarshalVkPhysicalDevicePresentWaitFeaturesKHR {
public:
    MarshalVkPhysicalDevicePresentWaitFeaturesKHR() {}
    VkPhysicalDevicePresentWaitFeaturesKHR s;
    MarshalVkPhysicalDevicePresentWaitFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePresentWaitFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePresentWaitFeaturesKHR* s);
    ~MarshalVkPhysicalDevicePresentWaitFeaturesKHR();
};

class MarshalVkHdrMetadataEXT {
public:
    MarshalVkHdrMetadataEXT() {}
    VkHdrMetadataEXT s;
    MarshalVkHdrMetadataEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkHdrMetadataEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkHdrMetadataEXT* s);
    ~MarshalVkHdrMetadataEXT();
};

class MarshalVkHdrVividDynamicMetadataHUAWEI {
public:
    MarshalVkHdrVividDynamicMetadataHUAWEI() {}
    VkHdrVividDynamicMetadataHUAWEI s;
    MarshalVkHdrVividDynamicMetadataHUAWEI(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkHdrVividDynamicMetadataHUAWEI* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkHdrVividDynamicMetadataHUAWEI* s);
    ~MarshalVkHdrVividDynamicMetadataHUAWEI();
};

class MarshalVkViewportWScalingNV {
public:
    MarshalVkViewportWScalingNV() {}
    VkViewportWScalingNV s;
    MarshalVkViewportWScalingNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkViewportWScalingNV* s);
};

class MarshalVkPipelineViewportWScalingStateCreateInfoNV {
public:
    MarshalVkPipelineViewportWScalingStateCreateInfoNV() {}
    VkPipelineViewportWScalingStateCreateInfoNV s;
    MarshalVkPipelineViewportWScalingStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportWScalingStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportWScalingStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportWScalingStateCreateInfoNV();
};

class MarshalVkViewportSwizzleNV {
public:
    MarshalVkViewportSwizzleNV() {}
    VkViewportSwizzleNV s;
    MarshalVkViewportSwizzleNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkViewportSwizzleNV* s);
};

class MarshalVkPipelineViewportSwizzleStateCreateInfoNV {
public:
    MarshalVkPipelineViewportSwizzleStateCreateInfoNV() {}
    VkPipelineViewportSwizzleStateCreateInfoNV s;
    MarshalVkPipelineViewportSwizzleStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportSwizzleStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportSwizzleStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportSwizzleStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceDiscardRectanglePropertiesEXT {
public:
    MarshalVkPhysicalDeviceDiscardRectanglePropertiesEXT() {}
    VkPhysicalDeviceDiscardRectanglePropertiesEXT s;
    MarshalVkPhysicalDeviceDiscardRectanglePropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDiscardRectanglePropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDiscardRectanglePropertiesEXT* s);
    ~MarshalVkPhysicalDeviceDiscardRectanglePropertiesEXT();
};

class MarshalVkPipelineDiscardRectangleStateCreateInfoEXT {
public:
    MarshalVkPipelineDiscardRectangleStateCreateInfoEXT() {}
    VkPipelineDiscardRectangleStateCreateInfoEXT s;
    MarshalVkPipelineDiscardRectangleStateCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineDiscardRectangleStateCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineDiscardRectangleStateCreateInfoEXT* s);
    ~MarshalVkPipelineDiscardRectangleStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX {
public:
    MarshalVkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX() {}
    VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX s;
    MarshalVkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* s);
    ~MarshalVkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX();
};

class MarshalVkInputAttachmentAspectReference {
public:
    MarshalVkInputAttachmentAspectReference() {}
    VkInputAttachmentAspectReference s;
    MarshalVkInputAttachmentAspectReference(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkInputAttachmentAspectReference* s);
};

class MarshalVkRenderPassInputAttachmentAspectCreateInfo {
public:
    MarshalVkRenderPassInputAttachmentAspectCreateInfo() {}
    VkRenderPassInputAttachmentAspectCreateInfo s;
    MarshalVkRenderPassInputAttachmentAspectCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassInputAttachmentAspectCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassInputAttachmentAspectCreateInfo* s);
    ~MarshalVkRenderPassInputAttachmentAspectCreateInfo();
};

class MarshalVkPhysicalDeviceSurfaceInfo2KHR {
public:
    MarshalVkPhysicalDeviceSurfaceInfo2KHR() {}
    VkPhysicalDeviceSurfaceInfo2KHR s;
    MarshalVkPhysicalDeviceSurfaceInfo2KHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSurfaceInfo2KHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSurfaceInfo2KHR* s);
    ~MarshalVkPhysicalDeviceSurfaceInfo2KHR();
};

class MarshalVkSurfaceCapabilities2KHR {
public:
    MarshalVkSurfaceCapabilities2KHR() {}
    VkSurfaceCapabilities2KHR s;
    MarshalVkSurfaceCapabilities2KHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceCapabilities2KHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceCapabilities2KHR* s);
    ~MarshalVkSurfaceCapabilities2KHR();
};

class MarshalVkSurfaceFormat2KHR {
public:
    MarshalVkSurfaceFormat2KHR() {}
    VkSurfaceFormat2KHR s;
    MarshalVkSurfaceFormat2KHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceFormat2KHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceFormat2KHR* s);
    ~MarshalVkSurfaceFormat2KHR();
};

class MarshalVkDisplayProperties2KHR {
public:
    MarshalVkDisplayProperties2KHR() {}
    VkDisplayProperties2KHR s;
    MarshalVkDisplayProperties2KHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayProperties2KHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayProperties2KHR* s);
    ~MarshalVkDisplayProperties2KHR();
};

class MarshalVkDisplayPlaneProperties2KHR {
public:
    MarshalVkDisplayPlaneProperties2KHR() {}
    VkDisplayPlaneProperties2KHR s;
    MarshalVkDisplayPlaneProperties2KHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPlaneProperties2KHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPlaneProperties2KHR* s);
    ~MarshalVkDisplayPlaneProperties2KHR();
};

class MarshalVkDisplayModeProperties2KHR {
public:
    MarshalVkDisplayModeProperties2KHR() {}
    VkDisplayModeProperties2KHR s;
    MarshalVkDisplayModeProperties2KHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayModeProperties2KHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayModeProperties2KHR* s);
    ~MarshalVkDisplayModeProperties2KHR();
};

class MarshalVkDisplayModeStereoPropertiesNV {
public:
    MarshalVkDisplayModeStereoPropertiesNV() {}
    VkDisplayModeStereoPropertiesNV s;
    MarshalVkDisplayModeStereoPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayModeStereoPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayModeStereoPropertiesNV* s);
    ~MarshalVkDisplayModeStereoPropertiesNV();
};

class MarshalVkDisplayPlaneInfo2KHR {
public:
    MarshalVkDisplayPlaneInfo2KHR() {}
    VkDisplayPlaneInfo2KHR s;
    MarshalVkDisplayPlaneInfo2KHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPlaneInfo2KHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPlaneInfo2KHR* s);
    ~MarshalVkDisplayPlaneInfo2KHR();
};

class MarshalVkDisplayPlaneCapabilities2KHR {
public:
    MarshalVkDisplayPlaneCapabilities2KHR() {}
    VkDisplayPlaneCapabilities2KHR s;
    MarshalVkDisplayPlaneCapabilities2KHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPlaneCapabilities2KHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPlaneCapabilities2KHR* s);
    ~MarshalVkDisplayPlaneCapabilities2KHR();
};

class MarshalVkPhysicalDevice16BitStorageFeatures {
public:
    MarshalVkPhysicalDevice16BitStorageFeatures() {}
    VkPhysicalDevice16BitStorageFeatures s;
    MarshalVkPhysicalDevice16BitStorageFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevice16BitStorageFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevice16BitStorageFeatures* s);
    ~MarshalVkPhysicalDevice16BitStorageFeatures();
};

class MarshalVkPhysicalDeviceSubgroupProperties {
public:
    MarshalVkPhysicalDeviceSubgroupProperties() {}
    VkPhysicalDeviceSubgroupProperties s;
    MarshalVkPhysicalDeviceSubgroupProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubgroupProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubgroupProperties* s);
    ~MarshalVkPhysicalDeviceSubgroupProperties();
};

class MarshalVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures {
public:
    MarshalVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures() {}
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures s;
    MarshalVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures* s);
    ~MarshalVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures();
};

class MarshalVkBufferMemoryRequirementsInfo2 {
public:
    MarshalVkBufferMemoryRequirementsInfo2() {}
    VkBufferMemoryRequirementsInfo2 s;
    MarshalVkBufferMemoryRequirementsInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferMemoryRequirementsInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferMemoryRequirementsInfo2* s);
    ~MarshalVkBufferMemoryRequirementsInfo2();
};

class MarshalVkDeviceBufferMemoryRequirements {
public:
    MarshalVkDeviceBufferMemoryRequirements() {}
    VkDeviceBufferMemoryRequirements s;
    MarshalVkDeviceBufferMemoryRequirements(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceBufferMemoryRequirements* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceBufferMemoryRequirements* s);
    ~MarshalVkDeviceBufferMemoryRequirements();
};

class MarshalVkImageMemoryRequirementsInfo2 {
public:
    MarshalVkImageMemoryRequirementsInfo2() {}
    VkImageMemoryRequirementsInfo2 s;
    MarshalVkImageMemoryRequirementsInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageMemoryRequirementsInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageMemoryRequirementsInfo2* s);
    ~MarshalVkImageMemoryRequirementsInfo2();
};

class MarshalVkImageSparseMemoryRequirementsInfo2 {
public:
    MarshalVkImageSparseMemoryRequirementsInfo2() {}
    VkImageSparseMemoryRequirementsInfo2 s;
    MarshalVkImageSparseMemoryRequirementsInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageSparseMemoryRequirementsInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageSparseMemoryRequirementsInfo2* s);
    ~MarshalVkImageSparseMemoryRequirementsInfo2();
};

class MarshalVkDeviceImageMemoryRequirements {
public:
    MarshalVkDeviceImageMemoryRequirements() {}
    VkDeviceImageMemoryRequirements s;
    MarshalVkDeviceImageMemoryRequirements(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceImageMemoryRequirements* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceImageMemoryRequirements* s);
    ~MarshalVkDeviceImageMemoryRequirements();
};

class MarshalVkMemoryRequirements2 {
public:
    MarshalVkMemoryRequirements2() {}
    VkMemoryRequirements2 s;
    MarshalVkMemoryRequirements2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryRequirements2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryRequirements2* s);
    ~MarshalVkMemoryRequirements2();
};

class MarshalVkSparseImageMemoryRequirements2 {
public:
    MarshalVkSparseImageMemoryRequirements2() {}
    VkSparseImageMemoryRequirements2 s;
    MarshalVkSparseImageMemoryRequirements2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageMemoryRequirements2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSparseImageMemoryRequirements2* s);
    ~MarshalVkSparseImageMemoryRequirements2();
};

class MarshalVkPhysicalDevicePointClippingProperties {
public:
    MarshalVkPhysicalDevicePointClippingProperties() {}
    VkPhysicalDevicePointClippingProperties s;
    MarshalVkPhysicalDevicePointClippingProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePointClippingProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePointClippingProperties* s);
    ~MarshalVkPhysicalDevicePointClippingProperties();
};

class MarshalVkMemoryDedicatedRequirements {
public:
    MarshalVkMemoryDedicatedRequirements() {}
    VkMemoryDedicatedRequirements s;
    MarshalVkMemoryDedicatedRequirements(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryDedicatedRequirements* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryDedicatedRequirements* s);
    ~MarshalVkMemoryDedicatedRequirements();
};

class MarshalVkMemoryDedicatedAllocateInfo {
public:
    MarshalVkMemoryDedicatedAllocateInfo() {}
    VkMemoryDedicatedAllocateInfo s;
    MarshalVkMemoryDedicatedAllocateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryDedicatedAllocateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryDedicatedAllocateInfo* s);
    ~MarshalVkMemoryDedicatedAllocateInfo();
};

class MarshalVkImageViewUsageCreateInfo {
public:
    MarshalVkImageViewUsageCreateInfo() {}
    VkImageViewUsageCreateInfo s;
    MarshalVkImageViewUsageCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewUsageCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewUsageCreateInfo* s);
    ~MarshalVkImageViewUsageCreateInfo();
};

class MarshalVkImageViewSlicedCreateInfoEXT {
public:
    MarshalVkImageViewSlicedCreateInfoEXT() {}
    VkImageViewSlicedCreateInfoEXT s;
    MarshalVkImageViewSlicedCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewSlicedCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewSlicedCreateInfoEXT* s);
    ~MarshalVkImageViewSlicedCreateInfoEXT();
};

class MarshalVkPipelineTessellationDomainOriginStateCreateInfo {
public:
    MarshalVkPipelineTessellationDomainOriginStateCreateInfo() {}
    VkPipelineTessellationDomainOriginStateCreateInfo s;
    MarshalVkPipelineTessellationDomainOriginStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineTessellationDomainOriginStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineTessellationDomainOriginStateCreateInfo* s);
    ~MarshalVkPipelineTessellationDomainOriginStateCreateInfo();
};

class MarshalVkSamplerYcbcrConversionInfo {
public:
    MarshalVkSamplerYcbcrConversionInfo() {}
    VkSamplerYcbcrConversionInfo s;
    MarshalVkSamplerYcbcrConversionInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerYcbcrConversionInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerYcbcrConversionInfo* s);
    ~MarshalVkSamplerYcbcrConversionInfo();
};

class MarshalVkSamplerYcbcrConversionCreateInfo {
public:
    MarshalVkSamplerYcbcrConversionCreateInfo() {}
    VkSamplerYcbcrConversionCreateInfo s;
    MarshalVkSamplerYcbcrConversionCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerYcbcrConversionCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerYcbcrConversionCreateInfo* s);
    ~MarshalVkSamplerYcbcrConversionCreateInfo();
};

class MarshalVkBindImagePlaneMemoryInfo {
public:
    MarshalVkBindImagePlaneMemoryInfo() {}
    VkBindImagePlaneMemoryInfo s;
    MarshalVkBindImagePlaneMemoryInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindImagePlaneMemoryInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindImagePlaneMemoryInfo* s);
    ~MarshalVkBindImagePlaneMemoryInfo();
};

class MarshalVkImagePlaneMemoryRequirementsInfo {
public:
    MarshalVkImagePlaneMemoryRequirementsInfo() {}
    VkImagePlaneMemoryRequirementsInfo s;
    MarshalVkImagePlaneMemoryRequirementsInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImagePlaneMemoryRequirementsInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImagePlaneMemoryRequirementsInfo* s);
    ~MarshalVkImagePlaneMemoryRequirementsInfo();
};

class MarshalVkPhysicalDeviceSamplerYcbcrConversionFeatures {
public:
    MarshalVkPhysicalDeviceSamplerYcbcrConversionFeatures() {}
    VkPhysicalDeviceSamplerYcbcrConversionFeatures s;
    MarshalVkPhysicalDeviceSamplerYcbcrConversionFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSamplerYcbcrConversionFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSamplerYcbcrConversionFeatures* s);
    ~MarshalVkPhysicalDeviceSamplerYcbcrConversionFeatures();
};

class MarshalVkSamplerYcbcrConversionImageFormatProperties {
public:
    MarshalVkSamplerYcbcrConversionImageFormatProperties() {}
    VkSamplerYcbcrConversionImageFormatProperties s;
    MarshalVkSamplerYcbcrConversionImageFormatProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerYcbcrConversionImageFormatProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerYcbcrConversionImageFormatProperties* s);
    ~MarshalVkSamplerYcbcrConversionImageFormatProperties();
};

class MarshalVkTextureLODGatherFormatPropertiesAMD {
public:
    MarshalVkTextureLODGatherFormatPropertiesAMD() {}
    VkTextureLODGatherFormatPropertiesAMD s;
    MarshalVkTextureLODGatherFormatPropertiesAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkTextureLODGatherFormatPropertiesAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkTextureLODGatherFormatPropertiesAMD* s);
    ~MarshalVkTextureLODGatherFormatPropertiesAMD();
};

class MarshalVkConditionalRenderingBeginInfoEXT {
public:
    MarshalVkConditionalRenderingBeginInfoEXT() {}
    VkConditionalRenderingBeginInfoEXT s;
    MarshalVkConditionalRenderingBeginInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkConditionalRenderingBeginInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkConditionalRenderingBeginInfoEXT* s);
    ~MarshalVkConditionalRenderingBeginInfoEXT();
};

class MarshalVkProtectedSubmitInfo {
public:
    MarshalVkProtectedSubmitInfo() {}
    VkProtectedSubmitInfo s;
    MarshalVkProtectedSubmitInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkProtectedSubmitInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkProtectedSubmitInfo* s);
    ~MarshalVkProtectedSubmitInfo();
};

class MarshalVkPhysicalDeviceProtectedMemoryFeatures {
public:
    MarshalVkPhysicalDeviceProtectedMemoryFeatures() {}
    VkPhysicalDeviceProtectedMemoryFeatures s;
    MarshalVkPhysicalDeviceProtectedMemoryFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProtectedMemoryFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProtectedMemoryFeatures* s);
    ~MarshalVkPhysicalDeviceProtectedMemoryFeatures();
};

class MarshalVkPhysicalDeviceProtectedMemoryProperties {
public:
    MarshalVkPhysicalDeviceProtectedMemoryProperties() {}
    VkPhysicalDeviceProtectedMemoryProperties s;
    MarshalVkPhysicalDeviceProtectedMemoryProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProtectedMemoryProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProtectedMemoryProperties* s);
    ~MarshalVkPhysicalDeviceProtectedMemoryProperties();
};

class MarshalVkDeviceQueueInfo2 {
public:
    MarshalVkDeviceQueueInfo2() {}
    VkDeviceQueueInfo2 s;
    MarshalVkDeviceQueueInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceQueueInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceQueueInfo2* s);
    ~MarshalVkDeviceQueueInfo2();
};

class MarshalVkPipelineCoverageToColorStateCreateInfoNV {
public:
    MarshalVkPipelineCoverageToColorStateCreateInfoNV() {}
    VkPipelineCoverageToColorStateCreateInfoNV s;
    MarshalVkPipelineCoverageToColorStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCoverageToColorStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCoverageToColorStateCreateInfoNV* s);
    ~MarshalVkPipelineCoverageToColorStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceSamplerFilterMinmaxProperties {
public:
    MarshalVkPhysicalDeviceSamplerFilterMinmaxProperties() {}
    VkPhysicalDeviceSamplerFilterMinmaxProperties s;
    MarshalVkPhysicalDeviceSamplerFilterMinmaxProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSamplerFilterMinmaxProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSamplerFilterMinmaxProperties* s);
    ~MarshalVkPhysicalDeviceSamplerFilterMinmaxProperties();
};

class MarshalVkSampleLocationEXT {
public:
    MarshalVkSampleLocationEXT() {}
    VkSampleLocationEXT s;
    MarshalVkSampleLocationEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSampleLocationEXT* s);
};

class MarshalVkSampleLocationsInfoEXT {
public:
    MarshalVkSampleLocationsInfoEXT() {}
    VkSampleLocationsInfoEXT s;
    MarshalVkSampleLocationsInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSampleLocationsInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSampleLocationsInfoEXT* s);
    ~MarshalVkSampleLocationsInfoEXT();
};

class MarshalVkAttachmentSampleLocationsEXT {
public:
    MarshalVkAttachmentSampleLocationsEXT() {}
    VkAttachmentSampleLocationsEXT s;
    MarshalVkAttachmentSampleLocationsEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentSampleLocationsEXT* s);
};

class MarshalVkSubpassSampleLocationsEXT {
public:
    MarshalVkSubpassSampleLocationsEXT() {}
    VkSubpassSampleLocationsEXT s;
    MarshalVkSubpassSampleLocationsEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassSampleLocationsEXT* s);
};

class MarshalVkRenderPassSampleLocationsBeginInfoEXT {
public:
    MarshalVkRenderPassSampleLocationsBeginInfoEXT() {}
    VkRenderPassSampleLocationsBeginInfoEXT s;
    MarshalVkRenderPassSampleLocationsBeginInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassSampleLocationsBeginInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassSampleLocationsBeginInfoEXT* s);
    ~MarshalVkRenderPassSampleLocationsBeginInfoEXT();
};

class MarshalVkPipelineSampleLocationsStateCreateInfoEXT {
public:
    MarshalVkPipelineSampleLocationsStateCreateInfoEXT() {}
    VkPipelineSampleLocationsStateCreateInfoEXT s;
    MarshalVkPipelineSampleLocationsStateCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineSampleLocationsStateCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineSampleLocationsStateCreateInfoEXT* s);
    ~MarshalVkPipelineSampleLocationsStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceSampleLocationsPropertiesEXT {
public:
    MarshalVkPhysicalDeviceSampleLocationsPropertiesEXT() {}
    VkPhysicalDeviceSampleLocationsPropertiesEXT s;
    MarshalVkPhysicalDeviceSampleLocationsPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSampleLocationsPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSampleLocationsPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceSampleLocationsPropertiesEXT();
};

class MarshalVkMultisamplePropertiesEXT {
public:
    MarshalVkMultisamplePropertiesEXT() {}
    VkMultisamplePropertiesEXT s;
    MarshalVkMultisamplePropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultisamplePropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultisamplePropertiesEXT* s);
    ~MarshalVkMultisamplePropertiesEXT();
};

class MarshalVkSamplerReductionModeCreateInfo {
public:
    MarshalVkSamplerReductionModeCreateInfo() {}
    VkSamplerReductionModeCreateInfo s;
    MarshalVkSamplerReductionModeCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerReductionModeCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerReductionModeCreateInfo* s);
    ~MarshalVkSamplerReductionModeCreateInfo();
};

class MarshalVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT {
public:
    MarshalVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT() {}
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT s;
    MarshalVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT();
};

class MarshalVkPhysicalDeviceMultiDrawFeaturesEXT {
public:
    MarshalVkPhysicalDeviceMultiDrawFeaturesEXT() {}
    VkPhysicalDeviceMultiDrawFeaturesEXT s;
    MarshalVkPhysicalDeviceMultiDrawFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiDrawFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiDrawFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceMultiDrawFeaturesEXT();
};

class MarshalVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT {
public:
    MarshalVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT() {}
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT s;
    MarshalVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT();
};

class MarshalVkPipelineColorBlendAdvancedStateCreateInfoEXT {
public:
    MarshalVkPipelineColorBlendAdvancedStateCreateInfoEXT() {}
    VkPipelineColorBlendAdvancedStateCreateInfoEXT s;
    MarshalVkPipelineColorBlendAdvancedStateCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineColorBlendAdvancedStateCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineColorBlendAdvancedStateCreateInfoEXT* s);
    ~MarshalVkPipelineColorBlendAdvancedStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceInlineUniformBlockFeatures {
public:
    MarshalVkPhysicalDeviceInlineUniformBlockFeatures() {}
    VkPhysicalDeviceInlineUniformBlockFeatures s;
    MarshalVkPhysicalDeviceInlineUniformBlockFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceInlineUniformBlockFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceInlineUniformBlockFeatures* s);
    ~MarshalVkPhysicalDeviceInlineUniformBlockFeatures();
};

class MarshalVkPhysicalDeviceInlineUniformBlockProperties {
public:
    MarshalVkPhysicalDeviceInlineUniformBlockProperties() {}
    VkPhysicalDeviceInlineUniformBlockProperties s;
    MarshalVkPhysicalDeviceInlineUniformBlockProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceInlineUniformBlockProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceInlineUniformBlockProperties* s);
    ~MarshalVkPhysicalDeviceInlineUniformBlockProperties();
};

class MarshalVkWriteDescriptorSetInlineUniformBlock {
public:
    MarshalVkWriteDescriptorSetInlineUniformBlock() {}
    VkWriteDescriptorSetInlineUniformBlock s;
    MarshalVkWriteDescriptorSetInlineUniformBlock(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteDescriptorSetInlineUniformBlock* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteDescriptorSetInlineUniformBlock* s);
    ~MarshalVkWriteDescriptorSetInlineUniformBlock();
};

class MarshalVkDescriptorPoolInlineUniformBlockCreateInfo {
public:
    MarshalVkDescriptorPoolInlineUniformBlockCreateInfo() {}
    VkDescriptorPoolInlineUniformBlockCreateInfo s;
    MarshalVkDescriptorPoolInlineUniformBlockCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorPoolInlineUniformBlockCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorPoolInlineUniformBlockCreateInfo* s);
    ~MarshalVkDescriptorPoolInlineUniformBlockCreateInfo();
};

class MarshalVkPipelineCoverageModulationStateCreateInfoNV {
public:
    MarshalVkPipelineCoverageModulationStateCreateInfoNV() {}
    VkPipelineCoverageModulationStateCreateInfoNV s;
    MarshalVkPipelineCoverageModulationStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCoverageModulationStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCoverageModulationStateCreateInfoNV* s);
    ~MarshalVkPipelineCoverageModulationStateCreateInfoNV();
};

class MarshalVkImageFormatListCreateInfo {
public:
    MarshalVkImageFormatListCreateInfo() {}
    VkImageFormatListCreateInfo s;
    MarshalVkImageFormatListCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageFormatListCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageFormatListCreateInfo* s);
    ~MarshalVkImageFormatListCreateInfo();
};

class MarshalVkValidationCacheCreateInfoEXT {
public:
    MarshalVkValidationCacheCreateInfoEXT() {}
    VkValidationCacheCreateInfoEXT s;
    MarshalVkValidationCacheCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkValidationCacheCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkValidationCacheCreateInfoEXT* s);
    ~MarshalVkValidationCacheCreateInfoEXT();
};

class MarshalVkShaderModuleValidationCacheCreateInfoEXT {
public:
    MarshalVkShaderModuleValidationCacheCreateInfoEXT() {}
    VkShaderModuleValidationCacheCreateInfoEXT s;
    MarshalVkShaderModuleValidationCacheCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkShaderModuleValidationCacheCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkShaderModuleValidationCacheCreateInfoEXT* s);
    ~MarshalVkShaderModuleValidationCacheCreateInfoEXT();
};

class MarshalVkPhysicalDeviceMaintenance3Properties {
public:
    MarshalVkPhysicalDeviceMaintenance3Properties() {}
    VkPhysicalDeviceMaintenance3Properties s;
    MarshalVkPhysicalDeviceMaintenance3Properties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance3Properties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance3Properties* s);
    ~MarshalVkPhysicalDeviceMaintenance3Properties();
};

class MarshalVkPhysicalDeviceMaintenance4Features {
public:
    MarshalVkPhysicalDeviceMaintenance4Features() {}
    VkPhysicalDeviceMaintenance4Features s;
    MarshalVkPhysicalDeviceMaintenance4Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance4Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance4Features* s);
    ~MarshalVkPhysicalDeviceMaintenance4Features();
};

class MarshalVkPhysicalDeviceMaintenance4Properties {
public:
    MarshalVkPhysicalDeviceMaintenance4Properties() {}
    VkPhysicalDeviceMaintenance4Properties s;
    MarshalVkPhysicalDeviceMaintenance4Properties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance4Properties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance4Properties* s);
    ~MarshalVkPhysicalDeviceMaintenance4Properties();
};

class MarshalVkPhysicalDeviceMaintenance5Features {
public:
    MarshalVkPhysicalDeviceMaintenance5Features() {}
    VkPhysicalDeviceMaintenance5Features s;
    MarshalVkPhysicalDeviceMaintenance5Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance5Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance5Features* s);
    ~MarshalVkPhysicalDeviceMaintenance5Features();
};

class MarshalVkPhysicalDeviceMaintenance5Properties {
public:
    MarshalVkPhysicalDeviceMaintenance5Properties() {}
    VkPhysicalDeviceMaintenance5Properties s;
    MarshalVkPhysicalDeviceMaintenance5Properties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance5Properties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance5Properties* s);
    ~MarshalVkPhysicalDeviceMaintenance5Properties();
};

class MarshalVkPhysicalDeviceMaintenance6Features {
public:
    MarshalVkPhysicalDeviceMaintenance6Features() {}
    VkPhysicalDeviceMaintenance6Features s;
    MarshalVkPhysicalDeviceMaintenance6Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance6Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance6Features* s);
    ~MarshalVkPhysicalDeviceMaintenance6Features();
};

class MarshalVkPhysicalDeviceMaintenance6Properties {
public:
    MarshalVkPhysicalDeviceMaintenance6Properties() {}
    VkPhysicalDeviceMaintenance6Properties s;
    MarshalVkPhysicalDeviceMaintenance6Properties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance6Properties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMaintenance6Properties* s);
    ~MarshalVkPhysicalDeviceMaintenance6Properties();
};

class MarshalVkRenderingAreaInfo {
public:
    MarshalVkRenderingAreaInfo() {}
    VkRenderingAreaInfo s;
    MarshalVkRenderingAreaInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingAreaInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingAreaInfo* s);
    ~MarshalVkRenderingAreaInfo();
};

class MarshalVkDescriptorSetLayoutSupport {
public:
    MarshalVkDescriptorSetLayoutSupport() {}
    VkDescriptorSetLayoutSupport s;
    MarshalVkDescriptorSetLayoutSupport(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetLayoutSupport* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetLayoutSupport* s);
    ~MarshalVkDescriptorSetLayoutSupport();
};

class MarshalVkPhysicalDeviceShaderDrawParametersFeatures {
public:
    MarshalVkPhysicalDeviceShaderDrawParametersFeatures() {}
    VkPhysicalDeviceShaderDrawParametersFeatures s;
    MarshalVkPhysicalDeviceShaderDrawParametersFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderDrawParametersFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderDrawParametersFeatures* s);
    ~MarshalVkPhysicalDeviceShaderDrawParametersFeatures();
};

class MarshalVkPhysicalDeviceShaderFloat16Int8Features {
public:
    MarshalVkPhysicalDeviceShaderFloat16Int8Features() {}
    VkPhysicalDeviceShaderFloat16Int8Features s;
    MarshalVkPhysicalDeviceShaderFloat16Int8Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderFloat16Int8Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderFloat16Int8Features* s);
    ~MarshalVkPhysicalDeviceShaderFloat16Int8Features();
};

class MarshalVkPhysicalDeviceFloatControlsProperties {
public:
    MarshalVkPhysicalDeviceFloatControlsProperties() {}
    VkPhysicalDeviceFloatControlsProperties s;
    MarshalVkPhysicalDeviceFloatControlsProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFloatControlsProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFloatControlsProperties* s);
    ~MarshalVkPhysicalDeviceFloatControlsProperties();
};

class MarshalVkPhysicalDeviceHostQueryResetFeatures {
public:
    MarshalVkPhysicalDeviceHostQueryResetFeatures() {}
    VkPhysicalDeviceHostQueryResetFeatures s;
    MarshalVkPhysicalDeviceHostQueryResetFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceHostQueryResetFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceHostQueryResetFeatures* s);
    ~MarshalVkPhysicalDeviceHostQueryResetFeatures();
};

class MarshalVkDeviceQueueGlobalPriorityCreateInfo {
public:
    MarshalVkDeviceQueueGlobalPriorityCreateInfo() {}
    VkDeviceQueueGlobalPriorityCreateInfo s;
    MarshalVkDeviceQueueGlobalPriorityCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceQueueGlobalPriorityCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceQueueGlobalPriorityCreateInfo* s);
    ~MarshalVkDeviceQueueGlobalPriorityCreateInfo();
};

class MarshalVkPhysicalDeviceGlobalPriorityQueryFeatures {
public:
    MarshalVkPhysicalDeviceGlobalPriorityQueryFeatures() {}
    VkPhysicalDeviceGlobalPriorityQueryFeatures s;
    MarshalVkPhysicalDeviceGlobalPriorityQueryFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceGlobalPriorityQueryFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceGlobalPriorityQueryFeatures* s);
    ~MarshalVkPhysicalDeviceGlobalPriorityQueryFeatures();
};

class MarshalVkQueueFamilyGlobalPriorityProperties {
public:
    MarshalVkQueueFamilyGlobalPriorityProperties() {}
    VkQueueFamilyGlobalPriorityProperties s;
    MarshalVkQueueFamilyGlobalPriorityProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyGlobalPriorityProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyGlobalPriorityProperties* s);
    ~MarshalVkQueueFamilyGlobalPriorityProperties();
};

class MarshalVkDebugUtilsObjectNameInfoEXT {
public:
    MarshalVkDebugUtilsObjectNameInfoEXT() {}
    VkDebugUtilsObjectNameInfoEXT s;
    MarshalVkDebugUtilsObjectNameInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsObjectNameInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsObjectNameInfoEXT* s);
    ~MarshalVkDebugUtilsObjectNameInfoEXT();
};

class MarshalVkDebugUtilsObjectTagInfoEXT {
public:
    MarshalVkDebugUtilsObjectTagInfoEXT() {}
    VkDebugUtilsObjectTagInfoEXT s;
    MarshalVkDebugUtilsObjectTagInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsObjectTagInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsObjectTagInfoEXT* s);
    ~MarshalVkDebugUtilsObjectTagInfoEXT();
};

class MarshalVkDebugUtilsLabelEXT {
public:
    MarshalVkDebugUtilsLabelEXT() {}
    VkDebugUtilsLabelEXT s;
    MarshalVkDebugUtilsLabelEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsLabelEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsLabelEXT* s);
    ~MarshalVkDebugUtilsLabelEXT();
};

class MarshalVkDebugUtilsMessengerCreateInfoEXT {
public:
    MarshalVkDebugUtilsMessengerCreateInfoEXT() {}
    VkDebugUtilsMessengerCreateInfoEXT s;
    MarshalVkDebugUtilsMessengerCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsMessengerCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsMessengerCreateInfoEXT* s);
    ~MarshalVkDebugUtilsMessengerCreateInfoEXT();
};

class MarshalVkDebugUtilsMessengerCallbackDataEXT {
public:
    MarshalVkDebugUtilsMessengerCallbackDataEXT() {}
    VkDebugUtilsMessengerCallbackDataEXT s;
    MarshalVkDebugUtilsMessengerCallbackDataEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsMessengerCallbackDataEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDebugUtilsMessengerCallbackDataEXT* s);
    ~MarshalVkDebugUtilsMessengerCallbackDataEXT();
};

class MarshalVkImportMemoryHostPointerInfoEXT {
public:
    MarshalVkImportMemoryHostPointerInfoEXT() {}
    VkImportMemoryHostPointerInfoEXT s;
    MarshalVkImportMemoryHostPointerInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImportMemoryHostPointerInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImportMemoryHostPointerInfoEXT* s);
};

class MarshalVkMemoryHostPointerPropertiesEXT {
public:
    MarshalVkMemoryHostPointerPropertiesEXT() {}
    VkMemoryHostPointerPropertiesEXT s;
    MarshalVkMemoryHostPointerPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryHostPointerPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryHostPointerPropertiesEXT* s);
    ~MarshalVkMemoryHostPointerPropertiesEXT();
};

class MarshalVkPhysicalDeviceExternalMemoryHostPropertiesEXT {
public:
    MarshalVkPhysicalDeviceExternalMemoryHostPropertiesEXT() {}
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT s;
    MarshalVkPhysicalDeviceExternalMemoryHostPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalMemoryHostPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExternalMemoryHostPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceExternalMemoryHostPropertiesEXT();
};

class MarshalVkPhysicalDeviceConservativeRasterizationPropertiesEXT {
public:
    MarshalVkPhysicalDeviceConservativeRasterizationPropertiesEXT() {}
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT s;
    MarshalVkPhysicalDeviceConservativeRasterizationPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceConservativeRasterizationPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceConservativeRasterizationPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceConservativeRasterizationPropertiesEXT();
};

class MarshalVkCalibratedTimestampInfoKHR {
public:
    MarshalVkCalibratedTimestampInfoKHR() {}
    VkCalibratedTimestampInfoKHR s;
    MarshalVkCalibratedTimestampInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCalibratedTimestampInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCalibratedTimestampInfoKHR* s);
    ~MarshalVkCalibratedTimestampInfoKHR();
};

class MarshalVkPhysicalDeviceShaderCorePropertiesAMD {
public:
    MarshalVkPhysicalDeviceShaderCorePropertiesAMD() {}
    VkPhysicalDeviceShaderCorePropertiesAMD s;
    MarshalVkPhysicalDeviceShaderCorePropertiesAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCorePropertiesAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCorePropertiesAMD* s);
    ~MarshalVkPhysicalDeviceShaderCorePropertiesAMD();
};

class MarshalVkPhysicalDeviceShaderCoreProperties2AMD {
public:
    MarshalVkPhysicalDeviceShaderCoreProperties2AMD() {}
    VkPhysicalDeviceShaderCoreProperties2AMD s;
    MarshalVkPhysicalDeviceShaderCoreProperties2AMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCoreProperties2AMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCoreProperties2AMD* s);
    ~MarshalVkPhysicalDeviceShaderCoreProperties2AMD();
};

class MarshalVkPipelineRasterizationConservativeStateCreateInfoEXT {
public:
    MarshalVkPipelineRasterizationConservativeStateCreateInfoEXT() {}
    VkPipelineRasterizationConservativeStateCreateInfoEXT s;
    MarshalVkPipelineRasterizationConservativeStateCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationConservativeStateCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationConservativeStateCreateInfoEXT* s);
    ~MarshalVkPipelineRasterizationConservativeStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceDescriptorIndexingFeatures {
public:
    MarshalVkPhysicalDeviceDescriptorIndexingFeatures() {}
    VkPhysicalDeviceDescriptorIndexingFeatures s;
    MarshalVkPhysicalDeviceDescriptorIndexingFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorIndexingFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorIndexingFeatures* s);
    ~MarshalVkPhysicalDeviceDescriptorIndexingFeatures();
};

class MarshalVkPhysicalDeviceDescriptorIndexingProperties {
public:
    MarshalVkPhysicalDeviceDescriptorIndexingProperties() {}
    VkPhysicalDeviceDescriptorIndexingProperties s;
    MarshalVkPhysicalDeviceDescriptorIndexingProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorIndexingProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorIndexingProperties* s);
    ~MarshalVkPhysicalDeviceDescriptorIndexingProperties();
};

class MarshalVkDescriptorSetLayoutBindingFlagsCreateInfo {
public:
    MarshalVkDescriptorSetLayoutBindingFlagsCreateInfo() {}
    VkDescriptorSetLayoutBindingFlagsCreateInfo s;
    MarshalVkDescriptorSetLayoutBindingFlagsCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetLayoutBindingFlagsCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetLayoutBindingFlagsCreateInfo* s);
    ~MarshalVkDescriptorSetLayoutBindingFlagsCreateInfo();
};

class MarshalVkDescriptorSetVariableDescriptorCountAllocateInfo {
public:
    MarshalVkDescriptorSetVariableDescriptorCountAllocateInfo() {}
    VkDescriptorSetVariableDescriptorCountAllocateInfo s;
    MarshalVkDescriptorSetVariableDescriptorCountAllocateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetVariableDescriptorCountAllocateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetVariableDescriptorCountAllocateInfo* s);
    ~MarshalVkDescriptorSetVariableDescriptorCountAllocateInfo();
};

class MarshalVkDescriptorSetVariableDescriptorCountLayoutSupport {
public:
    MarshalVkDescriptorSetVariableDescriptorCountLayoutSupport() {}
    VkDescriptorSetVariableDescriptorCountLayoutSupport s;
    MarshalVkDescriptorSetVariableDescriptorCountLayoutSupport(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetVariableDescriptorCountLayoutSupport* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetVariableDescriptorCountLayoutSupport* s);
    ~MarshalVkDescriptorSetVariableDescriptorCountLayoutSupport();
};

class MarshalVkAttachmentDescription2 {
public:
    MarshalVkAttachmentDescription2() {}
    VkAttachmentDescription2 s;
    MarshalVkAttachmentDescription2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentDescription2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentDescription2* s);
    ~MarshalVkAttachmentDescription2();
};

class MarshalVkAttachmentReference2 {
public:
    MarshalVkAttachmentReference2() {}
    VkAttachmentReference2 s;
    MarshalVkAttachmentReference2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentReference2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentReference2* s);
    ~MarshalVkAttachmentReference2();
};

class MarshalVkSubpassDescription2 {
public:
    MarshalVkSubpassDescription2() {}
    VkSubpassDescription2 s;
    MarshalVkSubpassDescription2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassDescription2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassDescription2* s);
    ~MarshalVkSubpassDescription2();
};

class MarshalVkSubpassDependency2 {
public:
    MarshalVkSubpassDependency2() {}
    VkSubpassDependency2 s;
    MarshalVkSubpassDependency2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassDependency2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassDependency2* s);
    ~MarshalVkSubpassDependency2();
};

class MarshalVkRenderPassCreateInfo2 {
public:
    MarshalVkRenderPassCreateInfo2() {}
    VkRenderPassCreateInfo2 s;
    MarshalVkRenderPassCreateInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassCreateInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassCreateInfo2* s);
    ~MarshalVkRenderPassCreateInfo2();
};

class MarshalVkSubpassBeginInfo {
public:
    MarshalVkSubpassBeginInfo() {}
    VkSubpassBeginInfo s;
    MarshalVkSubpassBeginInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassBeginInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassBeginInfo* s);
    ~MarshalVkSubpassBeginInfo();
};

class MarshalVkSubpassEndInfo {
public:
    MarshalVkSubpassEndInfo() {}
    VkSubpassEndInfo s;
    MarshalVkSubpassEndInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassEndInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassEndInfo* s);
    ~MarshalVkSubpassEndInfo();
};

class MarshalVkPhysicalDeviceTimelineSemaphoreFeatures {
public:
    MarshalVkPhysicalDeviceTimelineSemaphoreFeatures() {}
    VkPhysicalDeviceTimelineSemaphoreFeatures s;
    MarshalVkPhysicalDeviceTimelineSemaphoreFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTimelineSemaphoreFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTimelineSemaphoreFeatures* s);
    ~MarshalVkPhysicalDeviceTimelineSemaphoreFeatures();
};

class MarshalVkPhysicalDeviceTimelineSemaphoreProperties {
public:
    MarshalVkPhysicalDeviceTimelineSemaphoreProperties() {}
    VkPhysicalDeviceTimelineSemaphoreProperties s;
    MarshalVkPhysicalDeviceTimelineSemaphoreProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTimelineSemaphoreProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTimelineSemaphoreProperties* s);
    ~MarshalVkPhysicalDeviceTimelineSemaphoreProperties();
};

class MarshalVkSemaphoreTypeCreateInfo {
public:
    MarshalVkSemaphoreTypeCreateInfo() {}
    VkSemaphoreTypeCreateInfo s;
    MarshalVkSemaphoreTypeCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreTypeCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreTypeCreateInfo* s);
    ~MarshalVkSemaphoreTypeCreateInfo();
};

class MarshalVkTimelineSemaphoreSubmitInfo {
public:
    MarshalVkTimelineSemaphoreSubmitInfo() {}
    VkTimelineSemaphoreSubmitInfo s;
    MarshalVkTimelineSemaphoreSubmitInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkTimelineSemaphoreSubmitInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkTimelineSemaphoreSubmitInfo* s);
    ~MarshalVkTimelineSemaphoreSubmitInfo();
};

class MarshalVkSemaphoreWaitInfo {
public:
    MarshalVkSemaphoreWaitInfo() {}
    VkSemaphoreWaitInfo s;
    MarshalVkSemaphoreWaitInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreWaitInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreWaitInfo* s);
    ~MarshalVkSemaphoreWaitInfo();
};

class MarshalVkSemaphoreSignalInfo {
public:
    MarshalVkSemaphoreSignalInfo() {}
    VkSemaphoreSignalInfo s;
    MarshalVkSemaphoreSignalInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreSignalInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreSignalInfo* s);
    ~MarshalVkSemaphoreSignalInfo();
};

class MarshalVkVertexInputBindingDivisorDescription {
public:
    MarshalVkVertexInputBindingDivisorDescription() {}
    VkVertexInputBindingDivisorDescription s;
    MarshalVkVertexInputBindingDivisorDescription(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVertexInputBindingDivisorDescription* s);
};

class MarshalVkPipelineVertexInputDivisorStateCreateInfo {
public:
    MarshalVkPipelineVertexInputDivisorStateCreateInfo() {}
    VkPipelineVertexInputDivisorStateCreateInfo s;
    MarshalVkPipelineVertexInputDivisorStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineVertexInputDivisorStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineVertexInputDivisorStateCreateInfo* s);
    ~MarshalVkPipelineVertexInputDivisorStateCreateInfo();
};

class MarshalVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT {
public:
    MarshalVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT() {}
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT s;
    MarshalVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT();
};

class MarshalVkPhysicalDeviceVertexAttributeDivisorProperties {
public:
    MarshalVkPhysicalDeviceVertexAttributeDivisorProperties() {}
    VkPhysicalDeviceVertexAttributeDivisorProperties s;
    MarshalVkPhysicalDeviceVertexAttributeDivisorProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorProperties* s);
    ~MarshalVkPhysicalDeviceVertexAttributeDivisorProperties();
};

class MarshalVkPhysicalDevicePCIBusInfoPropertiesEXT {
public:
    MarshalVkPhysicalDevicePCIBusInfoPropertiesEXT() {}
    VkPhysicalDevicePCIBusInfoPropertiesEXT s;
    MarshalVkPhysicalDevicePCIBusInfoPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePCIBusInfoPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePCIBusInfoPropertiesEXT* s);
    ~MarshalVkPhysicalDevicePCIBusInfoPropertiesEXT();
};

class MarshalVkCommandBufferInheritanceConditionalRenderingInfoEXT {
public:
    MarshalVkCommandBufferInheritanceConditionalRenderingInfoEXT() {}
    VkCommandBufferInheritanceConditionalRenderingInfoEXT s;
    MarshalVkCommandBufferInheritanceConditionalRenderingInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceConditionalRenderingInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceConditionalRenderingInfoEXT* s);
    ~MarshalVkCommandBufferInheritanceConditionalRenderingInfoEXT();
};

class MarshalVkPhysicalDevice8BitStorageFeatures {
public:
    MarshalVkPhysicalDevice8BitStorageFeatures() {}
    VkPhysicalDevice8BitStorageFeatures s;
    MarshalVkPhysicalDevice8BitStorageFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevice8BitStorageFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevice8BitStorageFeatures* s);
    ~MarshalVkPhysicalDevice8BitStorageFeatures();
};

class MarshalVkPhysicalDeviceConditionalRenderingFeaturesEXT {
public:
    MarshalVkPhysicalDeviceConditionalRenderingFeaturesEXT() {}
    VkPhysicalDeviceConditionalRenderingFeaturesEXT s;
    MarshalVkPhysicalDeviceConditionalRenderingFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceConditionalRenderingFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceConditionalRenderingFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceConditionalRenderingFeaturesEXT();
};

class MarshalVkPhysicalDeviceVulkanMemoryModelFeatures {
public:
    MarshalVkPhysicalDeviceVulkanMemoryModelFeatures() {}
    VkPhysicalDeviceVulkanMemoryModelFeatures s;
    MarshalVkPhysicalDeviceVulkanMemoryModelFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkanMemoryModelFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkanMemoryModelFeatures* s);
    ~MarshalVkPhysicalDeviceVulkanMemoryModelFeatures();
};

class MarshalVkPhysicalDeviceShaderAtomicInt64Features {
public:
    MarshalVkPhysicalDeviceShaderAtomicInt64Features() {}
    VkPhysicalDeviceShaderAtomicInt64Features s;
    MarshalVkPhysicalDeviceShaderAtomicInt64Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicInt64Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicInt64Features* s);
    ~MarshalVkPhysicalDeviceShaderAtomicInt64Features();
};

class MarshalVkPhysicalDeviceShaderAtomicFloatFeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderAtomicFloatFeaturesEXT() {}
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT s;
    MarshalVkPhysicalDeviceShaderAtomicFloatFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderAtomicFloatFeaturesEXT();
};

class MarshalVkPhysicalDeviceShaderAtomicFloat2FeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderAtomicFloat2FeaturesEXT() {}
    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT s;
    MarshalVkPhysicalDeviceShaderAtomicFloat2FeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderAtomicFloat2FeaturesEXT();
};

class MarshalVkPhysicalDeviceVertexAttributeDivisorFeatures {
public:
    MarshalVkPhysicalDeviceVertexAttributeDivisorFeatures() {}
    VkPhysicalDeviceVertexAttributeDivisorFeatures s;
    MarshalVkPhysicalDeviceVertexAttributeDivisorFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorFeatures* s);
    ~MarshalVkPhysicalDeviceVertexAttributeDivisorFeatures();
};

class MarshalVkQueueFamilyCheckpointPropertiesNV {
public:
    MarshalVkQueueFamilyCheckpointPropertiesNV() {}
    VkQueueFamilyCheckpointPropertiesNV s;
    MarshalVkQueueFamilyCheckpointPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyCheckpointPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyCheckpointPropertiesNV* s);
    ~MarshalVkQueueFamilyCheckpointPropertiesNV();
};

class MarshalVkCheckpointDataNV {
public:
    MarshalVkCheckpointDataNV() {}
    VkCheckpointDataNV s;
    MarshalVkCheckpointDataNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCheckpointDataNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCheckpointDataNV* s);
    ~MarshalVkCheckpointDataNV();
};

class MarshalVkPhysicalDeviceDepthStencilResolveProperties {
public:
    MarshalVkPhysicalDeviceDepthStencilResolveProperties() {}
    VkPhysicalDeviceDepthStencilResolveProperties s;
    MarshalVkPhysicalDeviceDepthStencilResolveProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthStencilResolveProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthStencilResolveProperties* s);
    ~MarshalVkPhysicalDeviceDepthStencilResolveProperties();
};

class MarshalVkSubpassDescriptionDepthStencilResolve {
public:
    MarshalVkSubpassDescriptionDepthStencilResolve() {}
    VkSubpassDescriptionDepthStencilResolve s;
    MarshalVkSubpassDescriptionDepthStencilResolve(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassDescriptionDepthStencilResolve* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassDescriptionDepthStencilResolve* s);
    ~MarshalVkSubpassDescriptionDepthStencilResolve();
};

class MarshalVkImageViewASTCDecodeModeEXT {
public:
    MarshalVkImageViewASTCDecodeModeEXT() {}
    VkImageViewASTCDecodeModeEXT s;
    MarshalVkImageViewASTCDecodeModeEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewASTCDecodeModeEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewASTCDecodeModeEXT* s);
    ~MarshalVkImageViewASTCDecodeModeEXT();
};

class MarshalVkPhysicalDeviceASTCDecodeFeaturesEXT {
public:
    MarshalVkPhysicalDeviceASTCDecodeFeaturesEXT() {}
    VkPhysicalDeviceASTCDecodeFeaturesEXT s;
    MarshalVkPhysicalDeviceASTCDecodeFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceASTCDecodeFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceASTCDecodeFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceASTCDecodeFeaturesEXT();
};

class MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT {
public:
    MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT() {}
    VkPhysicalDeviceTransformFeedbackFeaturesEXT s;
    MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTransformFeedbackFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTransformFeedbackFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT();
};

class MarshalVkPhysicalDeviceTransformFeedbackPropertiesEXT {
public:
    MarshalVkPhysicalDeviceTransformFeedbackPropertiesEXT() {}
    VkPhysicalDeviceTransformFeedbackPropertiesEXT s;
    MarshalVkPhysicalDeviceTransformFeedbackPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTransformFeedbackPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTransformFeedbackPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceTransformFeedbackPropertiesEXT();
};

class MarshalVkPipelineRasterizationStateStreamCreateInfoEXT {
public:
    MarshalVkPipelineRasterizationStateStreamCreateInfoEXT() {}
    VkPipelineRasterizationStateStreamCreateInfoEXT s;
    MarshalVkPipelineRasterizationStateStreamCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationStateStreamCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationStateStreamCreateInfoEXT* s);
    ~MarshalVkPipelineRasterizationStateStreamCreateInfoEXT();
};

class MarshalVkPhysicalDeviceRepresentativeFragmentTestFeaturesNV {
public:
    MarshalVkPhysicalDeviceRepresentativeFragmentTestFeaturesNV() {}
    VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV s;
    MarshalVkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* s);
    ~MarshalVkPhysicalDeviceRepresentativeFragmentTestFeaturesNV();
};

class MarshalVkPipelineRepresentativeFragmentTestStateCreateInfoNV {
public:
    MarshalVkPipelineRepresentativeFragmentTestStateCreateInfoNV() {}
    VkPipelineRepresentativeFragmentTestStateCreateInfoNV s;
    MarshalVkPipelineRepresentativeFragmentTestStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRepresentativeFragmentTestStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRepresentativeFragmentTestStateCreateInfoNV* s);
    ~MarshalVkPipelineRepresentativeFragmentTestStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceExclusiveScissorFeaturesNV {
public:
    MarshalVkPhysicalDeviceExclusiveScissorFeaturesNV() {}
    VkPhysicalDeviceExclusiveScissorFeaturesNV s;
    MarshalVkPhysicalDeviceExclusiveScissorFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExclusiveScissorFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExclusiveScissorFeaturesNV* s);
    ~MarshalVkPhysicalDeviceExclusiveScissorFeaturesNV();
};

class MarshalVkPipelineViewportExclusiveScissorStateCreateInfoNV {
public:
    MarshalVkPipelineViewportExclusiveScissorStateCreateInfoNV() {}
    VkPipelineViewportExclusiveScissorStateCreateInfoNV s;
    MarshalVkPipelineViewportExclusiveScissorStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportExclusiveScissorStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportExclusiveScissorStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportExclusiveScissorStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceCornerSampledImageFeaturesNV {
public:
    MarshalVkPhysicalDeviceCornerSampledImageFeaturesNV() {}
    VkPhysicalDeviceCornerSampledImageFeaturesNV s;
    MarshalVkPhysicalDeviceCornerSampledImageFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCornerSampledImageFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCornerSampledImageFeaturesNV* s);
    ~MarshalVkPhysicalDeviceCornerSampledImageFeaturesNV();
};

class MarshalVkPhysicalDeviceComputeShaderDerivativesFeaturesKHR {
public:
    MarshalVkPhysicalDeviceComputeShaderDerivativesFeaturesKHR() {}
    VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR s;
    MarshalVkPhysicalDeviceComputeShaderDerivativesFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceComputeShaderDerivativesFeaturesKHR();
};

class MarshalVkPhysicalDeviceComputeShaderDerivativesPropertiesKHR {
public:
    MarshalVkPhysicalDeviceComputeShaderDerivativesPropertiesKHR() {}
    VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR s;
    MarshalVkPhysicalDeviceComputeShaderDerivativesPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR* s);
    ~MarshalVkPhysicalDeviceComputeShaderDerivativesPropertiesKHR();
};

class MarshalVkPhysicalDeviceShaderImageFootprintFeaturesNV {
public:
    MarshalVkPhysicalDeviceShaderImageFootprintFeaturesNV() {}
    VkPhysicalDeviceShaderImageFootprintFeaturesNV s;
    MarshalVkPhysicalDeviceShaderImageFootprintFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderImageFootprintFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderImageFootprintFeaturesNV* s);
    ~MarshalVkPhysicalDeviceShaderImageFootprintFeaturesNV();
};

class MarshalVkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV {
public:
    MarshalVkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV() {}
    VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV s;
    MarshalVkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* s);
    ~MarshalVkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV();
};

class MarshalVkPhysicalDeviceCopyMemoryIndirectFeaturesNV {
public:
    MarshalVkPhysicalDeviceCopyMemoryIndirectFeaturesNV() {}
    VkPhysicalDeviceCopyMemoryIndirectFeaturesNV s;
    MarshalVkPhysicalDeviceCopyMemoryIndirectFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCopyMemoryIndirectFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCopyMemoryIndirectFeaturesNV* s);
    ~MarshalVkPhysicalDeviceCopyMemoryIndirectFeaturesNV();
};

class MarshalVkPhysicalDeviceCopyMemoryIndirectPropertiesNV {
public:
    MarshalVkPhysicalDeviceCopyMemoryIndirectPropertiesNV() {}
    VkPhysicalDeviceCopyMemoryIndirectPropertiesNV s;
    MarshalVkPhysicalDeviceCopyMemoryIndirectPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCopyMemoryIndirectPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCopyMemoryIndirectPropertiesNV* s);
    ~MarshalVkPhysicalDeviceCopyMemoryIndirectPropertiesNV();
};

class MarshalVkPhysicalDeviceMemoryDecompressionFeaturesNV {
public:
    MarshalVkPhysicalDeviceMemoryDecompressionFeaturesNV() {}
    VkPhysicalDeviceMemoryDecompressionFeaturesNV s;
    MarshalVkPhysicalDeviceMemoryDecompressionFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryDecompressionFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryDecompressionFeaturesNV* s);
    ~MarshalVkPhysicalDeviceMemoryDecompressionFeaturesNV();
};

class MarshalVkPhysicalDeviceMemoryDecompressionPropertiesNV {
public:
    MarshalVkPhysicalDeviceMemoryDecompressionPropertiesNV() {}
    VkPhysicalDeviceMemoryDecompressionPropertiesNV s;
    MarshalVkPhysicalDeviceMemoryDecompressionPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryDecompressionPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryDecompressionPropertiesNV* s);
    ~MarshalVkPhysicalDeviceMemoryDecompressionPropertiesNV();
};

class MarshalVkShadingRatePaletteNV {
public:
    MarshalVkShadingRatePaletteNV() {}
    VkShadingRatePaletteNV s;
    MarshalVkShadingRatePaletteNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkShadingRatePaletteNV* s);
    ~MarshalVkShadingRatePaletteNV();
};

class MarshalVkPipelineViewportShadingRateImageStateCreateInfoNV {
public:
    MarshalVkPipelineViewportShadingRateImageStateCreateInfoNV() {}
    VkPipelineViewportShadingRateImageStateCreateInfoNV s;
    MarshalVkPipelineViewportShadingRateImageStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportShadingRateImageStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportShadingRateImageStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportShadingRateImageStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceShadingRateImageFeaturesNV {
public:
    MarshalVkPhysicalDeviceShadingRateImageFeaturesNV() {}
    VkPhysicalDeviceShadingRateImageFeaturesNV s;
    MarshalVkPhysicalDeviceShadingRateImageFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShadingRateImageFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShadingRateImageFeaturesNV* s);
    ~MarshalVkPhysicalDeviceShadingRateImageFeaturesNV();
};

class MarshalVkPhysicalDeviceShadingRateImagePropertiesNV {
public:
    MarshalVkPhysicalDeviceShadingRateImagePropertiesNV() {}
    VkPhysicalDeviceShadingRateImagePropertiesNV s;
    MarshalVkPhysicalDeviceShadingRateImagePropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShadingRateImagePropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShadingRateImagePropertiesNV* s);
    ~MarshalVkPhysicalDeviceShadingRateImagePropertiesNV();
};

class MarshalVkPhysicalDeviceInvocationMaskFeaturesHUAWEI {
public:
    MarshalVkPhysicalDeviceInvocationMaskFeaturesHUAWEI() {}
    VkPhysicalDeviceInvocationMaskFeaturesHUAWEI s;
    MarshalVkPhysicalDeviceInvocationMaskFeaturesHUAWEI(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceInvocationMaskFeaturesHUAWEI* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceInvocationMaskFeaturesHUAWEI* s);
    ~MarshalVkPhysicalDeviceInvocationMaskFeaturesHUAWEI();
};

class MarshalVkCoarseSampleLocationNV {
public:
    MarshalVkCoarseSampleLocationNV() {}
    VkCoarseSampleLocationNV s;
    MarshalVkCoarseSampleLocationNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCoarseSampleLocationNV* s);
};

class MarshalVkCoarseSampleOrderCustomNV {
public:
    MarshalVkCoarseSampleOrderCustomNV() {}
    VkCoarseSampleOrderCustomNV s;
    MarshalVkCoarseSampleOrderCustomNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCoarseSampleOrderCustomNV* s);
    ~MarshalVkCoarseSampleOrderCustomNV();
};

class MarshalVkPipelineViewportCoarseSampleOrderStateCreateInfoNV {
public:
    MarshalVkPipelineViewportCoarseSampleOrderStateCreateInfoNV() {}
    VkPipelineViewportCoarseSampleOrderStateCreateInfoNV s;
    MarshalVkPipelineViewportCoarseSampleOrderStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportCoarseSampleOrderStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceMeshShaderFeaturesNV {
public:
    MarshalVkPhysicalDeviceMeshShaderFeaturesNV() {}
    VkPhysicalDeviceMeshShaderFeaturesNV s;
    MarshalVkPhysicalDeviceMeshShaderFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderFeaturesNV* s);
    ~MarshalVkPhysicalDeviceMeshShaderFeaturesNV();
};

class MarshalVkPhysicalDeviceMeshShaderPropertiesNV {
public:
    MarshalVkPhysicalDeviceMeshShaderPropertiesNV() {}
    VkPhysicalDeviceMeshShaderPropertiesNV s;
    MarshalVkPhysicalDeviceMeshShaderPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderPropertiesNV* s);
    ~MarshalVkPhysicalDeviceMeshShaderPropertiesNV();
};

class MarshalVkPhysicalDeviceMeshShaderFeaturesEXT {
public:
    MarshalVkPhysicalDeviceMeshShaderFeaturesEXT() {}
    VkPhysicalDeviceMeshShaderFeaturesEXT s;
    MarshalVkPhysicalDeviceMeshShaderFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceMeshShaderFeaturesEXT();
};

class MarshalVkPhysicalDeviceMeshShaderPropertiesEXT {
public:
    MarshalVkPhysicalDeviceMeshShaderPropertiesEXT() {}
    VkPhysicalDeviceMeshShaderPropertiesEXT s;
    MarshalVkPhysicalDeviceMeshShaderPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceMeshShaderPropertiesEXT();
};

class MarshalVkRayTracingShaderGroupCreateInfoNV {
public:
    MarshalVkRayTracingShaderGroupCreateInfoNV() {}
    VkRayTracingShaderGroupCreateInfoNV s;
    MarshalVkRayTracingShaderGroupCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingShaderGroupCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingShaderGroupCreateInfoNV* s);
    ~MarshalVkRayTracingShaderGroupCreateInfoNV();
};

class MarshalVkRayTracingShaderGroupCreateInfoKHR {
public:
    MarshalVkRayTracingShaderGroupCreateInfoKHR() {}
    VkRayTracingShaderGroupCreateInfoKHR s;
    MarshalVkRayTracingShaderGroupCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingShaderGroupCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingShaderGroupCreateInfoKHR* s);
    ~MarshalVkRayTracingShaderGroupCreateInfoKHR();
};

class MarshalVkRayTracingPipelineCreateInfoNV {
public:
    MarshalVkRayTracingPipelineCreateInfoNV() {}
    VkRayTracingPipelineCreateInfoNV s;
    MarshalVkRayTracingPipelineCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingPipelineCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingPipelineCreateInfoNV* s);
    ~MarshalVkRayTracingPipelineCreateInfoNV();
};

class MarshalVkRayTracingPipelineCreateInfoKHR {
public:
    MarshalVkRayTracingPipelineCreateInfoKHR() {}
    VkRayTracingPipelineCreateInfoKHR s;
    MarshalVkRayTracingPipelineCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingPipelineCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingPipelineCreateInfoKHR* s);
    ~MarshalVkRayTracingPipelineCreateInfoKHR();
};

class MarshalVkGeometryTrianglesNV {
public:
    MarshalVkGeometryTrianglesNV() {}
    VkGeometryTrianglesNV s;
    MarshalVkGeometryTrianglesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeometryTrianglesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeometryTrianglesNV* s);
    ~MarshalVkGeometryTrianglesNV();
};

class MarshalVkGeometryAABBNV {
public:
    MarshalVkGeometryAABBNV() {}
    VkGeometryAABBNV s;
    MarshalVkGeometryAABBNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeometryAABBNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeometryAABBNV* s);
    ~MarshalVkGeometryAABBNV();
};

class MarshalVkGeometryDataNV {
public:
    MarshalVkGeometryDataNV() {}
    VkGeometryDataNV s;
    MarshalVkGeometryDataNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeometryDataNV* s);
};

class MarshalVkGeometryNV {
public:
    MarshalVkGeometryNV() {}
    VkGeometryNV s;
    MarshalVkGeometryNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeometryNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeometryNV* s);
    ~MarshalVkGeometryNV();
};

class MarshalVkAccelerationStructureInfoNV {
public:
    MarshalVkAccelerationStructureInfoNV() {}
    VkAccelerationStructureInfoNV s;
    MarshalVkAccelerationStructureInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureInfoNV* s);
    ~MarshalVkAccelerationStructureInfoNV();
};

class MarshalVkAccelerationStructureCreateInfoNV {
public:
    MarshalVkAccelerationStructureCreateInfoNV() {}
    VkAccelerationStructureCreateInfoNV s;
    MarshalVkAccelerationStructureCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureCreateInfoNV* s);
    ~MarshalVkAccelerationStructureCreateInfoNV();
};

class MarshalVkBindAccelerationStructureMemoryInfoNV {
public:
    MarshalVkBindAccelerationStructureMemoryInfoNV() {}
    VkBindAccelerationStructureMemoryInfoNV s;
    MarshalVkBindAccelerationStructureMemoryInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindAccelerationStructureMemoryInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindAccelerationStructureMemoryInfoNV* s);
    ~MarshalVkBindAccelerationStructureMemoryInfoNV();
};

class MarshalVkWriteDescriptorSetAccelerationStructureKHR {
public:
    MarshalVkWriteDescriptorSetAccelerationStructureKHR() {}
    VkWriteDescriptorSetAccelerationStructureKHR s;
    MarshalVkWriteDescriptorSetAccelerationStructureKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteDescriptorSetAccelerationStructureKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteDescriptorSetAccelerationStructureKHR* s);
    ~MarshalVkWriteDescriptorSetAccelerationStructureKHR();
};

class MarshalVkWriteDescriptorSetAccelerationStructureNV {
public:
    MarshalVkWriteDescriptorSetAccelerationStructureNV() {}
    VkWriteDescriptorSetAccelerationStructureNV s;
    MarshalVkWriteDescriptorSetAccelerationStructureNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteDescriptorSetAccelerationStructureNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteDescriptorSetAccelerationStructureNV* s);
    ~MarshalVkWriteDescriptorSetAccelerationStructureNV();
};

class MarshalVkAccelerationStructureMemoryRequirementsInfoNV {
public:
    MarshalVkAccelerationStructureMemoryRequirementsInfoNV() {}
    VkAccelerationStructureMemoryRequirementsInfoNV s;
    MarshalVkAccelerationStructureMemoryRequirementsInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureMemoryRequirementsInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureMemoryRequirementsInfoNV* s);
    ~MarshalVkAccelerationStructureMemoryRequirementsInfoNV();
};

class MarshalVkPhysicalDeviceAccelerationStructureFeaturesKHR {
public:
    MarshalVkPhysicalDeviceAccelerationStructureFeaturesKHR() {}
    VkPhysicalDeviceAccelerationStructureFeaturesKHR s;
    MarshalVkPhysicalDeviceAccelerationStructureFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAccelerationStructureFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAccelerationStructureFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceAccelerationStructureFeaturesKHR();
};

class MarshalVkPhysicalDeviceRayTracingPipelineFeaturesKHR {
public:
    MarshalVkPhysicalDeviceRayTracingPipelineFeaturesKHR() {}
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR s;
    MarshalVkPhysicalDeviceRayTracingPipelineFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPipelineFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPipelineFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceRayTracingPipelineFeaturesKHR();
};

class MarshalVkPhysicalDeviceRayQueryFeaturesKHR {
public:
    MarshalVkPhysicalDeviceRayQueryFeaturesKHR() {}
    VkPhysicalDeviceRayQueryFeaturesKHR s;
    MarshalVkPhysicalDeviceRayQueryFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayQueryFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayQueryFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceRayQueryFeaturesKHR();
};

class MarshalVkPhysicalDeviceAccelerationStructurePropertiesKHR {
public:
    MarshalVkPhysicalDeviceAccelerationStructurePropertiesKHR() {}
    VkPhysicalDeviceAccelerationStructurePropertiesKHR s;
    MarshalVkPhysicalDeviceAccelerationStructurePropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAccelerationStructurePropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAccelerationStructurePropertiesKHR* s);
    ~MarshalVkPhysicalDeviceAccelerationStructurePropertiesKHR();
};

class MarshalVkPhysicalDeviceRayTracingPipelinePropertiesKHR {
public:
    MarshalVkPhysicalDeviceRayTracingPipelinePropertiesKHR() {}
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR s;
    MarshalVkPhysicalDeviceRayTracingPipelinePropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPipelinePropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPipelinePropertiesKHR* s);
    ~MarshalVkPhysicalDeviceRayTracingPipelinePropertiesKHR();
};

class MarshalVkPhysicalDeviceRayTracingPropertiesNV {
public:
    MarshalVkPhysicalDeviceRayTracingPropertiesNV() {}
    VkPhysicalDeviceRayTracingPropertiesNV s;
    MarshalVkPhysicalDeviceRayTracingPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPropertiesNV* s);
    ~MarshalVkPhysicalDeviceRayTracingPropertiesNV();
};

class MarshalVkStridedDeviceAddressRegionKHR {
public:
    MarshalVkStridedDeviceAddressRegionKHR() {}
    VkStridedDeviceAddressRegionKHR s;
    MarshalVkStridedDeviceAddressRegionKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkStridedDeviceAddressRegionKHR* s);
};

class MarshalVkPhysicalDeviceRayTracingMaintenance1FeaturesKHR {
public:
    MarshalVkPhysicalDeviceRayTracingMaintenance1FeaturesKHR() {}
    VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR s;
    MarshalVkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR* s);
    ~MarshalVkPhysicalDeviceRayTracingMaintenance1FeaturesKHR();
};

class MarshalVkImageStencilUsageCreateInfo {
public:
    MarshalVkImageStencilUsageCreateInfo() {}
    VkImageStencilUsageCreateInfo s;
    MarshalVkImageStencilUsageCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageStencilUsageCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageStencilUsageCreateInfo* s);
    ~MarshalVkImageStencilUsageCreateInfo();
};

class MarshalVkDeviceMemoryOverallocationCreateInfoAMD {
public:
    MarshalVkDeviceMemoryOverallocationCreateInfoAMD() {}
    VkDeviceMemoryOverallocationCreateInfoAMD s;
    MarshalVkDeviceMemoryOverallocationCreateInfoAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceMemoryOverallocationCreateInfoAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceMemoryOverallocationCreateInfoAMD* s);
    ~MarshalVkDeviceMemoryOverallocationCreateInfoAMD();
};

class MarshalVkPhysicalDeviceFragmentDensityMapFeaturesEXT {
public:
    MarshalVkPhysicalDeviceFragmentDensityMapFeaturesEXT() {}
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT s;
    MarshalVkPhysicalDeviceFragmentDensityMapFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMapFeaturesEXT();
};

class MarshalVkPhysicalDeviceFragmentDensityMap2FeaturesEXT {
public:
    MarshalVkPhysicalDeviceFragmentDensityMap2FeaturesEXT() {}
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT s;
    MarshalVkPhysicalDeviceFragmentDensityMap2FeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMap2FeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMap2FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMap2FeaturesEXT();
};

class MarshalVkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM {
public:
    MarshalVkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM() {}
    VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM s;
    MarshalVkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM();
};

class MarshalVkPhysicalDeviceFragmentDensityMapPropertiesEXT {
public:
    MarshalVkPhysicalDeviceFragmentDensityMapPropertiesEXT() {}
    VkPhysicalDeviceFragmentDensityMapPropertiesEXT s;
    MarshalVkPhysicalDeviceFragmentDensityMapPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMapPropertiesEXT();
};

class MarshalVkPhysicalDeviceFragmentDensityMap2PropertiesEXT {
public:
    MarshalVkPhysicalDeviceFragmentDensityMap2PropertiesEXT() {}
    VkPhysicalDeviceFragmentDensityMap2PropertiesEXT s;
    MarshalVkPhysicalDeviceFragmentDensityMap2PropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMap2PropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMap2PropertiesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMap2PropertiesEXT();
};

class MarshalVkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM {
public:
    MarshalVkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM() {}
    VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM s;
    MarshalVkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM();
};

class MarshalVkRenderPassFragmentDensityMapCreateInfoEXT {
public:
    MarshalVkRenderPassFragmentDensityMapCreateInfoEXT() {}
    VkRenderPassFragmentDensityMapCreateInfoEXT s;
    MarshalVkRenderPassFragmentDensityMapCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassFragmentDensityMapCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassFragmentDensityMapCreateInfoEXT* s);
    ~MarshalVkRenderPassFragmentDensityMapCreateInfoEXT();
};

class MarshalVkSubpassFragmentDensityMapOffsetEndInfoQCOM {
public:
    MarshalVkSubpassFragmentDensityMapOffsetEndInfoQCOM() {}
    VkSubpassFragmentDensityMapOffsetEndInfoQCOM s;
    MarshalVkSubpassFragmentDensityMapOffsetEndInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassFragmentDensityMapOffsetEndInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassFragmentDensityMapOffsetEndInfoQCOM* s);
    ~MarshalVkSubpassFragmentDensityMapOffsetEndInfoQCOM();
};

class MarshalVkPhysicalDeviceScalarBlockLayoutFeatures {
public:
    MarshalVkPhysicalDeviceScalarBlockLayoutFeatures() {}
    VkPhysicalDeviceScalarBlockLayoutFeatures s;
    MarshalVkPhysicalDeviceScalarBlockLayoutFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceScalarBlockLayoutFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceScalarBlockLayoutFeatures* s);
    ~MarshalVkPhysicalDeviceScalarBlockLayoutFeatures();
};

class MarshalVkPhysicalDeviceUniformBufferStandardLayoutFeatures {
public:
    MarshalVkPhysicalDeviceUniformBufferStandardLayoutFeatures() {}
    VkPhysicalDeviceUniformBufferStandardLayoutFeatures s;
    MarshalVkPhysicalDeviceUniformBufferStandardLayoutFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceUniformBufferStandardLayoutFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceUniformBufferStandardLayoutFeatures* s);
    ~MarshalVkPhysicalDeviceUniformBufferStandardLayoutFeatures();
};

class MarshalVkPhysicalDeviceDepthClipEnableFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDepthClipEnableFeaturesEXT() {}
    VkPhysicalDeviceDepthClipEnableFeaturesEXT s;
    MarshalVkPhysicalDeviceDepthClipEnableFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthClipEnableFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthClipEnableFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDepthClipEnableFeaturesEXT();
};

class MarshalVkPipelineRasterizationDepthClipStateCreateInfoEXT {
public:
    MarshalVkPipelineRasterizationDepthClipStateCreateInfoEXT() {}
    VkPipelineRasterizationDepthClipStateCreateInfoEXT s;
    MarshalVkPipelineRasterizationDepthClipStateCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationDepthClipStateCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationDepthClipStateCreateInfoEXT* s);
    ~MarshalVkPipelineRasterizationDepthClipStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceMemoryBudgetPropertiesEXT {
public:
    MarshalVkPhysicalDeviceMemoryBudgetPropertiesEXT() {}
    VkPhysicalDeviceMemoryBudgetPropertiesEXT s;
    MarshalVkPhysicalDeviceMemoryBudgetPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryBudgetPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryBudgetPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceMemoryBudgetPropertiesEXT();
};

class MarshalVkPhysicalDeviceMemoryPriorityFeaturesEXT {
public:
    MarshalVkPhysicalDeviceMemoryPriorityFeaturesEXT() {}
    VkPhysicalDeviceMemoryPriorityFeaturesEXT s;
    MarshalVkPhysicalDeviceMemoryPriorityFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryPriorityFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMemoryPriorityFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceMemoryPriorityFeaturesEXT();
};

class MarshalVkMemoryPriorityAllocateInfoEXT {
public:
    MarshalVkMemoryPriorityAllocateInfoEXT() {}
    VkMemoryPriorityAllocateInfoEXT s;
    MarshalVkMemoryPriorityAllocateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryPriorityAllocateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryPriorityAllocateInfoEXT* s);
    ~MarshalVkMemoryPriorityAllocateInfoEXT();
};

class MarshalVkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT {
public:
    MarshalVkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT() {}
    VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT s;
    MarshalVkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT* s);
    ~MarshalVkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT();
};

class MarshalVkPhysicalDeviceBufferDeviceAddressFeatures {
public:
    MarshalVkPhysicalDeviceBufferDeviceAddressFeatures() {}
    VkPhysicalDeviceBufferDeviceAddressFeatures s;
    MarshalVkPhysicalDeviceBufferDeviceAddressFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBufferDeviceAddressFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBufferDeviceAddressFeatures* s);
    ~MarshalVkPhysicalDeviceBufferDeviceAddressFeatures();
};

class MarshalVkPhysicalDeviceBufferDeviceAddressFeaturesEXT {
public:
    MarshalVkPhysicalDeviceBufferDeviceAddressFeaturesEXT() {}
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT s;
    MarshalVkPhysicalDeviceBufferDeviceAddressFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceBufferDeviceAddressFeaturesEXT();
};

class MarshalVkBufferDeviceAddressInfo {
public:
    MarshalVkBufferDeviceAddressInfo() {}
    VkBufferDeviceAddressInfo s;
    MarshalVkBufferDeviceAddressInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferDeviceAddressInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferDeviceAddressInfo* s);
    ~MarshalVkBufferDeviceAddressInfo();
};

class MarshalVkBufferOpaqueCaptureAddressCreateInfo {
public:
    MarshalVkBufferOpaqueCaptureAddressCreateInfo() {}
    VkBufferOpaqueCaptureAddressCreateInfo s;
    MarshalVkBufferOpaqueCaptureAddressCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferOpaqueCaptureAddressCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferOpaqueCaptureAddressCreateInfo* s);
    ~MarshalVkBufferOpaqueCaptureAddressCreateInfo();
};

class MarshalVkBufferDeviceAddressCreateInfoEXT {
public:
    MarshalVkBufferDeviceAddressCreateInfoEXT() {}
    VkBufferDeviceAddressCreateInfoEXT s;
    MarshalVkBufferDeviceAddressCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferDeviceAddressCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferDeviceAddressCreateInfoEXT* s);
    ~MarshalVkBufferDeviceAddressCreateInfoEXT();
};

class MarshalVkPhysicalDeviceImageViewImageFormatInfoEXT {
public:
    MarshalVkPhysicalDeviceImageViewImageFormatInfoEXT() {}
    VkPhysicalDeviceImageViewImageFormatInfoEXT s;
    MarshalVkPhysicalDeviceImageViewImageFormatInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageViewImageFormatInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageViewImageFormatInfoEXT* s);
    ~MarshalVkPhysicalDeviceImageViewImageFormatInfoEXT();
};

class MarshalVkFilterCubicImageViewImageFormatPropertiesEXT {
public:
    MarshalVkFilterCubicImageViewImageFormatPropertiesEXT() {}
    VkFilterCubicImageViewImageFormatPropertiesEXT s;
    MarshalVkFilterCubicImageViewImageFormatPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFilterCubicImageViewImageFormatPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFilterCubicImageViewImageFormatPropertiesEXT* s);
    ~MarshalVkFilterCubicImageViewImageFormatPropertiesEXT();
};

class MarshalVkPhysicalDeviceImagelessFramebufferFeatures {
public:
    MarshalVkPhysicalDeviceImagelessFramebufferFeatures() {}
    VkPhysicalDeviceImagelessFramebufferFeatures s;
    MarshalVkPhysicalDeviceImagelessFramebufferFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImagelessFramebufferFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImagelessFramebufferFeatures* s);
    ~MarshalVkPhysicalDeviceImagelessFramebufferFeatures();
};

class MarshalVkFramebufferAttachmentsCreateInfo {
public:
    MarshalVkFramebufferAttachmentsCreateInfo() {}
    VkFramebufferAttachmentsCreateInfo s;
    MarshalVkFramebufferAttachmentsCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFramebufferAttachmentsCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFramebufferAttachmentsCreateInfo* s);
    ~MarshalVkFramebufferAttachmentsCreateInfo();
};

class MarshalVkFramebufferAttachmentImageInfo {
public:
    MarshalVkFramebufferAttachmentImageInfo() {}
    VkFramebufferAttachmentImageInfo s;
    MarshalVkFramebufferAttachmentImageInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFramebufferAttachmentImageInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFramebufferAttachmentImageInfo* s);
    ~MarshalVkFramebufferAttachmentImageInfo();
};

class MarshalVkRenderPassAttachmentBeginInfo {
public:
    MarshalVkRenderPassAttachmentBeginInfo() {}
    VkRenderPassAttachmentBeginInfo s;
    MarshalVkRenderPassAttachmentBeginInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassAttachmentBeginInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassAttachmentBeginInfo* s);
    ~MarshalVkRenderPassAttachmentBeginInfo();
};

class MarshalVkPhysicalDeviceTextureCompressionASTCHDRFeatures {
public:
    MarshalVkPhysicalDeviceTextureCompressionASTCHDRFeatures() {}
    VkPhysicalDeviceTextureCompressionASTCHDRFeatures s;
    MarshalVkPhysicalDeviceTextureCompressionASTCHDRFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTextureCompressionASTCHDRFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTextureCompressionASTCHDRFeatures* s);
    ~MarshalVkPhysicalDeviceTextureCompressionASTCHDRFeatures();
};

class MarshalVkPhysicalDeviceCooperativeMatrixFeaturesNV {
public:
    MarshalVkPhysicalDeviceCooperativeMatrixFeaturesNV() {}
    VkPhysicalDeviceCooperativeMatrixFeaturesNV s;
    MarshalVkPhysicalDeviceCooperativeMatrixFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixFeaturesNV* s);
    ~MarshalVkPhysicalDeviceCooperativeMatrixFeaturesNV();
};

class MarshalVkPhysicalDeviceCooperativeMatrixPropertiesNV {
public:
    MarshalVkPhysicalDeviceCooperativeMatrixPropertiesNV() {}
    VkPhysicalDeviceCooperativeMatrixPropertiesNV s;
    MarshalVkPhysicalDeviceCooperativeMatrixPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixPropertiesNV* s);
    ~MarshalVkPhysicalDeviceCooperativeMatrixPropertiesNV();
};

class MarshalVkCooperativeMatrixPropertiesNV {
public:
    MarshalVkCooperativeMatrixPropertiesNV() {}
    VkCooperativeMatrixPropertiesNV s;
    MarshalVkCooperativeMatrixPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCooperativeMatrixPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCooperativeMatrixPropertiesNV* s);
    ~MarshalVkCooperativeMatrixPropertiesNV();
};

class MarshalVkPhysicalDeviceYcbcrImageArraysFeaturesEXT {
public:
    MarshalVkPhysicalDeviceYcbcrImageArraysFeaturesEXT() {}
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT s;
    MarshalVkPhysicalDeviceYcbcrImageArraysFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceYcbcrImageArraysFeaturesEXT();
};

class MarshalVkImageViewHandleInfoNVX {
public:
    MarshalVkImageViewHandleInfoNVX() {}
    VkImageViewHandleInfoNVX s;
    MarshalVkImageViewHandleInfoNVX(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewHandleInfoNVX* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewHandleInfoNVX* s);
    ~MarshalVkImageViewHandleInfoNVX();
};

class MarshalVkImageViewAddressPropertiesNVX {
public:
    MarshalVkImageViewAddressPropertiesNVX() {}
    VkImageViewAddressPropertiesNVX s;
    MarshalVkImageViewAddressPropertiesNVX(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewAddressPropertiesNVX* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewAddressPropertiesNVX* s);
    ~MarshalVkImageViewAddressPropertiesNVX();
};

class MarshalVkPipelineCreationFeedback {
public:
    MarshalVkPipelineCreationFeedback() {}
    VkPipelineCreationFeedback s;
    MarshalVkPipelineCreationFeedback(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCreationFeedback* s);
};

class MarshalVkPipelineCreationFeedbackCreateInfo {
public:
    MarshalVkPipelineCreationFeedbackCreateInfo() {}
    VkPipelineCreationFeedbackCreateInfo s;
    MarshalVkPipelineCreationFeedbackCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCreationFeedbackCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCreationFeedbackCreateInfo* s);
    ~MarshalVkPipelineCreationFeedbackCreateInfo();
};

class MarshalVkPhysicalDevicePresentBarrierFeaturesNV {
public:
    MarshalVkPhysicalDevicePresentBarrierFeaturesNV() {}
    VkPhysicalDevicePresentBarrierFeaturesNV s;
    MarshalVkPhysicalDevicePresentBarrierFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePresentBarrierFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePresentBarrierFeaturesNV* s);
    ~MarshalVkPhysicalDevicePresentBarrierFeaturesNV();
};

class MarshalVkSurfaceCapabilitiesPresentBarrierNV {
public:
    MarshalVkSurfaceCapabilitiesPresentBarrierNV() {}
    VkSurfaceCapabilitiesPresentBarrierNV s;
    MarshalVkSurfaceCapabilitiesPresentBarrierNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceCapabilitiesPresentBarrierNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfaceCapabilitiesPresentBarrierNV* s);
    ~MarshalVkSurfaceCapabilitiesPresentBarrierNV();
};

class MarshalVkSwapchainPresentBarrierCreateInfoNV {
public:
    MarshalVkSwapchainPresentBarrierCreateInfoNV() {}
    VkSwapchainPresentBarrierCreateInfoNV s;
    MarshalVkSwapchainPresentBarrierCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentBarrierCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentBarrierCreateInfoNV* s);
    ~MarshalVkSwapchainPresentBarrierCreateInfoNV();
};

class MarshalVkPhysicalDevicePerformanceQueryFeaturesKHR {
public:
    MarshalVkPhysicalDevicePerformanceQueryFeaturesKHR() {}
    VkPhysicalDevicePerformanceQueryFeaturesKHR s;
    MarshalVkPhysicalDevicePerformanceQueryFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePerformanceQueryFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePerformanceQueryFeaturesKHR* s);
    ~MarshalVkPhysicalDevicePerformanceQueryFeaturesKHR();
};

class MarshalVkPhysicalDevicePerformanceQueryPropertiesKHR {
public:
    MarshalVkPhysicalDevicePerformanceQueryPropertiesKHR() {}
    VkPhysicalDevicePerformanceQueryPropertiesKHR s;
    MarshalVkPhysicalDevicePerformanceQueryPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePerformanceQueryPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePerformanceQueryPropertiesKHR* s);
    ~MarshalVkPhysicalDevicePerformanceQueryPropertiesKHR();
};

class MarshalVkPerformanceCounterKHR {
public:
    MarshalVkPerformanceCounterKHR() {}
    VkPerformanceCounterKHR s;
    MarshalVkPerformanceCounterKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceCounterKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceCounterKHR* s);
    ~MarshalVkPerformanceCounterKHR();
};

class MarshalVkPerformanceCounterDescriptionKHR {
public:
    MarshalVkPerformanceCounterDescriptionKHR() {}
    VkPerformanceCounterDescriptionKHR s;
    MarshalVkPerformanceCounterDescriptionKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceCounterDescriptionKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceCounterDescriptionKHR* s);
    ~MarshalVkPerformanceCounterDescriptionKHR();
};

class MarshalVkQueryPoolPerformanceCreateInfoKHR {
public:
    MarshalVkQueryPoolPerformanceCreateInfoKHR() {}
    VkQueryPoolPerformanceCreateInfoKHR s;
    MarshalVkQueryPoolPerformanceCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryPoolPerformanceCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryPoolPerformanceCreateInfoKHR* s);
    ~MarshalVkQueryPoolPerformanceCreateInfoKHR();
};

class MarshalVkAcquireProfilingLockInfoKHR {
public:
    MarshalVkAcquireProfilingLockInfoKHR() {}
    VkAcquireProfilingLockInfoKHR s;
    MarshalVkAcquireProfilingLockInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAcquireProfilingLockInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAcquireProfilingLockInfoKHR* s);
    ~MarshalVkAcquireProfilingLockInfoKHR();
};

class MarshalVkPerformanceQuerySubmitInfoKHR {
public:
    MarshalVkPerformanceQuerySubmitInfoKHR() {}
    VkPerformanceQuerySubmitInfoKHR s;
    MarshalVkPerformanceQuerySubmitInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceQuerySubmitInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceQuerySubmitInfoKHR* s);
    ~MarshalVkPerformanceQuerySubmitInfoKHR();
};

class MarshalVkPhysicalDeviceCoverageReductionModeFeaturesNV {
public:
    MarshalVkPhysicalDeviceCoverageReductionModeFeaturesNV() {}
    VkPhysicalDeviceCoverageReductionModeFeaturesNV s;
    MarshalVkPhysicalDeviceCoverageReductionModeFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCoverageReductionModeFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCoverageReductionModeFeaturesNV* s);
    ~MarshalVkPhysicalDeviceCoverageReductionModeFeaturesNV();
};

class MarshalVkPipelineCoverageReductionStateCreateInfoNV {
public:
    MarshalVkPipelineCoverageReductionStateCreateInfoNV() {}
    VkPipelineCoverageReductionStateCreateInfoNV s;
    MarshalVkPipelineCoverageReductionStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCoverageReductionStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCoverageReductionStateCreateInfoNV* s);
    ~MarshalVkPipelineCoverageReductionStateCreateInfoNV();
};

class MarshalVkFramebufferMixedSamplesCombinationNV {
public:
    MarshalVkFramebufferMixedSamplesCombinationNV() {}
    VkFramebufferMixedSamplesCombinationNV s;
    MarshalVkFramebufferMixedSamplesCombinationNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFramebufferMixedSamplesCombinationNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFramebufferMixedSamplesCombinationNV* s);
    ~MarshalVkFramebufferMixedSamplesCombinationNV();
};

class MarshalVkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL {
public:
    MarshalVkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL() {}
    VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL s;
    MarshalVkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* s);
    ~MarshalVkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL();
};

class MarshalVkPerformanceValueINTEL {
public:
    MarshalVkPerformanceValueINTEL() {}
    VkPerformanceValueINTEL s;
    MarshalVkPerformanceValueINTEL(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceValueINTEL* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceValueINTEL* s);
    ~MarshalVkPerformanceValueINTEL();
};

class MarshalVkInitializePerformanceApiInfoINTEL {
public:
    MarshalVkInitializePerformanceApiInfoINTEL() {}
    VkInitializePerformanceApiInfoINTEL s;
    MarshalVkInitializePerformanceApiInfoINTEL(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkInitializePerformanceApiInfoINTEL* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkInitializePerformanceApiInfoINTEL* s);
    ~MarshalVkInitializePerformanceApiInfoINTEL();
};

class MarshalVkQueryPoolPerformanceQueryCreateInfoINTEL {
public:
    MarshalVkQueryPoolPerformanceQueryCreateInfoINTEL() {}
    VkQueryPoolPerformanceQueryCreateInfoINTEL s;
    MarshalVkQueryPoolPerformanceQueryCreateInfoINTEL(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryPoolPerformanceQueryCreateInfoINTEL* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryPoolPerformanceQueryCreateInfoINTEL* s);
    ~MarshalVkQueryPoolPerformanceQueryCreateInfoINTEL();
};

class MarshalVkPerformanceMarkerInfoINTEL {
public:
    MarshalVkPerformanceMarkerInfoINTEL() {}
    VkPerformanceMarkerInfoINTEL s;
    MarshalVkPerformanceMarkerInfoINTEL(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceMarkerInfoINTEL* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceMarkerInfoINTEL* s);
    ~MarshalVkPerformanceMarkerInfoINTEL();
};

class MarshalVkPerformanceStreamMarkerInfoINTEL {
public:
    MarshalVkPerformanceStreamMarkerInfoINTEL() {}
    VkPerformanceStreamMarkerInfoINTEL s;
    MarshalVkPerformanceStreamMarkerInfoINTEL(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceStreamMarkerInfoINTEL* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceStreamMarkerInfoINTEL* s);
    ~MarshalVkPerformanceStreamMarkerInfoINTEL();
};

class MarshalVkPerformanceOverrideInfoINTEL {
public:
    MarshalVkPerformanceOverrideInfoINTEL() {}
    VkPerformanceOverrideInfoINTEL s;
    MarshalVkPerformanceOverrideInfoINTEL(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceOverrideInfoINTEL* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceOverrideInfoINTEL* s);
    ~MarshalVkPerformanceOverrideInfoINTEL();
};

class MarshalVkPerformanceConfigurationAcquireInfoINTEL {
public:
    MarshalVkPerformanceConfigurationAcquireInfoINTEL() {}
    VkPerformanceConfigurationAcquireInfoINTEL s;
    MarshalVkPerformanceConfigurationAcquireInfoINTEL(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceConfigurationAcquireInfoINTEL* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPerformanceConfigurationAcquireInfoINTEL* s);
    ~MarshalVkPerformanceConfigurationAcquireInfoINTEL();
};

class MarshalVkPhysicalDeviceShaderClockFeaturesKHR {
public:
    MarshalVkPhysicalDeviceShaderClockFeaturesKHR() {}
    VkPhysicalDeviceShaderClockFeaturesKHR s;
    MarshalVkPhysicalDeviceShaderClockFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderClockFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderClockFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceShaderClockFeaturesKHR();
};

class MarshalVkPhysicalDeviceIndexTypeUint8Features {
public:
    MarshalVkPhysicalDeviceIndexTypeUint8Features() {}
    VkPhysicalDeviceIndexTypeUint8Features s;
    MarshalVkPhysicalDeviceIndexTypeUint8Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceIndexTypeUint8Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceIndexTypeUint8Features* s);
    ~MarshalVkPhysicalDeviceIndexTypeUint8Features();
};

class MarshalVkPhysicalDeviceShaderSMBuiltinsPropertiesNV {
public:
    MarshalVkPhysicalDeviceShaderSMBuiltinsPropertiesNV() {}
    VkPhysicalDeviceShaderSMBuiltinsPropertiesNV s;
    MarshalVkPhysicalDeviceShaderSMBuiltinsPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* s);
    ~MarshalVkPhysicalDeviceShaderSMBuiltinsPropertiesNV();
};

class MarshalVkPhysicalDeviceShaderSMBuiltinsFeaturesNV {
public:
    MarshalVkPhysicalDeviceShaderSMBuiltinsFeaturesNV() {}
    VkPhysicalDeviceShaderSMBuiltinsFeaturesNV s;
    MarshalVkPhysicalDeviceShaderSMBuiltinsFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* s);
    ~MarshalVkPhysicalDeviceShaderSMBuiltinsFeaturesNV();
};

class MarshalVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT {
public:
    MarshalVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT() {}
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT s;
    MarshalVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT();
};

class MarshalVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures {
public:
    MarshalVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures() {}
    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures s;
    MarshalVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures* s);
    ~MarshalVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures();
};

class MarshalVkAttachmentReferenceStencilLayout {
public:
    MarshalVkAttachmentReferenceStencilLayout() {}
    VkAttachmentReferenceStencilLayout s;
    MarshalVkAttachmentReferenceStencilLayout(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentReferenceStencilLayout* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentReferenceStencilLayout* s);
    ~MarshalVkAttachmentReferenceStencilLayout();
};

class MarshalVkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT {
public:
    MarshalVkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT() {}
    VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT s;
    MarshalVkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT* s);
    ~MarshalVkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT();
};

class MarshalVkAttachmentDescriptionStencilLayout {
public:
    MarshalVkAttachmentDescriptionStencilLayout() {}
    VkAttachmentDescriptionStencilLayout s;
    MarshalVkAttachmentDescriptionStencilLayout(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentDescriptionStencilLayout* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentDescriptionStencilLayout* s);
    ~MarshalVkAttachmentDescriptionStencilLayout();
};

class MarshalVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR {
public:
    MarshalVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR() {}
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR s;
    MarshalVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* s);
    ~MarshalVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR();
};

class MarshalVkPipelineInfoKHR {
public:
    MarshalVkPipelineInfoKHR() {}
    VkPipelineInfoKHR s;
    MarshalVkPipelineInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineInfoKHR* s);
    ~MarshalVkPipelineInfoKHR();
};

class MarshalVkPipelineExecutablePropertiesKHR {
public:
    MarshalVkPipelineExecutablePropertiesKHR() {}
    VkPipelineExecutablePropertiesKHR s;
    MarshalVkPipelineExecutablePropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineExecutablePropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineExecutablePropertiesKHR* s);
    ~MarshalVkPipelineExecutablePropertiesKHR();
};

class MarshalVkPipelineExecutableInfoKHR {
public:
    MarshalVkPipelineExecutableInfoKHR() {}
    VkPipelineExecutableInfoKHR s;
    MarshalVkPipelineExecutableInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineExecutableInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineExecutableInfoKHR* s);
    ~MarshalVkPipelineExecutableInfoKHR();
};

class MarshalVkPipelineExecutableStatisticKHR {
public:
    MarshalVkPipelineExecutableStatisticKHR() {}
    VkPipelineExecutableStatisticKHR s;
    MarshalVkPipelineExecutableStatisticKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineExecutableStatisticKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineExecutableStatisticKHR* s);
    ~MarshalVkPipelineExecutableStatisticKHR();
};

class MarshalVkPipelineExecutableInternalRepresentationKHR {
public:
    MarshalVkPipelineExecutableInternalRepresentationKHR() {}
    VkPipelineExecutableInternalRepresentationKHR s;
    MarshalVkPipelineExecutableInternalRepresentationKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineExecutableInternalRepresentationKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineExecutableInternalRepresentationKHR* s);
    ~MarshalVkPipelineExecutableInternalRepresentationKHR();
};

class MarshalVkPhysicalDeviceShaderDemoteToHelperInvocationFeatures {
public:
    MarshalVkPhysicalDeviceShaderDemoteToHelperInvocationFeatures() {}
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures s;
    MarshalVkPhysicalDeviceShaderDemoteToHelperInvocationFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures* s);
    ~MarshalVkPhysicalDeviceShaderDemoteToHelperInvocationFeatures();
};

class MarshalVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT {
public:
    MarshalVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT() {}
    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT s;
    MarshalVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT();
};

class MarshalVkPhysicalDeviceTexelBufferAlignmentProperties {
public:
    MarshalVkPhysicalDeviceTexelBufferAlignmentProperties() {}
    VkPhysicalDeviceTexelBufferAlignmentProperties s;
    MarshalVkPhysicalDeviceTexelBufferAlignmentProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTexelBufferAlignmentProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTexelBufferAlignmentProperties* s);
    ~MarshalVkPhysicalDeviceTexelBufferAlignmentProperties();
};

class MarshalVkPhysicalDeviceSubgroupSizeControlFeatures {
public:
    MarshalVkPhysicalDeviceSubgroupSizeControlFeatures() {}
    VkPhysicalDeviceSubgroupSizeControlFeatures s;
    MarshalVkPhysicalDeviceSubgroupSizeControlFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubgroupSizeControlFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubgroupSizeControlFeatures* s);
    ~MarshalVkPhysicalDeviceSubgroupSizeControlFeatures();
};

class MarshalVkPhysicalDeviceSubgroupSizeControlProperties {
public:
    MarshalVkPhysicalDeviceSubgroupSizeControlProperties() {}
    VkPhysicalDeviceSubgroupSizeControlProperties s;
    MarshalVkPhysicalDeviceSubgroupSizeControlProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubgroupSizeControlProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubgroupSizeControlProperties* s);
    ~MarshalVkPhysicalDeviceSubgroupSizeControlProperties();
};

class MarshalVkPipelineShaderStageRequiredSubgroupSizeCreateInfo {
public:
    MarshalVkPipelineShaderStageRequiredSubgroupSizeCreateInfo() {}
    VkPipelineShaderStageRequiredSubgroupSizeCreateInfo s;
    MarshalVkPipelineShaderStageRequiredSubgroupSizeCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineShaderStageRequiredSubgroupSizeCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineShaderStageRequiredSubgroupSizeCreateInfo* s);
    ~MarshalVkPipelineShaderStageRequiredSubgroupSizeCreateInfo();
};

class MarshalVkSubpassShadingPipelineCreateInfoHUAWEI {
public:
    MarshalVkSubpassShadingPipelineCreateInfoHUAWEI() {}
    VkSubpassShadingPipelineCreateInfoHUAWEI s;
    MarshalVkSubpassShadingPipelineCreateInfoHUAWEI(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassShadingPipelineCreateInfoHUAWEI* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassShadingPipelineCreateInfoHUAWEI* s);
    ~MarshalVkSubpassShadingPipelineCreateInfoHUAWEI();
};

class MarshalVkPhysicalDeviceSubpassShadingPropertiesHUAWEI {
public:
    MarshalVkPhysicalDeviceSubpassShadingPropertiesHUAWEI() {}
    VkPhysicalDeviceSubpassShadingPropertiesHUAWEI s;
    MarshalVkPhysicalDeviceSubpassShadingPropertiesHUAWEI(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubpassShadingPropertiesHUAWEI* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubpassShadingPropertiesHUAWEI* s);
    ~MarshalVkPhysicalDeviceSubpassShadingPropertiesHUAWEI();
};

class MarshalVkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI {
public:
    MarshalVkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI() {}
    VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI s;
    MarshalVkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI* s);
    ~MarshalVkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI();
};

class MarshalVkMemoryOpaqueCaptureAddressAllocateInfo {
public:
    MarshalVkMemoryOpaqueCaptureAddressAllocateInfo() {}
    VkMemoryOpaqueCaptureAddressAllocateInfo s;
    MarshalVkMemoryOpaqueCaptureAddressAllocateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryOpaqueCaptureAddressAllocateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryOpaqueCaptureAddressAllocateInfo* s);
    ~MarshalVkMemoryOpaqueCaptureAddressAllocateInfo();
};

class MarshalVkDeviceMemoryOpaqueCaptureAddressInfo {
public:
    MarshalVkDeviceMemoryOpaqueCaptureAddressInfo() {}
    VkDeviceMemoryOpaqueCaptureAddressInfo s;
    MarshalVkDeviceMemoryOpaqueCaptureAddressInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceMemoryOpaqueCaptureAddressInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceMemoryOpaqueCaptureAddressInfo* s);
    ~MarshalVkDeviceMemoryOpaqueCaptureAddressInfo();
};

class MarshalVkPhysicalDeviceLineRasterizationFeatures {
public:
    MarshalVkPhysicalDeviceLineRasterizationFeatures() {}
    VkPhysicalDeviceLineRasterizationFeatures s;
    MarshalVkPhysicalDeviceLineRasterizationFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLineRasterizationFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLineRasterizationFeatures* s);
    ~MarshalVkPhysicalDeviceLineRasterizationFeatures();
};

class MarshalVkPhysicalDeviceLineRasterizationProperties {
public:
    MarshalVkPhysicalDeviceLineRasterizationProperties() {}
    VkPhysicalDeviceLineRasterizationProperties s;
    MarshalVkPhysicalDeviceLineRasterizationProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLineRasterizationProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLineRasterizationProperties* s);
    ~MarshalVkPhysicalDeviceLineRasterizationProperties();
};

class MarshalVkPipelineRasterizationLineStateCreateInfo {
public:
    MarshalVkPipelineRasterizationLineStateCreateInfo() {}
    VkPipelineRasterizationLineStateCreateInfo s;
    MarshalVkPipelineRasterizationLineStateCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationLineStateCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationLineStateCreateInfo* s);
    ~MarshalVkPipelineRasterizationLineStateCreateInfo();
};

class MarshalVkPhysicalDevicePipelineCreationCacheControlFeatures {
public:
    MarshalVkPhysicalDevicePipelineCreationCacheControlFeatures() {}
    VkPhysicalDevicePipelineCreationCacheControlFeatures s;
    MarshalVkPhysicalDevicePipelineCreationCacheControlFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineCreationCacheControlFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineCreationCacheControlFeatures* s);
    ~MarshalVkPhysicalDevicePipelineCreationCacheControlFeatures();
};

class MarshalVkPhysicalDeviceVulkan11Features {
public:
    MarshalVkPhysicalDeviceVulkan11Features() {}
    VkPhysicalDeviceVulkan11Features s;
    MarshalVkPhysicalDeviceVulkan11Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan11Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan11Features* s);
    ~MarshalVkPhysicalDeviceVulkan11Features();
};

class MarshalVkPhysicalDeviceVulkan11Properties {
public:
    MarshalVkPhysicalDeviceVulkan11Properties() {}
    VkPhysicalDeviceVulkan11Properties s;
    MarshalVkPhysicalDeviceVulkan11Properties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan11Properties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan11Properties* s);
    ~MarshalVkPhysicalDeviceVulkan11Properties();
};

class MarshalVkPhysicalDeviceVulkan12Features {
public:
    MarshalVkPhysicalDeviceVulkan12Features() {}
    VkPhysicalDeviceVulkan12Features s;
    MarshalVkPhysicalDeviceVulkan12Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan12Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan12Features* s);
    ~MarshalVkPhysicalDeviceVulkan12Features();
};

class MarshalVkPhysicalDeviceVulkan12Properties {
public:
    MarshalVkPhysicalDeviceVulkan12Properties() {}
    VkPhysicalDeviceVulkan12Properties s;
    MarshalVkPhysicalDeviceVulkan12Properties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan12Properties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan12Properties* s);
    ~MarshalVkPhysicalDeviceVulkan12Properties();
};

class MarshalVkPhysicalDeviceVulkan13Features {
public:
    MarshalVkPhysicalDeviceVulkan13Features() {}
    VkPhysicalDeviceVulkan13Features s;
    MarshalVkPhysicalDeviceVulkan13Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan13Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan13Features* s);
    ~MarshalVkPhysicalDeviceVulkan13Features();
};

class MarshalVkPhysicalDeviceVulkan13Properties {
public:
    MarshalVkPhysicalDeviceVulkan13Properties() {}
    VkPhysicalDeviceVulkan13Properties s;
    MarshalVkPhysicalDeviceVulkan13Properties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan13Properties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan13Properties* s);
    ~MarshalVkPhysicalDeviceVulkan13Properties();
};

class MarshalVkPhysicalDeviceVulkan14Features {
public:
    MarshalVkPhysicalDeviceVulkan14Features() {}
    VkPhysicalDeviceVulkan14Features s;
    MarshalVkPhysicalDeviceVulkan14Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan14Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan14Features* s);
    ~MarshalVkPhysicalDeviceVulkan14Features();
};

class MarshalVkPhysicalDeviceVulkan14Properties {
public:
    MarshalVkPhysicalDeviceVulkan14Properties() {}
    VkPhysicalDeviceVulkan14Properties s;
    MarshalVkPhysicalDeviceVulkan14Properties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan14Properties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVulkan14Properties* s);
    ~MarshalVkPhysicalDeviceVulkan14Properties();
};

class MarshalVkPipelineCompilerControlCreateInfoAMD {
public:
    MarshalVkPipelineCompilerControlCreateInfoAMD() {}
    VkPipelineCompilerControlCreateInfoAMD s;
    MarshalVkPipelineCompilerControlCreateInfoAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCompilerControlCreateInfoAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineCompilerControlCreateInfoAMD* s);
    ~MarshalVkPipelineCompilerControlCreateInfoAMD();
};

class MarshalVkPhysicalDeviceCoherentMemoryFeaturesAMD {
public:
    MarshalVkPhysicalDeviceCoherentMemoryFeaturesAMD() {}
    VkPhysicalDeviceCoherentMemoryFeaturesAMD s;
    MarshalVkPhysicalDeviceCoherentMemoryFeaturesAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCoherentMemoryFeaturesAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCoherentMemoryFeaturesAMD* s);
    ~MarshalVkPhysicalDeviceCoherentMemoryFeaturesAMD();
};

class MarshalVkPhysicalDeviceToolProperties {
public:
    MarshalVkPhysicalDeviceToolProperties() {}
    VkPhysicalDeviceToolProperties s;
    MarshalVkPhysicalDeviceToolProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceToolProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceToolProperties* s);
    ~MarshalVkPhysicalDeviceToolProperties();
};

class MarshalVkSamplerCustomBorderColorCreateInfoEXT {
public:
    MarshalVkSamplerCustomBorderColorCreateInfoEXT() {}
    VkSamplerCustomBorderColorCreateInfoEXT s;
    MarshalVkSamplerCustomBorderColorCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerCustomBorderColorCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerCustomBorderColorCreateInfoEXT* s);
    ~MarshalVkSamplerCustomBorderColorCreateInfoEXT();
};

class MarshalVkPhysicalDeviceCustomBorderColorPropertiesEXT {
public:
    MarshalVkPhysicalDeviceCustomBorderColorPropertiesEXT() {}
    VkPhysicalDeviceCustomBorderColorPropertiesEXT s;
    MarshalVkPhysicalDeviceCustomBorderColorPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCustomBorderColorPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCustomBorderColorPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceCustomBorderColorPropertiesEXT();
};

class MarshalVkPhysicalDeviceCustomBorderColorFeaturesEXT {
public:
    MarshalVkPhysicalDeviceCustomBorderColorFeaturesEXT() {}
    VkPhysicalDeviceCustomBorderColorFeaturesEXT s;
    MarshalVkPhysicalDeviceCustomBorderColorFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCustomBorderColorFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCustomBorderColorFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceCustomBorderColorFeaturesEXT();
};

class MarshalVkSamplerBorderColorComponentMappingCreateInfoEXT {
public:
    MarshalVkSamplerBorderColorComponentMappingCreateInfoEXT() {}
    VkSamplerBorderColorComponentMappingCreateInfoEXT s;
    MarshalVkSamplerBorderColorComponentMappingCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerBorderColorComponentMappingCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerBorderColorComponentMappingCreateInfoEXT* s);
    ~MarshalVkSamplerBorderColorComponentMappingCreateInfoEXT();
};

class MarshalVkPhysicalDeviceBorderColorSwizzleFeaturesEXT {
public:
    MarshalVkPhysicalDeviceBorderColorSwizzleFeaturesEXT() {}
    VkPhysicalDeviceBorderColorSwizzleFeaturesEXT s;
    MarshalVkPhysicalDeviceBorderColorSwizzleFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBorderColorSwizzleFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceBorderColorSwizzleFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceBorderColorSwizzleFeaturesEXT();
};

class MarshalVkAccelerationStructureGeometryTrianglesDataKHR {
public:
    MarshalVkAccelerationStructureGeometryTrianglesDataKHR() {}
    VkAccelerationStructureGeometryTrianglesDataKHR s;
    MarshalVkAccelerationStructureGeometryTrianglesDataKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryTrianglesDataKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryTrianglesDataKHR* s);
};

class MarshalVkAccelerationStructureGeometryAabbsDataKHR {
public:
    MarshalVkAccelerationStructureGeometryAabbsDataKHR() {}
    VkAccelerationStructureGeometryAabbsDataKHR s;
    MarshalVkAccelerationStructureGeometryAabbsDataKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryAabbsDataKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryAabbsDataKHR* s);
};

class MarshalVkAccelerationStructureGeometryInstancesDataKHR {
public:
    MarshalVkAccelerationStructureGeometryInstancesDataKHR() {}
    VkAccelerationStructureGeometryInstancesDataKHR s;
    MarshalVkAccelerationStructureGeometryInstancesDataKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryInstancesDataKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryInstancesDataKHR* s);
};

class MarshalVkAccelerationStructureGeometryKHR {
public:
    MarshalVkAccelerationStructureGeometryKHR() {}
    VkAccelerationStructureGeometryKHR s;
    MarshalVkAccelerationStructureGeometryKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryKHR* s);
    ~MarshalVkAccelerationStructureGeometryKHR();
};

class MarshalVkAccelerationStructureBuildGeometryInfoKHR {
public:
    MarshalVkAccelerationStructureBuildGeometryInfoKHR() {}
    VkAccelerationStructureBuildGeometryInfoKHR s;
    MarshalVkAccelerationStructureBuildGeometryInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureBuildGeometryInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureBuildGeometryInfoKHR* s);
};

class MarshalVkAccelerationStructureBuildRangeInfoKHR {
public:
    MarshalVkAccelerationStructureBuildRangeInfoKHR() {}
    VkAccelerationStructureBuildRangeInfoKHR s;
    MarshalVkAccelerationStructureBuildRangeInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureBuildRangeInfoKHR* s);
};

class MarshalVkAccelerationStructureCreateInfoKHR {
public:
    MarshalVkAccelerationStructureCreateInfoKHR() {}
    VkAccelerationStructureCreateInfoKHR s;
    MarshalVkAccelerationStructureCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureCreateInfoKHR* s);
    ~MarshalVkAccelerationStructureCreateInfoKHR();
};

class MarshalVkAccelerationStructureDeviceAddressInfoKHR {
public:
    MarshalVkAccelerationStructureDeviceAddressInfoKHR() {}
    VkAccelerationStructureDeviceAddressInfoKHR s;
    MarshalVkAccelerationStructureDeviceAddressInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureDeviceAddressInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureDeviceAddressInfoKHR* s);
    ~MarshalVkAccelerationStructureDeviceAddressInfoKHR();
};

class MarshalVkAccelerationStructureVersionInfoKHR {
public:
    MarshalVkAccelerationStructureVersionInfoKHR() {}
    VkAccelerationStructureVersionInfoKHR s;
    MarshalVkAccelerationStructureVersionInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureVersionInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureVersionInfoKHR* s);
    ~MarshalVkAccelerationStructureVersionInfoKHR();
};

class MarshalVkCopyAccelerationStructureInfoKHR {
public:
    MarshalVkCopyAccelerationStructureInfoKHR() {}
    VkCopyAccelerationStructureInfoKHR s;
    MarshalVkCopyAccelerationStructureInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyAccelerationStructureInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyAccelerationStructureInfoKHR* s);
    ~MarshalVkCopyAccelerationStructureInfoKHR();
};

class MarshalVkCopyAccelerationStructureToMemoryInfoKHR {
public:
    MarshalVkCopyAccelerationStructureToMemoryInfoKHR() {}
    VkCopyAccelerationStructureToMemoryInfoKHR s;
    MarshalVkCopyAccelerationStructureToMemoryInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyAccelerationStructureToMemoryInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyAccelerationStructureToMemoryInfoKHR* s);
};

class MarshalVkCopyMemoryToAccelerationStructureInfoKHR {
public:
    MarshalVkCopyMemoryToAccelerationStructureInfoKHR() {}
    VkCopyMemoryToAccelerationStructureInfoKHR s;
    MarshalVkCopyMemoryToAccelerationStructureInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMemoryToAccelerationStructureInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMemoryToAccelerationStructureInfoKHR* s);
};

class MarshalVkRayTracingPipelineInterfaceCreateInfoKHR {
public:
    MarshalVkRayTracingPipelineInterfaceCreateInfoKHR() {}
    VkRayTracingPipelineInterfaceCreateInfoKHR s;
    MarshalVkRayTracingPipelineInterfaceCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingPipelineInterfaceCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRayTracingPipelineInterfaceCreateInfoKHR* s);
    ~MarshalVkRayTracingPipelineInterfaceCreateInfoKHR();
};

class MarshalVkPipelineLibraryCreateInfoKHR {
public:
    MarshalVkPipelineLibraryCreateInfoKHR() {}
    VkPipelineLibraryCreateInfoKHR s;
    MarshalVkPipelineLibraryCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineLibraryCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineLibraryCreateInfoKHR* s);
    ~MarshalVkPipelineLibraryCreateInfoKHR();
};

class MarshalVkPhysicalDeviceExtendedDynamicStateFeaturesEXT {
public:
    MarshalVkPhysicalDeviceExtendedDynamicStateFeaturesEXT() {}
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT s;
    MarshalVkPhysicalDeviceExtendedDynamicStateFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceExtendedDynamicStateFeaturesEXT();
};

class MarshalVkPhysicalDeviceExtendedDynamicState2FeaturesEXT {
public:
    MarshalVkPhysicalDeviceExtendedDynamicState2FeaturesEXT() {}
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT s;
    MarshalVkPhysicalDeviceExtendedDynamicState2FeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceExtendedDynamicState2FeaturesEXT();
};

class MarshalVkPhysicalDeviceExtendedDynamicState3FeaturesEXT {
public:
    MarshalVkPhysicalDeviceExtendedDynamicState3FeaturesEXT() {}
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT s;
    MarshalVkPhysicalDeviceExtendedDynamicState3FeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicState3FeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicState3FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceExtendedDynamicState3FeaturesEXT();
};

class MarshalVkPhysicalDeviceExtendedDynamicState3PropertiesEXT {
public:
    MarshalVkPhysicalDeviceExtendedDynamicState3PropertiesEXT() {}
    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT s;
    MarshalVkPhysicalDeviceExtendedDynamicState3PropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicState3PropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicState3PropertiesEXT* s);
    ~MarshalVkPhysicalDeviceExtendedDynamicState3PropertiesEXT();
};

class MarshalVkColorBlendEquationEXT {
public:
    MarshalVkColorBlendEquationEXT() {}
    VkColorBlendEquationEXT s;
    MarshalVkColorBlendEquationEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkColorBlendEquationEXT* s);
};

class MarshalVkColorBlendAdvancedEXT {
public:
    MarshalVkColorBlendAdvancedEXT() {}
    VkColorBlendAdvancedEXT s;
    MarshalVkColorBlendAdvancedEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkColorBlendAdvancedEXT* s);
};

class MarshalVkRenderPassTransformBeginInfoQCOM {
public:
    MarshalVkRenderPassTransformBeginInfoQCOM() {}
    VkRenderPassTransformBeginInfoQCOM s;
    MarshalVkRenderPassTransformBeginInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassTransformBeginInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassTransformBeginInfoQCOM* s);
    ~MarshalVkRenderPassTransformBeginInfoQCOM();
};

class MarshalVkCopyCommandTransformInfoQCOM {
public:
    MarshalVkCopyCommandTransformInfoQCOM() {}
    VkCopyCommandTransformInfoQCOM s;
    MarshalVkCopyCommandTransformInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyCommandTransformInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyCommandTransformInfoQCOM* s);
    ~MarshalVkCopyCommandTransformInfoQCOM();
};

class MarshalVkCommandBufferInheritanceRenderPassTransformInfoQCOM {
public:
    MarshalVkCommandBufferInheritanceRenderPassTransformInfoQCOM() {}
    VkCommandBufferInheritanceRenderPassTransformInfoQCOM s;
    MarshalVkCommandBufferInheritanceRenderPassTransformInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceRenderPassTransformInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceRenderPassTransformInfoQCOM* s);
    ~MarshalVkCommandBufferInheritanceRenderPassTransformInfoQCOM();
};

class MarshalVkPhysicalDeviceDiagnosticsConfigFeaturesNV {
public:
    MarshalVkPhysicalDeviceDiagnosticsConfigFeaturesNV() {}
    VkPhysicalDeviceDiagnosticsConfigFeaturesNV s;
    MarshalVkPhysicalDeviceDiagnosticsConfigFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDiagnosticsConfigFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDiagnosticsConfigFeaturesNV* s);
    ~MarshalVkPhysicalDeviceDiagnosticsConfigFeaturesNV();
};

class MarshalVkDeviceDiagnosticsConfigCreateInfoNV {
public:
    MarshalVkDeviceDiagnosticsConfigCreateInfoNV() {}
    VkDeviceDiagnosticsConfigCreateInfoNV s;
    MarshalVkDeviceDiagnosticsConfigCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceDiagnosticsConfigCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceDiagnosticsConfigCreateInfoNV* s);
    ~MarshalVkDeviceDiagnosticsConfigCreateInfoNV();
};

class MarshalVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures {
public:
    MarshalVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures() {}
    VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures s;
    MarshalVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures* s);
    ~MarshalVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures();
};

class MarshalVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR {
public:
    MarshalVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR() {}
    VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR s;
    MarshalVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR();
};

class MarshalVkPhysicalDeviceRobustness2FeaturesEXT {
public:
    MarshalVkPhysicalDeviceRobustness2FeaturesEXT() {}
    VkPhysicalDeviceRobustness2FeaturesEXT s;
    MarshalVkPhysicalDeviceRobustness2FeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRobustness2FeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRobustness2FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceRobustness2FeaturesEXT();
};

class MarshalVkPhysicalDeviceRobustness2PropertiesEXT {
public:
    MarshalVkPhysicalDeviceRobustness2PropertiesEXT() {}
    VkPhysicalDeviceRobustness2PropertiesEXT s;
    MarshalVkPhysicalDeviceRobustness2PropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRobustness2PropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRobustness2PropertiesEXT* s);
    ~MarshalVkPhysicalDeviceRobustness2PropertiesEXT();
};

class MarshalVkPhysicalDeviceImageRobustnessFeatures {
public:
    MarshalVkPhysicalDeviceImageRobustnessFeatures() {}
    VkPhysicalDeviceImageRobustnessFeatures s;
    MarshalVkPhysicalDeviceImageRobustnessFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageRobustnessFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageRobustnessFeatures* s);
    ~MarshalVkPhysicalDeviceImageRobustnessFeatures();
};

class MarshalVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR {
public:
    MarshalVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR() {}
    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR s;
    MarshalVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR();
};

class MarshalVkPhysicalDevice4444FormatsFeaturesEXT {
public:
    MarshalVkPhysicalDevice4444FormatsFeaturesEXT() {}
    VkPhysicalDevice4444FormatsFeaturesEXT s;
    MarshalVkPhysicalDevice4444FormatsFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevice4444FormatsFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevice4444FormatsFeaturesEXT* s);
    ~MarshalVkPhysicalDevice4444FormatsFeaturesEXT();
};

class MarshalVkPhysicalDeviceSubpassShadingFeaturesHUAWEI {
public:
    MarshalVkPhysicalDeviceSubpassShadingFeaturesHUAWEI() {}
    VkPhysicalDeviceSubpassShadingFeaturesHUAWEI s;
    MarshalVkPhysicalDeviceSubpassShadingFeaturesHUAWEI(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubpassShadingFeaturesHUAWEI* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubpassShadingFeaturesHUAWEI* s);
    ~MarshalVkPhysicalDeviceSubpassShadingFeaturesHUAWEI();
};

class MarshalVkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI {
public:
    MarshalVkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI() {}
    VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI s;
    MarshalVkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI* s);
    ~MarshalVkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI();
};

class MarshalVkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI {
public:
    MarshalVkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI() {}
    VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI s;
    MarshalVkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI* s);
    ~MarshalVkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI();
};

class MarshalVkBufferCopy2 {
public:
    MarshalVkBufferCopy2() {}
    VkBufferCopy2 s;
    MarshalVkBufferCopy2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferCopy2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferCopy2* s);
    ~MarshalVkBufferCopy2();
};

class MarshalVkImageCopy2 {
public:
    MarshalVkImageCopy2() {}
    VkImageCopy2 s;
    MarshalVkImageCopy2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCopy2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCopy2* s);
    ~MarshalVkImageCopy2();
};

class MarshalVkImageBlit2 {
public:
    MarshalVkImageBlit2() {}
    VkImageBlit2 s;
    MarshalVkImageBlit2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageBlit2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageBlit2* s);
    ~MarshalVkImageBlit2();
};

class MarshalVkBufferImageCopy2 {
public:
    MarshalVkBufferImageCopy2() {}
    VkBufferImageCopy2 s;
    MarshalVkBufferImageCopy2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferImageCopy2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferImageCopy2* s);
    ~MarshalVkBufferImageCopy2();
};

class MarshalVkImageResolve2 {
public:
    MarshalVkImageResolve2() {}
    VkImageResolve2 s;
    MarshalVkImageResolve2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageResolve2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageResolve2* s);
    ~MarshalVkImageResolve2();
};

class MarshalVkCopyBufferInfo2 {
public:
    MarshalVkCopyBufferInfo2() {}
    VkCopyBufferInfo2 s;
    MarshalVkCopyBufferInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyBufferInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyBufferInfo2* s);
    ~MarshalVkCopyBufferInfo2();
};

class MarshalVkCopyImageInfo2 {
public:
    MarshalVkCopyImageInfo2() {}
    VkCopyImageInfo2 s;
    MarshalVkCopyImageInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyImageInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyImageInfo2* s);
    ~MarshalVkCopyImageInfo2();
};

class MarshalVkBlitImageInfo2 {
public:
    MarshalVkBlitImageInfo2() {}
    VkBlitImageInfo2 s;
    MarshalVkBlitImageInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBlitImageInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBlitImageInfo2* s);
    ~MarshalVkBlitImageInfo2();
};

class MarshalVkCopyBufferToImageInfo2 {
public:
    MarshalVkCopyBufferToImageInfo2() {}
    VkCopyBufferToImageInfo2 s;
    MarshalVkCopyBufferToImageInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyBufferToImageInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyBufferToImageInfo2* s);
    ~MarshalVkCopyBufferToImageInfo2();
};

class MarshalVkCopyImageToBufferInfo2 {
public:
    MarshalVkCopyImageToBufferInfo2() {}
    VkCopyImageToBufferInfo2 s;
    MarshalVkCopyImageToBufferInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyImageToBufferInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyImageToBufferInfo2* s);
    ~MarshalVkCopyImageToBufferInfo2();
};

class MarshalVkResolveImageInfo2 {
public:
    MarshalVkResolveImageInfo2() {}
    VkResolveImageInfo2 s;
    MarshalVkResolveImageInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkResolveImageInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkResolveImageInfo2* s);
    ~MarshalVkResolveImageInfo2();
};

class MarshalVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT() {}
    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT s;
    MarshalVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT();
};

class MarshalVkFragmentShadingRateAttachmentInfoKHR {
public:
    MarshalVkFragmentShadingRateAttachmentInfoKHR() {}
    VkFragmentShadingRateAttachmentInfoKHR s;
    MarshalVkFragmentShadingRateAttachmentInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFragmentShadingRateAttachmentInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFragmentShadingRateAttachmentInfoKHR* s);
    ~MarshalVkFragmentShadingRateAttachmentInfoKHR();
};

class MarshalVkPipelineFragmentShadingRateStateCreateInfoKHR {
public:
    MarshalVkPipelineFragmentShadingRateStateCreateInfoKHR() {}
    VkPipelineFragmentShadingRateStateCreateInfoKHR s;
    MarshalVkPipelineFragmentShadingRateStateCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineFragmentShadingRateStateCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineFragmentShadingRateStateCreateInfoKHR* s);
    ~MarshalVkPipelineFragmentShadingRateStateCreateInfoKHR();
};

class MarshalVkPhysicalDeviceFragmentShadingRateFeaturesKHR {
public:
    MarshalVkPhysicalDeviceFragmentShadingRateFeaturesKHR() {}
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR s;
    MarshalVkPhysicalDeviceFragmentShadingRateFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRateFeaturesKHR();
};

class MarshalVkPhysicalDeviceFragmentShadingRatePropertiesKHR {
public:
    MarshalVkPhysicalDeviceFragmentShadingRatePropertiesKHR() {}
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR s;
    MarshalVkPhysicalDeviceFragmentShadingRatePropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRatePropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRatePropertiesKHR* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRatePropertiesKHR();
};

class MarshalVkPhysicalDeviceFragmentShadingRateKHR {
public:
    MarshalVkPhysicalDeviceFragmentShadingRateKHR() {}
    VkPhysicalDeviceFragmentShadingRateKHR s;
    MarshalVkPhysicalDeviceFragmentShadingRateKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateKHR* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRateKHR();
};

class MarshalVkPhysicalDeviceShaderTerminateInvocationFeatures {
public:
    MarshalVkPhysicalDeviceShaderTerminateInvocationFeatures() {}
    VkPhysicalDeviceShaderTerminateInvocationFeatures s;
    MarshalVkPhysicalDeviceShaderTerminateInvocationFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderTerminateInvocationFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderTerminateInvocationFeatures* s);
    ~MarshalVkPhysicalDeviceShaderTerminateInvocationFeatures();
};

class MarshalVkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV {
public:
    MarshalVkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV() {}
    VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV s;
    MarshalVkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV();
};

class MarshalVkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV {
public:
    MarshalVkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV() {}
    VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV s;
    MarshalVkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV();
};

class MarshalVkPipelineFragmentShadingRateEnumStateCreateInfoNV {
public:
    MarshalVkPipelineFragmentShadingRateEnumStateCreateInfoNV() {}
    VkPipelineFragmentShadingRateEnumStateCreateInfoNV s;
    MarshalVkPipelineFragmentShadingRateEnumStateCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineFragmentShadingRateEnumStateCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineFragmentShadingRateEnumStateCreateInfoNV* s);
    ~MarshalVkPipelineFragmentShadingRateEnumStateCreateInfoNV();
};

class MarshalVkAccelerationStructureBuildSizesInfoKHR {
public:
    MarshalVkAccelerationStructureBuildSizesInfoKHR() {}
    VkAccelerationStructureBuildSizesInfoKHR s;
    MarshalVkAccelerationStructureBuildSizesInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureBuildSizesInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureBuildSizesInfoKHR* s);
    ~MarshalVkAccelerationStructureBuildSizesInfoKHR();
};

class MarshalVkPhysicalDeviceImage2DViewOf3DFeaturesEXT {
public:
    MarshalVkPhysicalDeviceImage2DViewOf3DFeaturesEXT() {}
    VkPhysicalDeviceImage2DViewOf3DFeaturesEXT s;
    MarshalVkPhysicalDeviceImage2DViewOf3DFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImage2DViewOf3DFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImage2DViewOf3DFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceImage2DViewOf3DFeaturesEXT();
};

class MarshalVkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT {
public:
    MarshalVkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT() {}
    VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT s;
    MarshalVkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT();
};

class MarshalVkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT {
public:
    MarshalVkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT() {}
    VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT s;
    MarshalVkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT();
};

class MarshalVkPhysicalDeviceLegacyVertexAttributesFeaturesEXT {
public:
    MarshalVkPhysicalDeviceLegacyVertexAttributesFeaturesEXT() {}
    VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT s;
    MarshalVkPhysicalDeviceLegacyVertexAttributesFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceLegacyVertexAttributesFeaturesEXT();
};

class MarshalVkPhysicalDeviceLegacyVertexAttributesPropertiesEXT {
public:
    MarshalVkPhysicalDeviceLegacyVertexAttributesPropertiesEXT() {}
    VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT s;
    MarshalVkPhysicalDeviceLegacyVertexAttributesPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceLegacyVertexAttributesPropertiesEXT();
};

class MarshalVkPhysicalDeviceMutableDescriptorTypeFeaturesEXT {
public:
    MarshalVkPhysicalDeviceMutableDescriptorTypeFeaturesEXT() {}
    VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT s;
    MarshalVkPhysicalDeviceMutableDescriptorTypeFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceMutableDescriptorTypeFeaturesEXT();
};

class MarshalVkMutableDescriptorTypeListEXT {
public:
    MarshalVkMutableDescriptorTypeListEXT() {}
    VkMutableDescriptorTypeListEXT s;
    MarshalVkMutableDescriptorTypeListEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMutableDescriptorTypeListEXT* s);
    ~MarshalVkMutableDescriptorTypeListEXT();
};

class MarshalVkMutableDescriptorTypeCreateInfoEXT {
public:
    MarshalVkMutableDescriptorTypeCreateInfoEXT() {}
    VkMutableDescriptorTypeCreateInfoEXT s;
    MarshalVkMutableDescriptorTypeCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMutableDescriptorTypeCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMutableDescriptorTypeCreateInfoEXT* s);
    ~MarshalVkMutableDescriptorTypeCreateInfoEXT();
};

class MarshalVkPhysicalDeviceDepthClipControlFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDepthClipControlFeaturesEXT() {}
    VkPhysicalDeviceDepthClipControlFeaturesEXT s;
    MarshalVkPhysicalDeviceDepthClipControlFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthClipControlFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthClipControlFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDepthClipControlFeaturesEXT();
};

class MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT() {}
    VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT s;
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT();
};

class MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT {
public:
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT() {}
    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT s;
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT();
};

class MarshalVkGeneratedCommandsPipelineInfoEXT {
public:
    MarshalVkGeneratedCommandsPipelineInfoEXT() {}
    VkGeneratedCommandsPipelineInfoEXT s;
    MarshalVkGeneratedCommandsPipelineInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsPipelineInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsPipelineInfoEXT* s);
    ~MarshalVkGeneratedCommandsPipelineInfoEXT();
};

class MarshalVkGeneratedCommandsShaderInfoEXT {
public:
    MarshalVkGeneratedCommandsShaderInfoEXT() {}
    VkGeneratedCommandsShaderInfoEXT s;
    MarshalVkGeneratedCommandsShaderInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsShaderInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsShaderInfoEXT* s);
    ~MarshalVkGeneratedCommandsShaderInfoEXT();
};

class MarshalVkGeneratedCommandsMemoryRequirementsInfoEXT {
public:
    MarshalVkGeneratedCommandsMemoryRequirementsInfoEXT() {}
    VkGeneratedCommandsMemoryRequirementsInfoEXT s;
    MarshalVkGeneratedCommandsMemoryRequirementsInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsMemoryRequirementsInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsMemoryRequirementsInfoEXT* s);
    ~MarshalVkGeneratedCommandsMemoryRequirementsInfoEXT();
};

class MarshalVkIndirectExecutionSetPipelineInfoEXT {
public:
    MarshalVkIndirectExecutionSetPipelineInfoEXT() {}
    VkIndirectExecutionSetPipelineInfoEXT s;
    MarshalVkIndirectExecutionSetPipelineInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectExecutionSetPipelineInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectExecutionSetPipelineInfoEXT* s);
    ~MarshalVkIndirectExecutionSetPipelineInfoEXT();
};

class MarshalVkIndirectExecutionSetShaderLayoutInfoEXT {
public:
    MarshalVkIndirectExecutionSetShaderLayoutInfoEXT() {}
    VkIndirectExecutionSetShaderLayoutInfoEXT s;
    MarshalVkIndirectExecutionSetShaderLayoutInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectExecutionSetShaderLayoutInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectExecutionSetShaderLayoutInfoEXT* s);
    ~MarshalVkIndirectExecutionSetShaderLayoutInfoEXT();
};

class MarshalVkIndirectExecutionSetShaderInfoEXT {
public:
    MarshalVkIndirectExecutionSetShaderInfoEXT() {}
    VkIndirectExecutionSetShaderInfoEXT s;
    MarshalVkIndirectExecutionSetShaderInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectExecutionSetShaderInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectExecutionSetShaderInfoEXT* s);
    ~MarshalVkIndirectExecutionSetShaderInfoEXT();
};

class MarshalVkIndirectExecutionSetCreateInfoEXT {
public:
    MarshalVkIndirectExecutionSetCreateInfoEXT() {}
    VkIndirectExecutionSetCreateInfoEXT s;
    MarshalVkIndirectExecutionSetCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectExecutionSetCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectExecutionSetCreateInfoEXT* s);
    ~MarshalVkIndirectExecutionSetCreateInfoEXT();
};

class MarshalVkGeneratedCommandsInfoEXT {
public:
    MarshalVkGeneratedCommandsInfoEXT() {}
    VkGeneratedCommandsInfoEXT s;
    MarshalVkGeneratedCommandsInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGeneratedCommandsInfoEXT* s);
    ~MarshalVkGeneratedCommandsInfoEXT();
};

class MarshalVkWriteIndirectExecutionSetPipelineEXT {
public:
    MarshalVkWriteIndirectExecutionSetPipelineEXT() {}
    VkWriteIndirectExecutionSetPipelineEXT s;
    MarshalVkWriteIndirectExecutionSetPipelineEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteIndirectExecutionSetPipelineEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteIndirectExecutionSetPipelineEXT* s);
    ~MarshalVkWriteIndirectExecutionSetPipelineEXT();
};

class MarshalVkWriteIndirectExecutionSetShaderEXT {
public:
    MarshalVkWriteIndirectExecutionSetShaderEXT() {}
    VkWriteIndirectExecutionSetShaderEXT s;
    MarshalVkWriteIndirectExecutionSetShaderEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteIndirectExecutionSetShaderEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkWriteIndirectExecutionSetShaderEXT* s);
    ~MarshalVkWriteIndirectExecutionSetShaderEXT();
};

class MarshalVkIndirectCommandsLayoutCreateInfoEXT {
public:
    MarshalVkIndirectCommandsLayoutCreateInfoEXT() {}
    VkIndirectCommandsLayoutCreateInfoEXT s;
    MarshalVkIndirectCommandsLayoutCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectCommandsLayoutCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectCommandsLayoutCreateInfoEXT* s);
    ~MarshalVkIndirectCommandsLayoutCreateInfoEXT();
};

class MarshalVkIndirectCommandsLayoutTokenEXT {
public:
    MarshalVkIndirectCommandsLayoutTokenEXT() {}
    VkIndirectCommandsLayoutTokenEXT s;
    MarshalVkIndirectCommandsLayoutTokenEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectCommandsLayoutTokenEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkIndirectCommandsLayoutTokenEXT* s);
    ~MarshalVkIndirectCommandsLayoutTokenEXT();
};

class MarshalVkPipelineViewportDepthClipControlCreateInfoEXT {
public:
    MarshalVkPipelineViewportDepthClipControlCreateInfoEXT() {}
    VkPipelineViewportDepthClipControlCreateInfoEXT s;
    MarshalVkPipelineViewportDepthClipControlCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportDepthClipControlCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportDepthClipControlCreateInfoEXT* s);
    ~MarshalVkPipelineViewportDepthClipControlCreateInfoEXT();
};

class MarshalVkPhysicalDeviceDepthClampControlFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDepthClampControlFeaturesEXT() {}
    VkPhysicalDeviceDepthClampControlFeaturesEXT s;
    MarshalVkPhysicalDeviceDepthClampControlFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthClampControlFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthClampControlFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDepthClampControlFeaturesEXT();
};

class MarshalVkPipelineViewportDepthClampControlCreateInfoEXT {
public:
    MarshalVkPipelineViewportDepthClampControlCreateInfoEXT() {}
    VkPipelineViewportDepthClampControlCreateInfoEXT s;
    MarshalVkPipelineViewportDepthClampControlCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportDepthClampControlCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineViewportDepthClampControlCreateInfoEXT* s);
    ~MarshalVkPipelineViewportDepthClampControlCreateInfoEXT();
};

class MarshalVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT {
public:
    MarshalVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT() {}
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT s;
    MarshalVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT();
};

class MarshalVkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR {
public:
    MarshalVkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR() {}
    VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR s;
    MarshalVkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR();
};

class MarshalVkVertexInputBindingDescription2EXT {
public:
    MarshalVkVertexInputBindingDescription2EXT() {}
    VkVertexInputBindingDescription2EXT s;
    MarshalVkVertexInputBindingDescription2EXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVertexInputBindingDescription2EXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVertexInputBindingDescription2EXT* s);
    ~MarshalVkVertexInputBindingDescription2EXT();
};

class MarshalVkVertexInputAttributeDescription2EXT {
public:
    MarshalVkVertexInputAttributeDescription2EXT() {}
    VkVertexInputAttributeDescription2EXT s;
    MarshalVkVertexInputAttributeDescription2EXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVertexInputAttributeDescription2EXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVertexInputAttributeDescription2EXT* s);
    ~MarshalVkVertexInputAttributeDescription2EXT();
};

class MarshalVkPhysicalDeviceColorWriteEnableFeaturesEXT {
public:
    MarshalVkPhysicalDeviceColorWriteEnableFeaturesEXT() {}
    VkPhysicalDeviceColorWriteEnableFeaturesEXT s;
    MarshalVkPhysicalDeviceColorWriteEnableFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceColorWriteEnableFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceColorWriteEnableFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceColorWriteEnableFeaturesEXT();
};

class MarshalVkPipelineColorWriteCreateInfoEXT {
public:
    MarshalVkPipelineColorWriteCreateInfoEXT() {}
    VkPipelineColorWriteCreateInfoEXT s;
    MarshalVkPipelineColorWriteCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineColorWriteCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineColorWriteCreateInfoEXT* s);
    ~MarshalVkPipelineColorWriteCreateInfoEXT();
};

class MarshalVkMemoryBarrier2 {
public:
    MarshalVkMemoryBarrier2() {}
    VkMemoryBarrier2 s;
    MarshalVkMemoryBarrier2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryBarrier2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryBarrier2* s);
    ~MarshalVkMemoryBarrier2();
};

class MarshalVkImageMemoryBarrier2 {
public:
    MarshalVkImageMemoryBarrier2() {}
    VkImageMemoryBarrier2 s;
    MarshalVkImageMemoryBarrier2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageMemoryBarrier2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageMemoryBarrier2* s);
    ~MarshalVkImageMemoryBarrier2();
};

class MarshalVkBufferMemoryBarrier2 {
public:
    MarshalVkBufferMemoryBarrier2() {}
    VkBufferMemoryBarrier2 s;
    MarshalVkBufferMemoryBarrier2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferMemoryBarrier2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferMemoryBarrier2* s);
    ~MarshalVkBufferMemoryBarrier2();
};

class MarshalVkDependencyInfo {
public:
    MarshalVkDependencyInfo() {}
    VkDependencyInfo s;
    MarshalVkDependencyInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDependencyInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDependencyInfo* s);
    ~MarshalVkDependencyInfo();
};

class MarshalVkSemaphoreSubmitInfo {
public:
    MarshalVkSemaphoreSubmitInfo() {}
    VkSemaphoreSubmitInfo s;
    MarshalVkSemaphoreSubmitInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreSubmitInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSemaphoreSubmitInfo* s);
    ~MarshalVkSemaphoreSubmitInfo();
};

class MarshalVkCommandBufferSubmitInfo {
public:
    MarshalVkCommandBufferSubmitInfo() {}
    VkCommandBufferSubmitInfo s;
    MarshalVkCommandBufferSubmitInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferSubmitInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferSubmitInfo* s);
    ~MarshalVkCommandBufferSubmitInfo();
};

class MarshalVkSubmitInfo2 {
public:
    MarshalVkSubmitInfo2() {}
    VkSubmitInfo2 s;
    MarshalVkSubmitInfo2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubmitInfo2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubmitInfo2* s);
    ~MarshalVkSubmitInfo2();
};

class MarshalVkQueueFamilyCheckpointProperties2NV {
public:
    MarshalVkQueueFamilyCheckpointProperties2NV() {}
    VkQueueFamilyCheckpointProperties2NV s;
    MarshalVkQueueFamilyCheckpointProperties2NV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyCheckpointProperties2NV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyCheckpointProperties2NV* s);
    ~MarshalVkQueueFamilyCheckpointProperties2NV();
};

class MarshalVkCheckpointData2NV {
public:
    MarshalVkCheckpointData2NV() {}
    VkCheckpointData2NV s;
    MarshalVkCheckpointData2NV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCheckpointData2NV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCheckpointData2NV* s);
    ~MarshalVkCheckpointData2NV();
};

class MarshalVkPhysicalDeviceSynchronization2Features {
public:
    MarshalVkPhysicalDeviceSynchronization2Features() {}
    VkPhysicalDeviceSynchronization2Features s;
    MarshalVkPhysicalDeviceSynchronization2Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSynchronization2Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSynchronization2Features* s);
    ~MarshalVkPhysicalDeviceSynchronization2Features();
};

class MarshalVkPhysicalDeviceHostImageCopyFeatures {
public:
    MarshalVkPhysicalDeviceHostImageCopyFeatures() {}
    VkPhysicalDeviceHostImageCopyFeatures s;
    MarshalVkPhysicalDeviceHostImageCopyFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceHostImageCopyFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceHostImageCopyFeatures* s);
    ~MarshalVkPhysicalDeviceHostImageCopyFeatures();
};

class MarshalVkPhysicalDeviceHostImageCopyProperties {
public:
    MarshalVkPhysicalDeviceHostImageCopyProperties() {}
    VkPhysicalDeviceHostImageCopyProperties s;
    MarshalVkPhysicalDeviceHostImageCopyProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceHostImageCopyProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceHostImageCopyProperties* s);
    ~MarshalVkPhysicalDeviceHostImageCopyProperties();
};

class MarshalVkMemoryToImageCopy {
public:
    MarshalVkMemoryToImageCopy() {}
    VkMemoryToImageCopy s;
    MarshalVkMemoryToImageCopy(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryToImageCopy* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryToImageCopy* s);
};

class MarshalVkImageToMemoryCopy {
public:
    MarshalVkImageToMemoryCopy() {}
    VkImageToMemoryCopy s;
    MarshalVkImageToMemoryCopy(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageToMemoryCopy* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageToMemoryCopy* s);
};

class MarshalVkCopyMemoryToImageInfo {
public:
    MarshalVkCopyMemoryToImageInfo() {}
    VkCopyMemoryToImageInfo s;
    MarshalVkCopyMemoryToImageInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMemoryToImageInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMemoryToImageInfo* s);
    ~MarshalVkCopyMemoryToImageInfo();
};

class MarshalVkCopyImageToMemoryInfo {
public:
    MarshalVkCopyImageToMemoryInfo() {}
    VkCopyImageToMemoryInfo s;
    MarshalVkCopyImageToMemoryInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyImageToMemoryInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyImageToMemoryInfo* s);
    ~MarshalVkCopyImageToMemoryInfo();
};

class MarshalVkCopyImageToImageInfo {
public:
    MarshalVkCopyImageToImageInfo() {}
    VkCopyImageToImageInfo s;
    MarshalVkCopyImageToImageInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyImageToImageInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyImageToImageInfo* s);
    ~MarshalVkCopyImageToImageInfo();
};

class MarshalVkHostImageLayoutTransitionInfo {
public:
    MarshalVkHostImageLayoutTransitionInfo() {}
    VkHostImageLayoutTransitionInfo s;
    MarshalVkHostImageLayoutTransitionInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkHostImageLayoutTransitionInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkHostImageLayoutTransitionInfo* s);
    ~MarshalVkHostImageLayoutTransitionInfo();
};

class MarshalVkSubresourceHostMemcpySize {
public:
    MarshalVkSubresourceHostMemcpySize() {}
    VkSubresourceHostMemcpySize s;
    MarshalVkSubresourceHostMemcpySize(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubresourceHostMemcpySize* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubresourceHostMemcpySize* s);
    ~MarshalVkSubresourceHostMemcpySize();
};

class MarshalVkHostImageCopyDevicePerformanceQuery {
public:
    MarshalVkHostImageCopyDevicePerformanceQuery() {}
    VkHostImageCopyDevicePerformanceQuery s;
    MarshalVkHostImageCopyDevicePerformanceQuery(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkHostImageCopyDevicePerformanceQuery* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkHostImageCopyDevicePerformanceQuery* s);
    ~MarshalVkHostImageCopyDevicePerformanceQuery();
};

class MarshalVkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT {
public:
    MarshalVkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT() {}
    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT s;
    MarshalVkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT* s);
    ~MarshalVkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT();
};

class MarshalVkPhysicalDeviceLegacyDitheringFeaturesEXT {
public:
    MarshalVkPhysicalDeviceLegacyDitheringFeaturesEXT() {}
    VkPhysicalDeviceLegacyDitheringFeaturesEXT s;
    MarshalVkPhysicalDeviceLegacyDitheringFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLegacyDitheringFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLegacyDitheringFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceLegacyDitheringFeaturesEXT();
};

class MarshalVkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT {
public:
    MarshalVkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT() {}
    VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT s;
    MarshalVkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT();
};

class MarshalVkSubpassResolvePerformanceQueryEXT {
public:
    MarshalVkSubpassResolvePerformanceQueryEXT() {}
    VkSubpassResolvePerformanceQueryEXT s;
    MarshalVkSubpassResolvePerformanceQueryEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassResolvePerformanceQueryEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubpassResolvePerformanceQueryEXT* s);
    ~MarshalVkSubpassResolvePerformanceQueryEXT();
};

class MarshalVkMultisampledRenderToSingleSampledInfoEXT {
public:
    MarshalVkMultisampledRenderToSingleSampledInfoEXT() {}
    VkMultisampledRenderToSingleSampledInfoEXT s;
    MarshalVkMultisampledRenderToSingleSampledInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultisampledRenderToSingleSampledInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultisampledRenderToSingleSampledInfoEXT* s);
    ~MarshalVkMultisampledRenderToSingleSampledInfoEXT();
};

class MarshalVkPhysicalDevicePipelineProtectedAccessFeatures {
public:
    MarshalVkPhysicalDevicePipelineProtectedAccessFeatures() {}
    VkPhysicalDevicePipelineProtectedAccessFeatures s;
    MarshalVkPhysicalDevicePipelineProtectedAccessFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineProtectedAccessFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineProtectedAccessFeatures* s);
    ~MarshalVkPhysicalDevicePipelineProtectedAccessFeatures();
};

class MarshalVkQueueFamilyVideoPropertiesKHR {
public:
    MarshalVkQueueFamilyVideoPropertiesKHR() {}
    VkQueueFamilyVideoPropertiesKHR s;
    MarshalVkQueueFamilyVideoPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyVideoPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyVideoPropertiesKHR* s);
    ~MarshalVkQueueFamilyVideoPropertiesKHR();
};

class MarshalVkQueueFamilyQueryResultStatusPropertiesKHR {
public:
    MarshalVkQueueFamilyQueryResultStatusPropertiesKHR() {}
    VkQueueFamilyQueryResultStatusPropertiesKHR s;
    MarshalVkQueueFamilyQueryResultStatusPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyQueryResultStatusPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueueFamilyQueryResultStatusPropertiesKHR* s);
    ~MarshalVkQueueFamilyQueryResultStatusPropertiesKHR();
};

class MarshalVkVideoProfileListInfoKHR {
public:
    MarshalVkVideoProfileListInfoKHR() {}
    VkVideoProfileListInfoKHR s;
    MarshalVkVideoProfileListInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoProfileListInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoProfileListInfoKHR* s);
    ~MarshalVkVideoProfileListInfoKHR();
};

class MarshalVkPhysicalDeviceVideoFormatInfoKHR {
public:
    MarshalVkPhysicalDeviceVideoFormatInfoKHR() {}
    VkPhysicalDeviceVideoFormatInfoKHR s;
    MarshalVkPhysicalDeviceVideoFormatInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoFormatInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoFormatInfoKHR* s);
    ~MarshalVkPhysicalDeviceVideoFormatInfoKHR();
};

class MarshalVkVideoFormatPropertiesKHR {
public:
    MarshalVkVideoFormatPropertiesKHR() {}
    VkVideoFormatPropertiesKHR s;
    MarshalVkVideoFormatPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoFormatPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoFormatPropertiesKHR* s);
    ~MarshalVkVideoFormatPropertiesKHR();
};

class MarshalVkVideoEncodeQuantizationMapCapabilitiesKHR {
public:
    MarshalVkVideoEncodeQuantizationMapCapabilitiesKHR() {}
    VkVideoEncodeQuantizationMapCapabilitiesKHR s;
    MarshalVkVideoEncodeQuantizationMapCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQuantizationMapCapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQuantizationMapCapabilitiesKHR* s);
    ~MarshalVkVideoEncodeQuantizationMapCapabilitiesKHR();
};

class MarshalVkVideoEncodeH264QuantizationMapCapabilitiesKHR {
public:
    MarshalVkVideoEncodeH264QuantizationMapCapabilitiesKHR() {}
    VkVideoEncodeH264QuantizationMapCapabilitiesKHR s;
    MarshalVkVideoEncodeH264QuantizationMapCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264QuantizationMapCapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264QuantizationMapCapabilitiesKHR* s);
    ~MarshalVkVideoEncodeH264QuantizationMapCapabilitiesKHR();
};

class MarshalVkVideoEncodeH265QuantizationMapCapabilitiesKHR {
public:
    MarshalVkVideoEncodeH265QuantizationMapCapabilitiesKHR() {}
    VkVideoEncodeH265QuantizationMapCapabilitiesKHR s;
    MarshalVkVideoEncodeH265QuantizationMapCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265QuantizationMapCapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265QuantizationMapCapabilitiesKHR* s);
    ~MarshalVkVideoEncodeH265QuantizationMapCapabilitiesKHR();
};

class MarshalVkVideoEncodeAV1QuantizationMapCapabilitiesKHR {
public:
    MarshalVkVideoEncodeAV1QuantizationMapCapabilitiesKHR() {}
    VkVideoEncodeAV1QuantizationMapCapabilitiesKHR s;
    MarshalVkVideoEncodeAV1QuantizationMapCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1QuantizationMapCapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1QuantizationMapCapabilitiesKHR* s);
    ~MarshalVkVideoEncodeAV1QuantizationMapCapabilitiesKHR();
};

class MarshalVkVideoFormatQuantizationMapPropertiesKHR {
public:
    MarshalVkVideoFormatQuantizationMapPropertiesKHR() {}
    VkVideoFormatQuantizationMapPropertiesKHR s;
    MarshalVkVideoFormatQuantizationMapPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoFormatQuantizationMapPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoFormatQuantizationMapPropertiesKHR* s);
    ~MarshalVkVideoFormatQuantizationMapPropertiesKHR();
};

class MarshalVkVideoFormatH265QuantizationMapPropertiesKHR {
public:
    MarshalVkVideoFormatH265QuantizationMapPropertiesKHR() {}
    VkVideoFormatH265QuantizationMapPropertiesKHR s;
    MarshalVkVideoFormatH265QuantizationMapPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoFormatH265QuantizationMapPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoFormatH265QuantizationMapPropertiesKHR* s);
    ~MarshalVkVideoFormatH265QuantizationMapPropertiesKHR();
};

class MarshalVkVideoFormatAV1QuantizationMapPropertiesKHR {
public:
    MarshalVkVideoFormatAV1QuantizationMapPropertiesKHR() {}
    VkVideoFormatAV1QuantizationMapPropertiesKHR s;
    MarshalVkVideoFormatAV1QuantizationMapPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoFormatAV1QuantizationMapPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoFormatAV1QuantizationMapPropertiesKHR* s);
    ~MarshalVkVideoFormatAV1QuantizationMapPropertiesKHR();
};

class MarshalVkVideoProfileInfoKHR {
public:
    MarshalVkVideoProfileInfoKHR() {}
    VkVideoProfileInfoKHR s;
    MarshalVkVideoProfileInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoProfileInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoProfileInfoKHR* s);
    ~MarshalVkVideoProfileInfoKHR();
};

class MarshalVkVideoCapabilitiesKHR {
public:
    MarshalVkVideoCapabilitiesKHR() {}
    VkVideoCapabilitiesKHR s;
    MarshalVkVideoCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoCapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoCapabilitiesKHR* s);
    ~MarshalVkVideoCapabilitiesKHR();
};

class MarshalVkVideoSessionMemoryRequirementsKHR {
public:
    MarshalVkVideoSessionMemoryRequirementsKHR() {}
    VkVideoSessionMemoryRequirementsKHR s;
    MarshalVkVideoSessionMemoryRequirementsKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoSessionMemoryRequirementsKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoSessionMemoryRequirementsKHR* s);
    ~MarshalVkVideoSessionMemoryRequirementsKHR();
};

class MarshalVkBindVideoSessionMemoryInfoKHR {
public:
    MarshalVkBindVideoSessionMemoryInfoKHR() {}
    VkBindVideoSessionMemoryInfoKHR s;
    MarshalVkBindVideoSessionMemoryInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindVideoSessionMemoryInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindVideoSessionMemoryInfoKHR* s);
    ~MarshalVkBindVideoSessionMemoryInfoKHR();
};

class MarshalVkVideoPictureResourceInfoKHR {
public:
    MarshalVkVideoPictureResourceInfoKHR() {}
    VkVideoPictureResourceInfoKHR s;
    MarshalVkVideoPictureResourceInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoPictureResourceInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoPictureResourceInfoKHR* s);
    ~MarshalVkVideoPictureResourceInfoKHR();
};

class MarshalVkVideoReferenceSlotInfoKHR {
public:
    MarshalVkVideoReferenceSlotInfoKHR() {}
    VkVideoReferenceSlotInfoKHR s;
    MarshalVkVideoReferenceSlotInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoReferenceSlotInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoReferenceSlotInfoKHR* s);
    ~MarshalVkVideoReferenceSlotInfoKHR();
};

class MarshalVkVideoDecodeCapabilitiesKHR {
public:
    MarshalVkVideoDecodeCapabilitiesKHR() {}
    VkVideoDecodeCapabilitiesKHR s;
    MarshalVkVideoDecodeCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeCapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeCapabilitiesKHR* s);
    ~MarshalVkVideoDecodeCapabilitiesKHR();
};

class MarshalVkVideoDecodeUsageInfoKHR {
public:
    MarshalVkVideoDecodeUsageInfoKHR() {}
    VkVideoDecodeUsageInfoKHR s;
    MarshalVkVideoDecodeUsageInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeUsageInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeUsageInfoKHR* s);
    ~MarshalVkVideoDecodeUsageInfoKHR();
};

class MarshalVkVideoDecodeInfoKHR {
public:
    MarshalVkVideoDecodeInfoKHR() {}
    VkVideoDecodeInfoKHR s;
    MarshalVkVideoDecodeInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeInfoKHR* s);
    ~MarshalVkVideoDecodeInfoKHR();
};

class MarshalVkPhysicalDeviceVideoMaintenance1FeaturesKHR {
public:
    MarshalVkPhysicalDeviceVideoMaintenance1FeaturesKHR() {}
    VkPhysicalDeviceVideoMaintenance1FeaturesKHR s;
    MarshalVkPhysicalDeviceVideoMaintenance1FeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoMaintenance1FeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoMaintenance1FeaturesKHR* s);
    ~MarshalVkPhysicalDeviceVideoMaintenance1FeaturesKHR();
};

class MarshalVkVideoInlineQueryInfoKHR {
public:
    MarshalVkVideoInlineQueryInfoKHR() {}
    VkVideoInlineQueryInfoKHR s;
    MarshalVkVideoInlineQueryInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoInlineQueryInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoInlineQueryInfoKHR* s);
    ~MarshalVkVideoInlineQueryInfoKHR();
};

class MarshalStdVideoDecodeH264PictureInfo {
public:
    MarshalStdVideoDecodeH264PictureInfo() {}
    StdVideoDecodeH264PictureInfo s;
    MarshalStdVideoDecodeH264PictureInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoDecodeH264PictureInfo* s);
};

class MarshalStdVideoDecodeH264ReferenceInfo {
public:
    MarshalStdVideoDecodeH264ReferenceInfo() {}
    StdVideoDecodeH264ReferenceInfo s;
    MarshalStdVideoDecodeH264ReferenceInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoDecodeH264ReferenceInfo* s);
};

class MarshalVkVideoDecodeH264ProfileInfoKHR {
public:
    MarshalVkVideoDecodeH264ProfileInfoKHR() {}
    VkVideoDecodeH264ProfileInfoKHR s;
    MarshalVkVideoDecodeH264ProfileInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264ProfileInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264ProfileInfoKHR* s);
    ~MarshalVkVideoDecodeH264ProfileInfoKHR();
};

class MarshalVkVideoDecodeH264CapabilitiesKHR {
public:
    MarshalVkVideoDecodeH264CapabilitiesKHR() {}
    VkVideoDecodeH264CapabilitiesKHR s;
    MarshalVkVideoDecodeH264CapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264CapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264CapabilitiesKHR* s);
    ~MarshalVkVideoDecodeH264CapabilitiesKHR();
};

class MarshalStdVideoH264SequenceParameterSet {
public:
    MarshalStdVideoH264SequenceParameterSet() {}
    StdVideoH264SequenceParameterSet s;
    MarshalStdVideoH264SequenceParameterSet(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH264SequenceParameterSet* s);
};

class MarshalStdVideoH264PictureParameterSet {
public:
    MarshalStdVideoH264PictureParameterSet() {}
    StdVideoH264PictureParameterSet s;
    MarshalStdVideoH264PictureParameterSet(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH264PictureParameterSet* s);
    ~MarshalStdVideoH264PictureParameterSet();
};

class MarshalVkVideoDecodeH264SessionParametersAddInfoKHR {
public:
    MarshalVkVideoDecodeH264SessionParametersAddInfoKHR() {}
    VkVideoDecodeH264SessionParametersAddInfoKHR s;
    MarshalVkVideoDecodeH264SessionParametersAddInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264SessionParametersAddInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264SessionParametersAddInfoKHR* s);
    ~MarshalVkVideoDecodeH264SessionParametersAddInfoKHR();
};

class MarshalVkVideoDecodeH264SessionParametersCreateInfoKHR {
public:
    MarshalVkVideoDecodeH264SessionParametersCreateInfoKHR() {}
    VkVideoDecodeH264SessionParametersCreateInfoKHR s;
    MarshalVkVideoDecodeH264SessionParametersCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264SessionParametersCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264SessionParametersCreateInfoKHR* s);
    ~MarshalVkVideoDecodeH264SessionParametersCreateInfoKHR();
};

class MarshalVkVideoDecodeH264PictureInfoKHR {
public:
    MarshalVkVideoDecodeH264PictureInfoKHR() {}
    VkVideoDecodeH264PictureInfoKHR s;
    MarshalVkVideoDecodeH264PictureInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264PictureInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264PictureInfoKHR* s);
    ~MarshalVkVideoDecodeH264PictureInfoKHR();
};

class MarshalVkVideoDecodeH264DpbSlotInfoKHR {
public:
    MarshalVkVideoDecodeH264DpbSlotInfoKHR() {}
    VkVideoDecodeH264DpbSlotInfoKHR s;
    MarshalVkVideoDecodeH264DpbSlotInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264DpbSlotInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH264DpbSlotInfoKHR* s);
    ~MarshalVkVideoDecodeH264DpbSlotInfoKHR();
};

class MarshalStdVideoH265VideoParameterSet {
public:
    MarshalStdVideoH265VideoParameterSet() {}
    StdVideoH265VideoParameterSet s;
    MarshalStdVideoH265VideoParameterSet(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265VideoParameterSet* s);
    ~MarshalStdVideoH265VideoParameterSet();
};

class MarshalStdVideoH265SequenceParameterSet {
public:
    MarshalStdVideoH265SequenceParameterSet() {}
    StdVideoH265SequenceParameterSet s;
    MarshalStdVideoH265SequenceParameterSet(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265SequenceParameterSet* s);
    ~MarshalStdVideoH265SequenceParameterSet();
};

class MarshalStdVideoH265PictureParameterSet {
public:
    MarshalStdVideoH265PictureParameterSet() {}
    StdVideoH265PictureParameterSet s;
    MarshalStdVideoH265PictureParameterSet(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265PictureParameterSet* s);
    ~MarshalStdVideoH265PictureParameterSet();
};

class MarshalStdVideoDecodeH265PictureInfo {
public:
    MarshalStdVideoDecodeH265PictureInfo() {}
    StdVideoDecodeH265PictureInfo s;
    MarshalStdVideoDecodeH265PictureInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoDecodeH265PictureInfo* s);
};

class MarshalStdVideoDecodeH265ReferenceInfo {
public:
    MarshalStdVideoDecodeH265ReferenceInfo() {}
    StdVideoDecodeH265ReferenceInfo s;
    MarshalStdVideoDecodeH265ReferenceInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoDecodeH265ReferenceInfo* s);
};

class MarshalVkVideoDecodeH265ProfileInfoKHR {
public:
    MarshalVkVideoDecodeH265ProfileInfoKHR() {}
    VkVideoDecodeH265ProfileInfoKHR s;
    MarshalVkVideoDecodeH265ProfileInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265ProfileInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265ProfileInfoKHR* s);
    ~MarshalVkVideoDecodeH265ProfileInfoKHR();
};

class MarshalVkVideoDecodeH265CapabilitiesKHR {
public:
    MarshalVkVideoDecodeH265CapabilitiesKHR() {}
    VkVideoDecodeH265CapabilitiesKHR s;
    MarshalVkVideoDecodeH265CapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265CapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265CapabilitiesKHR* s);
    ~MarshalVkVideoDecodeH265CapabilitiesKHR();
};

class MarshalVkVideoDecodeH265SessionParametersAddInfoKHR {
public:
    MarshalVkVideoDecodeH265SessionParametersAddInfoKHR() {}
    VkVideoDecodeH265SessionParametersAddInfoKHR s;
    MarshalVkVideoDecodeH265SessionParametersAddInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265SessionParametersAddInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265SessionParametersAddInfoKHR* s);
    ~MarshalVkVideoDecodeH265SessionParametersAddInfoKHR();
};

class MarshalVkVideoDecodeH265SessionParametersCreateInfoKHR {
public:
    MarshalVkVideoDecodeH265SessionParametersCreateInfoKHR() {}
    VkVideoDecodeH265SessionParametersCreateInfoKHR s;
    MarshalVkVideoDecodeH265SessionParametersCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265SessionParametersCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265SessionParametersCreateInfoKHR* s);
    ~MarshalVkVideoDecodeH265SessionParametersCreateInfoKHR();
};

class MarshalVkVideoDecodeH265PictureInfoKHR {
public:
    MarshalVkVideoDecodeH265PictureInfoKHR() {}
    VkVideoDecodeH265PictureInfoKHR s;
    MarshalVkVideoDecodeH265PictureInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265PictureInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265PictureInfoKHR* s);
    ~MarshalVkVideoDecodeH265PictureInfoKHR();
};

class MarshalVkVideoDecodeH265DpbSlotInfoKHR {
public:
    MarshalVkVideoDecodeH265DpbSlotInfoKHR() {}
    VkVideoDecodeH265DpbSlotInfoKHR s;
    MarshalVkVideoDecodeH265DpbSlotInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265DpbSlotInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeH265DpbSlotInfoKHR* s);
    ~MarshalVkVideoDecodeH265DpbSlotInfoKHR();
};

class MarshalStdVideoAV1SequenceHeader {
public:
    MarshalStdVideoAV1SequenceHeader() {}
    StdVideoAV1SequenceHeader s;
    MarshalStdVideoAV1SequenceHeader(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1SequenceHeader* s);
    ~MarshalStdVideoAV1SequenceHeader();
};

class MarshalStdVideoDecodeAV1PictureInfo {
public:
    MarshalStdVideoDecodeAV1PictureInfo() {}
    StdVideoDecodeAV1PictureInfo s;
    MarshalStdVideoDecodeAV1PictureInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoDecodeAV1PictureInfo* s);
    ~MarshalStdVideoDecodeAV1PictureInfo();
};

class MarshalStdVideoDecodeAV1ReferenceInfo {
public:
    MarshalStdVideoDecodeAV1ReferenceInfo() {}
    StdVideoDecodeAV1ReferenceInfo s;
    MarshalStdVideoDecodeAV1ReferenceInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoDecodeAV1ReferenceInfo* s);
};

class MarshalVkVideoDecodeAV1ProfileInfoKHR {
public:
    MarshalVkVideoDecodeAV1ProfileInfoKHR() {}
    VkVideoDecodeAV1ProfileInfoKHR s;
    MarshalVkVideoDecodeAV1ProfileInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1ProfileInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1ProfileInfoKHR* s);
    ~MarshalVkVideoDecodeAV1ProfileInfoKHR();
};

class MarshalVkVideoDecodeAV1CapabilitiesKHR {
public:
    MarshalVkVideoDecodeAV1CapabilitiesKHR() {}
    VkVideoDecodeAV1CapabilitiesKHR s;
    MarshalVkVideoDecodeAV1CapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1CapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1CapabilitiesKHR* s);
    ~MarshalVkVideoDecodeAV1CapabilitiesKHR();
};

class MarshalVkVideoDecodeAV1SessionParametersCreateInfoKHR {
public:
    MarshalVkVideoDecodeAV1SessionParametersCreateInfoKHR() {}
    VkVideoDecodeAV1SessionParametersCreateInfoKHR s;
    MarshalVkVideoDecodeAV1SessionParametersCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1SessionParametersCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1SessionParametersCreateInfoKHR* s);
    ~MarshalVkVideoDecodeAV1SessionParametersCreateInfoKHR();
};

class MarshalVkVideoDecodeAV1PictureInfoKHR {
public:
    MarshalVkVideoDecodeAV1PictureInfoKHR() {}
    VkVideoDecodeAV1PictureInfoKHR s;
    MarshalVkVideoDecodeAV1PictureInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1PictureInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1PictureInfoKHR* s);
    ~MarshalVkVideoDecodeAV1PictureInfoKHR();
};

class MarshalVkVideoDecodeAV1DpbSlotInfoKHR {
public:
    MarshalVkVideoDecodeAV1DpbSlotInfoKHR() {}
    VkVideoDecodeAV1DpbSlotInfoKHR s;
    MarshalVkVideoDecodeAV1DpbSlotInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1DpbSlotInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoDecodeAV1DpbSlotInfoKHR* s);
    ~MarshalVkVideoDecodeAV1DpbSlotInfoKHR();
};

class MarshalVkVideoSessionCreateInfoKHR {
public:
    MarshalVkVideoSessionCreateInfoKHR() {}
    VkVideoSessionCreateInfoKHR s;
    MarshalVkVideoSessionCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoSessionCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoSessionCreateInfoKHR* s);
    ~MarshalVkVideoSessionCreateInfoKHR();
};

class MarshalVkVideoSessionParametersCreateInfoKHR {
public:
    MarshalVkVideoSessionParametersCreateInfoKHR() {}
    VkVideoSessionParametersCreateInfoKHR s;
    MarshalVkVideoSessionParametersCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoSessionParametersCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoSessionParametersCreateInfoKHR* s);
    ~MarshalVkVideoSessionParametersCreateInfoKHR();
};

class MarshalVkVideoSessionParametersUpdateInfoKHR {
public:
    MarshalVkVideoSessionParametersUpdateInfoKHR() {}
    VkVideoSessionParametersUpdateInfoKHR s;
    MarshalVkVideoSessionParametersUpdateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoSessionParametersUpdateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoSessionParametersUpdateInfoKHR* s);
    ~MarshalVkVideoSessionParametersUpdateInfoKHR();
};

class MarshalVkVideoEncodeSessionParametersGetInfoKHR {
public:
    MarshalVkVideoEncodeSessionParametersGetInfoKHR() {}
    VkVideoEncodeSessionParametersGetInfoKHR s;
    MarshalVkVideoEncodeSessionParametersGetInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeSessionParametersGetInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeSessionParametersGetInfoKHR* s);
    ~MarshalVkVideoEncodeSessionParametersGetInfoKHR();
};

class MarshalVkVideoEncodeSessionParametersFeedbackInfoKHR {
public:
    MarshalVkVideoEncodeSessionParametersFeedbackInfoKHR() {}
    VkVideoEncodeSessionParametersFeedbackInfoKHR s;
    MarshalVkVideoEncodeSessionParametersFeedbackInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeSessionParametersFeedbackInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeSessionParametersFeedbackInfoKHR* s);
    ~MarshalVkVideoEncodeSessionParametersFeedbackInfoKHR();
};

class MarshalVkVideoBeginCodingInfoKHR {
public:
    MarshalVkVideoBeginCodingInfoKHR() {}
    VkVideoBeginCodingInfoKHR s;
    MarshalVkVideoBeginCodingInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoBeginCodingInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoBeginCodingInfoKHR* s);
    ~MarshalVkVideoBeginCodingInfoKHR();
};

class MarshalVkVideoEndCodingInfoKHR {
public:
    MarshalVkVideoEndCodingInfoKHR() {}
    VkVideoEndCodingInfoKHR s;
    MarshalVkVideoEndCodingInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEndCodingInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEndCodingInfoKHR* s);
    ~MarshalVkVideoEndCodingInfoKHR();
};

class MarshalVkVideoCodingControlInfoKHR {
public:
    MarshalVkVideoCodingControlInfoKHR() {}
    VkVideoCodingControlInfoKHR s;
    MarshalVkVideoCodingControlInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoCodingControlInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoCodingControlInfoKHR* s);
    ~MarshalVkVideoCodingControlInfoKHR();
};

class MarshalVkVideoEncodeUsageInfoKHR {
public:
    MarshalVkVideoEncodeUsageInfoKHR() {}
    VkVideoEncodeUsageInfoKHR s;
    MarshalVkVideoEncodeUsageInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeUsageInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeUsageInfoKHR* s);
    ~MarshalVkVideoEncodeUsageInfoKHR();
};

class MarshalVkVideoEncodeInfoKHR {
public:
    MarshalVkVideoEncodeInfoKHR() {}
    VkVideoEncodeInfoKHR s;
    MarshalVkVideoEncodeInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeInfoKHR* s);
    ~MarshalVkVideoEncodeInfoKHR();
};

class MarshalVkVideoEncodeQuantizationMapInfoKHR {
public:
    MarshalVkVideoEncodeQuantizationMapInfoKHR() {}
    VkVideoEncodeQuantizationMapInfoKHR s;
    MarshalVkVideoEncodeQuantizationMapInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQuantizationMapInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQuantizationMapInfoKHR* s);
    ~MarshalVkVideoEncodeQuantizationMapInfoKHR();
};

class MarshalVkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR {
public:
    MarshalVkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR() {}
    VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR s;
    MarshalVkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR* s);
    ~MarshalVkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR();
};

class MarshalVkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR {
public:
    MarshalVkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR() {}
    VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR s;
    MarshalVkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR();
};

class MarshalVkQueryPoolVideoEncodeFeedbackCreateInfoKHR {
public:
    MarshalVkQueryPoolVideoEncodeFeedbackCreateInfoKHR() {}
    VkQueryPoolVideoEncodeFeedbackCreateInfoKHR s;
    MarshalVkQueryPoolVideoEncodeFeedbackCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryPoolVideoEncodeFeedbackCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryPoolVideoEncodeFeedbackCreateInfoKHR* s);
    ~MarshalVkQueryPoolVideoEncodeFeedbackCreateInfoKHR();
};

class MarshalVkVideoEncodeQualityLevelInfoKHR {
public:
    MarshalVkVideoEncodeQualityLevelInfoKHR() {}
    VkVideoEncodeQualityLevelInfoKHR s;
    MarshalVkVideoEncodeQualityLevelInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQualityLevelInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQualityLevelInfoKHR* s);
    ~MarshalVkVideoEncodeQualityLevelInfoKHR();
};

class MarshalVkPhysicalDeviceVideoEncodeQualityLevelInfoKHR {
public:
    MarshalVkPhysicalDeviceVideoEncodeQualityLevelInfoKHR() {}
    VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR s;
    MarshalVkPhysicalDeviceVideoEncodeQualityLevelInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* s);
    ~MarshalVkPhysicalDeviceVideoEncodeQualityLevelInfoKHR();
};

class MarshalVkVideoEncodeQualityLevelPropertiesKHR {
public:
    MarshalVkVideoEncodeQualityLevelPropertiesKHR() {}
    VkVideoEncodeQualityLevelPropertiesKHR s;
    MarshalVkVideoEncodeQualityLevelPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQualityLevelPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeQualityLevelPropertiesKHR* s);
    ~MarshalVkVideoEncodeQualityLevelPropertiesKHR();
};

class MarshalVkVideoEncodeRateControlInfoKHR {
public:
    MarshalVkVideoEncodeRateControlInfoKHR() {}
    VkVideoEncodeRateControlInfoKHR s;
    MarshalVkVideoEncodeRateControlInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeRateControlInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeRateControlInfoKHR* s);
    ~MarshalVkVideoEncodeRateControlInfoKHR();
};

class MarshalVkVideoEncodeRateControlLayerInfoKHR {
public:
    MarshalVkVideoEncodeRateControlLayerInfoKHR() {}
    VkVideoEncodeRateControlLayerInfoKHR s;
    MarshalVkVideoEncodeRateControlLayerInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeRateControlLayerInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeRateControlLayerInfoKHR* s);
    ~MarshalVkVideoEncodeRateControlLayerInfoKHR();
};

class MarshalVkVideoEncodeCapabilitiesKHR {
public:
    MarshalVkVideoEncodeCapabilitiesKHR() {}
    VkVideoEncodeCapabilitiesKHR s;
    MarshalVkVideoEncodeCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeCapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeCapabilitiesKHR* s);
    ~MarshalVkVideoEncodeCapabilitiesKHR();
};

class MarshalVkVideoEncodeH264CapabilitiesKHR {
public:
    MarshalVkVideoEncodeH264CapabilitiesKHR() {}
    VkVideoEncodeH264CapabilitiesKHR s;
    MarshalVkVideoEncodeH264CapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264CapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264CapabilitiesKHR* s);
    ~MarshalVkVideoEncodeH264CapabilitiesKHR();
};

class MarshalVkVideoEncodeH264QualityLevelPropertiesKHR {
public:
    MarshalVkVideoEncodeH264QualityLevelPropertiesKHR() {}
    VkVideoEncodeH264QualityLevelPropertiesKHR s;
    MarshalVkVideoEncodeH264QualityLevelPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264QualityLevelPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264QualityLevelPropertiesKHR* s);
    ~MarshalVkVideoEncodeH264QualityLevelPropertiesKHR();
};

class MarshalStdVideoEncodeH264SliceHeader {
public:
    MarshalStdVideoEncodeH264SliceHeader() {}
    StdVideoEncodeH264SliceHeader s;
    MarshalStdVideoEncodeH264SliceHeader(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH264SliceHeader* s);
    ~MarshalStdVideoEncodeH264SliceHeader();
};

class MarshalStdVideoEncodeH264PictureInfo {
public:
    MarshalStdVideoEncodeH264PictureInfo() {}
    StdVideoEncodeH264PictureInfo s;
    MarshalStdVideoEncodeH264PictureInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH264PictureInfo* s);
    ~MarshalStdVideoEncodeH264PictureInfo();
};

class MarshalStdVideoEncodeH264ReferenceInfo {
public:
    MarshalStdVideoEncodeH264ReferenceInfo() {}
    StdVideoEncodeH264ReferenceInfo s;
    MarshalStdVideoEncodeH264ReferenceInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH264ReferenceInfo* s);
};

class MarshalVkVideoEncodeH264SessionCreateInfoKHR {
public:
    MarshalVkVideoEncodeH264SessionCreateInfoKHR() {}
    VkVideoEncodeH264SessionCreateInfoKHR s;
    MarshalVkVideoEncodeH264SessionCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionCreateInfoKHR* s);
    ~MarshalVkVideoEncodeH264SessionCreateInfoKHR();
};

class MarshalVkVideoEncodeH264SessionParametersAddInfoKHR {
public:
    MarshalVkVideoEncodeH264SessionParametersAddInfoKHR() {}
    VkVideoEncodeH264SessionParametersAddInfoKHR s;
    MarshalVkVideoEncodeH264SessionParametersAddInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionParametersAddInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionParametersAddInfoKHR* s);
    ~MarshalVkVideoEncodeH264SessionParametersAddInfoKHR();
};

class MarshalVkVideoEncodeH264SessionParametersCreateInfoKHR {
public:
    MarshalVkVideoEncodeH264SessionParametersCreateInfoKHR() {}
    VkVideoEncodeH264SessionParametersCreateInfoKHR s;
    MarshalVkVideoEncodeH264SessionParametersCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionParametersCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionParametersCreateInfoKHR* s);
    ~MarshalVkVideoEncodeH264SessionParametersCreateInfoKHR();
};

class MarshalVkVideoEncodeH264SessionParametersGetInfoKHR {
public:
    MarshalVkVideoEncodeH264SessionParametersGetInfoKHR() {}
    VkVideoEncodeH264SessionParametersGetInfoKHR s;
    MarshalVkVideoEncodeH264SessionParametersGetInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionParametersGetInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionParametersGetInfoKHR* s);
    ~MarshalVkVideoEncodeH264SessionParametersGetInfoKHR();
};

class MarshalVkVideoEncodeH264SessionParametersFeedbackInfoKHR {
public:
    MarshalVkVideoEncodeH264SessionParametersFeedbackInfoKHR() {}
    VkVideoEncodeH264SessionParametersFeedbackInfoKHR s;
    MarshalVkVideoEncodeH264SessionParametersFeedbackInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionParametersFeedbackInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264SessionParametersFeedbackInfoKHR* s);
    ~MarshalVkVideoEncodeH264SessionParametersFeedbackInfoKHR();
};

class MarshalVkVideoEncodeH264DpbSlotInfoKHR {
public:
    MarshalVkVideoEncodeH264DpbSlotInfoKHR() {}
    VkVideoEncodeH264DpbSlotInfoKHR s;
    MarshalVkVideoEncodeH264DpbSlotInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264DpbSlotInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264DpbSlotInfoKHR* s);
    ~MarshalVkVideoEncodeH264DpbSlotInfoKHR();
};

class MarshalVkVideoEncodeH264PictureInfoKHR {
public:
    MarshalVkVideoEncodeH264PictureInfoKHR() {}
    VkVideoEncodeH264PictureInfoKHR s;
    MarshalVkVideoEncodeH264PictureInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264PictureInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264PictureInfoKHR* s);
    ~MarshalVkVideoEncodeH264PictureInfoKHR();
};

class MarshalVkVideoEncodeH264ProfileInfoKHR {
public:
    MarshalVkVideoEncodeH264ProfileInfoKHR() {}
    VkVideoEncodeH264ProfileInfoKHR s;
    MarshalVkVideoEncodeH264ProfileInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264ProfileInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264ProfileInfoKHR* s);
    ~MarshalVkVideoEncodeH264ProfileInfoKHR();
};

class MarshalVkVideoEncodeH264NaluSliceInfoKHR {
public:
    MarshalVkVideoEncodeH264NaluSliceInfoKHR() {}
    VkVideoEncodeH264NaluSliceInfoKHR s;
    MarshalVkVideoEncodeH264NaluSliceInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264NaluSliceInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264NaluSliceInfoKHR* s);
    ~MarshalVkVideoEncodeH264NaluSliceInfoKHR();
};

class MarshalVkVideoEncodeH264RateControlInfoKHR {
public:
    MarshalVkVideoEncodeH264RateControlInfoKHR() {}
    VkVideoEncodeH264RateControlInfoKHR s;
    MarshalVkVideoEncodeH264RateControlInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264RateControlInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264RateControlInfoKHR* s);
    ~MarshalVkVideoEncodeH264RateControlInfoKHR();
};

class MarshalVkVideoEncodeH264QpKHR {
public:
    MarshalVkVideoEncodeH264QpKHR() {}
    VkVideoEncodeH264QpKHR s;
    MarshalVkVideoEncodeH264QpKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264QpKHR* s);
};

class MarshalVkVideoEncodeH264FrameSizeKHR {
public:
    MarshalVkVideoEncodeH264FrameSizeKHR() {}
    VkVideoEncodeH264FrameSizeKHR s;
    MarshalVkVideoEncodeH264FrameSizeKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264FrameSizeKHR* s);
};

class MarshalVkVideoEncodeH264GopRemainingFrameInfoKHR {
public:
    MarshalVkVideoEncodeH264GopRemainingFrameInfoKHR() {}
    VkVideoEncodeH264GopRemainingFrameInfoKHR s;
    MarshalVkVideoEncodeH264GopRemainingFrameInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264GopRemainingFrameInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264GopRemainingFrameInfoKHR* s);
    ~MarshalVkVideoEncodeH264GopRemainingFrameInfoKHR();
};

class MarshalVkVideoEncodeH264RateControlLayerInfoKHR {
public:
    MarshalVkVideoEncodeH264RateControlLayerInfoKHR() {}
    VkVideoEncodeH264RateControlLayerInfoKHR s;
    MarshalVkVideoEncodeH264RateControlLayerInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264RateControlLayerInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH264RateControlLayerInfoKHR* s);
    ~MarshalVkVideoEncodeH264RateControlLayerInfoKHR();
};

class MarshalVkVideoEncodeH265CapabilitiesKHR {
public:
    MarshalVkVideoEncodeH265CapabilitiesKHR() {}
    VkVideoEncodeH265CapabilitiesKHR s;
    MarshalVkVideoEncodeH265CapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265CapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265CapabilitiesKHR* s);
    ~MarshalVkVideoEncodeH265CapabilitiesKHR();
};

class MarshalVkVideoEncodeH265QualityLevelPropertiesKHR {
public:
    MarshalVkVideoEncodeH265QualityLevelPropertiesKHR() {}
    VkVideoEncodeH265QualityLevelPropertiesKHR s;
    MarshalVkVideoEncodeH265QualityLevelPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265QualityLevelPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265QualityLevelPropertiesKHR* s);
    ~MarshalVkVideoEncodeH265QualityLevelPropertiesKHR();
};

class MarshalStdVideoEncodeH265PictureInfo {
public:
    MarshalStdVideoEncodeH265PictureInfo() {}
    StdVideoEncodeH265PictureInfo s;
    MarshalStdVideoEncodeH265PictureInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH265PictureInfo* s);
    ~MarshalStdVideoEncodeH265PictureInfo();
};

class MarshalStdVideoEncodeH265SliceSegmentHeader {
public:
    MarshalStdVideoEncodeH265SliceSegmentHeader() {}
    StdVideoEncodeH265SliceSegmentHeader s;
    MarshalStdVideoEncodeH265SliceSegmentHeader(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH265SliceSegmentHeader* s);
    ~MarshalStdVideoEncodeH265SliceSegmentHeader();
};

class MarshalStdVideoEncodeH265ReferenceInfo {
public:
    MarshalStdVideoEncodeH265ReferenceInfo() {}
    StdVideoEncodeH265ReferenceInfo s;
    MarshalStdVideoEncodeH265ReferenceInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH265ReferenceInfo* s);
};

class MarshalVkVideoEncodeH265SessionCreateInfoKHR {
public:
    MarshalVkVideoEncodeH265SessionCreateInfoKHR() {}
    VkVideoEncodeH265SessionCreateInfoKHR s;
    MarshalVkVideoEncodeH265SessionCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionCreateInfoKHR* s);
    ~MarshalVkVideoEncodeH265SessionCreateInfoKHR();
};

class MarshalVkVideoEncodeH265SessionParametersAddInfoKHR {
public:
    MarshalVkVideoEncodeH265SessionParametersAddInfoKHR() {}
    VkVideoEncodeH265SessionParametersAddInfoKHR s;
    MarshalVkVideoEncodeH265SessionParametersAddInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionParametersAddInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionParametersAddInfoKHR* s);
    ~MarshalVkVideoEncodeH265SessionParametersAddInfoKHR();
};

class MarshalVkVideoEncodeH265SessionParametersCreateInfoKHR {
public:
    MarshalVkVideoEncodeH265SessionParametersCreateInfoKHR() {}
    VkVideoEncodeH265SessionParametersCreateInfoKHR s;
    MarshalVkVideoEncodeH265SessionParametersCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionParametersCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionParametersCreateInfoKHR* s);
    ~MarshalVkVideoEncodeH265SessionParametersCreateInfoKHR();
};

class MarshalVkVideoEncodeH265SessionParametersGetInfoKHR {
public:
    MarshalVkVideoEncodeH265SessionParametersGetInfoKHR() {}
    VkVideoEncodeH265SessionParametersGetInfoKHR s;
    MarshalVkVideoEncodeH265SessionParametersGetInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionParametersGetInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionParametersGetInfoKHR* s);
    ~MarshalVkVideoEncodeH265SessionParametersGetInfoKHR();
};

class MarshalVkVideoEncodeH265SessionParametersFeedbackInfoKHR {
public:
    MarshalVkVideoEncodeH265SessionParametersFeedbackInfoKHR() {}
    VkVideoEncodeH265SessionParametersFeedbackInfoKHR s;
    MarshalVkVideoEncodeH265SessionParametersFeedbackInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionParametersFeedbackInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265SessionParametersFeedbackInfoKHR* s);
    ~MarshalVkVideoEncodeH265SessionParametersFeedbackInfoKHR();
};

class MarshalVkVideoEncodeH265PictureInfoKHR {
public:
    MarshalVkVideoEncodeH265PictureInfoKHR() {}
    VkVideoEncodeH265PictureInfoKHR s;
    MarshalVkVideoEncodeH265PictureInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265PictureInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265PictureInfoKHR* s);
    ~MarshalVkVideoEncodeH265PictureInfoKHR();
};

class MarshalVkVideoEncodeH265NaluSliceSegmentInfoKHR {
public:
    MarshalVkVideoEncodeH265NaluSliceSegmentInfoKHR() {}
    VkVideoEncodeH265NaluSliceSegmentInfoKHR s;
    MarshalVkVideoEncodeH265NaluSliceSegmentInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265NaluSliceSegmentInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265NaluSliceSegmentInfoKHR* s);
    ~MarshalVkVideoEncodeH265NaluSliceSegmentInfoKHR();
};

class MarshalVkVideoEncodeH265RateControlInfoKHR {
public:
    MarshalVkVideoEncodeH265RateControlInfoKHR() {}
    VkVideoEncodeH265RateControlInfoKHR s;
    MarshalVkVideoEncodeH265RateControlInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265RateControlInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265RateControlInfoKHR* s);
    ~MarshalVkVideoEncodeH265RateControlInfoKHR();
};

class MarshalVkVideoEncodeH265QpKHR {
public:
    MarshalVkVideoEncodeH265QpKHR() {}
    VkVideoEncodeH265QpKHR s;
    MarshalVkVideoEncodeH265QpKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265QpKHR* s);
};

class MarshalVkVideoEncodeH265FrameSizeKHR {
public:
    MarshalVkVideoEncodeH265FrameSizeKHR() {}
    VkVideoEncodeH265FrameSizeKHR s;
    MarshalVkVideoEncodeH265FrameSizeKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265FrameSizeKHR* s);
};

class MarshalVkVideoEncodeH265GopRemainingFrameInfoKHR {
public:
    MarshalVkVideoEncodeH265GopRemainingFrameInfoKHR() {}
    VkVideoEncodeH265GopRemainingFrameInfoKHR s;
    MarshalVkVideoEncodeH265GopRemainingFrameInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265GopRemainingFrameInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265GopRemainingFrameInfoKHR* s);
    ~MarshalVkVideoEncodeH265GopRemainingFrameInfoKHR();
};

class MarshalVkVideoEncodeH265RateControlLayerInfoKHR {
public:
    MarshalVkVideoEncodeH265RateControlLayerInfoKHR() {}
    VkVideoEncodeH265RateControlLayerInfoKHR s;
    MarshalVkVideoEncodeH265RateControlLayerInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265RateControlLayerInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265RateControlLayerInfoKHR* s);
    ~MarshalVkVideoEncodeH265RateControlLayerInfoKHR();
};

class MarshalVkVideoEncodeH265ProfileInfoKHR {
public:
    MarshalVkVideoEncodeH265ProfileInfoKHR() {}
    VkVideoEncodeH265ProfileInfoKHR s;
    MarshalVkVideoEncodeH265ProfileInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265ProfileInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265ProfileInfoKHR* s);
    ~MarshalVkVideoEncodeH265ProfileInfoKHR();
};

class MarshalVkVideoEncodeH265DpbSlotInfoKHR {
public:
    MarshalVkVideoEncodeH265DpbSlotInfoKHR() {}
    VkVideoEncodeH265DpbSlotInfoKHR s;
    MarshalVkVideoEncodeH265DpbSlotInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265DpbSlotInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeH265DpbSlotInfoKHR* s);
    ~MarshalVkVideoEncodeH265DpbSlotInfoKHR();
};

class MarshalVkVideoEncodeAV1CapabilitiesKHR {
public:
    MarshalVkVideoEncodeAV1CapabilitiesKHR() {}
    VkVideoEncodeAV1CapabilitiesKHR s;
    MarshalVkVideoEncodeAV1CapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1CapabilitiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1CapabilitiesKHR* s);
    ~MarshalVkVideoEncodeAV1CapabilitiesKHR();
};

class MarshalVkVideoEncodeAV1QualityLevelPropertiesKHR {
public:
    MarshalVkVideoEncodeAV1QualityLevelPropertiesKHR() {}
    VkVideoEncodeAV1QualityLevelPropertiesKHR s;
    MarshalVkVideoEncodeAV1QualityLevelPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1QualityLevelPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1QualityLevelPropertiesKHR* s);
    ~MarshalVkVideoEncodeAV1QualityLevelPropertiesKHR();
};

class MarshalStdVideoEncodeAV1ExtensionHeader {
public:
    MarshalStdVideoEncodeAV1ExtensionHeader() {}
    StdVideoEncodeAV1ExtensionHeader s;
    MarshalStdVideoEncodeAV1ExtensionHeader(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeAV1ExtensionHeader* s);
};

class MarshalStdVideoEncodeAV1DecoderModelInfo {
public:
    MarshalStdVideoEncodeAV1DecoderModelInfo() {}
    StdVideoEncodeAV1DecoderModelInfo s;
    MarshalStdVideoEncodeAV1DecoderModelInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeAV1DecoderModelInfo* s);
};

class MarshalStdVideoEncodeAV1OperatingPointInfo {
public:
    MarshalStdVideoEncodeAV1OperatingPointInfo() {}
    StdVideoEncodeAV1OperatingPointInfo s;
    MarshalStdVideoEncodeAV1OperatingPointInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeAV1OperatingPointInfo* s);
};

class MarshalStdVideoEncodeAV1PictureInfo {
public:
    MarshalStdVideoEncodeAV1PictureInfo() {}
    StdVideoEncodeAV1PictureInfo s;
    MarshalStdVideoEncodeAV1PictureInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeAV1PictureInfo* s);
    ~MarshalStdVideoEncodeAV1PictureInfo();
};

class MarshalStdVideoEncodeAV1ReferenceInfo {
public:
    MarshalStdVideoEncodeAV1ReferenceInfo() {}
    StdVideoEncodeAV1ReferenceInfo s;
    MarshalStdVideoEncodeAV1ReferenceInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeAV1ReferenceInfo* s);
    ~MarshalStdVideoEncodeAV1ReferenceInfo();
};

class MarshalVkPhysicalDeviceVideoEncodeAV1FeaturesKHR {
public:
    MarshalVkPhysicalDeviceVideoEncodeAV1FeaturesKHR() {}
    VkPhysicalDeviceVideoEncodeAV1FeaturesKHR s;
    MarshalVkPhysicalDeviceVideoEncodeAV1FeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoEncodeAV1FeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVideoEncodeAV1FeaturesKHR* s);
    ~MarshalVkPhysicalDeviceVideoEncodeAV1FeaturesKHR();
};

class MarshalVkVideoEncodeAV1SessionCreateInfoKHR {
public:
    MarshalVkVideoEncodeAV1SessionCreateInfoKHR() {}
    VkVideoEncodeAV1SessionCreateInfoKHR s;
    MarshalVkVideoEncodeAV1SessionCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1SessionCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1SessionCreateInfoKHR* s);
    ~MarshalVkVideoEncodeAV1SessionCreateInfoKHR();
};

class MarshalVkVideoEncodeAV1SessionParametersCreateInfoKHR {
public:
    MarshalVkVideoEncodeAV1SessionParametersCreateInfoKHR() {}
    VkVideoEncodeAV1SessionParametersCreateInfoKHR s;
    MarshalVkVideoEncodeAV1SessionParametersCreateInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1SessionParametersCreateInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1SessionParametersCreateInfoKHR* s);
    ~MarshalVkVideoEncodeAV1SessionParametersCreateInfoKHR();
};

class MarshalVkVideoEncodeAV1DpbSlotInfoKHR {
public:
    MarshalVkVideoEncodeAV1DpbSlotInfoKHR() {}
    VkVideoEncodeAV1DpbSlotInfoKHR s;
    MarshalVkVideoEncodeAV1DpbSlotInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1DpbSlotInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1DpbSlotInfoKHR* s);
    ~MarshalVkVideoEncodeAV1DpbSlotInfoKHR();
};

class MarshalVkVideoEncodeAV1PictureInfoKHR {
public:
    MarshalVkVideoEncodeAV1PictureInfoKHR() {}
    VkVideoEncodeAV1PictureInfoKHR s;
    MarshalVkVideoEncodeAV1PictureInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1PictureInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1PictureInfoKHR* s);
    ~MarshalVkVideoEncodeAV1PictureInfoKHR();
};

class MarshalVkVideoEncodeAV1ProfileInfoKHR {
public:
    MarshalVkVideoEncodeAV1ProfileInfoKHR() {}
    VkVideoEncodeAV1ProfileInfoKHR s;
    MarshalVkVideoEncodeAV1ProfileInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1ProfileInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1ProfileInfoKHR* s);
    ~MarshalVkVideoEncodeAV1ProfileInfoKHR();
};

class MarshalVkVideoEncodeAV1RateControlInfoKHR {
public:
    MarshalVkVideoEncodeAV1RateControlInfoKHR() {}
    VkVideoEncodeAV1RateControlInfoKHR s;
    MarshalVkVideoEncodeAV1RateControlInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1RateControlInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1RateControlInfoKHR* s);
    ~MarshalVkVideoEncodeAV1RateControlInfoKHR();
};

class MarshalVkVideoEncodeAV1QIndexKHR {
public:
    MarshalVkVideoEncodeAV1QIndexKHR() {}
    VkVideoEncodeAV1QIndexKHR s;
    MarshalVkVideoEncodeAV1QIndexKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1QIndexKHR* s);
};

class MarshalVkVideoEncodeAV1FrameSizeKHR {
public:
    MarshalVkVideoEncodeAV1FrameSizeKHR() {}
    VkVideoEncodeAV1FrameSizeKHR s;
    MarshalVkVideoEncodeAV1FrameSizeKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1FrameSizeKHR* s);
};

class MarshalVkVideoEncodeAV1GopRemainingFrameInfoKHR {
public:
    MarshalVkVideoEncodeAV1GopRemainingFrameInfoKHR() {}
    VkVideoEncodeAV1GopRemainingFrameInfoKHR s;
    MarshalVkVideoEncodeAV1GopRemainingFrameInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1GopRemainingFrameInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1GopRemainingFrameInfoKHR* s);
    ~MarshalVkVideoEncodeAV1GopRemainingFrameInfoKHR();
};

class MarshalVkVideoEncodeAV1RateControlLayerInfoKHR {
public:
    MarshalVkVideoEncodeAV1RateControlLayerInfoKHR() {}
    VkVideoEncodeAV1RateControlLayerInfoKHR s;
    MarshalVkVideoEncodeAV1RateControlLayerInfoKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1RateControlLayerInfoKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkVideoEncodeAV1RateControlLayerInfoKHR* s);
    ~MarshalVkVideoEncodeAV1RateControlLayerInfoKHR();
};

class MarshalVkPhysicalDeviceInheritedViewportScissorFeaturesNV {
public:
    MarshalVkPhysicalDeviceInheritedViewportScissorFeaturesNV() {}
    VkPhysicalDeviceInheritedViewportScissorFeaturesNV s;
    MarshalVkPhysicalDeviceInheritedViewportScissorFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceInheritedViewportScissorFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceInheritedViewportScissorFeaturesNV* s);
    ~MarshalVkPhysicalDeviceInheritedViewportScissorFeaturesNV();
};

class MarshalVkCommandBufferInheritanceViewportScissorInfoNV {
public:
    MarshalVkCommandBufferInheritanceViewportScissorInfoNV() {}
    VkCommandBufferInheritanceViewportScissorInfoNV s;
    MarshalVkCommandBufferInheritanceViewportScissorInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceViewportScissorInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceViewportScissorInfoNV* s);
    ~MarshalVkCommandBufferInheritanceViewportScissorInfoNV();
};

class MarshalVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT {
public:
    MarshalVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT() {}
    VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT s;
    MarshalVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT();
};

class MarshalVkPhysicalDeviceProvokingVertexFeaturesEXT {
public:
    MarshalVkPhysicalDeviceProvokingVertexFeaturesEXT() {}
    VkPhysicalDeviceProvokingVertexFeaturesEXT s;
    MarshalVkPhysicalDeviceProvokingVertexFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProvokingVertexFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProvokingVertexFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceProvokingVertexFeaturesEXT();
};

class MarshalVkPhysicalDeviceProvokingVertexPropertiesEXT {
public:
    MarshalVkPhysicalDeviceProvokingVertexPropertiesEXT() {}
    VkPhysicalDeviceProvokingVertexPropertiesEXT s;
    MarshalVkPhysicalDeviceProvokingVertexPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProvokingVertexPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceProvokingVertexPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceProvokingVertexPropertiesEXT();
};

class MarshalVkPipelineRasterizationProvokingVertexStateCreateInfoEXT {
public:
    MarshalVkPipelineRasterizationProvokingVertexStateCreateInfoEXT() {}
    VkPipelineRasterizationProvokingVertexStateCreateInfoEXT s;
    MarshalVkPipelineRasterizationProvokingVertexStateCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationProvokingVertexStateCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRasterizationProvokingVertexStateCreateInfoEXT* s);
    ~MarshalVkPipelineRasterizationProvokingVertexStateCreateInfoEXT();
};

class MarshalVkCuModuleCreateInfoNVX {
public:
    MarshalVkCuModuleCreateInfoNVX() {}
    VkCuModuleCreateInfoNVX s;
    MarshalVkCuModuleCreateInfoNVX(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCuModuleCreateInfoNVX* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCuModuleCreateInfoNVX* s);
    ~MarshalVkCuModuleCreateInfoNVX();
};

class MarshalVkCuModuleTexturingModeCreateInfoNVX {
public:
    MarshalVkCuModuleTexturingModeCreateInfoNVX() {}
    VkCuModuleTexturingModeCreateInfoNVX s;
    MarshalVkCuModuleTexturingModeCreateInfoNVX(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCuModuleTexturingModeCreateInfoNVX* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCuModuleTexturingModeCreateInfoNVX* s);
    ~MarshalVkCuModuleTexturingModeCreateInfoNVX();
};

class MarshalVkCuFunctionCreateInfoNVX {
public:
    MarshalVkCuFunctionCreateInfoNVX() {}
    VkCuFunctionCreateInfoNVX s;
    MarshalVkCuFunctionCreateInfoNVX(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCuFunctionCreateInfoNVX* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCuFunctionCreateInfoNVX* s);
    ~MarshalVkCuFunctionCreateInfoNVX();
};

class MarshalVkCuLaunchInfoNVX {
public:
    MarshalVkCuLaunchInfoNVX() {}
    VkCuLaunchInfoNVX s;
    MarshalVkCuLaunchInfoNVX(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCuLaunchInfoNVX* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCuLaunchInfoNVX* s);
};

class MarshalVkPhysicalDeviceDescriptorBufferFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDescriptorBufferFeaturesEXT() {}
    VkPhysicalDeviceDescriptorBufferFeaturesEXT s;
    MarshalVkPhysicalDeviceDescriptorBufferFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorBufferFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorBufferFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDescriptorBufferFeaturesEXT();
};

class MarshalVkPhysicalDeviceDescriptorBufferPropertiesEXT {
public:
    MarshalVkPhysicalDeviceDescriptorBufferPropertiesEXT() {}
    VkPhysicalDeviceDescriptorBufferPropertiesEXT s;
    MarshalVkPhysicalDeviceDescriptorBufferPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorBufferPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorBufferPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceDescriptorBufferPropertiesEXT();
};

class MarshalVkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT {
public:
    MarshalVkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT() {}
    VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT s;
    MarshalVkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT();
};

class MarshalVkDescriptorAddressInfoEXT {
public:
    MarshalVkDescriptorAddressInfoEXT() {}
    VkDescriptorAddressInfoEXT s;
    MarshalVkDescriptorAddressInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorAddressInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorAddressInfoEXT* s);
    ~MarshalVkDescriptorAddressInfoEXT();
};

class MarshalVkDescriptorBufferBindingInfoEXT {
public:
    MarshalVkDescriptorBufferBindingInfoEXT() {}
    VkDescriptorBufferBindingInfoEXT s;
    MarshalVkDescriptorBufferBindingInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorBufferBindingInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorBufferBindingInfoEXT* s);
    ~MarshalVkDescriptorBufferBindingInfoEXT();
};

class MarshalVkDescriptorBufferBindingPushDescriptorBufferHandleEXT {
public:
    MarshalVkDescriptorBufferBindingPushDescriptorBufferHandleEXT() {}
    VkDescriptorBufferBindingPushDescriptorBufferHandleEXT s;
    MarshalVkDescriptorBufferBindingPushDescriptorBufferHandleEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorBufferBindingPushDescriptorBufferHandleEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorBufferBindingPushDescriptorBufferHandleEXT* s);
    ~MarshalVkDescriptorBufferBindingPushDescriptorBufferHandleEXT();
};

class MarshalVkDescriptorGetInfoEXT {
public:
    MarshalVkDescriptorGetInfoEXT() {}
    VkDescriptorGetInfoEXT s;
    MarshalVkDescriptorGetInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorGetInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorGetInfoEXT* s);
    ~MarshalVkDescriptorGetInfoEXT();
};

class MarshalVkBufferCaptureDescriptorDataInfoEXT {
public:
    MarshalVkBufferCaptureDescriptorDataInfoEXT() {}
    VkBufferCaptureDescriptorDataInfoEXT s;
    MarshalVkBufferCaptureDescriptorDataInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferCaptureDescriptorDataInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBufferCaptureDescriptorDataInfoEXT* s);
    ~MarshalVkBufferCaptureDescriptorDataInfoEXT();
};

class MarshalVkImageCaptureDescriptorDataInfoEXT {
public:
    MarshalVkImageCaptureDescriptorDataInfoEXT() {}
    VkImageCaptureDescriptorDataInfoEXT s;
    MarshalVkImageCaptureDescriptorDataInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCaptureDescriptorDataInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCaptureDescriptorDataInfoEXT* s);
    ~MarshalVkImageCaptureDescriptorDataInfoEXT();
};

class MarshalVkImageViewCaptureDescriptorDataInfoEXT {
public:
    MarshalVkImageViewCaptureDescriptorDataInfoEXT() {}
    VkImageViewCaptureDescriptorDataInfoEXT s;
    MarshalVkImageViewCaptureDescriptorDataInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewCaptureDescriptorDataInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewCaptureDescriptorDataInfoEXT* s);
    ~MarshalVkImageViewCaptureDescriptorDataInfoEXT();
};

class MarshalVkSamplerCaptureDescriptorDataInfoEXT {
public:
    MarshalVkSamplerCaptureDescriptorDataInfoEXT() {}
    VkSamplerCaptureDescriptorDataInfoEXT s;
    MarshalVkSamplerCaptureDescriptorDataInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerCaptureDescriptorDataInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerCaptureDescriptorDataInfoEXT* s);
    ~MarshalVkSamplerCaptureDescriptorDataInfoEXT();
};

class MarshalVkAccelerationStructureCaptureDescriptorDataInfoEXT {
public:
    MarshalVkAccelerationStructureCaptureDescriptorDataInfoEXT() {}
    VkAccelerationStructureCaptureDescriptorDataInfoEXT s;
    MarshalVkAccelerationStructureCaptureDescriptorDataInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureCaptureDescriptorDataInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureCaptureDescriptorDataInfoEXT* s);
    ~MarshalVkAccelerationStructureCaptureDescriptorDataInfoEXT();
};

class MarshalVkOpaqueCaptureDescriptorDataCreateInfoEXT {
public:
    MarshalVkOpaqueCaptureDescriptorDataCreateInfoEXT() {}
    VkOpaqueCaptureDescriptorDataCreateInfoEXT s;
    MarshalVkOpaqueCaptureDescriptorDataCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpaqueCaptureDescriptorDataCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpaqueCaptureDescriptorDataCreateInfoEXT* s);
};

class MarshalVkPhysicalDeviceShaderIntegerDotProductFeatures {
public:
    MarshalVkPhysicalDeviceShaderIntegerDotProductFeatures() {}
    VkPhysicalDeviceShaderIntegerDotProductFeatures s;
    MarshalVkPhysicalDeviceShaderIntegerDotProductFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderIntegerDotProductFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderIntegerDotProductFeatures* s);
    ~MarshalVkPhysicalDeviceShaderIntegerDotProductFeatures();
};

class MarshalVkPhysicalDeviceShaderIntegerDotProductProperties {
public:
    MarshalVkPhysicalDeviceShaderIntegerDotProductProperties() {}
    VkPhysicalDeviceShaderIntegerDotProductProperties s;
    MarshalVkPhysicalDeviceShaderIntegerDotProductProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderIntegerDotProductProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderIntegerDotProductProperties* s);
    ~MarshalVkPhysicalDeviceShaderIntegerDotProductProperties();
};

class MarshalVkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR {
public:
    MarshalVkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR() {}
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR s;
    MarshalVkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR();
};

class MarshalVkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR {
public:
    MarshalVkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR() {}
    VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR s;
    MarshalVkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR* s);
    ~MarshalVkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR();
};

class MarshalVkPhysicalDeviceRayTracingMotionBlurFeaturesNV {
public:
    MarshalVkPhysicalDeviceRayTracingMotionBlurFeaturesNV() {}
    VkPhysicalDeviceRayTracingMotionBlurFeaturesNV s;
    MarshalVkPhysicalDeviceRayTracingMotionBlurFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingMotionBlurFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingMotionBlurFeaturesNV* s);
    ~MarshalVkPhysicalDeviceRayTracingMotionBlurFeaturesNV();
};

class MarshalVkPhysicalDeviceRayTracingValidationFeaturesNV {
public:
    MarshalVkPhysicalDeviceRayTracingValidationFeaturesNV() {}
    VkPhysicalDeviceRayTracingValidationFeaturesNV s;
    MarshalVkPhysicalDeviceRayTracingValidationFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingValidationFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingValidationFeaturesNV* s);
    ~MarshalVkPhysicalDeviceRayTracingValidationFeaturesNV();
};

class MarshalVkAccelerationStructureGeometryMotionTrianglesDataNV {
public:
    MarshalVkAccelerationStructureGeometryMotionTrianglesDataNV() {}
    VkAccelerationStructureGeometryMotionTrianglesDataNV s;
    MarshalVkAccelerationStructureGeometryMotionTrianglesDataNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryMotionTrianglesDataNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureGeometryMotionTrianglesDataNV* s);
};

class MarshalVkAccelerationStructureMotionInfoNV {
public:
    MarshalVkAccelerationStructureMotionInfoNV() {}
    VkAccelerationStructureMotionInfoNV s;
    MarshalVkAccelerationStructureMotionInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureMotionInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureMotionInfoNV* s);
    ~MarshalVkAccelerationStructureMotionInfoNV();
};

class MarshalVkCudaModuleCreateInfoNV {
public:
    MarshalVkCudaModuleCreateInfoNV() {}
    VkCudaModuleCreateInfoNV s;
    MarshalVkCudaModuleCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCudaModuleCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCudaModuleCreateInfoNV* s);
    ~MarshalVkCudaModuleCreateInfoNV();
};

class MarshalVkCudaFunctionCreateInfoNV {
public:
    MarshalVkCudaFunctionCreateInfoNV() {}
    VkCudaFunctionCreateInfoNV s;
    MarshalVkCudaFunctionCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCudaFunctionCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCudaFunctionCreateInfoNV* s);
    ~MarshalVkCudaFunctionCreateInfoNV();
};

class MarshalVkCudaLaunchInfoNV {
public:
    MarshalVkCudaLaunchInfoNV() {}
    VkCudaLaunchInfoNV s;
    MarshalVkCudaLaunchInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCudaLaunchInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCudaLaunchInfoNV* s);
};

class MarshalVkPhysicalDeviceRGBA10X6FormatsFeaturesEXT {
public:
    MarshalVkPhysicalDeviceRGBA10X6FormatsFeaturesEXT() {}
    VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT s;
    MarshalVkPhysicalDeviceRGBA10X6FormatsFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceRGBA10X6FormatsFeaturesEXT();
};

class MarshalVkFormatProperties3 {
public:
    MarshalVkFormatProperties3() {}
    VkFormatProperties3 s;
    MarshalVkFormatProperties3(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFormatProperties3* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFormatProperties3* s);
    ~MarshalVkFormatProperties3();
};

class MarshalVkPipelineRenderingCreateInfo {
public:
    MarshalVkPipelineRenderingCreateInfo() {}
    VkPipelineRenderingCreateInfo s;
    MarshalVkPipelineRenderingCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRenderingCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRenderingCreateInfo* s);
    ~MarshalVkPipelineRenderingCreateInfo();
};

class MarshalVkRenderingInfo {
public:
    MarshalVkRenderingInfo() {}
    VkRenderingInfo s;
    MarshalVkRenderingInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingInfo* s);
    ~MarshalVkRenderingInfo();
};

class MarshalVkRenderingAttachmentInfo {
public:
    MarshalVkRenderingAttachmentInfo() {}
    VkRenderingAttachmentInfo s;
    MarshalVkRenderingAttachmentInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingAttachmentInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingAttachmentInfo* s);
    ~MarshalVkRenderingAttachmentInfo();
};

class MarshalVkRenderingFragmentDensityMapAttachmentInfoEXT {
public:
    MarshalVkRenderingFragmentDensityMapAttachmentInfoEXT() {}
    VkRenderingFragmentDensityMapAttachmentInfoEXT s;
    MarshalVkRenderingFragmentDensityMapAttachmentInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingFragmentDensityMapAttachmentInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingFragmentDensityMapAttachmentInfoEXT* s);
    ~MarshalVkRenderingFragmentDensityMapAttachmentInfoEXT();
};

class MarshalVkPhysicalDeviceDynamicRenderingFeatures {
public:
    MarshalVkPhysicalDeviceDynamicRenderingFeatures() {}
    VkPhysicalDeviceDynamicRenderingFeatures s;
    MarshalVkPhysicalDeviceDynamicRenderingFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDynamicRenderingFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDynamicRenderingFeatures* s);
    ~MarshalVkPhysicalDeviceDynamicRenderingFeatures();
};

class MarshalVkCommandBufferInheritanceRenderingInfo {
public:
    MarshalVkCommandBufferInheritanceRenderingInfo() {}
    VkCommandBufferInheritanceRenderingInfo s;
    MarshalVkCommandBufferInheritanceRenderingInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceRenderingInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCommandBufferInheritanceRenderingInfo* s);
    ~MarshalVkCommandBufferInheritanceRenderingInfo();
};

class MarshalVkAttachmentSampleCountInfoAMD {
public:
    MarshalVkAttachmentSampleCountInfoAMD() {}
    VkAttachmentSampleCountInfoAMD s;
    MarshalVkAttachmentSampleCountInfoAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentSampleCountInfoAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAttachmentSampleCountInfoAMD* s);
    ~MarshalVkAttachmentSampleCountInfoAMD();
};

class MarshalVkMultiviewPerViewAttributesInfoNVX {
public:
    MarshalVkMultiviewPerViewAttributesInfoNVX() {}
    VkMultiviewPerViewAttributesInfoNVX s;
    MarshalVkMultiviewPerViewAttributesInfoNVX(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultiviewPerViewAttributesInfoNVX* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultiviewPerViewAttributesInfoNVX* s);
    ~MarshalVkMultiviewPerViewAttributesInfoNVX();
};

class MarshalVkPhysicalDeviceImageViewMinLodFeaturesEXT {
public:
    MarshalVkPhysicalDeviceImageViewMinLodFeaturesEXT() {}
    VkPhysicalDeviceImageViewMinLodFeaturesEXT s;
    MarshalVkPhysicalDeviceImageViewMinLodFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageViewMinLodFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageViewMinLodFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceImageViewMinLodFeaturesEXT();
};

class MarshalVkImageViewMinLodCreateInfoEXT {
public:
    MarshalVkImageViewMinLodCreateInfoEXT() {}
    VkImageViewMinLodCreateInfoEXT s;
    MarshalVkImageViewMinLodCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewMinLodCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewMinLodCreateInfoEXT* s);
    ~MarshalVkImageViewMinLodCreateInfoEXT();
};

class MarshalVkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT {
public:
    MarshalVkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT() {}
    VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT s;
    MarshalVkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT();
};

class MarshalVkPhysicalDeviceLinearColorAttachmentFeaturesNV {
public:
    MarshalVkPhysicalDeviceLinearColorAttachmentFeaturesNV() {}
    VkPhysicalDeviceLinearColorAttachmentFeaturesNV s;
    MarshalVkPhysicalDeviceLinearColorAttachmentFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLinearColorAttachmentFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLinearColorAttachmentFeaturesNV* s);
    ~MarshalVkPhysicalDeviceLinearColorAttachmentFeaturesNV();
};

class MarshalVkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT {
public:
    MarshalVkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT() {}
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT s;
    MarshalVkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT();
};

class MarshalVkPhysicalDevicePipelineBinaryFeaturesKHR {
public:
    MarshalVkPhysicalDevicePipelineBinaryFeaturesKHR() {}
    VkPhysicalDevicePipelineBinaryFeaturesKHR s;
    MarshalVkPhysicalDevicePipelineBinaryFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineBinaryFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineBinaryFeaturesKHR* s);
    ~MarshalVkPhysicalDevicePipelineBinaryFeaturesKHR();
};

class MarshalVkDevicePipelineBinaryInternalCacheControlKHR {
public:
    MarshalVkDevicePipelineBinaryInternalCacheControlKHR() {}
    VkDevicePipelineBinaryInternalCacheControlKHR s;
    MarshalVkDevicePipelineBinaryInternalCacheControlKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDevicePipelineBinaryInternalCacheControlKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDevicePipelineBinaryInternalCacheControlKHR* s);
    ~MarshalVkDevicePipelineBinaryInternalCacheControlKHR();
};

class MarshalVkPhysicalDevicePipelineBinaryPropertiesKHR {
public:
    MarshalVkPhysicalDevicePipelineBinaryPropertiesKHR() {}
    VkPhysicalDevicePipelineBinaryPropertiesKHR s;
    MarshalVkPhysicalDevicePipelineBinaryPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineBinaryPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineBinaryPropertiesKHR* s);
    ~MarshalVkPhysicalDevicePipelineBinaryPropertiesKHR();
};

class MarshalVkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT {
public:
    MarshalVkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT() {}
    VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT s;
    MarshalVkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT();
};

class MarshalVkGraphicsPipelineLibraryCreateInfoEXT {
public:
    MarshalVkGraphicsPipelineLibraryCreateInfoEXT() {}
    VkGraphicsPipelineLibraryCreateInfoEXT s;
    MarshalVkGraphicsPipelineLibraryCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGraphicsPipelineLibraryCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGraphicsPipelineLibraryCreateInfoEXT* s);
    ~MarshalVkGraphicsPipelineLibraryCreateInfoEXT();
};

class MarshalVkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE {
public:
    MarshalVkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE() {}
    VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE s;
    MarshalVkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE* s);
    ~MarshalVkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE();
};

class MarshalVkDescriptorSetBindingReferenceVALVE {
public:
    MarshalVkDescriptorSetBindingReferenceVALVE() {}
    VkDescriptorSetBindingReferenceVALVE s;
    MarshalVkDescriptorSetBindingReferenceVALVE(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetBindingReferenceVALVE* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetBindingReferenceVALVE* s);
    ~MarshalVkDescriptorSetBindingReferenceVALVE();
};

class MarshalVkDescriptorSetLayoutHostMappingInfoVALVE {
public:
    MarshalVkDescriptorSetLayoutHostMappingInfoVALVE() {}
    VkDescriptorSetLayoutHostMappingInfoVALVE s;
    MarshalVkDescriptorSetLayoutHostMappingInfoVALVE(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetLayoutHostMappingInfoVALVE* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDescriptorSetLayoutHostMappingInfoVALVE* s);
    ~MarshalVkDescriptorSetLayoutHostMappingInfoVALVE();
};

class MarshalVkPhysicalDeviceNestedCommandBufferFeaturesEXT {
public:
    MarshalVkPhysicalDeviceNestedCommandBufferFeaturesEXT() {}
    VkPhysicalDeviceNestedCommandBufferFeaturesEXT s;
    MarshalVkPhysicalDeviceNestedCommandBufferFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceNestedCommandBufferFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceNestedCommandBufferFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceNestedCommandBufferFeaturesEXT();
};

class MarshalVkPhysicalDeviceNestedCommandBufferPropertiesEXT {
public:
    MarshalVkPhysicalDeviceNestedCommandBufferPropertiesEXT() {}
    VkPhysicalDeviceNestedCommandBufferPropertiesEXT s;
    MarshalVkPhysicalDeviceNestedCommandBufferPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceNestedCommandBufferPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceNestedCommandBufferPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceNestedCommandBufferPropertiesEXT();
};

class MarshalVkPhysicalDeviceShaderModuleIdentifierFeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderModuleIdentifierFeaturesEXT() {}
    VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT s;
    MarshalVkPhysicalDeviceShaderModuleIdentifierFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderModuleIdentifierFeaturesEXT();
};

class MarshalVkPhysicalDeviceShaderModuleIdentifierPropertiesEXT {
public:
    MarshalVkPhysicalDeviceShaderModuleIdentifierPropertiesEXT() {}
    VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT s;
    MarshalVkPhysicalDeviceShaderModuleIdentifierPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceShaderModuleIdentifierPropertiesEXT();
};

class MarshalVkPipelineShaderStageModuleIdentifierCreateInfoEXT {
public:
    MarshalVkPipelineShaderStageModuleIdentifierCreateInfoEXT() {}
    VkPipelineShaderStageModuleIdentifierCreateInfoEXT s;
    MarshalVkPipelineShaderStageModuleIdentifierCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineShaderStageModuleIdentifierCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineShaderStageModuleIdentifierCreateInfoEXT* s);
    ~MarshalVkPipelineShaderStageModuleIdentifierCreateInfoEXT();
};

class MarshalVkShaderModuleIdentifierEXT {
public:
    MarshalVkShaderModuleIdentifierEXT() {}
    VkShaderModuleIdentifierEXT s;
    MarshalVkShaderModuleIdentifierEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkShaderModuleIdentifierEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkShaderModuleIdentifierEXT* s);
    ~MarshalVkShaderModuleIdentifierEXT();
};

class MarshalVkImageCompressionControlEXT {
public:
    MarshalVkImageCompressionControlEXT() {}
    VkImageCompressionControlEXT s;
    MarshalVkImageCompressionControlEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCompressionControlEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCompressionControlEXT* s);
    ~MarshalVkImageCompressionControlEXT();
};

class MarshalVkPhysicalDeviceImageCompressionControlFeaturesEXT {
public:
    MarshalVkPhysicalDeviceImageCompressionControlFeaturesEXT() {}
    VkPhysicalDeviceImageCompressionControlFeaturesEXT s;
    MarshalVkPhysicalDeviceImageCompressionControlFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageCompressionControlFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageCompressionControlFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceImageCompressionControlFeaturesEXT();
};

class MarshalVkImageCompressionPropertiesEXT {
public:
    MarshalVkImageCompressionPropertiesEXT() {}
    VkImageCompressionPropertiesEXT s;
    MarshalVkImageCompressionPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCompressionPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageCompressionPropertiesEXT* s);
    ~MarshalVkImageCompressionPropertiesEXT();
};

class MarshalVkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT {
public:
    MarshalVkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT() {}
    VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT s;
    MarshalVkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT();
};

class MarshalVkImageSubresource2 {
public:
    MarshalVkImageSubresource2() {}
    VkImageSubresource2 s;
    MarshalVkImageSubresource2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageSubresource2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageSubresource2* s);
    ~MarshalVkImageSubresource2();
};

class MarshalVkSubresourceLayout2 {
public:
    MarshalVkSubresourceLayout2() {}
    VkSubresourceLayout2 s;
    MarshalVkSubresourceLayout2(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubresourceLayout2* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSubresourceLayout2* s);
    ~MarshalVkSubresourceLayout2();
};

class MarshalVkRenderPassCreationControlEXT {
public:
    MarshalVkRenderPassCreationControlEXT() {}
    VkRenderPassCreationControlEXT s;
    MarshalVkRenderPassCreationControlEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassCreationControlEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassCreationControlEXT* s);
    ~MarshalVkRenderPassCreationControlEXT();
};

class MarshalVkRenderPassCreationFeedbackInfoEXT {
public:
    MarshalVkRenderPassCreationFeedbackInfoEXT() {}
    VkRenderPassCreationFeedbackInfoEXT s;
    MarshalVkRenderPassCreationFeedbackInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassCreationFeedbackInfoEXT* s);
};

class MarshalVkRenderPassCreationFeedbackCreateInfoEXT {
public:
    MarshalVkRenderPassCreationFeedbackCreateInfoEXT() {}
    VkRenderPassCreationFeedbackCreateInfoEXT s;
    MarshalVkRenderPassCreationFeedbackCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassCreationFeedbackCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassCreationFeedbackCreateInfoEXT* s);
    ~MarshalVkRenderPassCreationFeedbackCreateInfoEXT();
};

class MarshalVkRenderPassSubpassFeedbackInfoEXT {
public:
    MarshalVkRenderPassSubpassFeedbackInfoEXT() {}
    VkRenderPassSubpassFeedbackInfoEXT s;
    MarshalVkRenderPassSubpassFeedbackInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassSubpassFeedbackInfoEXT* s);
};

class MarshalVkRenderPassSubpassFeedbackCreateInfoEXT {
public:
    MarshalVkRenderPassSubpassFeedbackCreateInfoEXT() {}
    VkRenderPassSubpassFeedbackCreateInfoEXT s;
    MarshalVkRenderPassSubpassFeedbackCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassSubpassFeedbackCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassSubpassFeedbackCreateInfoEXT* s);
    ~MarshalVkRenderPassSubpassFeedbackCreateInfoEXT();
};

class MarshalVkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT {
public:
    MarshalVkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT() {}
    VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT s;
    MarshalVkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT();
};

class MarshalVkMicromapBuildInfoEXT {
public:
    MarshalVkMicromapBuildInfoEXT() {}
    VkMicromapBuildInfoEXT s;
    MarshalVkMicromapBuildInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMicromapBuildInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMicromapBuildInfoEXT* s);
};

class MarshalVkMicromapCreateInfoEXT {
public:
    MarshalVkMicromapCreateInfoEXT() {}
    VkMicromapCreateInfoEXT s;
    MarshalVkMicromapCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMicromapCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMicromapCreateInfoEXT* s);
    ~MarshalVkMicromapCreateInfoEXT();
};

class MarshalVkMicromapVersionInfoEXT {
public:
    MarshalVkMicromapVersionInfoEXT() {}
    VkMicromapVersionInfoEXT s;
    MarshalVkMicromapVersionInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMicromapVersionInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMicromapVersionInfoEXT* s);
    ~MarshalVkMicromapVersionInfoEXT();
};

class MarshalVkCopyMicromapInfoEXT {
public:
    MarshalVkCopyMicromapInfoEXT() {}
    VkCopyMicromapInfoEXT s;
    MarshalVkCopyMicromapInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMicromapInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMicromapInfoEXT* s);
    ~MarshalVkCopyMicromapInfoEXT();
};

class MarshalVkCopyMicromapToMemoryInfoEXT {
public:
    MarshalVkCopyMicromapToMemoryInfoEXT() {}
    VkCopyMicromapToMemoryInfoEXT s;
    MarshalVkCopyMicromapToMemoryInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMicromapToMemoryInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMicromapToMemoryInfoEXT* s);
};

class MarshalVkCopyMemoryToMicromapInfoEXT {
public:
    MarshalVkCopyMemoryToMicromapInfoEXT() {}
    VkCopyMemoryToMicromapInfoEXT s;
    MarshalVkCopyMemoryToMicromapInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMemoryToMicromapInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCopyMemoryToMicromapInfoEXT* s);
};

class MarshalVkMicromapBuildSizesInfoEXT {
public:
    MarshalVkMicromapBuildSizesInfoEXT() {}
    VkMicromapBuildSizesInfoEXT s;
    MarshalVkMicromapBuildSizesInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMicromapBuildSizesInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMicromapBuildSizesInfoEXT* s);
    ~MarshalVkMicromapBuildSizesInfoEXT();
};

class MarshalVkPhysicalDeviceOpacityMicromapFeaturesEXT {
public:
    MarshalVkPhysicalDeviceOpacityMicromapFeaturesEXT() {}
    VkPhysicalDeviceOpacityMicromapFeaturesEXT s;
    MarshalVkPhysicalDeviceOpacityMicromapFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceOpacityMicromapFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceOpacityMicromapFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceOpacityMicromapFeaturesEXT();
};

class MarshalVkPhysicalDeviceOpacityMicromapPropertiesEXT {
public:
    MarshalVkPhysicalDeviceOpacityMicromapPropertiesEXT() {}
    VkPhysicalDeviceOpacityMicromapPropertiesEXT s;
    MarshalVkPhysicalDeviceOpacityMicromapPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceOpacityMicromapPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceOpacityMicromapPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceOpacityMicromapPropertiesEXT();
};

class MarshalVkAccelerationStructureTrianglesOpacityMicromapEXT {
public:
    MarshalVkAccelerationStructureTrianglesOpacityMicromapEXT() {}
    VkAccelerationStructureTrianglesOpacityMicromapEXT s;
    MarshalVkAccelerationStructureTrianglesOpacityMicromapEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureTrianglesOpacityMicromapEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAccelerationStructureTrianglesOpacityMicromapEXT* s);
};

class MarshalVkPipelinePropertiesIdentifierEXT {
public:
    MarshalVkPipelinePropertiesIdentifierEXT() {}
    VkPipelinePropertiesIdentifierEXT s;
    MarshalVkPipelinePropertiesIdentifierEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelinePropertiesIdentifierEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelinePropertiesIdentifierEXT* s);
    ~MarshalVkPipelinePropertiesIdentifierEXT();
};

class MarshalVkPhysicalDevicePipelinePropertiesFeaturesEXT {
public:
    MarshalVkPhysicalDevicePipelinePropertiesFeaturesEXT() {}
    VkPhysicalDevicePipelinePropertiesFeaturesEXT s;
    MarshalVkPhysicalDevicePipelinePropertiesFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelinePropertiesFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelinePropertiesFeaturesEXT* s);
    ~MarshalVkPhysicalDevicePipelinePropertiesFeaturesEXT();
};

class MarshalVkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD {
public:
    MarshalVkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD() {}
    VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD s;
    MarshalVkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD* s);
    ~MarshalVkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD();
};

class MarshalVkExternalMemoryAcquireUnmodifiedEXT {
public:
    MarshalVkExternalMemoryAcquireUnmodifiedEXT() {}
    VkExternalMemoryAcquireUnmodifiedEXT s;
    MarshalVkExternalMemoryAcquireUnmodifiedEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalMemoryAcquireUnmodifiedEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkExternalMemoryAcquireUnmodifiedEXT* s);
    ~MarshalVkExternalMemoryAcquireUnmodifiedEXT();
};

class MarshalVkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT {
public:
    MarshalVkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT() {}
    VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT s;
    MarshalVkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT();
};

class MarshalVkPhysicalDevicePipelineRobustnessFeatures {
public:
    MarshalVkPhysicalDevicePipelineRobustnessFeatures() {}
    VkPhysicalDevicePipelineRobustnessFeatures s;
    MarshalVkPhysicalDevicePipelineRobustnessFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineRobustnessFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineRobustnessFeatures* s);
    ~MarshalVkPhysicalDevicePipelineRobustnessFeatures();
};

class MarshalVkPipelineRobustnessCreateInfo {
public:
    MarshalVkPipelineRobustnessCreateInfo() {}
    VkPipelineRobustnessCreateInfo s;
    MarshalVkPipelineRobustnessCreateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRobustnessCreateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPipelineRobustnessCreateInfo* s);
    ~MarshalVkPipelineRobustnessCreateInfo();
};

class MarshalVkPhysicalDevicePipelineRobustnessProperties {
public:
    MarshalVkPhysicalDevicePipelineRobustnessProperties() {}
    VkPhysicalDevicePipelineRobustnessProperties s;
    MarshalVkPhysicalDevicePipelineRobustnessProperties(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineRobustnessProperties* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineRobustnessProperties* s);
    ~MarshalVkPhysicalDevicePipelineRobustnessProperties();
};

class MarshalVkImageViewSampleWeightCreateInfoQCOM {
public:
    MarshalVkImageViewSampleWeightCreateInfoQCOM() {}
    VkImageViewSampleWeightCreateInfoQCOM s;
    MarshalVkImageViewSampleWeightCreateInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewSampleWeightCreateInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageViewSampleWeightCreateInfoQCOM* s);
    ~MarshalVkImageViewSampleWeightCreateInfoQCOM();
};

class MarshalVkPhysicalDeviceImageProcessingFeaturesQCOM {
public:
    MarshalVkPhysicalDeviceImageProcessingFeaturesQCOM() {}
    VkPhysicalDeviceImageProcessingFeaturesQCOM s;
    MarshalVkPhysicalDeviceImageProcessingFeaturesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageProcessingFeaturesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageProcessingFeaturesQCOM* s);
    ~MarshalVkPhysicalDeviceImageProcessingFeaturesQCOM();
};

class MarshalVkPhysicalDeviceImageProcessingPropertiesQCOM {
public:
    MarshalVkPhysicalDeviceImageProcessingPropertiesQCOM() {}
    VkPhysicalDeviceImageProcessingPropertiesQCOM s;
    MarshalVkPhysicalDeviceImageProcessingPropertiesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageProcessingPropertiesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageProcessingPropertiesQCOM* s);
    ~MarshalVkPhysicalDeviceImageProcessingPropertiesQCOM();
};

class MarshalVkPhysicalDeviceTilePropertiesFeaturesQCOM {
public:
    MarshalVkPhysicalDeviceTilePropertiesFeaturesQCOM() {}
    VkPhysicalDeviceTilePropertiesFeaturesQCOM s;
    MarshalVkPhysicalDeviceTilePropertiesFeaturesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTilePropertiesFeaturesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceTilePropertiesFeaturesQCOM* s);
    ~MarshalVkPhysicalDeviceTilePropertiesFeaturesQCOM();
};

class MarshalVkTilePropertiesQCOM {
public:
    MarshalVkTilePropertiesQCOM() {}
    VkTilePropertiesQCOM s;
    MarshalVkTilePropertiesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkTilePropertiesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkTilePropertiesQCOM* s);
    ~MarshalVkTilePropertiesQCOM();
};

class MarshalVkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT {
public:
    MarshalVkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT() {}
    VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT s;
    MarshalVkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT();
};

class MarshalVkPhysicalDeviceDepthClampZeroOneFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDepthClampZeroOneFeaturesEXT() {}
    VkPhysicalDeviceDepthClampZeroOneFeaturesEXT s;
    MarshalVkPhysicalDeviceDepthClampZeroOneFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthClampZeroOneFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthClampZeroOneFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDepthClampZeroOneFeaturesEXT();
};

class MarshalVkPhysicalDeviceAddressBindingReportFeaturesEXT {
public:
    MarshalVkPhysicalDeviceAddressBindingReportFeaturesEXT() {}
    VkPhysicalDeviceAddressBindingReportFeaturesEXT s;
    MarshalVkPhysicalDeviceAddressBindingReportFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAddressBindingReportFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAddressBindingReportFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceAddressBindingReportFeaturesEXT();
};

class MarshalVkDeviceAddressBindingCallbackDataEXT {
public:
    MarshalVkDeviceAddressBindingCallbackDataEXT() {}
    VkDeviceAddressBindingCallbackDataEXT s;
    MarshalVkDeviceAddressBindingCallbackDataEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceAddressBindingCallbackDataEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceAddressBindingCallbackDataEXT* s);
    ~MarshalVkDeviceAddressBindingCallbackDataEXT();
};

class MarshalVkPhysicalDeviceOpticalFlowFeaturesNV {
public:
    MarshalVkPhysicalDeviceOpticalFlowFeaturesNV() {}
    VkPhysicalDeviceOpticalFlowFeaturesNV s;
    MarshalVkPhysicalDeviceOpticalFlowFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceOpticalFlowFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceOpticalFlowFeaturesNV* s);
    ~MarshalVkPhysicalDeviceOpticalFlowFeaturesNV();
};

class MarshalVkPhysicalDeviceOpticalFlowPropertiesNV {
public:
    MarshalVkPhysicalDeviceOpticalFlowPropertiesNV() {}
    VkPhysicalDeviceOpticalFlowPropertiesNV s;
    MarshalVkPhysicalDeviceOpticalFlowPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceOpticalFlowPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceOpticalFlowPropertiesNV* s);
    ~MarshalVkPhysicalDeviceOpticalFlowPropertiesNV();
};

class MarshalVkOpticalFlowImageFormatInfoNV {
public:
    MarshalVkOpticalFlowImageFormatInfoNV() {}
    VkOpticalFlowImageFormatInfoNV s;
    MarshalVkOpticalFlowImageFormatInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowImageFormatInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowImageFormatInfoNV* s);
    ~MarshalVkOpticalFlowImageFormatInfoNV();
};

class MarshalVkOpticalFlowImageFormatPropertiesNV {
public:
    MarshalVkOpticalFlowImageFormatPropertiesNV() {}
    VkOpticalFlowImageFormatPropertiesNV s;
    MarshalVkOpticalFlowImageFormatPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowImageFormatPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowImageFormatPropertiesNV* s);
    ~MarshalVkOpticalFlowImageFormatPropertiesNV();
};

class MarshalVkOpticalFlowSessionCreateInfoNV {
public:
    MarshalVkOpticalFlowSessionCreateInfoNV() {}
    VkOpticalFlowSessionCreateInfoNV s;
    MarshalVkOpticalFlowSessionCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowSessionCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowSessionCreateInfoNV* s);
    ~MarshalVkOpticalFlowSessionCreateInfoNV();
};

class MarshalVkOpticalFlowSessionCreatePrivateDataInfoNV {
public:
    MarshalVkOpticalFlowSessionCreatePrivateDataInfoNV() {}
    VkOpticalFlowSessionCreatePrivateDataInfoNV s;
    MarshalVkOpticalFlowSessionCreatePrivateDataInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowSessionCreatePrivateDataInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowSessionCreatePrivateDataInfoNV* s);
};

class MarshalVkOpticalFlowExecuteInfoNV {
public:
    MarshalVkOpticalFlowExecuteInfoNV() {}
    VkOpticalFlowExecuteInfoNV s;
    MarshalVkOpticalFlowExecuteInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowExecuteInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOpticalFlowExecuteInfoNV* s);
    ~MarshalVkOpticalFlowExecuteInfoNV();
};

class MarshalVkPhysicalDeviceFaultFeaturesEXT {
public:
    MarshalVkPhysicalDeviceFaultFeaturesEXT() {}
    VkPhysicalDeviceFaultFeaturesEXT s;
    MarshalVkPhysicalDeviceFaultFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFaultFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFaultFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceFaultFeaturesEXT();
};

class MarshalVkDeviceFaultCountsEXT {
public:
    MarshalVkDeviceFaultCountsEXT() {}
    VkDeviceFaultCountsEXT s;
    MarshalVkDeviceFaultCountsEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceFaultCountsEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceFaultCountsEXT* s);
    ~MarshalVkDeviceFaultCountsEXT();
};

class MarshalVkDeviceFaultInfoEXT {
public:
    MarshalVkDeviceFaultInfoEXT() {}
    VkDeviceFaultInfoEXT s;
    MarshalVkDeviceFaultInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceFaultInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceFaultInfoEXT* s);
};

class MarshalVkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT {
public:
    MarshalVkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT() {}
    VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT s;
    MarshalVkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT* s);
    ~MarshalVkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT();
};

class MarshalVkDepthBiasInfoEXT {
public:
    MarshalVkDepthBiasInfoEXT() {}
    VkDepthBiasInfoEXT s;
    MarshalVkDepthBiasInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDepthBiasInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDepthBiasInfoEXT* s);
    ~MarshalVkDepthBiasInfoEXT();
};

class MarshalVkDepthBiasRepresentationInfoEXT {
public:
    MarshalVkDepthBiasRepresentationInfoEXT() {}
    VkDepthBiasRepresentationInfoEXT s;
    MarshalVkDepthBiasRepresentationInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDepthBiasRepresentationInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDepthBiasRepresentationInfoEXT* s);
    ~MarshalVkDepthBiasRepresentationInfoEXT();
};

class MarshalVkDecompressMemoryRegionNV {
public:
    MarshalVkDecompressMemoryRegionNV() {}
    VkDecompressMemoryRegionNV s;
    MarshalVkDecompressMemoryRegionNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDecompressMemoryRegionNV* s);
};

class MarshalVkPhysicalDeviceShaderCoreBuiltinsPropertiesARM {
public:
    MarshalVkPhysicalDeviceShaderCoreBuiltinsPropertiesARM() {}
    VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM s;
    MarshalVkPhysicalDeviceShaderCoreBuiltinsPropertiesARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM* s);
    ~MarshalVkPhysicalDeviceShaderCoreBuiltinsPropertiesARM();
};

class MarshalVkPhysicalDeviceShaderCoreBuiltinsFeaturesARM {
public:
    MarshalVkPhysicalDeviceShaderCoreBuiltinsFeaturesARM() {}
    VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM s;
    MarshalVkPhysicalDeviceShaderCoreBuiltinsFeaturesARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM* s);
    ~MarshalVkPhysicalDeviceShaderCoreBuiltinsFeaturesARM();
};

class MarshalVkFrameBoundaryEXT {
public:
    MarshalVkFrameBoundaryEXT() {}
    VkFrameBoundaryEXT s;
    MarshalVkFrameBoundaryEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFrameBoundaryEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkFrameBoundaryEXT* s);
    ~MarshalVkFrameBoundaryEXT();
};

class MarshalVkPhysicalDeviceFrameBoundaryFeaturesEXT {
public:
    MarshalVkPhysicalDeviceFrameBoundaryFeaturesEXT() {}
    VkPhysicalDeviceFrameBoundaryFeaturesEXT s;
    MarshalVkPhysicalDeviceFrameBoundaryFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFrameBoundaryFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceFrameBoundaryFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceFrameBoundaryFeaturesEXT();
};

class MarshalVkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT() {}
    VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT s;
    MarshalVkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT();
};

class MarshalVkSurfacePresentModeEXT {
public:
    MarshalVkSurfacePresentModeEXT() {}
    VkSurfacePresentModeEXT s;
    MarshalVkSurfacePresentModeEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfacePresentModeEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfacePresentModeEXT* s);
    ~MarshalVkSurfacePresentModeEXT();
};

class MarshalVkSurfacePresentScalingCapabilitiesEXT {
public:
    MarshalVkSurfacePresentScalingCapabilitiesEXT() {}
    VkSurfacePresentScalingCapabilitiesEXT s;
    MarshalVkSurfacePresentScalingCapabilitiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfacePresentScalingCapabilitiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfacePresentScalingCapabilitiesEXT* s);
    ~MarshalVkSurfacePresentScalingCapabilitiesEXT();
};

class MarshalVkSurfacePresentModeCompatibilityEXT {
public:
    MarshalVkSurfacePresentModeCompatibilityEXT() {}
    VkSurfacePresentModeCompatibilityEXT s;
    MarshalVkSurfacePresentModeCompatibilityEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfacePresentModeCompatibilityEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSurfacePresentModeCompatibilityEXT* s);
    ~MarshalVkSurfacePresentModeCompatibilityEXT();
};

class MarshalVkPhysicalDeviceSwapchainMaintenance1FeaturesEXT {
public:
    MarshalVkPhysicalDeviceSwapchainMaintenance1FeaturesEXT() {}
    VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT s;
    MarshalVkPhysicalDeviceSwapchainMaintenance1FeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceSwapchainMaintenance1FeaturesEXT();
};

class MarshalVkSwapchainPresentFenceInfoEXT {
public:
    MarshalVkSwapchainPresentFenceInfoEXT() {}
    VkSwapchainPresentFenceInfoEXT s;
    MarshalVkSwapchainPresentFenceInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentFenceInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentFenceInfoEXT* s);
    ~MarshalVkSwapchainPresentFenceInfoEXT();
};

class MarshalVkSwapchainPresentModesCreateInfoEXT {
public:
    MarshalVkSwapchainPresentModesCreateInfoEXT() {}
    VkSwapchainPresentModesCreateInfoEXT s;
    MarshalVkSwapchainPresentModesCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentModesCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentModesCreateInfoEXT* s);
    ~MarshalVkSwapchainPresentModesCreateInfoEXT();
};

class MarshalVkSwapchainPresentModeInfoEXT {
public:
    MarshalVkSwapchainPresentModeInfoEXT() {}
    VkSwapchainPresentModeInfoEXT s;
    MarshalVkSwapchainPresentModeInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentModeInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentModeInfoEXT* s);
    ~MarshalVkSwapchainPresentModeInfoEXT();
};

class MarshalVkSwapchainPresentScalingCreateInfoEXT {
public:
    MarshalVkSwapchainPresentScalingCreateInfoEXT() {}
    VkSwapchainPresentScalingCreateInfoEXT s;
    MarshalVkSwapchainPresentScalingCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentScalingCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainPresentScalingCreateInfoEXT* s);
    ~MarshalVkSwapchainPresentScalingCreateInfoEXT();
};

class MarshalVkReleaseSwapchainImagesInfoEXT {
public:
    MarshalVkReleaseSwapchainImagesInfoEXT() {}
    VkReleaseSwapchainImagesInfoEXT s;
    MarshalVkReleaseSwapchainImagesInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkReleaseSwapchainImagesInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkReleaseSwapchainImagesInfoEXT* s);
    ~MarshalVkReleaseSwapchainImagesInfoEXT();
};

class MarshalVkPhysicalDeviceDepthBiasControlFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDepthBiasControlFeaturesEXT() {}
    VkPhysicalDeviceDepthBiasControlFeaturesEXT s;
    MarshalVkPhysicalDeviceDepthBiasControlFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthBiasControlFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDepthBiasControlFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDepthBiasControlFeaturesEXT();
};

class MarshalVkPhysicalDeviceRayTracingInvocationReorderFeaturesNV {
public:
    MarshalVkPhysicalDeviceRayTracingInvocationReorderFeaturesNV() {}
    VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV s;
    MarshalVkPhysicalDeviceRayTracingInvocationReorderFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV* s);
    ~MarshalVkPhysicalDeviceRayTracingInvocationReorderFeaturesNV();
};

class MarshalVkPhysicalDeviceRayTracingInvocationReorderPropertiesNV {
public:
    MarshalVkPhysicalDeviceRayTracingInvocationReorderPropertiesNV() {}
    VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV s;
    MarshalVkPhysicalDeviceRayTracingInvocationReorderPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV* s);
    ~MarshalVkPhysicalDeviceRayTracingInvocationReorderPropertiesNV();
};

class MarshalVkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV {
public:
    MarshalVkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV() {}
    VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV s;
    MarshalVkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV* s);
    ~MarshalVkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV();
};

class MarshalVkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV {
public:
    MarshalVkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV() {}
    VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV s;
    MarshalVkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV* s);
    ~MarshalVkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV();
};

class MarshalVkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM {
public:
    MarshalVkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM() {}
    VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM s;
    MarshalVkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM* s);
    ~MarshalVkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM();
};

class MarshalVkPhysicalDeviceRayTracingPositionFetchFeaturesKHR {
public:
    MarshalVkPhysicalDeviceRayTracingPositionFetchFeaturesKHR() {}
    VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR s;
    MarshalVkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceRayTracingPositionFetchFeaturesKHR();
};

class MarshalVkDeviceImageSubresourceInfo {
public:
    MarshalVkDeviceImageSubresourceInfo() {}
    VkDeviceImageSubresourceInfo s;
    MarshalVkDeviceImageSubresourceInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceImageSubresourceInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceImageSubresourceInfo* s);
    ~MarshalVkDeviceImageSubresourceInfo();
};

class MarshalVkPhysicalDeviceShaderCorePropertiesARM {
public:
    MarshalVkPhysicalDeviceShaderCorePropertiesARM() {}
    VkPhysicalDeviceShaderCorePropertiesARM s;
    MarshalVkPhysicalDeviceShaderCorePropertiesARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCorePropertiesARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderCorePropertiesARM* s);
    ~MarshalVkPhysicalDeviceShaderCorePropertiesARM();
};

class MarshalVkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM {
public:
    MarshalVkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM() {}
    VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM s;
    MarshalVkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM* s);
    ~MarshalVkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM();
};

class MarshalVkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM {
public:
    MarshalVkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM() {}
    VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM s;
    MarshalVkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM* s);
    ~MarshalVkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM();
};

class MarshalVkQueryLowLatencySupportNV {
public:
    MarshalVkQueryLowLatencySupportNV() {}
    VkQueryLowLatencySupportNV s;
    MarshalVkQueryLowLatencySupportNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryLowLatencySupportNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkQueryLowLatencySupportNV* s);
};

class MarshalVkMemoryMapInfo {
public:
    MarshalVkMemoryMapInfo() {}
    VkMemoryMapInfo s;
    MarshalVkMemoryMapInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryMapInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryMapInfo* s);
    ~MarshalVkMemoryMapInfo();
};

class MarshalVkMemoryUnmapInfo {
public:
    MarshalVkMemoryUnmapInfo() {}
    VkMemoryUnmapInfo s;
    MarshalVkMemoryUnmapInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryUnmapInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryUnmapInfo* s);
    ~MarshalVkMemoryUnmapInfo();
};

class MarshalVkPhysicalDeviceShaderObjectFeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderObjectFeaturesEXT() {}
    VkPhysicalDeviceShaderObjectFeaturesEXT s;
    MarshalVkPhysicalDeviceShaderObjectFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderObjectFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderObjectFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderObjectFeaturesEXT();
};

class MarshalVkPhysicalDeviceShaderObjectPropertiesEXT {
public:
    MarshalVkPhysicalDeviceShaderObjectPropertiesEXT() {}
    VkPhysicalDeviceShaderObjectPropertiesEXT s;
    MarshalVkPhysicalDeviceShaderObjectPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderObjectPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderObjectPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceShaderObjectPropertiesEXT();
};

class MarshalVkShaderCreateInfoEXT {
public:
    MarshalVkShaderCreateInfoEXT() {}
    VkShaderCreateInfoEXT s;
    MarshalVkShaderCreateInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkShaderCreateInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkShaderCreateInfoEXT* s);
    ~MarshalVkShaderCreateInfoEXT();
};

class MarshalVkPhysicalDeviceShaderTileImageFeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderTileImageFeaturesEXT() {}
    VkPhysicalDeviceShaderTileImageFeaturesEXT s;
    MarshalVkPhysicalDeviceShaderTileImageFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderTileImageFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderTileImageFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderTileImageFeaturesEXT();
};

class MarshalVkPhysicalDeviceShaderTileImagePropertiesEXT {
public:
    MarshalVkPhysicalDeviceShaderTileImagePropertiesEXT() {}
    VkPhysicalDeviceShaderTileImagePropertiesEXT s;
    MarshalVkPhysicalDeviceShaderTileImagePropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderTileImagePropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderTileImagePropertiesEXT* s);
    ~MarshalVkPhysicalDeviceShaderTileImagePropertiesEXT();
};

class MarshalVkPhysicalDeviceCooperativeMatrixFeaturesKHR {
public:
    MarshalVkPhysicalDeviceCooperativeMatrixFeaturesKHR() {}
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR s;
    MarshalVkPhysicalDeviceCooperativeMatrixFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceCooperativeMatrixFeaturesKHR();
};

class MarshalVkCooperativeMatrixPropertiesKHR {
public:
    MarshalVkCooperativeMatrixPropertiesKHR() {}
    VkCooperativeMatrixPropertiesKHR s;
    MarshalVkCooperativeMatrixPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCooperativeMatrixPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCooperativeMatrixPropertiesKHR* s);
    ~MarshalVkCooperativeMatrixPropertiesKHR();
};

class MarshalVkPhysicalDeviceCooperativeMatrixPropertiesKHR {
public:
    MarshalVkPhysicalDeviceCooperativeMatrixPropertiesKHR() {}
    VkPhysicalDeviceCooperativeMatrixPropertiesKHR s;
    MarshalVkPhysicalDeviceCooperativeMatrixPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixPropertiesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixPropertiesKHR* s);
    ~MarshalVkPhysicalDeviceCooperativeMatrixPropertiesKHR();
};

class MarshalVkPhysicalDeviceAntiLagFeaturesAMD {
public:
    MarshalVkPhysicalDeviceAntiLagFeaturesAMD() {}
    VkPhysicalDeviceAntiLagFeaturesAMD s;
    MarshalVkPhysicalDeviceAntiLagFeaturesAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAntiLagFeaturesAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceAntiLagFeaturesAMD* s);
    ~MarshalVkPhysicalDeviceAntiLagFeaturesAMD();
};

class MarshalVkAntiLagDataAMD {
public:
    MarshalVkAntiLagDataAMD() {}
    VkAntiLagDataAMD s;
    MarshalVkAntiLagDataAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAntiLagDataAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAntiLagDataAMD* s);
    ~MarshalVkAntiLagDataAMD();
};

class MarshalVkAntiLagPresentationInfoAMD {
public:
    MarshalVkAntiLagPresentationInfoAMD() {}
    VkAntiLagPresentationInfoAMD s;
    MarshalVkAntiLagPresentationInfoAMD(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAntiLagPresentationInfoAMD* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkAntiLagPresentationInfoAMD* s);
    ~MarshalVkAntiLagPresentationInfoAMD();
};

class MarshalVkBindMemoryStatus {
public:
    MarshalVkBindMemoryStatus() {}
    VkBindMemoryStatus s;
    MarshalVkBindMemoryStatus(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindMemoryStatus* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindMemoryStatus* s);
};

class MarshalVkBindDescriptorSetsInfo {
public:
    MarshalVkBindDescriptorSetsInfo() {}
    VkBindDescriptorSetsInfo s;
    MarshalVkBindDescriptorSetsInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindDescriptorSetsInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindDescriptorSetsInfo* s);
    ~MarshalVkBindDescriptorSetsInfo();
};

class MarshalVkPushConstantsInfo {
public:
    MarshalVkPushConstantsInfo() {}
    VkPushConstantsInfo s;
    MarshalVkPushConstantsInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPushConstantsInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPushConstantsInfo* s);
    ~MarshalVkPushConstantsInfo();
};

class MarshalVkPushDescriptorSetInfo {
public:
    MarshalVkPushDescriptorSetInfo() {}
    VkPushDescriptorSetInfo s;
    MarshalVkPushDescriptorSetInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPushDescriptorSetInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPushDescriptorSetInfo* s);
    ~MarshalVkPushDescriptorSetInfo();
};

class MarshalVkPushDescriptorSetWithTemplateInfo {
public:
    MarshalVkPushDescriptorSetWithTemplateInfo() {}
    VkPushDescriptorSetWithTemplateInfo s;
    MarshalVkPushDescriptorSetWithTemplateInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPushDescriptorSetWithTemplateInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPushDescriptorSetWithTemplateInfo* s);
};

class MarshalVkSetDescriptorBufferOffsetsInfoEXT {
public:
    MarshalVkSetDescriptorBufferOffsetsInfoEXT() {}
    VkSetDescriptorBufferOffsetsInfoEXT s;
    MarshalVkSetDescriptorBufferOffsetsInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSetDescriptorBufferOffsetsInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSetDescriptorBufferOffsetsInfoEXT* s);
    ~MarshalVkSetDescriptorBufferOffsetsInfoEXT();
};

class MarshalVkBindDescriptorBufferEmbeddedSamplersInfoEXT {
public:
    MarshalVkBindDescriptorBufferEmbeddedSamplersInfoEXT() {}
    VkBindDescriptorBufferEmbeddedSamplersInfoEXT s;
    MarshalVkBindDescriptorBufferEmbeddedSamplersInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindDescriptorBufferEmbeddedSamplersInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBindDescriptorBufferEmbeddedSamplersInfoEXT* s);
    ~MarshalVkBindDescriptorBufferEmbeddedSamplersInfoEXT();
};

class MarshalVkPhysicalDeviceCubicClampFeaturesQCOM {
public:
    MarshalVkPhysicalDeviceCubicClampFeaturesQCOM() {}
    VkPhysicalDeviceCubicClampFeaturesQCOM s;
    MarshalVkPhysicalDeviceCubicClampFeaturesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCubicClampFeaturesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCubicClampFeaturesQCOM* s);
    ~MarshalVkPhysicalDeviceCubicClampFeaturesQCOM();
};

class MarshalVkPhysicalDeviceYcbcrDegammaFeaturesQCOM {
public:
    MarshalVkPhysicalDeviceYcbcrDegammaFeaturesQCOM() {}
    VkPhysicalDeviceYcbcrDegammaFeaturesQCOM s;
    MarshalVkPhysicalDeviceYcbcrDegammaFeaturesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceYcbcrDegammaFeaturesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceYcbcrDegammaFeaturesQCOM* s);
    ~MarshalVkPhysicalDeviceYcbcrDegammaFeaturesQCOM();
};

class MarshalVkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM {
public:
    MarshalVkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM() {}
    VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM s;
    MarshalVkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM* s);
    ~MarshalVkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM();
};

class MarshalVkPhysicalDeviceCubicWeightsFeaturesQCOM {
public:
    MarshalVkPhysicalDeviceCubicWeightsFeaturesQCOM() {}
    VkPhysicalDeviceCubicWeightsFeaturesQCOM s;
    MarshalVkPhysicalDeviceCubicWeightsFeaturesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCubicWeightsFeaturesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCubicWeightsFeaturesQCOM* s);
    ~MarshalVkPhysicalDeviceCubicWeightsFeaturesQCOM();
};

class MarshalVkSamplerCubicWeightsCreateInfoQCOM {
public:
    MarshalVkSamplerCubicWeightsCreateInfoQCOM() {}
    VkSamplerCubicWeightsCreateInfoQCOM s;
    MarshalVkSamplerCubicWeightsCreateInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerCubicWeightsCreateInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerCubicWeightsCreateInfoQCOM* s);
    ~MarshalVkSamplerCubicWeightsCreateInfoQCOM();
};

class MarshalVkBlitImageCubicWeightsInfoQCOM {
public:
    MarshalVkBlitImageCubicWeightsInfoQCOM() {}
    VkBlitImageCubicWeightsInfoQCOM s;
    MarshalVkBlitImageCubicWeightsInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBlitImageCubicWeightsInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkBlitImageCubicWeightsInfoQCOM* s);
    ~MarshalVkBlitImageCubicWeightsInfoQCOM();
};

class MarshalVkPhysicalDeviceImageProcessing2FeaturesQCOM {
public:
    MarshalVkPhysicalDeviceImageProcessing2FeaturesQCOM() {}
    VkPhysicalDeviceImageProcessing2FeaturesQCOM s;
    MarshalVkPhysicalDeviceImageProcessing2FeaturesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageProcessing2FeaturesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageProcessing2FeaturesQCOM* s);
    ~MarshalVkPhysicalDeviceImageProcessing2FeaturesQCOM();
};

class MarshalVkPhysicalDeviceImageProcessing2PropertiesQCOM {
public:
    MarshalVkPhysicalDeviceImageProcessing2PropertiesQCOM() {}
    VkPhysicalDeviceImageProcessing2PropertiesQCOM s;
    MarshalVkPhysicalDeviceImageProcessing2PropertiesQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageProcessing2PropertiesQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageProcessing2PropertiesQCOM* s);
    ~MarshalVkPhysicalDeviceImageProcessing2PropertiesQCOM();
};

class MarshalVkSamplerBlockMatchWindowCreateInfoQCOM {
public:
    MarshalVkSamplerBlockMatchWindowCreateInfoQCOM() {}
    VkSamplerBlockMatchWindowCreateInfoQCOM s;
    MarshalVkSamplerBlockMatchWindowCreateInfoQCOM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerBlockMatchWindowCreateInfoQCOM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSamplerBlockMatchWindowCreateInfoQCOM* s);
    ~MarshalVkSamplerBlockMatchWindowCreateInfoQCOM();
};

class MarshalVkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV {
public:
    MarshalVkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV() {}
    VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV s;
    MarshalVkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV* s);
    ~MarshalVkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV();
};

class MarshalVkPhysicalDeviceLayeredDriverPropertiesMSFT {
public:
    MarshalVkPhysicalDeviceLayeredDriverPropertiesMSFT() {}
    VkPhysicalDeviceLayeredDriverPropertiesMSFT s;
    MarshalVkPhysicalDeviceLayeredDriverPropertiesMSFT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLayeredDriverPropertiesMSFT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceLayeredDriverPropertiesMSFT* s);
    ~MarshalVkPhysicalDeviceLayeredDriverPropertiesMSFT();
};

class MarshalVkPhysicalDevicePerStageDescriptorSetFeaturesNV {
public:
    MarshalVkPhysicalDevicePerStageDescriptorSetFeaturesNV() {}
    VkPhysicalDevicePerStageDescriptorSetFeaturesNV s;
    MarshalVkPhysicalDevicePerStageDescriptorSetFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePerStageDescriptorSetFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePerStageDescriptorSetFeaturesNV* s);
    ~MarshalVkPhysicalDevicePerStageDescriptorSetFeaturesNV();
};

class MarshalVkLatencySleepModeInfoNV {
public:
    MarshalVkLatencySleepModeInfoNV() {}
    VkLatencySleepModeInfoNV s;
    MarshalVkLatencySleepModeInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencySleepModeInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencySleepModeInfoNV* s);
    ~MarshalVkLatencySleepModeInfoNV();
};

class MarshalVkLatencySleepInfoNV {
public:
    MarshalVkLatencySleepInfoNV() {}
    VkLatencySleepInfoNV s;
    MarshalVkLatencySleepInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencySleepInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencySleepInfoNV* s);
    ~MarshalVkLatencySleepInfoNV();
};

class MarshalVkSetLatencyMarkerInfoNV {
public:
    MarshalVkSetLatencyMarkerInfoNV() {}
    VkSetLatencyMarkerInfoNV s;
    MarshalVkSetLatencyMarkerInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSetLatencyMarkerInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSetLatencyMarkerInfoNV* s);
    ~MarshalVkSetLatencyMarkerInfoNV();
};

class MarshalVkGetLatencyMarkerInfoNV {
public:
    MarshalVkGetLatencyMarkerInfoNV() {}
    VkGetLatencyMarkerInfoNV s;
    MarshalVkGetLatencyMarkerInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGetLatencyMarkerInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkGetLatencyMarkerInfoNV* s);
    ~MarshalVkGetLatencyMarkerInfoNV();
};

class MarshalVkLatencyTimingsFrameReportNV {
public:
    MarshalVkLatencyTimingsFrameReportNV() {}
    VkLatencyTimingsFrameReportNV s;
    MarshalVkLatencyTimingsFrameReportNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencyTimingsFrameReportNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencyTimingsFrameReportNV* s);
    ~MarshalVkLatencyTimingsFrameReportNV();
};

class MarshalVkOutOfBandQueueTypeInfoNV {
public:
    MarshalVkOutOfBandQueueTypeInfoNV() {}
    VkOutOfBandQueueTypeInfoNV s;
    MarshalVkOutOfBandQueueTypeInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOutOfBandQueueTypeInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkOutOfBandQueueTypeInfoNV* s);
    ~MarshalVkOutOfBandQueueTypeInfoNV();
};

class MarshalVkLatencySubmissionPresentIdNV {
public:
    MarshalVkLatencySubmissionPresentIdNV() {}
    VkLatencySubmissionPresentIdNV s;
    MarshalVkLatencySubmissionPresentIdNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencySubmissionPresentIdNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencySubmissionPresentIdNV* s);
    ~MarshalVkLatencySubmissionPresentIdNV();
};

class MarshalVkSwapchainLatencyCreateInfoNV {
public:
    MarshalVkSwapchainLatencyCreateInfoNV() {}
    VkSwapchainLatencyCreateInfoNV s;
    MarshalVkSwapchainLatencyCreateInfoNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainLatencyCreateInfoNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkSwapchainLatencyCreateInfoNV* s);
    ~MarshalVkSwapchainLatencyCreateInfoNV();
};

class MarshalVkLatencySurfaceCapabilitiesNV {
public:
    MarshalVkLatencySurfaceCapabilitiesNV() {}
    VkLatencySurfaceCapabilitiesNV s;
    MarshalVkLatencySurfaceCapabilitiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencySurfaceCapabilitiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkLatencySurfaceCapabilitiesNV* s);
    ~MarshalVkLatencySurfaceCapabilitiesNV();
};

class MarshalVkPhysicalDeviceCudaKernelLaunchFeaturesNV {
public:
    MarshalVkPhysicalDeviceCudaKernelLaunchFeaturesNV() {}
    VkPhysicalDeviceCudaKernelLaunchFeaturesNV s;
    MarshalVkPhysicalDeviceCudaKernelLaunchFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCudaKernelLaunchFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCudaKernelLaunchFeaturesNV* s);
    ~MarshalVkPhysicalDeviceCudaKernelLaunchFeaturesNV();
};

class MarshalVkPhysicalDeviceCudaKernelLaunchPropertiesNV {
public:
    MarshalVkPhysicalDeviceCudaKernelLaunchPropertiesNV() {}
    VkPhysicalDeviceCudaKernelLaunchPropertiesNV s;
    MarshalVkPhysicalDeviceCudaKernelLaunchPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCudaKernelLaunchPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCudaKernelLaunchPropertiesNV* s);
    ~MarshalVkPhysicalDeviceCudaKernelLaunchPropertiesNV();
};

class MarshalVkDeviceQueueShaderCoreControlCreateInfoARM {
public:
    MarshalVkDeviceQueueShaderCoreControlCreateInfoARM() {}
    VkDeviceQueueShaderCoreControlCreateInfoARM s;
    MarshalVkDeviceQueueShaderCoreControlCreateInfoARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceQueueShaderCoreControlCreateInfoARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDeviceQueueShaderCoreControlCreateInfoARM* s);
    ~MarshalVkDeviceQueueShaderCoreControlCreateInfoARM();
};

class MarshalVkPhysicalDeviceSchedulingControlsFeaturesARM {
public:
    MarshalVkPhysicalDeviceSchedulingControlsFeaturesARM() {}
    VkPhysicalDeviceSchedulingControlsFeaturesARM s;
    MarshalVkPhysicalDeviceSchedulingControlsFeaturesARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSchedulingControlsFeaturesARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSchedulingControlsFeaturesARM* s);
    ~MarshalVkPhysicalDeviceSchedulingControlsFeaturesARM();
};

class MarshalVkPhysicalDeviceSchedulingControlsPropertiesARM {
public:
    MarshalVkPhysicalDeviceSchedulingControlsPropertiesARM() {}
    VkPhysicalDeviceSchedulingControlsPropertiesARM s;
    MarshalVkPhysicalDeviceSchedulingControlsPropertiesARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSchedulingControlsPropertiesARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceSchedulingControlsPropertiesARM* s);
    ~MarshalVkPhysicalDeviceSchedulingControlsPropertiesARM();
};

class MarshalVkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG {
public:
    MarshalVkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG() {}
    VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG s;
    MarshalVkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG* s);
    ~MarshalVkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG();
};

class MarshalVkPhysicalDeviceRenderPassStripedFeaturesARM {
public:
    MarshalVkPhysicalDeviceRenderPassStripedFeaturesARM() {}
    VkPhysicalDeviceRenderPassStripedFeaturesARM s;
    MarshalVkPhysicalDeviceRenderPassStripedFeaturesARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRenderPassStripedFeaturesARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRenderPassStripedFeaturesARM* s);
    ~MarshalVkPhysicalDeviceRenderPassStripedFeaturesARM();
};

class MarshalVkPhysicalDeviceRenderPassStripedPropertiesARM {
public:
    MarshalVkPhysicalDeviceRenderPassStripedPropertiesARM() {}
    VkPhysicalDeviceRenderPassStripedPropertiesARM s;
    MarshalVkPhysicalDeviceRenderPassStripedPropertiesARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRenderPassStripedPropertiesARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRenderPassStripedPropertiesARM* s);
    ~MarshalVkPhysicalDeviceRenderPassStripedPropertiesARM();
};

class MarshalVkRenderPassStripeInfoARM {
public:
    MarshalVkRenderPassStripeInfoARM() {}
    VkRenderPassStripeInfoARM s;
    MarshalVkRenderPassStripeInfoARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassStripeInfoARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassStripeInfoARM* s);
    ~MarshalVkRenderPassStripeInfoARM();
};

class MarshalVkRenderPassStripeBeginInfoARM {
public:
    MarshalVkRenderPassStripeBeginInfoARM() {}
    VkRenderPassStripeBeginInfoARM s;
    MarshalVkRenderPassStripeBeginInfoARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassStripeBeginInfoARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassStripeBeginInfoARM* s);
    ~MarshalVkRenderPassStripeBeginInfoARM();
};

class MarshalVkRenderPassStripeSubmitInfoARM {
public:
    MarshalVkRenderPassStripeSubmitInfoARM() {}
    VkRenderPassStripeSubmitInfoARM s;
    MarshalVkRenderPassStripeSubmitInfoARM(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassStripeSubmitInfoARM* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderPassStripeSubmitInfoARM* s);
    ~MarshalVkRenderPassStripeSubmitInfoARM();
};

class MarshalVkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR {
public:
    MarshalVkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR() {}
    VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR s;
    MarshalVkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR();
};

class MarshalVkPhysicalDeviceShaderSubgroupRotateFeatures {
public:
    MarshalVkPhysicalDeviceShaderSubgroupRotateFeatures() {}
    VkPhysicalDeviceShaderSubgroupRotateFeatures s;
    MarshalVkPhysicalDeviceShaderSubgroupRotateFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupRotateFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupRotateFeatures* s);
    ~MarshalVkPhysicalDeviceShaderSubgroupRotateFeatures();
};

class MarshalVkPhysicalDeviceShaderExpectAssumeFeatures {
public:
    MarshalVkPhysicalDeviceShaderExpectAssumeFeatures() {}
    VkPhysicalDeviceShaderExpectAssumeFeatures s;
    MarshalVkPhysicalDeviceShaderExpectAssumeFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderExpectAssumeFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderExpectAssumeFeatures* s);
    ~MarshalVkPhysicalDeviceShaderExpectAssumeFeatures();
};

class MarshalVkPhysicalDeviceShaderFloatControls2Features {
public:
    MarshalVkPhysicalDeviceShaderFloatControls2Features() {}
    VkPhysicalDeviceShaderFloatControls2Features s;
    MarshalVkPhysicalDeviceShaderFloatControls2Features(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderFloatControls2Features* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderFloatControls2Features* s);
    ~MarshalVkPhysicalDeviceShaderFloatControls2Features();
};

class MarshalVkPhysicalDeviceDynamicRenderingLocalReadFeatures {
public:
    MarshalVkPhysicalDeviceDynamicRenderingLocalReadFeatures() {}
    VkPhysicalDeviceDynamicRenderingLocalReadFeatures s;
    MarshalVkPhysicalDeviceDynamicRenderingLocalReadFeatures(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDynamicRenderingLocalReadFeatures* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceDynamicRenderingLocalReadFeatures* s);
    ~MarshalVkPhysicalDeviceDynamicRenderingLocalReadFeatures();
};

class MarshalVkRenderingAttachmentLocationInfo {
public:
    MarshalVkRenderingAttachmentLocationInfo() {}
    VkRenderingAttachmentLocationInfo s;
    MarshalVkRenderingAttachmentLocationInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingAttachmentLocationInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingAttachmentLocationInfo* s);
    ~MarshalVkRenderingAttachmentLocationInfo();
};

class MarshalVkRenderingInputAttachmentIndexInfo {
public:
    MarshalVkRenderingInputAttachmentIndexInfo() {}
    VkRenderingInputAttachmentIndexInfo s;
    MarshalVkRenderingInputAttachmentIndexInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingInputAttachmentIndexInfo* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkRenderingInputAttachmentIndexInfo* s);
};

class MarshalVkPhysicalDeviceShaderQuadControlFeaturesKHR {
public:
    MarshalVkPhysicalDeviceShaderQuadControlFeaturesKHR() {}
    VkPhysicalDeviceShaderQuadControlFeaturesKHR s;
    MarshalVkPhysicalDeviceShaderQuadControlFeaturesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderQuadControlFeaturesKHR* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderQuadControlFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceShaderQuadControlFeaturesKHR();
};

class MarshalVkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV {
public:
    MarshalVkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV() {}
    VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV s;
    MarshalVkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV* s);
    ~MarshalVkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV();
};

class MarshalVkPhysicalDeviceMapMemoryPlacedFeaturesEXT {
public:
    MarshalVkPhysicalDeviceMapMemoryPlacedFeaturesEXT() {}
    VkPhysicalDeviceMapMemoryPlacedFeaturesEXT s;
    MarshalVkPhysicalDeviceMapMemoryPlacedFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMapMemoryPlacedFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMapMemoryPlacedFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceMapMemoryPlacedFeaturesEXT();
};

class MarshalVkPhysicalDeviceMapMemoryPlacedPropertiesEXT {
public:
    MarshalVkPhysicalDeviceMapMemoryPlacedPropertiesEXT() {}
    VkPhysicalDeviceMapMemoryPlacedPropertiesEXT s;
    MarshalVkPhysicalDeviceMapMemoryPlacedPropertiesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMapMemoryPlacedPropertiesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceMapMemoryPlacedPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceMapMemoryPlacedPropertiesEXT();
};

class MarshalVkMemoryMapPlacedInfoEXT {
public:
    MarshalVkMemoryMapPlacedInfoEXT() {}
    VkMemoryMapPlacedInfoEXT s;
    MarshalVkMemoryMapPlacedInfoEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryMapPlacedInfoEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkMemoryMapPlacedInfoEXT* s);
};

class MarshalVkPhysicalDeviceRawAccessChainsFeaturesNV {
public:
    MarshalVkPhysicalDeviceRawAccessChainsFeaturesNV() {}
    VkPhysicalDeviceRawAccessChainsFeaturesNV s;
    MarshalVkPhysicalDeviceRawAccessChainsFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRawAccessChainsFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceRawAccessChainsFeaturesNV* s);
    ~MarshalVkPhysicalDeviceRawAccessChainsFeaturesNV();
};

class MarshalVkPhysicalDeviceCommandBufferInheritanceFeaturesNV {
public:
    MarshalVkPhysicalDeviceCommandBufferInheritanceFeaturesNV() {}
    VkPhysicalDeviceCommandBufferInheritanceFeaturesNV s;
    MarshalVkPhysicalDeviceCommandBufferInheritanceFeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCommandBufferInheritanceFeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCommandBufferInheritanceFeaturesNV* s);
    ~MarshalVkPhysicalDeviceCommandBufferInheritanceFeaturesNV();
};

class MarshalVkPhysicalDeviceImageAlignmentControlFeaturesMESA {
public:
    MarshalVkPhysicalDeviceImageAlignmentControlFeaturesMESA() {}
    VkPhysicalDeviceImageAlignmentControlFeaturesMESA s;
    MarshalVkPhysicalDeviceImageAlignmentControlFeaturesMESA(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageAlignmentControlFeaturesMESA* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageAlignmentControlFeaturesMESA* s);
    ~MarshalVkPhysicalDeviceImageAlignmentControlFeaturesMESA();
};

class MarshalVkPhysicalDeviceImageAlignmentControlPropertiesMESA {
public:
    MarshalVkPhysicalDeviceImageAlignmentControlPropertiesMESA() {}
    VkPhysicalDeviceImageAlignmentControlPropertiesMESA s;
    MarshalVkPhysicalDeviceImageAlignmentControlPropertiesMESA(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageAlignmentControlPropertiesMESA* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceImageAlignmentControlPropertiesMESA* s);
    ~MarshalVkPhysicalDeviceImageAlignmentControlPropertiesMESA();
};

class MarshalVkImageAlignmentControlCreateInfoMESA {
public:
    MarshalVkImageAlignmentControlCreateInfoMESA() {}
    VkImageAlignmentControlCreateInfoMESA s;
    MarshalVkImageAlignmentControlCreateInfoMESA(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageAlignmentControlCreateInfoMESA* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkImageAlignmentControlCreateInfoMESA* s);
    ~MarshalVkImageAlignmentControlCreateInfoMESA();
};

class MarshalVkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT() {}
    VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT s;
    MarshalVkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT();
};

class MarshalVkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT {
public:
    MarshalVkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT() {}
    VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT s;
    MarshalVkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT* s);
    ~MarshalVkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT();
};

class MarshalVkDepthClampRangeEXT {
public:
    MarshalVkDepthClampRangeEXT() {}
    VkDepthClampRangeEXT s;
    MarshalVkDepthClampRangeEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDepthClampRangeEXT* s);
};

class MarshalVkPhysicalDeviceCooperativeMatrix2FeaturesNV {
public:
    MarshalVkPhysicalDeviceCooperativeMatrix2FeaturesNV() {}
    VkPhysicalDeviceCooperativeMatrix2FeaturesNV s;
    MarshalVkPhysicalDeviceCooperativeMatrix2FeaturesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrix2FeaturesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrix2FeaturesNV* s);
    ~MarshalVkPhysicalDeviceCooperativeMatrix2FeaturesNV();
};

class MarshalVkPhysicalDeviceCooperativeMatrix2PropertiesNV {
public:
    MarshalVkPhysicalDeviceCooperativeMatrix2PropertiesNV() {}
    VkPhysicalDeviceCooperativeMatrix2PropertiesNV s;
    MarshalVkPhysicalDeviceCooperativeMatrix2PropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrix2PropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrix2PropertiesNV* s);
    ~MarshalVkPhysicalDeviceCooperativeMatrix2PropertiesNV();
};

class MarshalVkCooperativeMatrixFlexibleDimensionsPropertiesNV {
public:
    MarshalVkCooperativeMatrixFlexibleDimensionsPropertiesNV() {}
    VkCooperativeMatrixFlexibleDimensionsPropertiesNV s;
    MarshalVkCooperativeMatrixFlexibleDimensionsPropertiesNV(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* s);
    ~MarshalVkCooperativeMatrixFlexibleDimensionsPropertiesNV();
};

class MarshalVkPhysicalDeviceHdrVividFeaturesHUAWEI {
public:
    MarshalVkPhysicalDeviceHdrVividFeaturesHUAWEI() {}
    VkPhysicalDeviceHdrVividFeaturesHUAWEI s;
    MarshalVkPhysicalDeviceHdrVividFeaturesHUAWEI(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceHdrVividFeaturesHUAWEI* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceHdrVividFeaturesHUAWEI* s);
    ~MarshalVkPhysicalDeviceHdrVividFeaturesHUAWEI();
};

class MarshalVkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT {
public:
    MarshalVkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT() {}
    VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT s;
    MarshalVkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT* s);
    static void write(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT();
};

class MarshalStdVideoEncodeH264WeightTableFlags {
public:
    MarshalStdVideoEncodeH264WeightTableFlags() {}
    StdVideoEncodeH264WeightTableFlags s;
    MarshalStdVideoEncodeH264WeightTableFlags(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH264WeightTableFlags* s);
};

class MarshalStdVideoEncodeH264WeightTable {
public:
    MarshalStdVideoEncodeH264WeightTable() {}
    StdVideoEncodeH264WeightTable s;
    MarshalStdVideoEncodeH264WeightTable(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH264WeightTable* s);
};

class MarshalStdVideoH265SubLayerHrdParameters {
public:
    MarshalStdVideoH265SubLayerHrdParameters() {}
    StdVideoH265SubLayerHrdParameters s;
    MarshalStdVideoH265SubLayerHrdParameters(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265SubLayerHrdParameters* s);
};

class MarshalStdVideoH265DecPicBufMgr {
public:
    MarshalStdVideoH265DecPicBufMgr() {}
    StdVideoH265DecPicBufMgr s;
    MarshalStdVideoH265DecPicBufMgr(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265DecPicBufMgr* s);
};

class MarshalStdVideoH265HrdParameters {
public:
    MarshalStdVideoH265HrdParameters() {}
    StdVideoH265HrdParameters s;
    MarshalStdVideoH265HrdParameters(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265HrdParameters* s);
    ~MarshalStdVideoH265HrdParameters();
};

class MarshalStdVideoH265ProfileTierLevel {
public:
    MarshalStdVideoH265ProfileTierLevel() {}
    StdVideoH265ProfileTierLevel s;
    MarshalStdVideoH265ProfileTierLevel(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265ProfileTierLevel* s);
};

class MarshalStdVideoH265ScalingLists {
public:
    MarshalStdVideoH265ScalingLists() {}
    StdVideoH265ScalingLists s;
    MarshalStdVideoH265ScalingLists(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265ScalingLists* s);
};

class MarshalStdVideoH265ShortTermRefPicSet {
public:
    MarshalStdVideoH265ShortTermRefPicSet() {}
    StdVideoH265ShortTermRefPicSet s;
    MarshalStdVideoH265ShortTermRefPicSet(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265ShortTermRefPicSet* s);
};

class MarshalStdVideoH265LongTermRefPicsSps {
public:
    MarshalStdVideoH265LongTermRefPicsSps() {}
    StdVideoH265LongTermRefPicsSps s;
    MarshalStdVideoH265LongTermRefPicsSps(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265LongTermRefPicsSps* s);
};

class MarshalStdVideoH265PredictorPaletteEntries {
public:
    MarshalStdVideoH265PredictorPaletteEntries() {}
    StdVideoH265PredictorPaletteEntries s;
    MarshalStdVideoH265PredictorPaletteEntries(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265PredictorPaletteEntries* s);
};

class MarshalStdVideoH265SequenceParameterSetVui {
public:
    MarshalStdVideoH265SequenceParameterSetVui() {}
    StdVideoH265SequenceParameterSetVui s;
    MarshalStdVideoH265SequenceParameterSetVui(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH265SequenceParameterSetVui* s);
    ~MarshalStdVideoH265SequenceParameterSetVui();
};

class MarshalStdVideoEncodeH265WeightTableFlags {
public:
    MarshalStdVideoEncodeH265WeightTableFlags() {}
    StdVideoEncodeH265WeightTableFlags s;
    MarshalStdVideoEncodeH265WeightTableFlags(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH265WeightTableFlags* s);
};

class MarshalStdVideoEncodeH265WeightTable {
public:
    MarshalStdVideoEncodeH265WeightTable() {}
    StdVideoEncodeH265WeightTable s;
    MarshalStdVideoEncodeH265WeightTable(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH265WeightTable* s);
};

class MarshalStdVideoEncodeH265ReferenceListsInfo {
public:
    MarshalStdVideoEncodeH265ReferenceListsInfo() {}
    StdVideoEncodeH265ReferenceListsInfo s;
    MarshalStdVideoEncodeH265ReferenceListsInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH265ReferenceListsInfo* s);
};

class MarshalStdVideoEncodeH265LongTermRefPics {
public:
    MarshalStdVideoEncodeH265LongTermRefPics() {}
    StdVideoEncodeH265LongTermRefPics s;
    MarshalStdVideoEncodeH265LongTermRefPics(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH265LongTermRefPics* s);
};

class MarshalStdVideoH264ScalingLists {
public:
    MarshalStdVideoH264ScalingLists() {}
    StdVideoH264ScalingLists s;
    MarshalStdVideoH264ScalingLists(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoH264ScalingLists* s);
};

class MarshalStdVideoEncodeH264RefListModEntry {
public:
    MarshalStdVideoEncodeH264RefListModEntry() {}
    StdVideoEncodeH264RefListModEntry s;
    MarshalStdVideoEncodeH264RefListModEntry(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH264RefListModEntry* s);
};

class MarshalStdVideoEncodeH264RefPicMarkingEntry {
public:
    MarshalStdVideoEncodeH264RefPicMarkingEntry() {}
    StdVideoEncodeH264RefPicMarkingEntry s;
    MarshalStdVideoEncodeH264RefPicMarkingEntry(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH264RefPicMarkingEntry* s);
};

class MarshalStdVideoEncodeH264ReferenceListsInfo {
public:
    MarshalStdVideoEncodeH264ReferenceListsInfo() {}
    StdVideoEncodeH264ReferenceListsInfo s;
    MarshalStdVideoEncodeH264ReferenceListsInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoEncodeH264ReferenceListsInfo* s);
    ~MarshalStdVideoEncodeH264ReferenceListsInfo();
};

class MarshalStdVideoAV1Quantization {
public:
    MarshalStdVideoAV1Quantization() {}
    StdVideoAV1Quantization s;
    MarshalStdVideoAV1Quantization(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1Quantization* s);
};

class MarshalStdVideoAV1TileInfo {
public:
    MarshalStdVideoAV1TileInfo() {}
    StdVideoAV1TileInfo s;
    MarshalStdVideoAV1TileInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1TileInfo* s);
};

class MarshalStdVideoAV1LoopFilter {
public:
    MarshalStdVideoAV1LoopFilter() {}
    StdVideoAV1LoopFilter s;
    MarshalStdVideoAV1LoopFilter(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1LoopFilter* s);
};

class MarshalStdVideoAV1Segmentation {
public:
    MarshalStdVideoAV1Segmentation() {}
    StdVideoAV1Segmentation s;
    MarshalStdVideoAV1Segmentation(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1Segmentation* s);
};

class MarshalStdVideoAV1CDEF {
public:
    MarshalStdVideoAV1CDEF() {}
    StdVideoAV1CDEF s;
    MarshalStdVideoAV1CDEF(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1CDEF* s);
};

class MarshalStdVideoAV1LoopRestoration {
public:
    MarshalStdVideoAV1LoopRestoration() {}
    StdVideoAV1LoopRestoration s;
    MarshalStdVideoAV1LoopRestoration(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1LoopRestoration* s);
};

class MarshalStdVideoAV1GlobalMotion {
public:
    MarshalStdVideoAV1GlobalMotion() {}
    StdVideoAV1GlobalMotion s;
    MarshalStdVideoAV1GlobalMotion(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1GlobalMotion* s);
};

class MarshalStdVideoAV1TimingInfo {
public:
    MarshalStdVideoAV1TimingInfo() {}
    StdVideoAV1TimingInfo s;
    MarshalStdVideoAV1TimingInfo(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1TimingInfo* s);
};

class MarshalStdVideoAV1ColorConfig {
public:
    MarshalStdVideoAV1ColorConfig() {}
    StdVideoAV1ColorConfig s;
    MarshalStdVideoAV1ColorConfig(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1ColorConfig* s);
};

class MarshalStdVideoAV1FilmGrain {
public:
    MarshalStdVideoAV1FilmGrain() {}
    StdVideoAV1FilmGrain s;
    MarshalStdVideoAV1FilmGrain(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, StdVideoAV1FilmGrain* s);
};

class MarshalVkDisplayModePropertiesKHR {
public:
    MarshalVkDisplayModePropertiesKHR() {}
    VkDisplayModePropertiesKHR s;
    MarshalVkDisplayModePropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayModePropertiesKHR* s);
};

class MarshalVkDisplayPropertiesKHR {
public:
    MarshalVkDisplayPropertiesKHR() {}
    VkDisplayPropertiesKHR s;
    MarshalVkDisplayPropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPropertiesKHR* s);
    ~MarshalVkDisplayPropertiesKHR();
};

class MarshalVkDisplayPlaneCapabilitiesKHR {
public:
    MarshalVkDisplayPlaneCapabilitiesKHR() {}
    VkDisplayPlaneCapabilitiesKHR s;
    MarshalVkDisplayPlaneCapabilitiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPlaneCapabilitiesKHR* s);
};

class MarshalVkDisplayPlanePropertiesKHR {
public:
    MarshalVkDisplayPlanePropertiesKHR() {}
    VkDisplayPlanePropertiesKHR s;
    MarshalVkDisplayPlanePropertiesKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayPlanePropertiesKHR* s);
};

class MarshalVkDisplayModeParametersKHR {
public:
    MarshalVkDisplayModeParametersKHR() {}
    VkDisplayModeParametersKHR s;
    MarshalVkDisplayModeParametersKHR(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address) {read(pBoxedInfo, memory, address, &this->s);}
    static void read(BoxedVulkanInfo* pBoxedInfo, KMemory* memory, U32 address, VkDisplayModeParametersKHR* s);
};

