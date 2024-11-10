struct MarshalFloat {
    union {
        float f;
        U32   i;
    };
};
class MarshalVkViewport {
public:
    MarshalVkViewport() {}
    VkViewport s;
    MarshalVkViewport(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkViewport* s);
};

class MarshalVkRect2D {
public:
    MarshalVkRect2D() {}
    VkRect2D s;
    MarshalVkRect2D(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRect2D* s);
};

class MarshalVkComponentMapping {
public:
    MarshalVkComponentMapping() {}
    VkComponentMapping s;
    MarshalVkComponentMapping(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkComponentMapping* s);
};

class MarshalVkPhysicalDeviceProperties {
public:
    MarshalVkPhysicalDeviceProperties() {}
    VkPhysicalDeviceProperties s;
    MarshalVkPhysicalDeviceProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceProperties* s);
};

class MarshalVkApplicationInfo {
public:
    MarshalVkApplicationInfo() {}
    VkApplicationInfo s;
    MarshalVkApplicationInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkApplicationInfo* s);
    static void write(KMemory* memory, U32 address, VkApplicationInfo* s);
    ~MarshalVkApplicationInfo();
};

class MarshalVkDeviceQueueCreateInfo {
public:
    MarshalVkDeviceQueueCreateInfo() {}
    VkDeviceQueueCreateInfo s;
    MarshalVkDeviceQueueCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceQueueCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkDeviceQueueCreateInfo* s);
    ~MarshalVkDeviceQueueCreateInfo();
};

class MarshalVkDeviceCreateInfo {
public:
    MarshalVkDeviceCreateInfo() {}
    VkDeviceCreateInfo s;
    MarshalVkDeviceCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkDeviceCreateInfo* s);
    ~MarshalVkDeviceCreateInfo();
};

class MarshalVkInstanceCreateInfo {
public:
    MarshalVkInstanceCreateInfo() {}
    VkInstanceCreateInfo s;
    MarshalVkInstanceCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkInstanceCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkInstanceCreateInfo* s);
    ~MarshalVkInstanceCreateInfo();
};

class MarshalVkMemoryAllocateInfo {
public:
    MarshalVkMemoryAllocateInfo() {}
    VkMemoryAllocateInfo s;
    MarshalVkMemoryAllocateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryAllocateInfo* s);
    static void write(KMemory* memory, U32 address, VkMemoryAllocateInfo* s);
    ~MarshalVkMemoryAllocateInfo();
};

class MarshalVkMappedMemoryRange {
public:
    MarshalVkMappedMemoryRange() {}
    VkMappedMemoryRange s;
    MarshalVkMappedMemoryRange(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMappedMemoryRange* s);
    static void write(KMemory* memory, U32 address, VkMappedMemoryRange* s);
    ~MarshalVkMappedMemoryRange();
};

class MarshalVkDescriptorBufferInfo {
public:
    MarshalVkDescriptorBufferInfo() {}
    VkDescriptorBufferInfo s;
    MarshalVkDescriptorBufferInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorBufferInfo* s);
};

class MarshalVkDescriptorImageInfo {
public:
    MarshalVkDescriptorImageInfo() {}
    VkDescriptorImageInfo s;
    MarshalVkDescriptorImageInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorImageInfo* s);
};

class MarshalVkWriteDescriptorSet {
public:
    MarshalVkWriteDescriptorSet() {}
    VkWriteDescriptorSet s;
    MarshalVkWriteDescriptorSet(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkWriteDescriptorSet* s);
    static void write(KMemory* memory, U32 address, VkWriteDescriptorSet* s);
    ~MarshalVkWriteDescriptorSet();
};

class MarshalVkCopyDescriptorSet {
public:
    MarshalVkCopyDescriptorSet() {}
    VkCopyDescriptorSet s;
    MarshalVkCopyDescriptorSet(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCopyDescriptorSet* s);
    static void write(KMemory* memory, U32 address, VkCopyDescriptorSet* s);
    ~MarshalVkCopyDescriptorSet();
};

class MarshalVkBufferCreateInfo {
public:
    MarshalVkBufferCreateInfo() {}
    VkBufferCreateInfo s;
    MarshalVkBufferCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkBufferCreateInfo* s);
    ~MarshalVkBufferCreateInfo();
};

class MarshalVkBufferViewCreateInfo {
public:
    MarshalVkBufferViewCreateInfo() {}
    VkBufferViewCreateInfo s;
    MarshalVkBufferViewCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferViewCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkBufferViewCreateInfo* s);
    ~MarshalVkBufferViewCreateInfo();
};

class MarshalVkMemoryBarrier {
public:
    MarshalVkMemoryBarrier() {}
    VkMemoryBarrier s;
    MarshalVkMemoryBarrier(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryBarrier* s);
    static void write(KMemory* memory, U32 address, VkMemoryBarrier* s);
    ~MarshalVkMemoryBarrier();
};

class MarshalVkBufferMemoryBarrier {
public:
    MarshalVkBufferMemoryBarrier() {}
    VkBufferMemoryBarrier s;
    MarshalVkBufferMemoryBarrier(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferMemoryBarrier* s);
    static void write(KMemory* memory, U32 address, VkBufferMemoryBarrier* s);
    ~MarshalVkBufferMemoryBarrier();
};

class MarshalVkImageMemoryBarrier {
public:
    MarshalVkImageMemoryBarrier() {}
    VkImageMemoryBarrier s;
    MarshalVkImageMemoryBarrier(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageMemoryBarrier* s);
    static void write(KMemory* memory, U32 address, VkImageMemoryBarrier* s);
    ~MarshalVkImageMemoryBarrier();
};

class MarshalVkImageCreateInfo {
public:
    MarshalVkImageCreateInfo() {}
    VkImageCreateInfo s;
    MarshalVkImageCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkImageCreateInfo* s);
    ~MarshalVkImageCreateInfo();
};

class MarshalVkSubresourceLayout {
public:
    MarshalVkSubresourceLayout() {}
    VkSubresourceLayout s;
    MarshalVkSubresourceLayout(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubresourceLayout* s);
};

class MarshalVkImageViewCreateInfo {
public:
    MarshalVkImageViewCreateInfo() {}
    VkImageViewCreateInfo s;
    MarshalVkImageViewCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageViewCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkImageViewCreateInfo* s);
    ~MarshalVkImageViewCreateInfo();
};

class MarshalVkSparseBufferMemoryBindInfo {
public:
    MarshalVkSparseBufferMemoryBindInfo() {}
    VkSparseBufferMemoryBindInfo s;
    MarshalVkSparseBufferMemoryBindInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSparseBufferMemoryBindInfo* s);
    ~MarshalVkSparseBufferMemoryBindInfo();
};

class MarshalVkSparseImageOpaqueMemoryBindInfo {
public:
    MarshalVkSparseImageOpaqueMemoryBindInfo() {}
    VkSparseImageOpaqueMemoryBindInfo s;
    MarshalVkSparseImageOpaqueMemoryBindInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSparseImageOpaqueMemoryBindInfo* s);
    ~MarshalVkSparseImageOpaqueMemoryBindInfo();
};

class MarshalVkSparseImageMemoryBindInfo {
public:
    MarshalVkSparseImageMemoryBindInfo() {}
    VkSparseImageMemoryBindInfo s;
    MarshalVkSparseImageMemoryBindInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSparseImageMemoryBindInfo* s);
    ~MarshalVkSparseImageMemoryBindInfo();
};

class MarshalVkBindSparseInfo {
public:
    MarshalVkBindSparseInfo() {}
    VkBindSparseInfo s;
    MarshalVkBindSparseInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBindSparseInfo* s);
    static void write(KMemory* memory, U32 address, VkBindSparseInfo* s);
    ~MarshalVkBindSparseInfo();
};

class MarshalVkShaderModuleCreateInfo {
public:
    MarshalVkShaderModuleCreateInfo() {}
    VkShaderModuleCreateInfo s;
    MarshalVkShaderModuleCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkShaderModuleCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkShaderModuleCreateInfo* s);
    ~MarshalVkShaderModuleCreateInfo();
};

class MarshalVkDescriptorSetLayoutBinding {
public:
    MarshalVkDescriptorSetLayoutBinding() {}
    VkDescriptorSetLayoutBinding s;
    MarshalVkDescriptorSetLayoutBinding(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorSetLayoutBinding* s);
    ~MarshalVkDescriptorSetLayoutBinding();
};

class MarshalVkDescriptorSetLayoutCreateInfo {
public:
    MarshalVkDescriptorSetLayoutCreateInfo() {}
    VkDescriptorSetLayoutCreateInfo s;
    MarshalVkDescriptorSetLayoutCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorSetLayoutCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkDescriptorSetLayoutCreateInfo* s);
    ~MarshalVkDescriptorSetLayoutCreateInfo();
};

class MarshalVkDescriptorPoolSize {
public:
    MarshalVkDescriptorPoolSize() {}
    VkDescriptorPoolSize s;
    MarshalVkDescriptorPoolSize(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorPoolSize* s);
};

class MarshalVkDescriptorPoolCreateInfo {
public:
    MarshalVkDescriptorPoolCreateInfo() {}
    VkDescriptorPoolCreateInfo s;
    MarshalVkDescriptorPoolCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorPoolCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkDescriptorPoolCreateInfo* s);
    ~MarshalVkDescriptorPoolCreateInfo();
};

class MarshalVkDescriptorSetAllocateInfo {
public:
    MarshalVkDescriptorSetAllocateInfo() {}
    VkDescriptorSetAllocateInfo s;
    MarshalVkDescriptorSetAllocateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorSetAllocateInfo* s);
    static void write(KMemory* memory, U32 address, VkDescriptorSetAllocateInfo* s);
    ~MarshalVkDescriptorSetAllocateInfo();
};

class MarshalVkSpecializationMapEntry {
public:
    MarshalVkSpecializationMapEntry() {}
    VkSpecializationMapEntry s;
    MarshalVkSpecializationMapEntry(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSpecializationMapEntry* s);
};

class MarshalVkSpecializationInfo {
public:
    MarshalVkSpecializationInfo() {}
    VkSpecializationInfo s;
    MarshalVkSpecializationInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSpecializationInfo* s);
    ~MarshalVkSpecializationInfo();
};

class MarshalVkPipelineShaderStageCreateInfo {
public:
    MarshalVkPipelineShaderStageCreateInfo() {}
    VkPipelineShaderStageCreateInfo s;
    MarshalVkPipelineShaderStageCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineShaderStageCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineShaderStageCreateInfo* s);
    ~MarshalVkPipelineShaderStageCreateInfo();
};

class MarshalVkComputePipelineCreateInfo {
public:
    MarshalVkComputePipelineCreateInfo() {}
    VkComputePipelineCreateInfo s;
    MarshalVkComputePipelineCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkComputePipelineCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkComputePipelineCreateInfo* s);
    ~MarshalVkComputePipelineCreateInfo();
};

class MarshalVkVertexInputBindingDescription {
public:
    MarshalVkVertexInputBindingDescription() {}
    VkVertexInputBindingDescription s;
    MarshalVkVertexInputBindingDescription(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkVertexInputBindingDescription* s);
};

class MarshalVkVertexInputAttributeDescription {
public:
    MarshalVkVertexInputAttributeDescription() {}
    VkVertexInputAttributeDescription s;
    MarshalVkVertexInputAttributeDescription(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkVertexInputAttributeDescription* s);
};

class MarshalVkPipelineVertexInputStateCreateInfo {
public:
    MarshalVkPipelineVertexInputStateCreateInfo() {}
    VkPipelineVertexInputStateCreateInfo s;
    MarshalVkPipelineVertexInputStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineVertexInputStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineVertexInputStateCreateInfo* s);
    ~MarshalVkPipelineVertexInputStateCreateInfo();
};

class MarshalVkPipelineInputAssemblyStateCreateInfo {
public:
    MarshalVkPipelineInputAssemblyStateCreateInfo() {}
    VkPipelineInputAssemblyStateCreateInfo s;
    MarshalVkPipelineInputAssemblyStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineInputAssemblyStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineInputAssemblyStateCreateInfo* s);
    ~MarshalVkPipelineInputAssemblyStateCreateInfo();
};

class MarshalVkPipelineTessellationStateCreateInfo {
public:
    MarshalVkPipelineTessellationStateCreateInfo() {}
    VkPipelineTessellationStateCreateInfo s;
    MarshalVkPipelineTessellationStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineTessellationStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineTessellationStateCreateInfo* s);
    ~MarshalVkPipelineTessellationStateCreateInfo();
};

class MarshalVkPipelineViewportStateCreateInfo {
public:
    MarshalVkPipelineViewportStateCreateInfo() {}
    VkPipelineViewportStateCreateInfo s;
    MarshalVkPipelineViewportStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineViewportStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineViewportStateCreateInfo* s);
    ~MarshalVkPipelineViewportStateCreateInfo();
};

class MarshalVkPipelineRasterizationStateCreateInfo {
public:
    MarshalVkPipelineRasterizationStateCreateInfo() {}
    VkPipelineRasterizationStateCreateInfo s;
    MarshalVkPipelineRasterizationStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineRasterizationStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineRasterizationStateCreateInfo* s);
    ~MarshalVkPipelineRasterizationStateCreateInfo();
};

class MarshalVkPipelineMultisampleStateCreateInfo {
public:
    MarshalVkPipelineMultisampleStateCreateInfo() {}
    VkPipelineMultisampleStateCreateInfo s;
    MarshalVkPipelineMultisampleStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineMultisampleStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineMultisampleStateCreateInfo* s);
    ~MarshalVkPipelineMultisampleStateCreateInfo();
};

class MarshalVkPipelineColorBlendAttachmentState {
public:
    MarshalVkPipelineColorBlendAttachmentState() {}
    VkPipelineColorBlendAttachmentState s;
    MarshalVkPipelineColorBlendAttachmentState(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineColorBlendAttachmentState* s);
};

class MarshalVkPipelineColorBlendStateCreateInfo {
public:
    MarshalVkPipelineColorBlendStateCreateInfo() {}
    VkPipelineColorBlendStateCreateInfo s;
    MarshalVkPipelineColorBlendStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineColorBlendStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineColorBlendStateCreateInfo* s);
    ~MarshalVkPipelineColorBlendStateCreateInfo();
};

class MarshalVkPipelineDynamicStateCreateInfo {
public:
    MarshalVkPipelineDynamicStateCreateInfo() {}
    VkPipelineDynamicStateCreateInfo s;
    MarshalVkPipelineDynamicStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineDynamicStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineDynamicStateCreateInfo* s);
    ~MarshalVkPipelineDynamicStateCreateInfo();
};

class MarshalVkStencilOpState {
public:
    MarshalVkStencilOpState() {}
    VkStencilOpState s;
    MarshalVkStencilOpState(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkStencilOpState* s);
};

class MarshalVkPipelineDepthStencilStateCreateInfo {
public:
    MarshalVkPipelineDepthStencilStateCreateInfo() {}
    VkPipelineDepthStencilStateCreateInfo s;
    MarshalVkPipelineDepthStencilStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineDepthStencilStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineDepthStencilStateCreateInfo* s);
    ~MarshalVkPipelineDepthStencilStateCreateInfo();
};

class MarshalVkGraphicsPipelineCreateInfo {
public:
    MarshalVkGraphicsPipelineCreateInfo() {}
    VkGraphicsPipelineCreateInfo s;
    MarshalVkGraphicsPipelineCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkGraphicsPipelineCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkGraphicsPipelineCreateInfo* s);
    ~MarshalVkGraphicsPipelineCreateInfo();
};

class MarshalVkPipelineCacheCreateInfo {
public:
    MarshalVkPipelineCacheCreateInfo() {}
    VkPipelineCacheCreateInfo s;
    MarshalVkPipelineCacheCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineCacheCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineCacheCreateInfo* s);
    ~MarshalVkPipelineCacheCreateInfo();
};

class MarshalVkPushConstantRange {
public:
    MarshalVkPushConstantRange() {}
    VkPushConstantRange s;
    MarshalVkPushConstantRange(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPushConstantRange* s);
};

class MarshalVkPipelineLayoutCreateInfo {
public:
    MarshalVkPipelineLayoutCreateInfo() {}
    VkPipelineLayoutCreateInfo s;
    MarshalVkPipelineLayoutCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineLayoutCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineLayoutCreateInfo* s);
    ~MarshalVkPipelineLayoutCreateInfo();
};

class MarshalVkSamplerCreateInfo {
public:
    MarshalVkSamplerCreateInfo() {}
    VkSamplerCreateInfo s;
    MarshalVkSamplerCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSamplerCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkSamplerCreateInfo* s);
    ~MarshalVkSamplerCreateInfo();
};

class MarshalVkCommandPoolCreateInfo {
public:
    MarshalVkCommandPoolCreateInfo() {}
    VkCommandPoolCreateInfo s;
    MarshalVkCommandPoolCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCommandPoolCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkCommandPoolCreateInfo* s);
    ~MarshalVkCommandPoolCreateInfo();
};

class MarshalVkCommandBufferAllocateInfo {
public:
    MarshalVkCommandBufferAllocateInfo() {}
    VkCommandBufferAllocateInfo s;
    MarshalVkCommandBufferAllocateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCommandBufferAllocateInfo* s);
    static void write(KMemory* memory, U32 address, VkCommandBufferAllocateInfo* s);
    ~MarshalVkCommandBufferAllocateInfo();
};

class MarshalVkCommandBufferInheritanceInfo {
public:
    MarshalVkCommandBufferInheritanceInfo() {}
    VkCommandBufferInheritanceInfo s;
    MarshalVkCommandBufferInheritanceInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCommandBufferInheritanceInfo* s);
    static void write(KMemory* memory, U32 address, VkCommandBufferInheritanceInfo* s);
    ~MarshalVkCommandBufferInheritanceInfo();
};

class MarshalVkCommandBufferBeginInfo {
public:
    MarshalVkCommandBufferBeginInfo() {}
    VkCommandBufferBeginInfo s;
    MarshalVkCommandBufferBeginInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCommandBufferBeginInfo* s);
    static void write(KMemory* memory, U32 address, VkCommandBufferBeginInfo* s);
    ~MarshalVkCommandBufferBeginInfo();
};

class MarshalVkRenderPassBeginInfo {
public:
    MarshalVkRenderPassBeginInfo() {}
    VkRenderPassBeginInfo s;
    MarshalVkRenderPassBeginInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRenderPassBeginInfo* s);
    static void write(KMemory* memory, U32 address, VkRenderPassBeginInfo* s);
    ~MarshalVkRenderPassBeginInfo();
};

class MarshalVkAttachmentDescription {
public:
    MarshalVkAttachmentDescription() {}
    VkAttachmentDescription s;
    MarshalVkAttachmentDescription(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAttachmentDescription* s);
};

class MarshalVkAttachmentReference {
public:
    MarshalVkAttachmentReference() {}
    VkAttachmentReference s;
    MarshalVkAttachmentReference(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAttachmentReference* s);
};

class MarshalVkSubpassDescription {
public:
    MarshalVkSubpassDescription() {}
    VkSubpassDescription s;
    MarshalVkSubpassDescription(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubpassDescription* s);
    ~MarshalVkSubpassDescription();
};

class MarshalVkSubpassDependency {
public:
    MarshalVkSubpassDependency() {}
    VkSubpassDependency s;
    MarshalVkSubpassDependency(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubpassDependency* s);
};

class MarshalVkRenderPassCreateInfo {
public:
    MarshalVkRenderPassCreateInfo() {}
    VkRenderPassCreateInfo s;
    MarshalVkRenderPassCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRenderPassCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkRenderPassCreateInfo* s);
    ~MarshalVkRenderPassCreateInfo();
};

class MarshalVkEventCreateInfo {
public:
    MarshalVkEventCreateInfo() {}
    VkEventCreateInfo s;
    MarshalVkEventCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkEventCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkEventCreateInfo* s);
    ~MarshalVkEventCreateInfo();
};

class MarshalVkFenceCreateInfo {
public:
    MarshalVkFenceCreateInfo() {}
    VkFenceCreateInfo s;
    MarshalVkFenceCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkFenceCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkFenceCreateInfo* s);
    ~MarshalVkFenceCreateInfo();
};

class MarshalVkSemaphoreCreateInfo {
public:
    MarshalVkSemaphoreCreateInfo() {}
    VkSemaphoreCreateInfo s;
    MarshalVkSemaphoreCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSemaphoreCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkSemaphoreCreateInfo* s);
    ~MarshalVkSemaphoreCreateInfo();
};

class MarshalVkQueryPoolCreateInfo {
public:
    MarshalVkQueryPoolCreateInfo() {}
    VkQueryPoolCreateInfo s;
    MarshalVkQueryPoolCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkQueryPoolCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkQueryPoolCreateInfo* s);
    ~MarshalVkQueryPoolCreateInfo();
};

class MarshalVkFramebufferCreateInfo {
public:
    MarshalVkFramebufferCreateInfo() {}
    VkFramebufferCreateInfo s;
    MarshalVkFramebufferCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkFramebufferCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkFramebufferCreateInfo* s);
    ~MarshalVkFramebufferCreateInfo();
};

class MarshalVkSubmitInfo {
public:
    MarshalVkSubmitInfo() {}
    VkSubmitInfo s;
    MarshalVkSubmitInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubmitInfo* s);
    static void write(KMemory* memory, U32 address, VkSubmitInfo* s);
    ~MarshalVkSubmitInfo();
};

class MarshalVkDisplayPropertiesKHR {
public:
    MarshalVkDisplayPropertiesKHR() {}
    VkDisplayPropertiesKHR s;
    MarshalVkDisplayPropertiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayPropertiesKHR* s);
    ~MarshalVkDisplayPropertiesKHR();
};

class MarshalVkDisplayModeCreateInfoKHR {
public:
    MarshalVkDisplayModeCreateInfoKHR() {}
    VkDisplayModeCreateInfoKHR s;
    MarshalVkDisplayModeCreateInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayModeCreateInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkDisplayModeCreateInfoKHR* s);
    ~MarshalVkDisplayModeCreateInfoKHR();
};

class MarshalVkDisplaySurfaceCreateInfoKHR {
public:
    MarshalVkDisplaySurfaceCreateInfoKHR() {}
    VkDisplaySurfaceCreateInfoKHR s;
    MarshalVkDisplaySurfaceCreateInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplaySurfaceCreateInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkDisplaySurfaceCreateInfoKHR* s);
    ~MarshalVkDisplaySurfaceCreateInfoKHR();
};

class MarshalVkDisplayPresentInfoKHR {
public:
    MarshalVkDisplayPresentInfoKHR() {}
    VkDisplayPresentInfoKHR s;
    MarshalVkDisplayPresentInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayPresentInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkDisplayPresentInfoKHR* s);
    ~MarshalVkDisplayPresentInfoKHR();
};

class MarshalVkSurfaceCapabilitiesKHR {
public:
    MarshalVkSurfaceCapabilitiesKHR() {}
    VkSurfaceCapabilitiesKHR s;
    MarshalVkSurfaceCapabilitiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSurfaceCapabilitiesKHR* s);
    static void write(KMemory* memory, U32 address, VkSurfaceCapabilitiesKHR* s);
};

class MarshalVkSurfaceFormatKHR {
public:
    MarshalVkSurfaceFormatKHR() {}
    VkSurfaceFormatKHR s;
    MarshalVkSurfaceFormatKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSurfaceFormatKHR* s);
    static void write(KMemory* memory, U32 address, VkSurfaceFormatKHR* s);
};

class MarshalVkSwapchainCreateInfoKHR {
public:
    MarshalVkSwapchainCreateInfoKHR() {}
    VkSwapchainCreateInfoKHR s;
    MarshalVkSwapchainCreateInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSwapchainCreateInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkSwapchainCreateInfoKHR* s);
    ~MarshalVkSwapchainCreateInfoKHR();
};

class MarshalVkPresentInfoKHR {
public:
    MarshalVkPresentInfoKHR() {}
    VkPresentInfoKHR s;
    MarshalVkPresentInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPresentInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkPresentInfoKHR* s);
    ~MarshalVkPresentInfoKHR();
};

class MarshalVkValidationFlagsEXT {
public:
    MarshalVkValidationFlagsEXT() {}
    VkValidationFlagsEXT s;
    MarshalVkValidationFlagsEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkValidationFlagsEXT* s);
    static void write(KMemory* memory, U32 address, VkValidationFlagsEXT* s);
    ~MarshalVkValidationFlagsEXT();
};

class MarshalVkValidationFeaturesEXT {
public:
    MarshalVkValidationFeaturesEXT() {}
    VkValidationFeaturesEXT s;
    MarshalVkValidationFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkValidationFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkValidationFeaturesEXT* s);
    ~MarshalVkValidationFeaturesEXT();
};

class MarshalVkPipelineRasterizationStateRasterizationOrderAMD {
public:
    MarshalVkPipelineRasterizationStateRasterizationOrderAMD() {}
    VkPipelineRasterizationStateRasterizationOrderAMD s;
    MarshalVkPipelineRasterizationStateRasterizationOrderAMD(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineRasterizationStateRasterizationOrderAMD* s);
    static void write(KMemory* memory, U32 address, VkPipelineRasterizationStateRasterizationOrderAMD* s);
    ~MarshalVkPipelineRasterizationStateRasterizationOrderAMD();
};

class MarshalVkDebugMarkerObjectNameInfoEXT {
public:
    MarshalVkDebugMarkerObjectNameInfoEXT() {}
    VkDebugMarkerObjectNameInfoEXT s;
    MarshalVkDebugMarkerObjectNameInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDebugMarkerObjectNameInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDebugMarkerObjectNameInfoEXT* s);
    ~MarshalVkDebugMarkerObjectNameInfoEXT();
};

class MarshalVkDebugMarkerObjectTagInfoEXT {
public:
    MarshalVkDebugMarkerObjectTagInfoEXT() {}
    VkDebugMarkerObjectTagInfoEXT s;
    MarshalVkDebugMarkerObjectTagInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDebugMarkerObjectTagInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDebugMarkerObjectTagInfoEXT* s);
    ~MarshalVkDebugMarkerObjectTagInfoEXT();
};

class MarshalVkDebugMarkerMarkerInfoEXT {
public:
    MarshalVkDebugMarkerMarkerInfoEXT() {}
    VkDebugMarkerMarkerInfoEXT s;
    MarshalVkDebugMarkerMarkerInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDebugMarkerMarkerInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDebugMarkerMarkerInfoEXT* s);
    ~MarshalVkDebugMarkerMarkerInfoEXT();
};

class MarshalVkDedicatedAllocationImageCreateInfoNV {
public:
    MarshalVkDedicatedAllocationImageCreateInfoNV() {}
    VkDedicatedAllocationImageCreateInfoNV s;
    MarshalVkDedicatedAllocationImageCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDedicatedAllocationImageCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkDedicatedAllocationImageCreateInfoNV* s);
    ~MarshalVkDedicatedAllocationImageCreateInfoNV();
};

class MarshalVkDedicatedAllocationBufferCreateInfoNV {
public:
    MarshalVkDedicatedAllocationBufferCreateInfoNV() {}
    VkDedicatedAllocationBufferCreateInfoNV s;
    MarshalVkDedicatedAllocationBufferCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDedicatedAllocationBufferCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkDedicatedAllocationBufferCreateInfoNV* s);
    ~MarshalVkDedicatedAllocationBufferCreateInfoNV();
};

class MarshalVkDedicatedAllocationMemoryAllocateInfoNV {
public:
    MarshalVkDedicatedAllocationMemoryAllocateInfoNV() {}
    VkDedicatedAllocationMemoryAllocateInfoNV s;
    MarshalVkDedicatedAllocationMemoryAllocateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDedicatedAllocationMemoryAllocateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkDedicatedAllocationMemoryAllocateInfoNV* s);
    ~MarshalVkDedicatedAllocationMemoryAllocateInfoNV();
};

class MarshalVkExternalMemoryImageCreateInfoNV {
public:
    MarshalVkExternalMemoryImageCreateInfoNV() {}
    VkExternalMemoryImageCreateInfoNV s;
    MarshalVkExternalMemoryImageCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExternalMemoryImageCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkExternalMemoryImageCreateInfoNV* s);
    ~MarshalVkExternalMemoryImageCreateInfoNV();
};

class MarshalVkExportMemoryAllocateInfoNV {
public:
    MarshalVkExportMemoryAllocateInfoNV() {}
    VkExportMemoryAllocateInfoNV s;
    MarshalVkExportMemoryAllocateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExportMemoryAllocateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkExportMemoryAllocateInfoNV* s);
    ~MarshalVkExportMemoryAllocateInfoNV();
};

class MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV {
public:
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV() {}
    VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV s;
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV* s);
    ~MarshalVkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV();
};

class MarshalVkDevicePrivateDataCreateInfoEXT {
public:
    MarshalVkDevicePrivateDataCreateInfoEXT() {}
    VkDevicePrivateDataCreateInfoEXT s;
    MarshalVkDevicePrivateDataCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDevicePrivateDataCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDevicePrivateDataCreateInfoEXT* s);
    ~MarshalVkDevicePrivateDataCreateInfoEXT();
};

class MarshalVkPrivateDataSlotCreateInfoEXT {
public:
    MarshalVkPrivateDataSlotCreateInfoEXT() {}
    VkPrivateDataSlotCreateInfoEXT s;
    MarshalVkPrivateDataSlotCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPrivateDataSlotCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPrivateDataSlotCreateInfoEXT* s);
    ~MarshalVkPrivateDataSlotCreateInfoEXT();
};

class MarshalVkPhysicalDevicePrivateDataFeaturesEXT {
public:
    MarshalVkPhysicalDevicePrivateDataFeaturesEXT() {}
    VkPhysicalDevicePrivateDataFeaturesEXT s;
    MarshalVkPhysicalDevicePrivateDataFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevicePrivateDataFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevicePrivateDataFeaturesEXT* s);
    ~MarshalVkPhysicalDevicePrivateDataFeaturesEXT();
};

class MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV {
public:
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV() {}
    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV s;
    MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV* s);
    ~MarshalVkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV();
};

class MarshalVkPhysicalDeviceMultiDrawPropertiesEXT {
public:
    MarshalVkPhysicalDeviceMultiDrawPropertiesEXT() {}
    VkPhysicalDeviceMultiDrawPropertiesEXT s;
    MarshalVkPhysicalDeviceMultiDrawPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMultiDrawPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMultiDrawPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceMultiDrawPropertiesEXT();
};

class MarshalVkGraphicsShaderGroupCreateInfoNV {
public:
    MarshalVkGraphicsShaderGroupCreateInfoNV() {}
    VkGraphicsShaderGroupCreateInfoNV s;
    MarshalVkGraphicsShaderGroupCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkGraphicsShaderGroupCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkGraphicsShaderGroupCreateInfoNV* s);
    ~MarshalVkGraphicsShaderGroupCreateInfoNV();
};

class MarshalVkGraphicsPipelineShaderGroupsCreateInfoNV {
public:
    MarshalVkGraphicsPipelineShaderGroupsCreateInfoNV() {}
    VkGraphicsPipelineShaderGroupsCreateInfoNV s;
    MarshalVkGraphicsPipelineShaderGroupsCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkGraphicsPipelineShaderGroupsCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkGraphicsPipelineShaderGroupsCreateInfoNV* s);
    ~MarshalVkGraphicsPipelineShaderGroupsCreateInfoNV();
};

class MarshalVkIndirectCommandsStreamNV {
public:
    MarshalVkIndirectCommandsStreamNV() {}
    VkIndirectCommandsStreamNV s;
    MarshalVkIndirectCommandsStreamNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkIndirectCommandsStreamNV* s);
};

class MarshalVkIndirectCommandsLayoutTokenNV {
public:
    MarshalVkIndirectCommandsLayoutTokenNV() {}
    VkIndirectCommandsLayoutTokenNV s;
    MarshalVkIndirectCommandsLayoutTokenNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkIndirectCommandsLayoutTokenNV* s);
    static void write(KMemory* memory, U32 address, VkIndirectCommandsLayoutTokenNV* s);
    ~MarshalVkIndirectCommandsLayoutTokenNV();
};

class MarshalVkIndirectCommandsLayoutCreateInfoNV {
public:
    MarshalVkIndirectCommandsLayoutCreateInfoNV() {}
    VkIndirectCommandsLayoutCreateInfoNV s;
    MarshalVkIndirectCommandsLayoutCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkIndirectCommandsLayoutCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkIndirectCommandsLayoutCreateInfoNV* s);
    ~MarshalVkIndirectCommandsLayoutCreateInfoNV();
};

class MarshalVkGeneratedCommandsInfoNV {
public:
    MarshalVkGeneratedCommandsInfoNV() {}
    VkGeneratedCommandsInfoNV s;
    MarshalVkGeneratedCommandsInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkGeneratedCommandsInfoNV* s);
    static void write(KMemory* memory, U32 address, VkGeneratedCommandsInfoNV* s);
    ~MarshalVkGeneratedCommandsInfoNV();
};

class MarshalVkGeneratedCommandsMemoryRequirementsInfoNV {
public:
    MarshalVkGeneratedCommandsMemoryRequirementsInfoNV() {}
    VkGeneratedCommandsMemoryRequirementsInfoNV s;
    MarshalVkGeneratedCommandsMemoryRequirementsInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkGeneratedCommandsMemoryRequirementsInfoNV* s);
    static void write(KMemory* memory, U32 address, VkGeneratedCommandsMemoryRequirementsInfoNV* s);
    ~MarshalVkGeneratedCommandsMemoryRequirementsInfoNV();
};

class MarshalVkPhysicalDeviceFeatures2 {
public:
    MarshalVkPhysicalDeviceFeatures2() {}
    VkPhysicalDeviceFeatures2 s;
    MarshalVkPhysicalDeviceFeatures2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFeatures2* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFeatures2* s);
    ~MarshalVkPhysicalDeviceFeatures2();
};

class MarshalVkPhysicalDeviceProperties2 {
public:
    MarshalVkPhysicalDeviceProperties2() {}
    VkPhysicalDeviceProperties2 s;
    MarshalVkPhysicalDeviceProperties2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceProperties2* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceProperties2* s);
    ~MarshalVkPhysicalDeviceProperties2();
};

class MarshalVkFormatProperties2 {
public:
    MarshalVkFormatProperties2() {}
    VkFormatProperties2 s;
    MarshalVkFormatProperties2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkFormatProperties2* s);
    static void write(KMemory* memory, U32 address, VkFormatProperties2* s);
    ~MarshalVkFormatProperties2();
};

class MarshalVkImageFormatProperties2 {
public:
    MarshalVkImageFormatProperties2() {}
    VkImageFormatProperties2 s;
    MarshalVkImageFormatProperties2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageFormatProperties2* s);
    static void write(KMemory* memory, U32 address, VkImageFormatProperties2* s);
    ~MarshalVkImageFormatProperties2();
};

class MarshalVkPhysicalDeviceImageFormatInfo2 {
public:
    MarshalVkPhysicalDeviceImageFormatInfo2() {}
    VkPhysicalDeviceImageFormatInfo2 s;
    MarshalVkPhysicalDeviceImageFormatInfo2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceImageFormatInfo2* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceImageFormatInfo2* s);
    ~MarshalVkPhysicalDeviceImageFormatInfo2();
};

class MarshalVkQueueFamilyProperties2 {
public:
    MarshalVkQueueFamilyProperties2() {}
    VkQueueFamilyProperties2 s;
    MarshalVkQueueFamilyProperties2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkQueueFamilyProperties2* s);
    static void write(KMemory* memory, U32 address, VkQueueFamilyProperties2* s);
    ~MarshalVkQueueFamilyProperties2();
};

class MarshalVkPhysicalDeviceMemoryProperties2 {
public:
    MarshalVkPhysicalDeviceMemoryProperties2() {}
    VkPhysicalDeviceMemoryProperties2 s;
    MarshalVkPhysicalDeviceMemoryProperties2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMemoryProperties2* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMemoryProperties2* s);
    ~MarshalVkPhysicalDeviceMemoryProperties2();
};

class MarshalVkSparseImageFormatProperties2 {
public:
    MarshalVkSparseImageFormatProperties2() {}
    VkSparseImageFormatProperties2 s;
    MarshalVkSparseImageFormatProperties2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSparseImageFormatProperties2* s);
    static void write(KMemory* memory, U32 address, VkSparseImageFormatProperties2* s);
    ~MarshalVkSparseImageFormatProperties2();
};

class MarshalVkPhysicalDeviceSparseImageFormatInfo2 {
public:
    MarshalVkPhysicalDeviceSparseImageFormatInfo2() {}
    VkPhysicalDeviceSparseImageFormatInfo2 s;
    MarshalVkPhysicalDeviceSparseImageFormatInfo2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSparseImageFormatInfo2* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSparseImageFormatInfo2* s);
    ~MarshalVkPhysicalDeviceSparseImageFormatInfo2();
};

class MarshalVkPhysicalDevicePushDescriptorPropertiesKHR {
public:
    MarshalVkPhysicalDevicePushDescriptorPropertiesKHR() {}
    VkPhysicalDevicePushDescriptorPropertiesKHR s;
    MarshalVkPhysicalDevicePushDescriptorPropertiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevicePushDescriptorPropertiesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevicePushDescriptorPropertiesKHR* s);
    ~MarshalVkPhysicalDevicePushDescriptorPropertiesKHR();
};

class MarshalVkPhysicalDeviceDriverProperties {
public:
    MarshalVkPhysicalDeviceDriverProperties() {}
    VkPhysicalDeviceDriverProperties s;
    MarshalVkPhysicalDeviceDriverProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDriverProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDriverProperties* s);
    ~MarshalVkPhysicalDeviceDriverProperties();
};

class MarshalVkPresentRegionsKHR {
public:
    MarshalVkPresentRegionsKHR() {}
    VkPresentRegionsKHR s;
    MarshalVkPresentRegionsKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPresentRegionsKHR* s);
    static void write(KMemory* memory, U32 address, VkPresentRegionsKHR* s);
    ~MarshalVkPresentRegionsKHR();
};

class MarshalVkPhysicalDeviceVariablePointersFeatures {
public:
    MarshalVkPhysicalDeviceVariablePointersFeatures() {}
    VkPhysicalDeviceVariablePointersFeatures s;
    MarshalVkPhysicalDeviceVariablePointersFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceVariablePointersFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceVariablePointersFeatures* s);
    ~MarshalVkPhysicalDeviceVariablePointersFeatures();
};

class MarshalVkPhysicalDeviceExternalImageFormatInfo {
public:
    MarshalVkPhysicalDeviceExternalImageFormatInfo() {}
    VkPhysicalDeviceExternalImageFormatInfo s;
    MarshalVkPhysicalDeviceExternalImageFormatInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceExternalImageFormatInfo* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceExternalImageFormatInfo* s);
    ~MarshalVkPhysicalDeviceExternalImageFormatInfo();
};

class MarshalVkExternalImageFormatProperties {
public:
    MarshalVkExternalImageFormatProperties() {}
    VkExternalImageFormatProperties s;
    MarshalVkExternalImageFormatProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExternalImageFormatProperties* s);
    static void write(KMemory* memory, U32 address, VkExternalImageFormatProperties* s);
    ~MarshalVkExternalImageFormatProperties();
};

class MarshalVkPhysicalDeviceExternalBufferInfo {
public:
    MarshalVkPhysicalDeviceExternalBufferInfo() {}
    VkPhysicalDeviceExternalBufferInfo s;
    MarshalVkPhysicalDeviceExternalBufferInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceExternalBufferInfo* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceExternalBufferInfo* s);
    ~MarshalVkPhysicalDeviceExternalBufferInfo();
};

class MarshalVkExternalBufferProperties {
public:
    MarshalVkExternalBufferProperties() {}
    VkExternalBufferProperties s;
    MarshalVkExternalBufferProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExternalBufferProperties* s);
    static void write(KMemory* memory, U32 address, VkExternalBufferProperties* s);
    ~MarshalVkExternalBufferProperties();
};

class MarshalVkPhysicalDeviceIDProperties {
public:
    MarshalVkPhysicalDeviceIDProperties() {}
    VkPhysicalDeviceIDProperties s;
    MarshalVkPhysicalDeviceIDProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceIDProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceIDProperties* s);
    ~MarshalVkPhysicalDeviceIDProperties();
};

class MarshalVkExternalMemoryImageCreateInfo {
public:
    MarshalVkExternalMemoryImageCreateInfo() {}
    VkExternalMemoryImageCreateInfo s;
    MarshalVkExternalMemoryImageCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExternalMemoryImageCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkExternalMemoryImageCreateInfo* s);
    ~MarshalVkExternalMemoryImageCreateInfo();
};

class MarshalVkExternalMemoryBufferCreateInfo {
public:
    MarshalVkExternalMemoryBufferCreateInfo() {}
    VkExternalMemoryBufferCreateInfo s;
    MarshalVkExternalMemoryBufferCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExternalMemoryBufferCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkExternalMemoryBufferCreateInfo* s);
    ~MarshalVkExternalMemoryBufferCreateInfo();
};

class MarshalVkExportMemoryAllocateInfo {
public:
    MarshalVkExportMemoryAllocateInfo() {}
    VkExportMemoryAllocateInfo s;
    MarshalVkExportMemoryAllocateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExportMemoryAllocateInfo* s);
    static void write(KMemory* memory, U32 address, VkExportMemoryAllocateInfo* s);
    ~MarshalVkExportMemoryAllocateInfo();
};

class MarshalVkImportMemoryFdInfoKHR {
public:
    MarshalVkImportMemoryFdInfoKHR() {}
    VkImportMemoryFdInfoKHR s;
    MarshalVkImportMemoryFdInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImportMemoryFdInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkImportMemoryFdInfoKHR* s);
    ~MarshalVkImportMemoryFdInfoKHR();
};

class MarshalVkMemoryFdPropertiesKHR {
public:
    MarshalVkMemoryFdPropertiesKHR() {}
    VkMemoryFdPropertiesKHR s;
    MarshalVkMemoryFdPropertiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryFdPropertiesKHR* s);
    static void write(KMemory* memory, U32 address, VkMemoryFdPropertiesKHR* s);
    ~MarshalVkMemoryFdPropertiesKHR();
};

class MarshalVkMemoryGetFdInfoKHR {
public:
    MarshalVkMemoryGetFdInfoKHR() {}
    VkMemoryGetFdInfoKHR s;
    MarshalVkMemoryGetFdInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryGetFdInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkMemoryGetFdInfoKHR* s);
    ~MarshalVkMemoryGetFdInfoKHR();
};

class MarshalVkPhysicalDeviceExternalSemaphoreInfo {
public:
    MarshalVkPhysicalDeviceExternalSemaphoreInfo() {}
    VkPhysicalDeviceExternalSemaphoreInfo s;
    MarshalVkPhysicalDeviceExternalSemaphoreInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceExternalSemaphoreInfo* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceExternalSemaphoreInfo* s);
    ~MarshalVkPhysicalDeviceExternalSemaphoreInfo();
};

class MarshalVkExternalSemaphoreProperties {
public:
    MarshalVkExternalSemaphoreProperties() {}
    VkExternalSemaphoreProperties s;
    MarshalVkExternalSemaphoreProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExternalSemaphoreProperties* s);
    static void write(KMemory* memory, U32 address, VkExternalSemaphoreProperties* s);
    ~MarshalVkExternalSemaphoreProperties();
};

class MarshalVkExportSemaphoreCreateInfo {
public:
    MarshalVkExportSemaphoreCreateInfo() {}
    VkExportSemaphoreCreateInfo s;
    MarshalVkExportSemaphoreCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExportSemaphoreCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkExportSemaphoreCreateInfo* s);
    ~MarshalVkExportSemaphoreCreateInfo();
};

class MarshalVkImportSemaphoreFdInfoKHR {
public:
    MarshalVkImportSemaphoreFdInfoKHR() {}
    VkImportSemaphoreFdInfoKHR s;
    MarshalVkImportSemaphoreFdInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImportSemaphoreFdInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkImportSemaphoreFdInfoKHR* s);
    ~MarshalVkImportSemaphoreFdInfoKHR();
};

class MarshalVkSemaphoreGetFdInfoKHR {
public:
    MarshalVkSemaphoreGetFdInfoKHR() {}
    VkSemaphoreGetFdInfoKHR s;
    MarshalVkSemaphoreGetFdInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSemaphoreGetFdInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkSemaphoreGetFdInfoKHR* s);
    ~MarshalVkSemaphoreGetFdInfoKHR();
};

class MarshalVkPhysicalDeviceExternalFenceInfo {
public:
    MarshalVkPhysicalDeviceExternalFenceInfo() {}
    VkPhysicalDeviceExternalFenceInfo s;
    MarshalVkPhysicalDeviceExternalFenceInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceExternalFenceInfo* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceExternalFenceInfo* s);
    ~MarshalVkPhysicalDeviceExternalFenceInfo();
};

class MarshalVkExternalFenceProperties {
public:
    MarshalVkExternalFenceProperties() {}
    VkExternalFenceProperties s;
    MarshalVkExternalFenceProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExternalFenceProperties* s);
    static void write(KMemory* memory, U32 address, VkExternalFenceProperties* s);
    ~MarshalVkExternalFenceProperties();
};

class MarshalVkExportFenceCreateInfo {
public:
    MarshalVkExportFenceCreateInfo() {}
    VkExportFenceCreateInfo s;
    MarshalVkExportFenceCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkExportFenceCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkExportFenceCreateInfo* s);
    ~MarshalVkExportFenceCreateInfo();
};

class MarshalVkImportFenceFdInfoKHR {
public:
    MarshalVkImportFenceFdInfoKHR() {}
    VkImportFenceFdInfoKHR s;
    MarshalVkImportFenceFdInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImportFenceFdInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkImportFenceFdInfoKHR* s);
    ~MarshalVkImportFenceFdInfoKHR();
};

class MarshalVkFenceGetFdInfoKHR {
public:
    MarshalVkFenceGetFdInfoKHR() {}
    VkFenceGetFdInfoKHR s;
    MarshalVkFenceGetFdInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkFenceGetFdInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkFenceGetFdInfoKHR* s);
    ~MarshalVkFenceGetFdInfoKHR();
};

class MarshalVkPhysicalDeviceMultiviewFeatures {
public:
    MarshalVkPhysicalDeviceMultiviewFeatures() {}
    VkPhysicalDeviceMultiviewFeatures s;
    MarshalVkPhysicalDeviceMultiviewFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMultiviewFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMultiviewFeatures* s);
    ~MarshalVkPhysicalDeviceMultiviewFeatures();
};

class MarshalVkPhysicalDeviceMultiviewProperties {
public:
    MarshalVkPhysicalDeviceMultiviewProperties() {}
    VkPhysicalDeviceMultiviewProperties s;
    MarshalVkPhysicalDeviceMultiviewProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMultiviewProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMultiviewProperties* s);
    ~MarshalVkPhysicalDeviceMultiviewProperties();
};

class MarshalVkRenderPassMultiviewCreateInfo {
public:
    MarshalVkRenderPassMultiviewCreateInfo() {}
    VkRenderPassMultiviewCreateInfo s;
    MarshalVkRenderPassMultiviewCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRenderPassMultiviewCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkRenderPassMultiviewCreateInfo* s);
    ~MarshalVkRenderPassMultiviewCreateInfo();
};

class MarshalVkSurfaceCapabilities2EXT {
public:
    MarshalVkSurfaceCapabilities2EXT() {}
    VkSurfaceCapabilities2EXT s;
    MarshalVkSurfaceCapabilities2EXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSurfaceCapabilities2EXT* s);
    static void write(KMemory* memory, U32 address, VkSurfaceCapabilities2EXT* s);
    ~MarshalVkSurfaceCapabilities2EXT();
};

class MarshalVkDisplayPowerInfoEXT {
public:
    MarshalVkDisplayPowerInfoEXT() {}
    VkDisplayPowerInfoEXT s;
    MarshalVkDisplayPowerInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayPowerInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDisplayPowerInfoEXT* s);
    ~MarshalVkDisplayPowerInfoEXT();
};

class MarshalVkDeviceEventInfoEXT {
public:
    MarshalVkDeviceEventInfoEXT() {}
    VkDeviceEventInfoEXT s;
    MarshalVkDeviceEventInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceEventInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDeviceEventInfoEXT* s);
    ~MarshalVkDeviceEventInfoEXT();
};

class MarshalVkDisplayEventInfoEXT {
public:
    MarshalVkDisplayEventInfoEXT() {}
    VkDisplayEventInfoEXT s;
    MarshalVkDisplayEventInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayEventInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDisplayEventInfoEXT* s);
    ~MarshalVkDisplayEventInfoEXT();
};

class MarshalVkSwapchainCounterCreateInfoEXT {
public:
    MarshalVkSwapchainCounterCreateInfoEXT() {}
    VkSwapchainCounterCreateInfoEXT s;
    MarshalVkSwapchainCounterCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSwapchainCounterCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkSwapchainCounterCreateInfoEXT* s);
    ~MarshalVkSwapchainCounterCreateInfoEXT();
};

class MarshalVkPhysicalDeviceGroupProperties {
public:
    MarshalVkPhysicalDeviceGroupProperties() {}
    VkPhysicalDeviceGroupProperties s;
    MarshalVkPhysicalDeviceGroupProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceGroupProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceGroupProperties* s);
    ~MarshalVkPhysicalDeviceGroupProperties();
};

class MarshalVkMemoryAllocateFlagsInfo {
public:
    MarshalVkMemoryAllocateFlagsInfo() {}
    VkMemoryAllocateFlagsInfo s;
    MarshalVkMemoryAllocateFlagsInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryAllocateFlagsInfo* s);
    static void write(KMemory* memory, U32 address, VkMemoryAllocateFlagsInfo* s);
    ~MarshalVkMemoryAllocateFlagsInfo();
};

class MarshalVkBindBufferMemoryInfo {
public:
    MarshalVkBindBufferMemoryInfo() {}
    VkBindBufferMemoryInfo s;
    MarshalVkBindBufferMemoryInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBindBufferMemoryInfo* s);
    static void write(KMemory* memory, U32 address, VkBindBufferMemoryInfo* s);
    ~MarshalVkBindBufferMemoryInfo();
};

class MarshalVkBindBufferMemoryDeviceGroupInfo {
public:
    MarshalVkBindBufferMemoryDeviceGroupInfo() {}
    VkBindBufferMemoryDeviceGroupInfo s;
    MarshalVkBindBufferMemoryDeviceGroupInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBindBufferMemoryDeviceGroupInfo* s);
    static void write(KMemory* memory, U32 address, VkBindBufferMemoryDeviceGroupInfo* s);
    ~MarshalVkBindBufferMemoryDeviceGroupInfo();
};

class MarshalVkBindImageMemoryInfo {
public:
    MarshalVkBindImageMemoryInfo() {}
    VkBindImageMemoryInfo s;
    MarshalVkBindImageMemoryInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBindImageMemoryInfo* s);
    static void write(KMemory* memory, U32 address, VkBindImageMemoryInfo* s);
    ~MarshalVkBindImageMemoryInfo();
};

class MarshalVkBindImageMemoryDeviceGroupInfo {
public:
    MarshalVkBindImageMemoryDeviceGroupInfo() {}
    VkBindImageMemoryDeviceGroupInfo s;
    MarshalVkBindImageMemoryDeviceGroupInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBindImageMemoryDeviceGroupInfo* s);
    static void write(KMemory* memory, U32 address, VkBindImageMemoryDeviceGroupInfo* s);
    ~MarshalVkBindImageMemoryDeviceGroupInfo();
};

class MarshalVkDeviceGroupRenderPassBeginInfo {
public:
    MarshalVkDeviceGroupRenderPassBeginInfo() {}
    VkDeviceGroupRenderPassBeginInfo s;
    MarshalVkDeviceGroupRenderPassBeginInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceGroupRenderPassBeginInfo* s);
    static void write(KMemory* memory, U32 address, VkDeviceGroupRenderPassBeginInfo* s);
    ~MarshalVkDeviceGroupRenderPassBeginInfo();
};

class MarshalVkDeviceGroupCommandBufferBeginInfo {
public:
    MarshalVkDeviceGroupCommandBufferBeginInfo() {}
    VkDeviceGroupCommandBufferBeginInfo s;
    MarshalVkDeviceGroupCommandBufferBeginInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceGroupCommandBufferBeginInfo* s);
    static void write(KMemory* memory, U32 address, VkDeviceGroupCommandBufferBeginInfo* s);
    ~MarshalVkDeviceGroupCommandBufferBeginInfo();
};

class MarshalVkDeviceGroupSubmitInfo {
public:
    MarshalVkDeviceGroupSubmitInfo() {}
    VkDeviceGroupSubmitInfo s;
    MarshalVkDeviceGroupSubmitInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceGroupSubmitInfo* s);
    static void write(KMemory* memory, U32 address, VkDeviceGroupSubmitInfo* s);
    ~MarshalVkDeviceGroupSubmitInfo();
};

class MarshalVkDeviceGroupBindSparseInfo {
public:
    MarshalVkDeviceGroupBindSparseInfo() {}
    VkDeviceGroupBindSparseInfo s;
    MarshalVkDeviceGroupBindSparseInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceGroupBindSparseInfo* s);
    static void write(KMemory* memory, U32 address, VkDeviceGroupBindSparseInfo* s);
    ~MarshalVkDeviceGroupBindSparseInfo();
};

class MarshalVkDeviceGroupPresentCapabilitiesKHR {
public:
    MarshalVkDeviceGroupPresentCapabilitiesKHR() {}
    VkDeviceGroupPresentCapabilitiesKHR s;
    MarshalVkDeviceGroupPresentCapabilitiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceGroupPresentCapabilitiesKHR* s);
    static void write(KMemory* memory, U32 address, VkDeviceGroupPresentCapabilitiesKHR* s);
    ~MarshalVkDeviceGroupPresentCapabilitiesKHR();
};

class MarshalVkImageSwapchainCreateInfoKHR {
public:
    MarshalVkImageSwapchainCreateInfoKHR() {}
    VkImageSwapchainCreateInfoKHR s;
    MarshalVkImageSwapchainCreateInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageSwapchainCreateInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkImageSwapchainCreateInfoKHR* s);
    ~MarshalVkImageSwapchainCreateInfoKHR();
};

class MarshalVkBindImageMemorySwapchainInfoKHR {
public:
    MarshalVkBindImageMemorySwapchainInfoKHR() {}
    VkBindImageMemorySwapchainInfoKHR s;
    MarshalVkBindImageMemorySwapchainInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBindImageMemorySwapchainInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkBindImageMemorySwapchainInfoKHR* s);
    ~MarshalVkBindImageMemorySwapchainInfoKHR();
};

class MarshalVkAcquireNextImageInfoKHR {
public:
    MarshalVkAcquireNextImageInfoKHR() {}
    VkAcquireNextImageInfoKHR s;
    MarshalVkAcquireNextImageInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAcquireNextImageInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkAcquireNextImageInfoKHR* s);
    ~MarshalVkAcquireNextImageInfoKHR();
};

class MarshalVkDeviceGroupPresentInfoKHR {
public:
    MarshalVkDeviceGroupPresentInfoKHR() {}
    VkDeviceGroupPresentInfoKHR s;
    MarshalVkDeviceGroupPresentInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceGroupPresentInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkDeviceGroupPresentInfoKHR* s);
    ~MarshalVkDeviceGroupPresentInfoKHR();
};

class MarshalVkDeviceGroupDeviceCreateInfo {
public:
    MarshalVkDeviceGroupDeviceCreateInfo() {}
    VkDeviceGroupDeviceCreateInfo s;
    MarshalVkDeviceGroupDeviceCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceGroupDeviceCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkDeviceGroupDeviceCreateInfo* s);
    ~MarshalVkDeviceGroupDeviceCreateInfo();
};

class MarshalVkDeviceGroupSwapchainCreateInfoKHR {
public:
    MarshalVkDeviceGroupSwapchainCreateInfoKHR() {}
    VkDeviceGroupSwapchainCreateInfoKHR s;
    MarshalVkDeviceGroupSwapchainCreateInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceGroupSwapchainCreateInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkDeviceGroupSwapchainCreateInfoKHR* s);
    ~MarshalVkDeviceGroupSwapchainCreateInfoKHR();
};

class MarshalVkDescriptorUpdateTemplateEntry {
public:
    MarshalVkDescriptorUpdateTemplateEntry() {}
    VkDescriptorUpdateTemplateEntry s;
    MarshalVkDescriptorUpdateTemplateEntry(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorUpdateTemplateEntry* s);
};

class MarshalVkDescriptorUpdateTemplateCreateInfo {
public:
    MarshalVkDescriptorUpdateTemplateCreateInfo() {}
    VkDescriptorUpdateTemplateCreateInfo s;
    MarshalVkDescriptorUpdateTemplateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorUpdateTemplateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkDescriptorUpdateTemplateCreateInfo* s);
    ~MarshalVkDescriptorUpdateTemplateCreateInfo();
};

class MarshalVkHdrMetadataEXT {
public:
    MarshalVkHdrMetadataEXT() {}
    VkHdrMetadataEXT s;
    MarshalVkHdrMetadataEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkHdrMetadataEXT* s);
    static void write(KMemory* memory, U32 address, VkHdrMetadataEXT* s);
    ~MarshalVkHdrMetadataEXT();
};

class MarshalVkDisplayNativeHdrSurfaceCapabilitiesAMD {
public:
    MarshalVkDisplayNativeHdrSurfaceCapabilitiesAMD() {}
    VkDisplayNativeHdrSurfaceCapabilitiesAMD s;
    MarshalVkDisplayNativeHdrSurfaceCapabilitiesAMD(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayNativeHdrSurfaceCapabilitiesAMD* s);
    static void write(KMemory* memory, U32 address, VkDisplayNativeHdrSurfaceCapabilitiesAMD* s);
    ~MarshalVkDisplayNativeHdrSurfaceCapabilitiesAMD();
};

class MarshalVkSwapchainDisplayNativeHdrCreateInfoAMD {
public:
    MarshalVkSwapchainDisplayNativeHdrCreateInfoAMD() {}
    VkSwapchainDisplayNativeHdrCreateInfoAMD s;
    MarshalVkSwapchainDisplayNativeHdrCreateInfoAMD(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSwapchainDisplayNativeHdrCreateInfoAMD* s);
    static void write(KMemory* memory, U32 address, VkSwapchainDisplayNativeHdrCreateInfoAMD* s);
    ~MarshalVkSwapchainDisplayNativeHdrCreateInfoAMD();
};

class MarshalVkPresentTimesInfoGOOGLE {
public:
    MarshalVkPresentTimesInfoGOOGLE() {}
    VkPresentTimesInfoGOOGLE s;
    MarshalVkPresentTimesInfoGOOGLE(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPresentTimesInfoGOOGLE* s);
    static void write(KMemory* memory, U32 address, VkPresentTimesInfoGOOGLE* s);
    ~MarshalVkPresentTimesInfoGOOGLE();
};

class MarshalVkViewportWScalingNV {
public:
    MarshalVkViewportWScalingNV() {}
    VkViewportWScalingNV s;
    MarshalVkViewportWScalingNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkViewportWScalingNV* s);
};

class MarshalVkPipelineViewportWScalingStateCreateInfoNV {
public:
    MarshalVkPipelineViewportWScalingStateCreateInfoNV() {}
    VkPipelineViewportWScalingStateCreateInfoNV s;
    MarshalVkPipelineViewportWScalingStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineViewportWScalingStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineViewportWScalingStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportWScalingStateCreateInfoNV();
};

class MarshalVkViewportSwizzleNV {
public:
    MarshalVkViewportSwizzleNV() {}
    VkViewportSwizzleNV s;
    MarshalVkViewportSwizzleNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkViewportSwizzleNV* s);
};

class MarshalVkPipelineViewportSwizzleStateCreateInfoNV {
public:
    MarshalVkPipelineViewportSwizzleStateCreateInfoNV() {}
    VkPipelineViewportSwizzleStateCreateInfoNV s;
    MarshalVkPipelineViewportSwizzleStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineViewportSwizzleStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineViewportSwizzleStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportSwizzleStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceDiscardRectanglePropertiesEXT {
public:
    MarshalVkPhysicalDeviceDiscardRectanglePropertiesEXT() {}
    VkPhysicalDeviceDiscardRectanglePropertiesEXT s;
    MarshalVkPhysicalDeviceDiscardRectanglePropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDiscardRectanglePropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDiscardRectanglePropertiesEXT* s);
    ~MarshalVkPhysicalDeviceDiscardRectanglePropertiesEXT();
};

class MarshalVkPipelineDiscardRectangleStateCreateInfoEXT {
public:
    MarshalVkPipelineDiscardRectangleStateCreateInfoEXT() {}
    VkPipelineDiscardRectangleStateCreateInfoEXT s;
    MarshalVkPipelineDiscardRectangleStateCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineDiscardRectangleStateCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineDiscardRectangleStateCreateInfoEXT* s);
    ~MarshalVkPipelineDiscardRectangleStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX {
public:
    MarshalVkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX() {}
    VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX s;
    MarshalVkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* s);
    ~MarshalVkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX();
};

class MarshalVkInputAttachmentAspectReference {
public:
    MarshalVkInputAttachmentAspectReference() {}
    VkInputAttachmentAspectReference s;
    MarshalVkInputAttachmentAspectReference(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkInputAttachmentAspectReference* s);
};

class MarshalVkRenderPassInputAttachmentAspectCreateInfo {
public:
    MarshalVkRenderPassInputAttachmentAspectCreateInfo() {}
    VkRenderPassInputAttachmentAspectCreateInfo s;
    MarshalVkRenderPassInputAttachmentAspectCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRenderPassInputAttachmentAspectCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkRenderPassInputAttachmentAspectCreateInfo* s);
    ~MarshalVkRenderPassInputAttachmentAspectCreateInfo();
};

class MarshalVkPhysicalDeviceSurfaceInfo2KHR {
public:
    MarshalVkPhysicalDeviceSurfaceInfo2KHR() {}
    VkPhysicalDeviceSurfaceInfo2KHR s;
    MarshalVkPhysicalDeviceSurfaceInfo2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSurfaceInfo2KHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSurfaceInfo2KHR* s);
    ~MarshalVkPhysicalDeviceSurfaceInfo2KHR();
};

class MarshalVkSurfaceCapabilities2KHR {
public:
    MarshalVkSurfaceCapabilities2KHR() {}
    VkSurfaceCapabilities2KHR s;
    MarshalVkSurfaceCapabilities2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSurfaceCapabilities2KHR* s);
    static void write(KMemory* memory, U32 address, VkSurfaceCapabilities2KHR* s);
    ~MarshalVkSurfaceCapabilities2KHR();
};

class MarshalVkSurfaceFormat2KHR {
public:
    MarshalVkSurfaceFormat2KHR() {}
    VkSurfaceFormat2KHR s;
    MarshalVkSurfaceFormat2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSurfaceFormat2KHR* s);
    static void write(KMemory* memory, U32 address, VkSurfaceFormat2KHR* s);
    ~MarshalVkSurfaceFormat2KHR();
};

class MarshalVkDisplayProperties2KHR {
public:
    MarshalVkDisplayProperties2KHR() {}
    VkDisplayProperties2KHR s;
    MarshalVkDisplayProperties2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayProperties2KHR* s);
    static void write(KMemory* memory, U32 address, VkDisplayProperties2KHR* s);
    ~MarshalVkDisplayProperties2KHR();
};

class MarshalVkDisplayPlaneProperties2KHR {
public:
    MarshalVkDisplayPlaneProperties2KHR() {}
    VkDisplayPlaneProperties2KHR s;
    MarshalVkDisplayPlaneProperties2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayPlaneProperties2KHR* s);
    static void write(KMemory* memory, U32 address, VkDisplayPlaneProperties2KHR* s);
    ~MarshalVkDisplayPlaneProperties2KHR();
};

class MarshalVkDisplayModeProperties2KHR {
public:
    MarshalVkDisplayModeProperties2KHR() {}
    VkDisplayModeProperties2KHR s;
    MarshalVkDisplayModeProperties2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayModeProperties2KHR* s);
    static void write(KMemory* memory, U32 address, VkDisplayModeProperties2KHR* s);
    ~MarshalVkDisplayModeProperties2KHR();
};

class MarshalVkDisplayPlaneInfo2KHR {
public:
    MarshalVkDisplayPlaneInfo2KHR() {}
    VkDisplayPlaneInfo2KHR s;
    MarshalVkDisplayPlaneInfo2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayPlaneInfo2KHR* s);
    static void write(KMemory* memory, U32 address, VkDisplayPlaneInfo2KHR* s);
    ~MarshalVkDisplayPlaneInfo2KHR();
};

class MarshalVkDisplayPlaneCapabilities2KHR {
public:
    MarshalVkDisplayPlaneCapabilities2KHR() {}
    VkDisplayPlaneCapabilities2KHR s;
    MarshalVkDisplayPlaneCapabilities2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDisplayPlaneCapabilities2KHR* s);
    static void write(KMemory* memory, U32 address, VkDisplayPlaneCapabilities2KHR* s);
    ~MarshalVkDisplayPlaneCapabilities2KHR();
};

class MarshalVkSharedPresentSurfaceCapabilitiesKHR {
public:
    MarshalVkSharedPresentSurfaceCapabilitiesKHR() {}
    VkSharedPresentSurfaceCapabilitiesKHR s;
    MarshalVkSharedPresentSurfaceCapabilitiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSharedPresentSurfaceCapabilitiesKHR* s);
    static void write(KMemory* memory, U32 address, VkSharedPresentSurfaceCapabilitiesKHR* s);
    ~MarshalVkSharedPresentSurfaceCapabilitiesKHR();
};

class MarshalVkPhysicalDevice16BitStorageFeatures {
public:
    MarshalVkPhysicalDevice16BitStorageFeatures() {}
    VkPhysicalDevice16BitStorageFeatures s;
    MarshalVkPhysicalDevice16BitStorageFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevice16BitStorageFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevice16BitStorageFeatures* s);
    ~MarshalVkPhysicalDevice16BitStorageFeatures();
};

class MarshalVkPhysicalDeviceSubgroupProperties {
public:
    MarshalVkPhysicalDeviceSubgroupProperties() {}
    VkPhysicalDeviceSubgroupProperties s;
    MarshalVkPhysicalDeviceSubgroupProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSubgroupProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSubgroupProperties* s);
    ~MarshalVkPhysicalDeviceSubgroupProperties();
};

class MarshalVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures {
public:
    MarshalVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures() {}
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures s;
    MarshalVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures* s);
    ~MarshalVkPhysicalDeviceShaderSubgroupExtendedTypesFeatures();
};

class MarshalVkBufferMemoryRequirementsInfo2 {
public:
    MarshalVkBufferMemoryRequirementsInfo2() {}
    VkBufferMemoryRequirementsInfo2 s;
    MarshalVkBufferMemoryRequirementsInfo2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferMemoryRequirementsInfo2* s);
    static void write(KMemory* memory, U32 address, VkBufferMemoryRequirementsInfo2* s);
    ~MarshalVkBufferMemoryRequirementsInfo2();
};

class MarshalVkImageMemoryRequirementsInfo2 {
public:
    MarshalVkImageMemoryRequirementsInfo2() {}
    VkImageMemoryRequirementsInfo2 s;
    MarshalVkImageMemoryRequirementsInfo2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageMemoryRequirementsInfo2* s);
    static void write(KMemory* memory, U32 address, VkImageMemoryRequirementsInfo2* s);
    ~MarshalVkImageMemoryRequirementsInfo2();
};

class MarshalVkImageSparseMemoryRequirementsInfo2 {
public:
    MarshalVkImageSparseMemoryRequirementsInfo2() {}
    VkImageSparseMemoryRequirementsInfo2 s;
    MarshalVkImageSparseMemoryRequirementsInfo2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageSparseMemoryRequirementsInfo2* s);
    static void write(KMemory* memory, U32 address, VkImageSparseMemoryRequirementsInfo2* s);
    ~MarshalVkImageSparseMemoryRequirementsInfo2();
};

class MarshalVkMemoryRequirements2 {
public:
    MarshalVkMemoryRequirements2() {}
    VkMemoryRequirements2 s;
    MarshalVkMemoryRequirements2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryRequirements2* s);
    static void write(KMemory* memory, U32 address, VkMemoryRequirements2* s);
    ~MarshalVkMemoryRequirements2();
};

class MarshalVkSparseImageMemoryRequirements2 {
public:
    MarshalVkSparseImageMemoryRequirements2() {}
    VkSparseImageMemoryRequirements2 s;
    MarshalVkSparseImageMemoryRequirements2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSparseImageMemoryRequirements2* s);
    static void write(KMemory* memory, U32 address, VkSparseImageMemoryRequirements2* s);
    ~MarshalVkSparseImageMemoryRequirements2();
};

class MarshalVkPhysicalDevicePointClippingProperties {
public:
    MarshalVkPhysicalDevicePointClippingProperties() {}
    VkPhysicalDevicePointClippingProperties s;
    MarshalVkPhysicalDevicePointClippingProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevicePointClippingProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevicePointClippingProperties* s);
    ~MarshalVkPhysicalDevicePointClippingProperties();
};

class MarshalVkMemoryDedicatedRequirements {
public:
    MarshalVkMemoryDedicatedRequirements() {}
    VkMemoryDedicatedRequirements s;
    MarshalVkMemoryDedicatedRequirements(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryDedicatedRequirements* s);
    static void write(KMemory* memory, U32 address, VkMemoryDedicatedRequirements* s);
    ~MarshalVkMemoryDedicatedRequirements();
};

class MarshalVkMemoryDedicatedAllocateInfo {
public:
    MarshalVkMemoryDedicatedAllocateInfo() {}
    VkMemoryDedicatedAllocateInfo s;
    MarshalVkMemoryDedicatedAllocateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryDedicatedAllocateInfo* s);
    static void write(KMemory* memory, U32 address, VkMemoryDedicatedAllocateInfo* s);
    ~MarshalVkMemoryDedicatedAllocateInfo();
};

class MarshalVkImageViewUsageCreateInfo {
public:
    MarshalVkImageViewUsageCreateInfo() {}
    VkImageViewUsageCreateInfo s;
    MarshalVkImageViewUsageCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageViewUsageCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkImageViewUsageCreateInfo* s);
    ~MarshalVkImageViewUsageCreateInfo();
};

class MarshalVkPipelineTessellationDomainOriginStateCreateInfo {
public:
    MarshalVkPipelineTessellationDomainOriginStateCreateInfo() {}
    VkPipelineTessellationDomainOriginStateCreateInfo s;
    MarshalVkPipelineTessellationDomainOriginStateCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineTessellationDomainOriginStateCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkPipelineTessellationDomainOriginStateCreateInfo* s);
    ~MarshalVkPipelineTessellationDomainOriginStateCreateInfo();
};

class MarshalVkSamplerYcbcrConversionInfo {
public:
    MarshalVkSamplerYcbcrConversionInfo() {}
    VkSamplerYcbcrConversionInfo s;
    MarshalVkSamplerYcbcrConversionInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSamplerYcbcrConversionInfo* s);
    static void write(KMemory* memory, U32 address, VkSamplerYcbcrConversionInfo* s);
    ~MarshalVkSamplerYcbcrConversionInfo();
};

class MarshalVkSamplerYcbcrConversionCreateInfo {
public:
    MarshalVkSamplerYcbcrConversionCreateInfo() {}
    VkSamplerYcbcrConversionCreateInfo s;
    MarshalVkSamplerYcbcrConversionCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSamplerYcbcrConversionCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkSamplerYcbcrConversionCreateInfo* s);
    ~MarshalVkSamplerYcbcrConversionCreateInfo();
};

class MarshalVkBindImagePlaneMemoryInfo {
public:
    MarshalVkBindImagePlaneMemoryInfo() {}
    VkBindImagePlaneMemoryInfo s;
    MarshalVkBindImagePlaneMemoryInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBindImagePlaneMemoryInfo* s);
    static void write(KMemory* memory, U32 address, VkBindImagePlaneMemoryInfo* s);
    ~MarshalVkBindImagePlaneMemoryInfo();
};

class MarshalVkImagePlaneMemoryRequirementsInfo {
public:
    MarshalVkImagePlaneMemoryRequirementsInfo() {}
    VkImagePlaneMemoryRequirementsInfo s;
    MarshalVkImagePlaneMemoryRequirementsInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImagePlaneMemoryRequirementsInfo* s);
    static void write(KMemory* memory, U32 address, VkImagePlaneMemoryRequirementsInfo* s);
    ~MarshalVkImagePlaneMemoryRequirementsInfo();
};

class MarshalVkPhysicalDeviceSamplerYcbcrConversionFeatures {
public:
    MarshalVkPhysicalDeviceSamplerYcbcrConversionFeatures() {}
    VkPhysicalDeviceSamplerYcbcrConversionFeatures s;
    MarshalVkPhysicalDeviceSamplerYcbcrConversionFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSamplerYcbcrConversionFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSamplerYcbcrConversionFeatures* s);
    ~MarshalVkPhysicalDeviceSamplerYcbcrConversionFeatures();
};

class MarshalVkSamplerYcbcrConversionImageFormatProperties {
public:
    MarshalVkSamplerYcbcrConversionImageFormatProperties() {}
    VkSamplerYcbcrConversionImageFormatProperties s;
    MarshalVkSamplerYcbcrConversionImageFormatProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSamplerYcbcrConversionImageFormatProperties* s);
    static void write(KMemory* memory, U32 address, VkSamplerYcbcrConversionImageFormatProperties* s);
    ~MarshalVkSamplerYcbcrConversionImageFormatProperties();
};

class MarshalVkTextureLODGatherFormatPropertiesAMD {
public:
    MarshalVkTextureLODGatherFormatPropertiesAMD() {}
    VkTextureLODGatherFormatPropertiesAMD s;
    MarshalVkTextureLODGatherFormatPropertiesAMD(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkTextureLODGatherFormatPropertiesAMD* s);
    static void write(KMemory* memory, U32 address, VkTextureLODGatherFormatPropertiesAMD* s);
    ~MarshalVkTextureLODGatherFormatPropertiesAMD();
};

class MarshalVkConditionalRenderingBeginInfoEXT {
public:
    MarshalVkConditionalRenderingBeginInfoEXT() {}
    VkConditionalRenderingBeginInfoEXT s;
    MarshalVkConditionalRenderingBeginInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkConditionalRenderingBeginInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkConditionalRenderingBeginInfoEXT* s);
    ~MarshalVkConditionalRenderingBeginInfoEXT();
};

class MarshalVkProtectedSubmitInfo {
public:
    MarshalVkProtectedSubmitInfo() {}
    VkProtectedSubmitInfo s;
    MarshalVkProtectedSubmitInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkProtectedSubmitInfo* s);
    static void write(KMemory* memory, U32 address, VkProtectedSubmitInfo* s);
    ~MarshalVkProtectedSubmitInfo();
};

class MarshalVkPhysicalDeviceProtectedMemoryFeatures {
public:
    MarshalVkPhysicalDeviceProtectedMemoryFeatures() {}
    VkPhysicalDeviceProtectedMemoryFeatures s;
    MarshalVkPhysicalDeviceProtectedMemoryFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceProtectedMemoryFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceProtectedMemoryFeatures* s);
    ~MarshalVkPhysicalDeviceProtectedMemoryFeatures();
};

class MarshalVkPhysicalDeviceProtectedMemoryProperties {
public:
    MarshalVkPhysicalDeviceProtectedMemoryProperties() {}
    VkPhysicalDeviceProtectedMemoryProperties s;
    MarshalVkPhysicalDeviceProtectedMemoryProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceProtectedMemoryProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceProtectedMemoryProperties* s);
    ~MarshalVkPhysicalDeviceProtectedMemoryProperties();
};

class MarshalVkDeviceQueueInfo2 {
public:
    MarshalVkDeviceQueueInfo2() {}
    VkDeviceQueueInfo2 s;
    MarshalVkDeviceQueueInfo2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceQueueInfo2* s);
    static void write(KMemory* memory, U32 address, VkDeviceQueueInfo2* s);
    ~MarshalVkDeviceQueueInfo2();
};

class MarshalVkPipelineCoverageToColorStateCreateInfoNV {
public:
    MarshalVkPipelineCoverageToColorStateCreateInfoNV() {}
    VkPipelineCoverageToColorStateCreateInfoNV s;
    MarshalVkPipelineCoverageToColorStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineCoverageToColorStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineCoverageToColorStateCreateInfoNV* s);
    ~MarshalVkPipelineCoverageToColorStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceSamplerFilterMinmaxProperties {
public:
    MarshalVkPhysicalDeviceSamplerFilterMinmaxProperties() {}
    VkPhysicalDeviceSamplerFilterMinmaxProperties s;
    MarshalVkPhysicalDeviceSamplerFilterMinmaxProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSamplerFilterMinmaxProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSamplerFilterMinmaxProperties* s);
    ~MarshalVkPhysicalDeviceSamplerFilterMinmaxProperties();
};

class MarshalVkSampleLocationEXT {
public:
    MarshalVkSampleLocationEXT() {}
    VkSampleLocationEXT s;
    MarshalVkSampleLocationEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSampleLocationEXT* s);
};

class MarshalVkSampleLocationsInfoEXT {
public:
    MarshalVkSampleLocationsInfoEXT() {}
    VkSampleLocationsInfoEXT s;
    MarshalVkSampleLocationsInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSampleLocationsInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkSampleLocationsInfoEXT* s);
    ~MarshalVkSampleLocationsInfoEXT();
};

class MarshalVkAttachmentSampleLocationsEXT {
public:
    MarshalVkAttachmentSampleLocationsEXT() {}
    VkAttachmentSampleLocationsEXT s;
    MarshalVkAttachmentSampleLocationsEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAttachmentSampleLocationsEXT* s);
};

class MarshalVkSubpassSampleLocationsEXT {
public:
    MarshalVkSubpassSampleLocationsEXT() {}
    VkSubpassSampleLocationsEXT s;
    MarshalVkSubpassSampleLocationsEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubpassSampleLocationsEXT* s);
};

class MarshalVkRenderPassSampleLocationsBeginInfoEXT {
public:
    MarshalVkRenderPassSampleLocationsBeginInfoEXT() {}
    VkRenderPassSampleLocationsBeginInfoEXT s;
    MarshalVkRenderPassSampleLocationsBeginInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRenderPassSampleLocationsBeginInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkRenderPassSampleLocationsBeginInfoEXT* s);
    ~MarshalVkRenderPassSampleLocationsBeginInfoEXT();
};

class MarshalVkPipelineSampleLocationsStateCreateInfoEXT {
public:
    MarshalVkPipelineSampleLocationsStateCreateInfoEXT() {}
    VkPipelineSampleLocationsStateCreateInfoEXT s;
    MarshalVkPipelineSampleLocationsStateCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineSampleLocationsStateCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineSampleLocationsStateCreateInfoEXT* s);
    ~MarshalVkPipelineSampleLocationsStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceSampleLocationsPropertiesEXT {
public:
    MarshalVkPhysicalDeviceSampleLocationsPropertiesEXT() {}
    VkPhysicalDeviceSampleLocationsPropertiesEXT s;
    MarshalVkPhysicalDeviceSampleLocationsPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSampleLocationsPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSampleLocationsPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceSampleLocationsPropertiesEXT();
};

class MarshalVkMultisamplePropertiesEXT {
public:
    MarshalVkMultisamplePropertiesEXT() {}
    VkMultisamplePropertiesEXT s;
    MarshalVkMultisamplePropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMultisamplePropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkMultisamplePropertiesEXT* s);
    ~MarshalVkMultisamplePropertiesEXT();
};

class MarshalVkSamplerReductionModeCreateInfo {
public:
    MarshalVkSamplerReductionModeCreateInfo() {}
    VkSamplerReductionModeCreateInfo s;
    MarshalVkSamplerReductionModeCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSamplerReductionModeCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkSamplerReductionModeCreateInfo* s);
    ~MarshalVkSamplerReductionModeCreateInfo();
};

class MarshalVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT {
public:
    MarshalVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT() {}
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT s;
    MarshalVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceBlendOperationAdvancedFeaturesEXT();
};

class MarshalVkPhysicalDeviceMultiDrawFeaturesEXT {
public:
    MarshalVkPhysicalDeviceMultiDrawFeaturesEXT() {}
    VkPhysicalDeviceMultiDrawFeaturesEXT s;
    MarshalVkPhysicalDeviceMultiDrawFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMultiDrawFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMultiDrawFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceMultiDrawFeaturesEXT();
};

class MarshalVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT {
public:
    MarshalVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT() {}
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT s;
    MarshalVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceBlendOperationAdvancedPropertiesEXT();
};

class MarshalVkPipelineColorBlendAdvancedStateCreateInfoEXT {
public:
    MarshalVkPipelineColorBlendAdvancedStateCreateInfoEXT() {}
    VkPipelineColorBlendAdvancedStateCreateInfoEXT s;
    MarshalVkPipelineColorBlendAdvancedStateCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineColorBlendAdvancedStateCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineColorBlendAdvancedStateCreateInfoEXT* s);
    ~MarshalVkPipelineColorBlendAdvancedStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceInlineUniformBlockFeaturesEXT {
public:
    MarshalVkPhysicalDeviceInlineUniformBlockFeaturesEXT() {}
    VkPhysicalDeviceInlineUniformBlockFeaturesEXT s;
    MarshalVkPhysicalDeviceInlineUniformBlockFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceInlineUniformBlockFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceInlineUniformBlockFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceInlineUniformBlockFeaturesEXT();
};

class MarshalVkPhysicalDeviceInlineUniformBlockPropertiesEXT {
public:
    MarshalVkPhysicalDeviceInlineUniformBlockPropertiesEXT() {}
    VkPhysicalDeviceInlineUniformBlockPropertiesEXT s;
    MarshalVkPhysicalDeviceInlineUniformBlockPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceInlineUniformBlockPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceInlineUniformBlockPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceInlineUniformBlockPropertiesEXT();
};

class MarshalVkWriteDescriptorSetInlineUniformBlockEXT {
public:
    MarshalVkWriteDescriptorSetInlineUniformBlockEXT() {}
    VkWriteDescriptorSetInlineUniformBlockEXT s;
    MarshalVkWriteDescriptorSetInlineUniformBlockEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkWriteDescriptorSetInlineUniformBlockEXT* s);
    static void write(KMemory* memory, U32 address, VkWriteDescriptorSetInlineUniformBlockEXT* s);
    ~MarshalVkWriteDescriptorSetInlineUniformBlockEXT();
};

class MarshalVkDescriptorPoolInlineUniformBlockCreateInfoEXT {
public:
    MarshalVkDescriptorPoolInlineUniformBlockCreateInfoEXT() {}
    VkDescriptorPoolInlineUniformBlockCreateInfoEXT s;
    MarshalVkDescriptorPoolInlineUniformBlockCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorPoolInlineUniformBlockCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDescriptorPoolInlineUniformBlockCreateInfoEXT* s);
    ~MarshalVkDescriptorPoolInlineUniformBlockCreateInfoEXT();
};

class MarshalVkPipelineCoverageModulationStateCreateInfoNV {
public:
    MarshalVkPipelineCoverageModulationStateCreateInfoNV() {}
    VkPipelineCoverageModulationStateCreateInfoNV s;
    MarshalVkPipelineCoverageModulationStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineCoverageModulationStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineCoverageModulationStateCreateInfoNV* s);
    ~MarshalVkPipelineCoverageModulationStateCreateInfoNV();
};

class MarshalVkImageFormatListCreateInfo {
public:
    MarshalVkImageFormatListCreateInfo() {}
    VkImageFormatListCreateInfo s;
    MarshalVkImageFormatListCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageFormatListCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkImageFormatListCreateInfo* s);
    ~MarshalVkImageFormatListCreateInfo();
};

class MarshalVkValidationCacheCreateInfoEXT {
public:
    MarshalVkValidationCacheCreateInfoEXT() {}
    VkValidationCacheCreateInfoEXT s;
    MarshalVkValidationCacheCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkValidationCacheCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkValidationCacheCreateInfoEXT* s);
    ~MarshalVkValidationCacheCreateInfoEXT();
};

class MarshalVkShaderModuleValidationCacheCreateInfoEXT {
public:
    MarshalVkShaderModuleValidationCacheCreateInfoEXT() {}
    VkShaderModuleValidationCacheCreateInfoEXT s;
    MarshalVkShaderModuleValidationCacheCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkShaderModuleValidationCacheCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkShaderModuleValidationCacheCreateInfoEXT* s);
    ~MarshalVkShaderModuleValidationCacheCreateInfoEXT();
};

class MarshalVkPhysicalDeviceMaintenance3Properties {
public:
    MarshalVkPhysicalDeviceMaintenance3Properties() {}
    VkPhysicalDeviceMaintenance3Properties s;
    MarshalVkPhysicalDeviceMaintenance3Properties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMaintenance3Properties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMaintenance3Properties* s);
    ~MarshalVkPhysicalDeviceMaintenance3Properties();
};

class MarshalVkDescriptorSetLayoutSupport {
public:
    MarshalVkDescriptorSetLayoutSupport() {}
    VkDescriptorSetLayoutSupport s;
    MarshalVkDescriptorSetLayoutSupport(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorSetLayoutSupport* s);
    static void write(KMemory* memory, U32 address, VkDescriptorSetLayoutSupport* s);
    ~MarshalVkDescriptorSetLayoutSupport();
};

class MarshalVkPhysicalDeviceShaderDrawParametersFeatures {
public:
    MarshalVkPhysicalDeviceShaderDrawParametersFeatures() {}
    VkPhysicalDeviceShaderDrawParametersFeatures s;
    MarshalVkPhysicalDeviceShaderDrawParametersFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderDrawParametersFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderDrawParametersFeatures* s);
    ~MarshalVkPhysicalDeviceShaderDrawParametersFeatures();
};

class MarshalVkPhysicalDeviceShaderFloat16Int8Features {
public:
    MarshalVkPhysicalDeviceShaderFloat16Int8Features() {}
    VkPhysicalDeviceShaderFloat16Int8Features s;
    MarshalVkPhysicalDeviceShaderFloat16Int8Features(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderFloat16Int8Features* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderFloat16Int8Features* s);
    ~MarshalVkPhysicalDeviceShaderFloat16Int8Features();
};

class MarshalVkPhysicalDeviceFloatControlsProperties {
public:
    MarshalVkPhysicalDeviceFloatControlsProperties() {}
    VkPhysicalDeviceFloatControlsProperties s;
    MarshalVkPhysicalDeviceFloatControlsProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFloatControlsProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFloatControlsProperties* s);
    ~MarshalVkPhysicalDeviceFloatControlsProperties();
};

class MarshalVkPhysicalDeviceHostQueryResetFeatures {
public:
    MarshalVkPhysicalDeviceHostQueryResetFeatures() {}
    VkPhysicalDeviceHostQueryResetFeatures s;
    MarshalVkPhysicalDeviceHostQueryResetFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceHostQueryResetFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceHostQueryResetFeatures* s);
    ~MarshalVkPhysicalDeviceHostQueryResetFeatures();
};

class MarshalVkDeviceQueueGlobalPriorityCreateInfoEXT {
public:
    MarshalVkDeviceQueueGlobalPriorityCreateInfoEXT() {}
    VkDeviceQueueGlobalPriorityCreateInfoEXT s;
    MarshalVkDeviceQueueGlobalPriorityCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceQueueGlobalPriorityCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDeviceQueueGlobalPriorityCreateInfoEXT* s);
    ~MarshalVkDeviceQueueGlobalPriorityCreateInfoEXT();
};

class MarshalVkPhysicalDeviceGlobalPriorityQueryFeaturesEXT {
public:
    MarshalVkPhysicalDeviceGlobalPriorityQueryFeaturesEXT() {}
    VkPhysicalDeviceGlobalPriorityQueryFeaturesEXT s;
    MarshalVkPhysicalDeviceGlobalPriorityQueryFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceGlobalPriorityQueryFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceGlobalPriorityQueryFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceGlobalPriorityQueryFeaturesEXT();
};

class MarshalVkQueueFamilyGlobalPriorityPropertiesEXT {
public:
    MarshalVkQueueFamilyGlobalPriorityPropertiesEXT() {}
    VkQueueFamilyGlobalPriorityPropertiesEXT s;
    MarshalVkQueueFamilyGlobalPriorityPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkQueueFamilyGlobalPriorityPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkQueueFamilyGlobalPriorityPropertiesEXT* s);
    ~MarshalVkQueueFamilyGlobalPriorityPropertiesEXT();
};

class MarshalVkDebugUtilsObjectNameInfoEXT {
public:
    MarshalVkDebugUtilsObjectNameInfoEXT() {}
    VkDebugUtilsObjectNameInfoEXT s;
    MarshalVkDebugUtilsObjectNameInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDebugUtilsObjectNameInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDebugUtilsObjectNameInfoEXT* s);
    ~MarshalVkDebugUtilsObjectNameInfoEXT();
};

class MarshalVkDebugUtilsObjectTagInfoEXT {
public:
    MarshalVkDebugUtilsObjectTagInfoEXT() {}
    VkDebugUtilsObjectTagInfoEXT s;
    MarshalVkDebugUtilsObjectTagInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDebugUtilsObjectTagInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkDebugUtilsObjectTagInfoEXT* s);
    ~MarshalVkDebugUtilsObjectTagInfoEXT();
};

class MarshalVkDebugUtilsLabelEXT {
public:
    MarshalVkDebugUtilsLabelEXT() {}
    VkDebugUtilsLabelEXT s;
    MarshalVkDebugUtilsLabelEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDebugUtilsLabelEXT* s);
    static void write(KMemory* memory, U32 address, VkDebugUtilsLabelEXT* s);
    ~MarshalVkDebugUtilsLabelEXT();
};

class MarshalVkDebugUtilsMessengerCallbackDataEXT {
public:
    MarshalVkDebugUtilsMessengerCallbackDataEXT() {}
    VkDebugUtilsMessengerCallbackDataEXT s;
    MarshalVkDebugUtilsMessengerCallbackDataEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDebugUtilsMessengerCallbackDataEXT* s);
    static void write(KMemory* memory, U32 address, VkDebugUtilsMessengerCallbackDataEXT* s);
    ~MarshalVkDebugUtilsMessengerCallbackDataEXT();
};

class MarshalVkPhysicalDeviceDeviceMemoryReportFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDeviceMemoryReportFeaturesEXT() {}
    VkPhysicalDeviceDeviceMemoryReportFeaturesEXT s;
    MarshalVkPhysicalDeviceDeviceMemoryReportFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDeviceMemoryReportFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDeviceMemoryReportFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDeviceMemoryReportFeaturesEXT();
};

class MarshalVkDeviceMemoryReportCallbackDataEXT {
public:
    MarshalVkDeviceMemoryReportCallbackDataEXT() {}
    VkDeviceMemoryReportCallbackDataEXT s;
    MarshalVkDeviceMemoryReportCallbackDataEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceMemoryReportCallbackDataEXT* s);
    static void write(KMemory* memory, U32 address, VkDeviceMemoryReportCallbackDataEXT* s);
    ~MarshalVkDeviceMemoryReportCallbackDataEXT();
};

class MarshalVkMemoryHostPointerPropertiesEXT {
public:
    MarshalVkMemoryHostPointerPropertiesEXT() {}
    VkMemoryHostPointerPropertiesEXT s;
    MarshalVkMemoryHostPointerPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryHostPointerPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkMemoryHostPointerPropertiesEXT* s);
    ~MarshalVkMemoryHostPointerPropertiesEXT();
};

class MarshalVkPhysicalDeviceExternalMemoryHostPropertiesEXT {
public:
    MarshalVkPhysicalDeviceExternalMemoryHostPropertiesEXT() {}
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT s;
    MarshalVkPhysicalDeviceExternalMemoryHostPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceExternalMemoryHostPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceExternalMemoryHostPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceExternalMemoryHostPropertiesEXT();
};

class MarshalVkPhysicalDeviceConservativeRasterizationPropertiesEXT {
public:
    MarshalVkPhysicalDeviceConservativeRasterizationPropertiesEXT() {}
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT s;
    MarshalVkPhysicalDeviceConservativeRasterizationPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceConservativeRasterizationPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceConservativeRasterizationPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceConservativeRasterizationPropertiesEXT();
};

class MarshalVkCalibratedTimestampInfoEXT {
public:
    MarshalVkCalibratedTimestampInfoEXT() {}
    VkCalibratedTimestampInfoEXT s;
    MarshalVkCalibratedTimestampInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCalibratedTimestampInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkCalibratedTimestampInfoEXT* s);
    ~MarshalVkCalibratedTimestampInfoEXT();
};

class MarshalVkPhysicalDeviceShaderCorePropertiesAMD {
public:
    MarshalVkPhysicalDeviceShaderCorePropertiesAMD() {}
    VkPhysicalDeviceShaderCorePropertiesAMD s;
    MarshalVkPhysicalDeviceShaderCorePropertiesAMD(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderCorePropertiesAMD* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderCorePropertiesAMD* s);
    ~MarshalVkPhysicalDeviceShaderCorePropertiesAMD();
};

class MarshalVkPhysicalDeviceShaderCoreProperties2AMD {
public:
    MarshalVkPhysicalDeviceShaderCoreProperties2AMD() {}
    VkPhysicalDeviceShaderCoreProperties2AMD s;
    MarshalVkPhysicalDeviceShaderCoreProperties2AMD(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderCoreProperties2AMD* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderCoreProperties2AMD* s);
    ~MarshalVkPhysicalDeviceShaderCoreProperties2AMD();
};

class MarshalVkPipelineRasterizationConservativeStateCreateInfoEXT {
public:
    MarshalVkPipelineRasterizationConservativeStateCreateInfoEXT() {}
    VkPipelineRasterizationConservativeStateCreateInfoEXT s;
    MarshalVkPipelineRasterizationConservativeStateCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineRasterizationConservativeStateCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineRasterizationConservativeStateCreateInfoEXT* s);
    ~MarshalVkPipelineRasterizationConservativeStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceDescriptorIndexingFeatures {
public:
    MarshalVkPhysicalDeviceDescriptorIndexingFeatures() {}
    VkPhysicalDeviceDescriptorIndexingFeatures s;
    MarshalVkPhysicalDeviceDescriptorIndexingFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDescriptorIndexingFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDescriptorIndexingFeatures* s);
    ~MarshalVkPhysicalDeviceDescriptorIndexingFeatures();
};

class MarshalVkPhysicalDeviceDescriptorIndexingProperties {
public:
    MarshalVkPhysicalDeviceDescriptorIndexingProperties() {}
    VkPhysicalDeviceDescriptorIndexingProperties s;
    MarshalVkPhysicalDeviceDescriptorIndexingProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDescriptorIndexingProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDescriptorIndexingProperties* s);
    ~MarshalVkPhysicalDeviceDescriptorIndexingProperties();
};

class MarshalVkDescriptorSetLayoutBindingFlagsCreateInfo {
public:
    MarshalVkDescriptorSetLayoutBindingFlagsCreateInfo() {}
    VkDescriptorSetLayoutBindingFlagsCreateInfo s;
    MarshalVkDescriptorSetLayoutBindingFlagsCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorSetLayoutBindingFlagsCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkDescriptorSetLayoutBindingFlagsCreateInfo* s);
    ~MarshalVkDescriptorSetLayoutBindingFlagsCreateInfo();
};

class MarshalVkDescriptorSetVariableDescriptorCountAllocateInfo {
public:
    MarshalVkDescriptorSetVariableDescriptorCountAllocateInfo() {}
    VkDescriptorSetVariableDescriptorCountAllocateInfo s;
    MarshalVkDescriptorSetVariableDescriptorCountAllocateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorSetVariableDescriptorCountAllocateInfo* s);
    static void write(KMemory* memory, U32 address, VkDescriptorSetVariableDescriptorCountAllocateInfo* s);
    ~MarshalVkDescriptorSetVariableDescriptorCountAllocateInfo();
};

class MarshalVkDescriptorSetVariableDescriptorCountLayoutSupport {
public:
    MarshalVkDescriptorSetVariableDescriptorCountLayoutSupport() {}
    VkDescriptorSetVariableDescriptorCountLayoutSupport s;
    MarshalVkDescriptorSetVariableDescriptorCountLayoutSupport(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDescriptorSetVariableDescriptorCountLayoutSupport* s);
    static void write(KMemory* memory, U32 address, VkDescriptorSetVariableDescriptorCountLayoutSupport* s);
    ~MarshalVkDescriptorSetVariableDescriptorCountLayoutSupport();
};

class MarshalVkAttachmentDescription2 {
public:
    MarshalVkAttachmentDescription2() {}
    VkAttachmentDescription2 s;
    MarshalVkAttachmentDescription2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAttachmentDescription2* s);
    static void write(KMemory* memory, U32 address, VkAttachmentDescription2* s);
    ~MarshalVkAttachmentDescription2();
};

class MarshalVkAttachmentReference2 {
public:
    MarshalVkAttachmentReference2() {}
    VkAttachmentReference2 s;
    MarshalVkAttachmentReference2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAttachmentReference2* s);
    static void write(KMemory* memory, U32 address, VkAttachmentReference2* s);
    ~MarshalVkAttachmentReference2();
};

class MarshalVkSubpassDescription2 {
public:
    MarshalVkSubpassDescription2() {}
    VkSubpassDescription2 s;
    MarshalVkSubpassDescription2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubpassDescription2* s);
    static void write(KMemory* memory, U32 address, VkSubpassDescription2* s);
    ~MarshalVkSubpassDescription2();
};

class MarshalVkSubpassDependency2 {
public:
    MarshalVkSubpassDependency2() {}
    VkSubpassDependency2 s;
    MarshalVkSubpassDependency2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubpassDependency2* s);
    static void write(KMemory* memory, U32 address, VkSubpassDependency2* s);
    ~MarshalVkSubpassDependency2();
};

class MarshalVkRenderPassCreateInfo2 {
public:
    MarshalVkRenderPassCreateInfo2() {}
    VkRenderPassCreateInfo2 s;
    MarshalVkRenderPassCreateInfo2(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRenderPassCreateInfo2* s);
    static void write(KMemory* memory, U32 address, VkRenderPassCreateInfo2* s);
    ~MarshalVkRenderPassCreateInfo2();
};

class MarshalVkSubpassBeginInfo {
public:
    MarshalVkSubpassBeginInfo() {}
    VkSubpassBeginInfo s;
    MarshalVkSubpassBeginInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubpassBeginInfo* s);
    static void write(KMemory* memory, U32 address, VkSubpassBeginInfo* s);
    ~MarshalVkSubpassBeginInfo();
};

class MarshalVkSubpassEndInfo {
public:
    MarshalVkSubpassEndInfo() {}
    VkSubpassEndInfo s;
    MarshalVkSubpassEndInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubpassEndInfo* s);
    static void write(KMemory* memory, U32 address, VkSubpassEndInfo* s);
    ~MarshalVkSubpassEndInfo();
};

class MarshalVkPhysicalDeviceTimelineSemaphoreFeatures {
public:
    MarshalVkPhysicalDeviceTimelineSemaphoreFeatures() {}
    VkPhysicalDeviceTimelineSemaphoreFeatures s;
    MarshalVkPhysicalDeviceTimelineSemaphoreFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceTimelineSemaphoreFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceTimelineSemaphoreFeatures* s);
    ~MarshalVkPhysicalDeviceTimelineSemaphoreFeatures();
};

class MarshalVkPhysicalDeviceTimelineSemaphoreProperties {
public:
    MarshalVkPhysicalDeviceTimelineSemaphoreProperties() {}
    VkPhysicalDeviceTimelineSemaphoreProperties s;
    MarshalVkPhysicalDeviceTimelineSemaphoreProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceTimelineSemaphoreProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceTimelineSemaphoreProperties* s);
    ~MarshalVkPhysicalDeviceTimelineSemaphoreProperties();
};

class MarshalVkSemaphoreTypeCreateInfo {
public:
    MarshalVkSemaphoreTypeCreateInfo() {}
    VkSemaphoreTypeCreateInfo s;
    MarshalVkSemaphoreTypeCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSemaphoreTypeCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkSemaphoreTypeCreateInfo* s);
    ~MarshalVkSemaphoreTypeCreateInfo();
};

class MarshalVkTimelineSemaphoreSubmitInfo {
public:
    MarshalVkTimelineSemaphoreSubmitInfo() {}
    VkTimelineSemaphoreSubmitInfo s;
    MarshalVkTimelineSemaphoreSubmitInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkTimelineSemaphoreSubmitInfo* s);
    static void write(KMemory* memory, U32 address, VkTimelineSemaphoreSubmitInfo* s);
    ~MarshalVkTimelineSemaphoreSubmitInfo();
};

class MarshalVkSemaphoreWaitInfo {
public:
    MarshalVkSemaphoreWaitInfo() {}
    VkSemaphoreWaitInfo s;
    MarshalVkSemaphoreWaitInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSemaphoreWaitInfo* s);
    static void write(KMemory* memory, U32 address, VkSemaphoreWaitInfo* s);
    ~MarshalVkSemaphoreWaitInfo();
};

class MarshalVkSemaphoreSignalInfo {
public:
    MarshalVkSemaphoreSignalInfo() {}
    VkSemaphoreSignalInfo s;
    MarshalVkSemaphoreSignalInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSemaphoreSignalInfo* s);
    static void write(KMemory* memory, U32 address, VkSemaphoreSignalInfo* s);
    ~MarshalVkSemaphoreSignalInfo();
};

class MarshalVkVertexInputBindingDivisorDescriptionEXT {
public:
    MarshalVkVertexInputBindingDivisorDescriptionEXT() {}
    VkVertexInputBindingDivisorDescriptionEXT s;
    MarshalVkVertexInputBindingDivisorDescriptionEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkVertexInputBindingDivisorDescriptionEXT* s);
};

class MarshalVkPipelineVertexInputDivisorStateCreateInfoEXT {
public:
    MarshalVkPipelineVertexInputDivisorStateCreateInfoEXT() {}
    VkPipelineVertexInputDivisorStateCreateInfoEXT s;
    MarshalVkPipelineVertexInputDivisorStateCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineVertexInputDivisorStateCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineVertexInputDivisorStateCreateInfoEXT* s);
    ~MarshalVkPipelineVertexInputDivisorStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT {
public:
    MarshalVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT() {}
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT s;
    MarshalVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceVertexAttributeDivisorPropertiesEXT();
};

class MarshalVkPhysicalDevicePCIBusInfoPropertiesEXT {
public:
    MarshalVkPhysicalDevicePCIBusInfoPropertiesEXT() {}
    VkPhysicalDevicePCIBusInfoPropertiesEXT s;
    MarshalVkPhysicalDevicePCIBusInfoPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevicePCIBusInfoPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevicePCIBusInfoPropertiesEXT* s);
    ~MarshalVkPhysicalDevicePCIBusInfoPropertiesEXT();
};

class MarshalVkCommandBufferInheritanceConditionalRenderingInfoEXT {
public:
    MarshalVkCommandBufferInheritanceConditionalRenderingInfoEXT() {}
    VkCommandBufferInheritanceConditionalRenderingInfoEXT s;
    MarshalVkCommandBufferInheritanceConditionalRenderingInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCommandBufferInheritanceConditionalRenderingInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkCommandBufferInheritanceConditionalRenderingInfoEXT* s);
    ~MarshalVkCommandBufferInheritanceConditionalRenderingInfoEXT();
};

class MarshalVkPhysicalDevice8BitStorageFeatures {
public:
    MarshalVkPhysicalDevice8BitStorageFeatures() {}
    VkPhysicalDevice8BitStorageFeatures s;
    MarshalVkPhysicalDevice8BitStorageFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevice8BitStorageFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevice8BitStorageFeatures* s);
    ~MarshalVkPhysicalDevice8BitStorageFeatures();
};

class MarshalVkPhysicalDeviceConditionalRenderingFeaturesEXT {
public:
    MarshalVkPhysicalDeviceConditionalRenderingFeaturesEXT() {}
    VkPhysicalDeviceConditionalRenderingFeaturesEXT s;
    MarshalVkPhysicalDeviceConditionalRenderingFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceConditionalRenderingFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceConditionalRenderingFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceConditionalRenderingFeaturesEXT();
};

class MarshalVkPhysicalDeviceVulkanMemoryModelFeatures {
public:
    MarshalVkPhysicalDeviceVulkanMemoryModelFeatures() {}
    VkPhysicalDeviceVulkanMemoryModelFeatures s;
    MarshalVkPhysicalDeviceVulkanMemoryModelFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceVulkanMemoryModelFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceVulkanMemoryModelFeatures* s);
    ~MarshalVkPhysicalDeviceVulkanMemoryModelFeatures();
};

class MarshalVkPhysicalDeviceShaderAtomicInt64Features {
public:
    MarshalVkPhysicalDeviceShaderAtomicInt64Features() {}
    VkPhysicalDeviceShaderAtomicInt64Features s;
    MarshalVkPhysicalDeviceShaderAtomicInt64Features(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicInt64Features* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicInt64Features* s);
    ~MarshalVkPhysicalDeviceShaderAtomicInt64Features();
};

class MarshalVkPhysicalDeviceShaderAtomicFloatFeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderAtomicFloatFeaturesEXT() {}
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT s;
    MarshalVkPhysicalDeviceShaderAtomicFloatFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderAtomicFloatFeaturesEXT();
};

class MarshalVkPhysicalDeviceVertexAttributeDivisorFeaturesEXT {
public:
    MarshalVkPhysicalDeviceVertexAttributeDivisorFeaturesEXT() {}
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT s;
    MarshalVkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceVertexAttributeDivisorFeaturesEXT();
};

class MarshalVkQueueFamilyCheckpointPropertiesNV {
public:
    MarshalVkQueueFamilyCheckpointPropertiesNV() {}
    VkQueueFamilyCheckpointPropertiesNV s;
    MarshalVkQueueFamilyCheckpointPropertiesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkQueueFamilyCheckpointPropertiesNV* s);
    static void write(KMemory* memory, U32 address, VkQueueFamilyCheckpointPropertiesNV* s);
    ~MarshalVkQueueFamilyCheckpointPropertiesNV();
};

class MarshalVkPhysicalDeviceDepthStencilResolveProperties {
public:
    MarshalVkPhysicalDeviceDepthStencilResolveProperties() {}
    VkPhysicalDeviceDepthStencilResolveProperties s;
    MarshalVkPhysicalDeviceDepthStencilResolveProperties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDepthStencilResolveProperties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDepthStencilResolveProperties* s);
    ~MarshalVkPhysicalDeviceDepthStencilResolveProperties();
};

class MarshalVkSubpassDescriptionDepthStencilResolve {
public:
    MarshalVkSubpassDescriptionDepthStencilResolve() {}
    VkSubpassDescriptionDepthStencilResolve s;
    MarshalVkSubpassDescriptionDepthStencilResolve(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubpassDescriptionDepthStencilResolve* s);
    static void write(KMemory* memory, U32 address, VkSubpassDescriptionDepthStencilResolve* s);
    ~MarshalVkSubpassDescriptionDepthStencilResolve();
};

class MarshalVkImageViewASTCDecodeModeEXT {
public:
    MarshalVkImageViewASTCDecodeModeEXT() {}
    VkImageViewASTCDecodeModeEXT s;
    MarshalVkImageViewASTCDecodeModeEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageViewASTCDecodeModeEXT* s);
    static void write(KMemory* memory, U32 address, VkImageViewASTCDecodeModeEXT* s);
    ~MarshalVkImageViewASTCDecodeModeEXT();
};

class MarshalVkPhysicalDeviceASTCDecodeFeaturesEXT {
public:
    MarshalVkPhysicalDeviceASTCDecodeFeaturesEXT() {}
    VkPhysicalDeviceASTCDecodeFeaturesEXT s;
    MarshalVkPhysicalDeviceASTCDecodeFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceASTCDecodeFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceASTCDecodeFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceASTCDecodeFeaturesEXT();
};

class MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT {
public:
    MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT() {}
    VkPhysicalDeviceTransformFeedbackFeaturesEXT s;
    MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceTransformFeedbackFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceTransformFeedbackFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT();
};

class MarshalVkPhysicalDeviceTransformFeedbackPropertiesEXT {
public:
    MarshalVkPhysicalDeviceTransformFeedbackPropertiesEXT() {}
    VkPhysicalDeviceTransformFeedbackPropertiesEXT s;
    MarshalVkPhysicalDeviceTransformFeedbackPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceTransformFeedbackPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceTransformFeedbackPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceTransformFeedbackPropertiesEXT();
};

class MarshalVkPipelineRasterizationStateStreamCreateInfoEXT {
public:
    MarshalVkPipelineRasterizationStateStreamCreateInfoEXT() {}
    VkPipelineRasterizationStateStreamCreateInfoEXT s;
    MarshalVkPipelineRasterizationStateStreamCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineRasterizationStateStreamCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineRasterizationStateStreamCreateInfoEXT* s);
    ~MarshalVkPipelineRasterizationStateStreamCreateInfoEXT();
};

class MarshalVkPhysicalDeviceRepresentativeFragmentTestFeaturesNV {
public:
    MarshalVkPhysicalDeviceRepresentativeFragmentTestFeaturesNV() {}
    VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV s;
    MarshalVkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* s);
    ~MarshalVkPhysicalDeviceRepresentativeFragmentTestFeaturesNV();
};

class MarshalVkPipelineRepresentativeFragmentTestStateCreateInfoNV {
public:
    MarshalVkPipelineRepresentativeFragmentTestStateCreateInfoNV() {}
    VkPipelineRepresentativeFragmentTestStateCreateInfoNV s;
    MarshalVkPipelineRepresentativeFragmentTestStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineRepresentativeFragmentTestStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineRepresentativeFragmentTestStateCreateInfoNV* s);
    ~MarshalVkPipelineRepresentativeFragmentTestStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceExclusiveScissorFeaturesNV {
public:
    MarshalVkPhysicalDeviceExclusiveScissorFeaturesNV() {}
    VkPhysicalDeviceExclusiveScissorFeaturesNV s;
    MarshalVkPhysicalDeviceExclusiveScissorFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceExclusiveScissorFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceExclusiveScissorFeaturesNV* s);
    ~MarshalVkPhysicalDeviceExclusiveScissorFeaturesNV();
};

class MarshalVkPipelineViewportExclusiveScissorStateCreateInfoNV {
public:
    MarshalVkPipelineViewportExclusiveScissorStateCreateInfoNV() {}
    VkPipelineViewportExclusiveScissorStateCreateInfoNV s;
    MarshalVkPipelineViewportExclusiveScissorStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineViewportExclusiveScissorStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineViewportExclusiveScissorStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportExclusiveScissorStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceCornerSampledImageFeaturesNV {
public:
    MarshalVkPhysicalDeviceCornerSampledImageFeaturesNV() {}
    VkPhysicalDeviceCornerSampledImageFeaturesNV s;
    MarshalVkPhysicalDeviceCornerSampledImageFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceCornerSampledImageFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceCornerSampledImageFeaturesNV* s);
    ~MarshalVkPhysicalDeviceCornerSampledImageFeaturesNV();
};

class MarshalVkPhysicalDeviceComputeShaderDerivativesFeaturesNV {
public:
    MarshalVkPhysicalDeviceComputeShaderDerivativesFeaturesNV() {}
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV s;
    MarshalVkPhysicalDeviceComputeShaderDerivativesFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceComputeShaderDerivativesFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceComputeShaderDerivativesFeaturesNV* s);
    ~MarshalVkPhysicalDeviceComputeShaderDerivativesFeaturesNV();
};

class MarshalVkPhysicalDeviceFragmentShaderBarycentricFeaturesNV {
public:
    MarshalVkPhysicalDeviceFragmentShaderBarycentricFeaturesNV() {}
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV s;
    MarshalVkPhysicalDeviceFragmentShaderBarycentricFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV* s);
    ~MarshalVkPhysicalDeviceFragmentShaderBarycentricFeaturesNV();
};

class MarshalVkPhysicalDeviceShaderImageFootprintFeaturesNV {
public:
    MarshalVkPhysicalDeviceShaderImageFootprintFeaturesNV() {}
    VkPhysicalDeviceShaderImageFootprintFeaturesNV s;
    MarshalVkPhysicalDeviceShaderImageFootprintFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderImageFootprintFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderImageFootprintFeaturesNV* s);
    ~MarshalVkPhysicalDeviceShaderImageFootprintFeaturesNV();
};

class MarshalVkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV {
public:
    MarshalVkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV() {}
    VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV s;
    MarshalVkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* s);
    ~MarshalVkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV();
};

class MarshalVkShadingRatePaletteNV {
public:
    MarshalVkShadingRatePaletteNV() {}
    VkShadingRatePaletteNV s;
    MarshalVkShadingRatePaletteNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkShadingRatePaletteNV* s);
    ~MarshalVkShadingRatePaletteNV();
};

class MarshalVkPipelineViewportShadingRateImageStateCreateInfoNV {
public:
    MarshalVkPipelineViewportShadingRateImageStateCreateInfoNV() {}
    VkPipelineViewportShadingRateImageStateCreateInfoNV s;
    MarshalVkPipelineViewportShadingRateImageStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineViewportShadingRateImageStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineViewportShadingRateImageStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportShadingRateImageStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceShadingRateImageFeaturesNV {
public:
    MarshalVkPhysicalDeviceShadingRateImageFeaturesNV() {}
    VkPhysicalDeviceShadingRateImageFeaturesNV s;
    MarshalVkPhysicalDeviceShadingRateImageFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShadingRateImageFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShadingRateImageFeaturesNV* s);
    ~MarshalVkPhysicalDeviceShadingRateImageFeaturesNV();
};

class MarshalVkPhysicalDeviceShadingRateImagePropertiesNV {
public:
    MarshalVkPhysicalDeviceShadingRateImagePropertiesNV() {}
    VkPhysicalDeviceShadingRateImagePropertiesNV s;
    MarshalVkPhysicalDeviceShadingRateImagePropertiesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShadingRateImagePropertiesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShadingRateImagePropertiesNV* s);
    ~MarshalVkPhysicalDeviceShadingRateImagePropertiesNV();
};

class MarshalVkCoarseSampleOrderCustomNV {
public:
    MarshalVkCoarseSampleOrderCustomNV() {}
    VkCoarseSampleOrderCustomNV s;
    MarshalVkCoarseSampleOrderCustomNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCoarseSampleOrderCustomNV* s);
    ~MarshalVkCoarseSampleOrderCustomNV();
};

class MarshalVkPipelineViewportCoarseSampleOrderStateCreateInfoNV {
public:
    MarshalVkPipelineViewportCoarseSampleOrderStateCreateInfoNV() {}
    VkPipelineViewportCoarseSampleOrderStateCreateInfoNV s;
    MarshalVkPipelineViewportCoarseSampleOrderStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* s);
    ~MarshalVkPipelineViewportCoarseSampleOrderStateCreateInfoNV();
};

class MarshalVkPhysicalDeviceMeshShaderFeaturesNV {
public:
    MarshalVkPhysicalDeviceMeshShaderFeaturesNV() {}
    VkPhysicalDeviceMeshShaderFeaturesNV s;
    MarshalVkPhysicalDeviceMeshShaderFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderFeaturesNV* s);
    ~MarshalVkPhysicalDeviceMeshShaderFeaturesNV();
};

class MarshalVkPhysicalDeviceMeshShaderPropertiesNV {
public:
    MarshalVkPhysicalDeviceMeshShaderPropertiesNV() {}
    VkPhysicalDeviceMeshShaderPropertiesNV s;
    MarshalVkPhysicalDeviceMeshShaderPropertiesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderPropertiesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMeshShaderPropertiesNV* s);
    ~MarshalVkPhysicalDeviceMeshShaderPropertiesNV();
};

class MarshalVkRayTracingShaderGroupCreateInfoNV {
public:
    MarshalVkRayTracingShaderGroupCreateInfoNV() {}
    VkRayTracingShaderGroupCreateInfoNV s;
    MarshalVkRayTracingShaderGroupCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRayTracingShaderGroupCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkRayTracingShaderGroupCreateInfoNV* s);
    ~MarshalVkRayTracingShaderGroupCreateInfoNV();
};

class MarshalVkRayTracingPipelineCreateInfoNV {
public:
    MarshalVkRayTracingPipelineCreateInfoNV() {}
    VkRayTracingPipelineCreateInfoNV s;
    MarshalVkRayTracingPipelineCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRayTracingPipelineCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkRayTracingPipelineCreateInfoNV* s);
    ~MarshalVkRayTracingPipelineCreateInfoNV();
};

class MarshalVkGeometryTrianglesNV {
public:
    MarshalVkGeometryTrianglesNV() {}
    VkGeometryTrianglesNV s;
    MarshalVkGeometryTrianglesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkGeometryTrianglesNV* s);
    static void write(KMemory* memory, U32 address, VkGeometryTrianglesNV* s);
    ~MarshalVkGeometryTrianglesNV();
};

class MarshalVkGeometryAABBNV {
public:
    MarshalVkGeometryAABBNV() {}
    VkGeometryAABBNV s;
    MarshalVkGeometryAABBNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkGeometryAABBNV* s);
    static void write(KMemory* memory, U32 address, VkGeometryAABBNV* s);
    ~MarshalVkGeometryAABBNV();
};

class MarshalVkGeometryDataNV {
public:
    MarshalVkGeometryDataNV() {}
    VkGeometryDataNV s;
    MarshalVkGeometryDataNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkGeometryDataNV* s);
};

class MarshalVkGeometryNV {
public:
    MarshalVkGeometryNV() {}
    VkGeometryNV s;
    MarshalVkGeometryNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkGeometryNV* s);
    static void write(KMemory* memory, U32 address, VkGeometryNV* s);
    ~MarshalVkGeometryNV();
};

class MarshalVkAccelerationStructureInfoNV {
public:
    MarshalVkAccelerationStructureInfoNV() {}
    VkAccelerationStructureInfoNV s;
    MarshalVkAccelerationStructureInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureInfoNV* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureInfoNV* s);
    ~MarshalVkAccelerationStructureInfoNV();
};

class MarshalVkAccelerationStructureCreateInfoNV {
public:
    MarshalVkAccelerationStructureCreateInfoNV() {}
    VkAccelerationStructureCreateInfoNV s;
    MarshalVkAccelerationStructureCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureCreateInfoNV* s);
    ~MarshalVkAccelerationStructureCreateInfoNV();
};

class MarshalVkBindAccelerationStructureMemoryInfoNV {
public:
    MarshalVkBindAccelerationStructureMemoryInfoNV() {}
    VkBindAccelerationStructureMemoryInfoNV s;
    MarshalVkBindAccelerationStructureMemoryInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBindAccelerationStructureMemoryInfoNV* s);
    static void write(KMemory* memory, U32 address, VkBindAccelerationStructureMemoryInfoNV* s);
    ~MarshalVkBindAccelerationStructureMemoryInfoNV();
};

class MarshalVkWriteDescriptorSetAccelerationStructureKHR {
public:
    MarshalVkWriteDescriptorSetAccelerationStructureKHR() {}
    VkWriteDescriptorSetAccelerationStructureKHR s;
    MarshalVkWriteDescriptorSetAccelerationStructureKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkWriteDescriptorSetAccelerationStructureKHR* s);
    static void write(KMemory* memory, U32 address, VkWriteDescriptorSetAccelerationStructureKHR* s);
    ~MarshalVkWriteDescriptorSetAccelerationStructureKHR();
};

class MarshalVkWriteDescriptorSetAccelerationStructureNV {
public:
    MarshalVkWriteDescriptorSetAccelerationStructureNV() {}
    VkWriteDescriptorSetAccelerationStructureNV s;
    MarshalVkWriteDescriptorSetAccelerationStructureNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkWriteDescriptorSetAccelerationStructureNV* s);
    static void write(KMemory* memory, U32 address, VkWriteDescriptorSetAccelerationStructureNV* s);
    ~MarshalVkWriteDescriptorSetAccelerationStructureNV();
};

class MarshalVkAccelerationStructureMemoryRequirementsInfoNV {
public:
    MarshalVkAccelerationStructureMemoryRequirementsInfoNV() {}
    VkAccelerationStructureMemoryRequirementsInfoNV s;
    MarshalVkAccelerationStructureMemoryRequirementsInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureMemoryRequirementsInfoNV* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureMemoryRequirementsInfoNV* s);
    ~MarshalVkAccelerationStructureMemoryRequirementsInfoNV();
};

class MarshalVkPhysicalDeviceAccelerationStructureFeaturesKHR {
public:
    MarshalVkPhysicalDeviceAccelerationStructureFeaturesKHR() {}
    VkPhysicalDeviceAccelerationStructureFeaturesKHR s;
    MarshalVkPhysicalDeviceAccelerationStructureFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceAccelerationStructureFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceAccelerationStructureFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceAccelerationStructureFeaturesKHR();
};

class MarshalVkPhysicalDeviceRayTracingPipelineFeaturesKHR {
public:
    MarshalVkPhysicalDeviceRayTracingPipelineFeaturesKHR() {}
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR s;
    MarshalVkPhysicalDeviceRayTracingPipelineFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPipelineFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPipelineFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceRayTracingPipelineFeaturesKHR();
};

class MarshalVkPhysicalDeviceRayQueryFeaturesKHR {
public:
    MarshalVkPhysicalDeviceRayQueryFeaturesKHR() {}
    VkPhysicalDeviceRayQueryFeaturesKHR s;
    MarshalVkPhysicalDeviceRayQueryFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceRayQueryFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceRayQueryFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceRayQueryFeaturesKHR();
};

class MarshalVkPhysicalDeviceAccelerationStructurePropertiesKHR {
public:
    MarshalVkPhysicalDeviceAccelerationStructurePropertiesKHR() {}
    VkPhysicalDeviceAccelerationStructurePropertiesKHR s;
    MarshalVkPhysicalDeviceAccelerationStructurePropertiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceAccelerationStructurePropertiesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceAccelerationStructurePropertiesKHR* s);
    ~MarshalVkPhysicalDeviceAccelerationStructurePropertiesKHR();
};

class MarshalVkPhysicalDeviceRayTracingPipelinePropertiesKHR {
public:
    MarshalVkPhysicalDeviceRayTracingPipelinePropertiesKHR() {}
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR s;
    MarshalVkPhysicalDeviceRayTracingPipelinePropertiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPipelinePropertiesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPipelinePropertiesKHR* s);
    ~MarshalVkPhysicalDeviceRayTracingPipelinePropertiesKHR();
};

class MarshalVkPhysicalDeviceRayTracingPropertiesNV {
public:
    MarshalVkPhysicalDeviceRayTracingPropertiesNV() {}
    VkPhysicalDeviceRayTracingPropertiesNV s;
    MarshalVkPhysicalDeviceRayTracingPropertiesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPropertiesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceRayTracingPropertiesNV* s);
    ~MarshalVkPhysicalDeviceRayTracingPropertiesNV();
};

class MarshalVkDrmFormatModifierPropertiesListEXT {
public:
    MarshalVkDrmFormatModifierPropertiesListEXT() {}
    VkDrmFormatModifierPropertiesListEXT s;
    MarshalVkDrmFormatModifierPropertiesListEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDrmFormatModifierPropertiesListEXT* s);
    static void write(KMemory* memory, U32 address, VkDrmFormatModifierPropertiesListEXT* s);
    ~MarshalVkDrmFormatModifierPropertiesListEXT();
};

class MarshalVkPhysicalDeviceImageDrmFormatModifierInfoEXT {
public:
    MarshalVkPhysicalDeviceImageDrmFormatModifierInfoEXT() {}
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT s;
    MarshalVkPhysicalDeviceImageDrmFormatModifierInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceImageDrmFormatModifierInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceImageDrmFormatModifierInfoEXT* s);
    ~MarshalVkPhysicalDeviceImageDrmFormatModifierInfoEXT();
};

class MarshalVkImageDrmFormatModifierListCreateInfoEXT {
public:
    MarshalVkImageDrmFormatModifierListCreateInfoEXT() {}
    VkImageDrmFormatModifierListCreateInfoEXT s;
    MarshalVkImageDrmFormatModifierListCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageDrmFormatModifierListCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkImageDrmFormatModifierListCreateInfoEXT* s);
    ~MarshalVkImageDrmFormatModifierListCreateInfoEXT();
};

class MarshalVkImageDrmFormatModifierExplicitCreateInfoEXT {
public:
    MarshalVkImageDrmFormatModifierExplicitCreateInfoEXT() {}
    VkImageDrmFormatModifierExplicitCreateInfoEXT s;
    MarshalVkImageDrmFormatModifierExplicitCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageDrmFormatModifierExplicitCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkImageDrmFormatModifierExplicitCreateInfoEXT* s);
    ~MarshalVkImageDrmFormatModifierExplicitCreateInfoEXT();
};

class MarshalVkImageDrmFormatModifierPropertiesEXT {
public:
    MarshalVkImageDrmFormatModifierPropertiesEXT() {}
    VkImageDrmFormatModifierPropertiesEXT s;
    MarshalVkImageDrmFormatModifierPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageDrmFormatModifierPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkImageDrmFormatModifierPropertiesEXT* s);
    ~MarshalVkImageDrmFormatModifierPropertiesEXT();
};

class MarshalVkImageStencilUsageCreateInfo {
public:
    MarshalVkImageStencilUsageCreateInfo() {}
    VkImageStencilUsageCreateInfo s;
    MarshalVkImageStencilUsageCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageStencilUsageCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkImageStencilUsageCreateInfo* s);
    ~MarshalVkImageStencilUsageCreateInfo();
};

class MarshalVkDeviceMemoryOverallocationCreateInfoAMD {
public:
    MarshalVkDeviceMemoryOverallocationCreateInfoAMD() {}
    VkDeviceMemoryOverallocationCreateInfoAMD s;
    MarshalVkDeviceMemoryOverallocationCreateInfoAMD(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceMemoryOverallocationCreateInfoAMD* s);
    static void write(KMemory* memory, U32 address, VkDeviceMemoryOverallocationCreateInfoAMD* s);
    ~MarshalVkDeviceMemoryOverallocationCreateInfoAMD();
};

class MarshalVkPhysicalDeviceFragmentDensityMapFeaturesEXT {
public:
    MarshalVkPhysicalDeviceFragmentDensityMapFeaturesEXT() {}
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT s;
    MarshalVkPhysicalDeviceFragmentDensityMapFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMapFeaturesEXT();
};

class MarshalVkPhysicalDeviceFragmentDensityMap2FeaturesEXT {
public:
    MarshalVkPhysicalDeviceFragmentDensityMap2FeaturesEXT() {}
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT s;
    MarshalVkPhysicalDeviceFragmentDensityMap2FeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMap2FeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMap2FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMap2FeaturesEXT();
};

class MarshalVkPhysicalDeviceFragmentDensityMapPropertiesEXT {
public:
    MarshalVkPhysicalDeviceFragmentDensityMapPropertiesEXT() {}
    VkPhysicalDeviceFragmentDensityMapPropertiesEXT s;
    MarshalVkPhysicalDeviceFragmentDensityMapPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMapPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMapPropertiesEXT();
};

class MarshalVkPhysicalDeviceFragmentDensityMap2PropertiesEXT {
public:
    MarshalVkPhysicalDeviceFragmentDensityMap2PropertiesEXT() {}
    VkPhysicalDeviceFragmentDensityMap2PropertiesEXT s;
    MarshalVkPhysicalDeviceFragmentDensityMap2PropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMap2PropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentDensityMap2PropertiesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentDensityMap2PropertiesEXT();
};

class MarshalVkRenderPassFragmentDensityMapCreateInfoEXT {
public:
    MarshalVkRenderPassFragmentDensityMapCreateInfoEXT() {}
    VkRenderPassFragmentDensityMapCreateInfoEXT s;
    MarshalVkRenderPassFragmentDensityMapCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRenderPassFragmentDensityMapCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkRenderPassFragmentDensityMapCreateInfoEXT* s);
    ~MarshalVkRenderPassFragmentDensityMapCreateInfoEXT();
};

class MarshalVkPhysicalDeviceScalarBlockLayoutFeatures {
public:
    MarshalVkPhysicalDeviceScalarBlockLayoutFeatures() {}
    VkPhysicalDeviceScalarBlockLayoutFeatures s;
    MarshalVkPhysicalDeviceScalarBlockLayoutFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceScalarBlockLayoutFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceScalarBlockLayoutFeatures* s);
    ~MarshalVkPhysicalDeviceScalarBlockLayoutFeatures();
};

class MarshalVkSurfaceProtectedCapabilitiesKHR {
public:
    MarshalVkSurfaceProtectedCapabilitiesKHR() {}
    VkSurfaceProtectedCapabilitiesKHR s;
    MarshalVkSurfaceProtectedCapabilitiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSurfaceProtectedCapabilitiesKHR* s);
    static void write(KMemory* memory, U32 address, VkSurfaceProtectedCapabilitiesKHR* s);
    ~MarshalVkSurfaceProtectedCapabilitiesKHR();
};

class MarshalVkPhysicalDeviceUniformBufferStandardLayoutFeatures {
public:
    MarshalVkPhysicalDeviceUniformBufferStandardLayoutFeatures() {}
    VkPhysicalDeviceUniformBufferStandardLayoutFeatures s;
    MarshalVkPhysicalDeviceUniformBufferStandardLayoutFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceUniformBufferStandardLayoutFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceUniformBufferStandardLayoutFeatures* s);
    ~MarshalVkPhysicalDeviceUniformBufferStandardLayoutFeatures();
};

class MarshalVkPhysicalDeviceDepthClipEnableFeaturesEXT {
public:
    MarshalVkPhysicalDeviceDepthClipEnableFeaturesEXT() {}
    VkPhysicalDeviceDepthClipEnableFeaturesEXT s;
    MarshalVkPhysicalDeviceDepthClipEnableFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDepthClipEnableFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDepthClipEnableFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceDepthClipEnableFeaturesEXT();
};

class MarshalVkPipelineRasterizationDepthClipStateCreateInfoEXT {
public:
    MarshalVkPipelineRasterizationDepthClipStateCreateInfoEXT() {}
    VkPipelineRasterizationDepthClipStateCreateInfoEXT s;
    MarshalVkPipelineRasterizationDepthClipStateCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineRasterizationDepthClipStateCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineRasterizationDepthClipStateCreateInfoEXT* s);
    ~MarshalVkPipelineRasterizationDepthClipStateCreateInfoEXT();
};

class MarshalVkPhysicalDeviceMemoryBudgetPropertiesEXT {
public:
    MarshalVkPhysicalDeviceMemoryBudgetPropertiesEXT() {}
    VkPhysicalDeviceMemoryBudgetPropertiesEXT s;
    MarshalVkPhysicalDeviceMemoryBudgetPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMemoryBudgetPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMemoryBudgetPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceMemoryBudgetPropertiesEXT();
};

class MarshalVkPhysicalDeviceMemoryPriorityFeaturesEXT {
public:
    MarshalVkPhysicalDeviceMemoryPriorityFeaturesEXT() {}
    VkPhysicalDeviceMemoryPriorityFeaturesEXT s;
    MarshalVkPhysicalDeviceMemoryPriorityFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMemoryPriorityFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMemoryPriorityFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceMemoryPriorityFeaturesEXT();
};

class MarshalVkMemoryPriorityAllocateInfoEXT {
public:
    MarshalVkMemoryPriorityAllocateInfoEXT() {}
    VkMemoryPriorityAllocateInfoEXT s;
    MarshalVkMemoryPriorityAllocateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryPriorityAllocateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkMemoryPriorityAllocateInfoEXT* s);
    ~MarshalVkMemoryPriorityAllocateInfoEXT();
};

class MarshalVkPhysicalDeviceBufferDeviceAddressFeatures {
public:
    MarshalVkPhysicalDeviceBufferDeviceAddressFeatures() {}
    VkPhysicalDeviceBufferDeviceAddressFeatures s;
    MarshalVkPhysicalDeviceBufferDeviceAddressFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceBufferDeviceAddressFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceBufferDeviceAddressFeatures* s);
    ~MarshalVkPhysicalDeviceBufferDeviceAddressFeatures();
};

class MarshalVkPhysicalDeviceBufferDeviceAddressFeaturesEXT {
public:
    MarshalVkPhysicalDeviceBufferDeviceAddressFeaturesEXT() {}
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT s;
    MarshalVkPhysicalDeviceBufferDeviceAddressFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceBufferDeviceAddressFeaturesEXT();
};

class MarshalVkBufferDeviceAddressInfo {
public:
    MarshalVkBufferDeviceAddressInfo() {}
    VkBufferDeviceAddressInfo s;
    MarshalVkBufferDeviceAddressInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferDeviceAddressInfo* s);
    static void write(KMemory* memory, U32 address, VkBufferDeviceAddressInfo* s);
    ~MarshalVkBufferDeviceAddressInfo();
};

class MarshalVkBufferOpaqueCaptureAddressCreateInfo {
public:
    MarshalVkBufferOpaqueCaptureAddressCreateInfo() {}
    VkBufferOpaqueCaptureAddressCreateInfo s;
    MarshalVkBufferOpaqueCaptureAddressCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferOpaqueCaptureAddressCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkBufferOpaqueCaptureAddressCreateInfo* s);
    ~MarshalVkBufferOpaqueCaptureAddressCreateInfo();
};

class MarshalVkBufferDeviceAddressCreateInfoEXT {
public:
    MarshalVkBufferDeviceAddressCreateInfoEXT() {}
    VkBufferDeviceAddressCreateInfoEXT s;
    MarshalVkBufferDeviceAddressCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferDeviceAddressCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkBufferDeviceAddressCreateInfoEXT* s);
    ~MarshalVkBufferDeviceAddressCreateInfoEXT();
};

class MarshalVkPhysicalDeviceImageViewImageFormatInfoEXT {
public:
    MarshalVkPhysicalDeviceImageViewImageFormatInfoEXT() {}
    VkPhysicalDeviceImageViewImageFormatInfoEXT s;
    MarshalVkPhysicalDeviceImageViewImageFormatInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceImageViewImageFormatInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceImageViewImageFormatInfoEXT* s);
    ~MarshalVkPhysicalDeviceImageViewImageFormatInfoEXT();
};

class MarshalVkFilterCubicImageViewImageFormatPropertiesEXT {
public:
    MarshalVkFilterCubicImageViewImageFormatPropertiesEXT() {}
    VkFilterCubicImageViewImageFormatPropertiesEXT s;
    MarshalVkFilterCubicImageViewImageFormatPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkFilterCubicImageViewImageFormatPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkFilterCubicImageViewImageFormatPropertiesEXT* s);
    ~MarshalVkFilterCubicImageViewImageFormatPropertiesEXT();
};

class MarshalVkPhysicalDeviceImagelessFramebufferFeatures {
public:
    MarshalVkPhysicalDeviceImagelessFramebufferFeatures() {}
    VkPhysicalDeviceImagelessFramebufferFeatures s;
    MarshalVkPhysicalDeviceImagelessFramebufferFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceImagelessFramebufferFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceImagelessFramebufferFeatures* s);
    ~MarshalVkPhysicalDeviceImagelessFramebufferFeatures();
};

class MarshalVkFramebufferAttachmentsCreateInfo {
public:
    MarshalVkFramebufferAttachmentsCreateInfo() {}
    VkFramebufferAttachmentsCreateInfo s;
    MarshalVkFramebufferAttachmentsCreateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkFramebufferAttachmentsCreateInfo* s);
    static void write(KMemory* memory, U32 address, VkFramebufferAttachmentsCreateInfo* s);
    ~MarshalVkFramebufferAttachmentsCreateInfo();
};

class MarshalVkRenderPassAttachmentBeginInfo {
public:
    MarshalVkRenderPassAttachmentBeginInfo() {}
    VkRenderPassAttachmentBeginInfo s;
    MarshalVkRenderPassAttachmentBeginInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRenderPassAttachmentBeginInfo* s);
    static void write(KMemory* memory, U32 address, VkRenderPassAttachmentBeginInfo* s);
    ~MarshalVkRenderPassAttachmentBeginInfo();
};

class MarshalVkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT {
public:
    MarshalVkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT() {}
    VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT s;
    MarshalVkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT();
};

class MarshalVkPhysicalDeviceCooperativeMatrixFeaturesNV {
public:
    MarshalVkPhysicalDeviceCooperativeMatrixFeaturesNV() {}
    VkPhysicalDeviceCooperativeMatrixFeaturesNV s;
    MarshalVkPhysicalDeviceCooperativeMatrixFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixFeaturesNV* s);
    ~MarshalVkPhysicalDeviceCooperativeMatrixFeaturesNV();
};

class MarshalVkPhysicalDeviceCooperativeMatrixPropertiesNV {
public:
    MarshalVkPhysicalDeviceCooperativeMatrixPropertiesNV() {}
    VkPhysicalDeviceCooperativeMatrixPropertiesNV s;
    MarshalVkPhysicalDeviceCooperativeMatrixPropertiesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixPropertiesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceCooperativeMatrixPropertiesNV* s);
    ~MarshalVkPhysicalDeviceCooperativeMatrixPropertiesNV();
};

class MarshalVkCooperativeMatrixPropertiesNV {
public:
    MarshalVkCooperativeMatrixPropertiesNV() {}
    VkCooperativeMatrixPropertiesNV s;
    MarshalVkCooperativeMatrixPropertiesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCooperativeMatrixPropertiesNV* s);
    static void write(KMemory* memory, U32 address, VkCooperativeMatrixPropertiesNV* s);
    ~MarshalVkCooperativeMatrixPropertiesNV();
};

class MarshalVkPhysicalDeviceYcbcrImageArraysFeaturesEXT {
public:
    MarshalVkPhysicalDeviceYcbcrImageArraysFeaturesEXT() {}
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT s;
    MarshalVkPhysicalDeviceYcbcrImageArraysFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceYcbcrImageArraysFeaturesEXT();
};

class MarshalVkImageViewHandleInfoNVX {
public:
    MarshalVkImageViewHandleInfoNVX() {}
    VkImageViewHandleInfoNVX s;
    MarshalVkImageViewHandleInfoNVX(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageViewHandleInfoNVX* s);
    static void write(KMemory* memory, U32 address, VkImageViewHandleInfoNVX* s);
    ~MarshalVkImageViewHandleInfoNVX();
};

class MarshalVkImageViewAddressPropertiesNVX {
public:
    MarshalVkImageViewAddressPropertiesNVX() {}
    VkImageViewAddressPropertiesNVX s;
    MarshalVkImageViewAddressPropertiesNVX(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageViewAddressPropertiesNVX* s);
    static void write(KMemory* memory, U32 address, VkImageViewAddressPropertiesNVX* s);
    ~MarshalVkImageViewAddressPropertiesNVX();
};

class MarshalVkPipelineCreationFeedbackEXT {
public:
    MarshalVkPipelineCreationFeedbackEXT() {}
    VkPipelineCreationFeedbackEXT s;
    MarshalVkPipelineCreationFeedbackEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineCreationFeedbackEXT* s);
};

class MarshalVkPipelineCreationFeedbackCreateInfoEXT {
public:
    MarshalVkPipelineCreationFeedbackCreateInfoEXT() {}
    VkPipelineCreationFeedbackCreateInfoEXT s;
    MarshalVkPipelineCreationFeedbackCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineCreationFeedbackCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineCreationFeedbackCreateInfoEXT* s);
    ~MarshalVkPipelineCreationFeedbackCreateInfoEXT();
};

class MarshalVkPhysicalDevicePerformanceQueryFeaturesKHR {
public:
    MarshalVkPhysicalDevicePerformanceQueryFeaturesKHR() {}
    VkPhysicalDevicePerformanceQueryFeaturesKHR s;
    MarshalVkPhysicalDevicePerformanceQueryFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevicePerformanceQueryFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevicePerformanceQueryFeaturesKHR* s);
    ~MarshalVkPhysicalDevicePerformanceQueryFeaturesKHR();
};

class MarshalVkPhysicalDevicePerformanceQueryPropertiesKHR {
public:
    MarshalVkPhysicalDevicePerformanceQueryPropertiesKHR() {}
    VkPhysicalDevicePerformanceQueryPropertiesKHR s;
    MarshalVkPhysicalDevicePerformanceQueryPropertiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevicePerformanceQueryPropertiesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevicePerformanceQueryPropertiesKHR* s);
    ~MarshalVkPhysicalDevicePerformanceQueryPropertiesKHR();
};

class MarshalVkPerformanceCounterKHR {
public:
    MarshalVkPerformanceCounterKHR() {}
    VkPerformanceCounterKHR s;
    MarshalVkPerformanceCounterKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPerformanceCounterKHR* s);
    static void write(KMemory* memory, U32 address, VkPerformanceCounterKHR* s);
    ~MarshalVkPerformanceCounterKHR();
};

class MarshalVkPerformanceCounterDescriptionKHR {
public:
    MarshalVkPerformanceCounterDescriptionKHR() {}
    VkPerformanceCounterDescriptionKHR s;
    MarshalVkPerformanceCounterDescriptionKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPerformanceCounterDescriptionKHR* s);
    static void write(KMemory* memory, U32 address, VkPerformanceCounterDescriptionKHR* s);
    ~MarshalVkPerformanceCounterDescriptionKHR();
};

class MarshalVkQueryPoolPerformanceCreateInfoKHR {
public:
    MarshalVkQueryPoolPerformanceCreateInfoKHR() {}
    VkQueryPoolPerformanceCreateInfoKHR s;
    MarshalVkQueryPoolPerformanceCreateInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkQueryPoolPerformanceCreateInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkQueryPoolPerformanceCreateInfoKHR* s);
    ~MarshalVkQueryPoolPerformanceCreateInfoKHR();
};

class MarshalVkAcquireProfilingLockInfoKHR {
public:
    MarshalVkAcquireProfilingLockInfoKHR() {}
    VkAcquireProfilingLockInfoKHR s;
    MarshalVkAcquireProfilingLockInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAcquireProfilingLockInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkAcquireProfilingLockInfoKHR* s);
    ~MarshalVkAcquireProfilingLockInfoKHR();
};

class MarshalVkPerformanceQuerySubmitInfoKHR {
public:
    MarshalVkPerformanceQuerySubmitInfoKHR() {}
    VkPerformanceQuerySubmitInfoKHR s;
    MarshalVkPerformanceQuerySubmitInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPerformanceQuerySubmitInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkPerformanceQuerySubmitInfoKHR* s);
    ~MarshalVkPerformanceQuerySubmitInfoKHR();
};

class MarshalVkHeadlessSurfaceCreateInfoEXT {
public:
    MarshalVkHeadlessSurfaceCreateInfoEXT() {}
    VkHeadlessSurfaceCreateInfoEXT s;
    MarshalVkHeadlessSurfaceCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkHeadlessSurfaceCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkHeadlessSurfaceCreateInfoEXT* s);
    ~MarshalVkHeadlessSurfaceCreateInfoEXT();
};

class MarshalVkPhysicalDeviceCoverageReductionModeFeaturesNV {
public:
    MarshalVkPhysicalDeviceCoverageReductionModeFeaturesNV() {}
    VkPhysicalDeviceCoverageReductionModeFeaturesNV s;
    MarshalVkPhysicalDeviceCoverageReductionModeFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceCoverageReductionModeFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceCoverageReductionModeFeaturesNV* s);
    ~MarshalVkPhysicalDeviceCoverageReductionModeFeaturesNV();
};

class MarshalVkPipelineCoverageReductionStateCreateInfoNV {
public:
    MarshalVkPipelineCoverageReductionStateCreateInfoNV() {}
    VkPipelineCoverageReductionStateCreateInfoNV s;
    MarshalVkPipelineCoverageReductionStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineCoverageReductionStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineCoverageReductionStateCreateInfoNV* s);
    ~MarshalVkPipelineCoverageReductionStateCreateInfoNV();
};

class MarshalVkFramebufferMixedSamplesCombinationNV {
public:
    MarshalVkFramebufferMixedSamplesCombinationNV() {}
    VkFramebufferMixedSamplesCombinationNV s;
    MarshalVkFramebufferMixedSamplesCombinationNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkFramebufferMixedSamplesCombinationNV* s);
    static void write(KMemory* memory, U32 address, VkFramebufferMixedSamplesCombinationNV* s);
    ~MarshalVkFramebufferMixedSamplesCombinationNV();
};

class MarshalVkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL {
public:
    MarshalVkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL() {}
    VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL s;
    MarshalVkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* s);
    ~MarshalVkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL();
};

class MarshalVkQueryPoolPerformanceQueryCreateInfoINTEL {
public:
    MarshalVkQueryPoolPerformanceQueryCreateInfoINTEL() {}
    VkQueryPoolPerformanceQueryCreateInfoINTEL s;
    MarshalVkQueryPoolPerformanceQueryCreateInfoINTEL(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkQueryPoolPerformanceQueryCreateInfoINTEL* s);
    static void write(KMemory* memory, U32 address, VkQueryPoolPerformanceQueryCreateInfoINTEL* s);
    ~MarshalVkQueryPoolPerformanceQueryCreateInfoINTEL();
};

class MarshalVkPerformanceMarkerInfoINTEL {
public:
    MarshalVkPerformanceMarkerInfoINTEL() {}
    VkPerformanceMarkerInfoINTEL s;
    MarshalVkPerformanceMarkerInfoINTEL(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPerformanceMarkerInfoINTEL* s);
    static void write(KMemory* memory, U32 address, VkPerformanceMarkerInfoINTEL* s);
    ~MarshalVkPerformanceMarkerInfoINTEL();
};

class MarshalVkPerformanceStreamMarkerInfoINTEL {
public:
    MarshalVkPerformanceStreamMarkerInfoINTEL() {}
    VkPerformanceStreamMarkerInfoINTEL s;
    MarshalVkPerformanceStreamMarkerInfoINTEL(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPerformanceStreamMarkerInfoINTEL* s);
    static void write(KMemory* memory, U32 address, VkPerformanceStreamMarkerInfoINTEL* s);
    ~MarshalVkPerformanceStreamMarkerInfoINTEL();
};

class MarshalVkPerformanceOverrideInfoINTEL {
public:
    MarshalVkPerformanceOverrideInfoINTEL() {}
    VkPerformanceOverrideInfoINTEL s;
    MarshalVkPerformanceOverrideInfoINTEL(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPerformanceOverrideInfoINTEL* s);
    static void write(KMemory* memory, U32 address, VkPerformanceOverrideInfoINTEL* s);
    ~MarshalVkPerformanceOverrideInfoINTEL();
};

class MarshalVkPerformanceConfigurationAcquireInfoINTEL {
public:
    MarshalVkPerformanceConfigurationAcquireInfoINTEL() {}
    VkPerformanceConfigurationAcquireInfoINTEL s;
    MarshalVkPerformanceConfigurationAcquireInfoINTEL(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPerformanceConfigurationAcquireInfoINTEL* s);
    static void write(KMemory* memory, U32 address, VkPerformanceConfigurationAcquireInfoINTEL* s);
    ~MarshalVkPerformanceConfigurationAcquireInfoINTEL();
};

class MarshalVkPhysicalDeviceShaderClockFeaturesKHR {
public:
    MarshalVkPhysicalDeviceShaderClockFeaturesKHR() {}
    VkPhysicalDeviceShaderClockFeaturesKHR s;
    MarshalVkPhysicalDeviceShaderClockFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderClockFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderClockFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceShaderClockFeaturesKHR();
};

class MarshalVkPhysicalDeviceIndexTypeUint8FeaturesEXT {
public:
    MarshalVkPhysicalDeviceIndexTypeUint8FeaturesEXT() {}
    VkPhysicalDeviceIndexTypeUint8FeaturesEXT s;
    MarshalVkPhysicalDeviceIndexTypeUint8FeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceIndexTypeUint8FeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceIndexTypeUint8FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceIndexTypeUint8FeaturesEXT();
};

class MarshalVkPhysicalDeviceShaderSMBuiltinsPropertiesNV {
public:
    MarshalVkPhysicalDeviceShaderSMBuiltinsPropertiesNV() {}
    VkPhysicalDeviceShaderSMBuiltinsPropertiesNV s;
    MarshalVkPhysicalDeviceShaderSMBuiltinsPropertiesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* s);
    ~MarshalVkPhysicalDeviceShaderSMBuiltinsPropertiesNV();
};

class MarshalVkPhysicalDeviceShaderSMBuiltinsFeaturesNV {
public:
    MarshalVkPhysicalDeviceShaderSMBuiltinsFeaturesNV() {}
    VkPhysicalDeviceShaderSMBuiltinsFeaturesNV s;
    MarshalVkPhysicalDeviceShaderSMBuiltinsFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* s);
    ~MarshalVkPhysicalDeviceShaderSMBuiltinsFeaturesNV();
};

class MarshalVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT {
public:
    MarshalVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT() {}
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT s;
    MarshalVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceFragmentShaderInterlockFeaturesEXT();
};

class MarshalVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures {
public:
    MarshalVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures() {}
    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures s;
    MarshalVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures* s);
    ~MarshalVkPhysicalDeviceSeparateDepthStencilLayoutsFeatures();
};

class MarshalVkAttachmentReferenceStencilLayout {
public:
    MarshalVkAttachmentReferenceStencilLayout() {}
    VkAttachmentReferenceStencilLayout s;
    MarshalVkAttachmentReferenceStencilLayout(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAttachmentReferenceStencilLayout* s);
    static void write(KMemory* memory, U32 address, VkAttachmentReferenceStencilLayout* s);
    ~MarshalVkAttachmentReferenceStencilLayout();
};

class MarshalVkAttachmentDescriptionStencilLayout {
public:
    MarshalVkAttachmentDescriptionStencilLayout() {}
    VkAttachmentDescriptionStencilLayout s;
    MarshalVkAttachmentDescriptionStencilLayout(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAttachmentDescriptionStencilLayout* s);
    static void write(KMemory* memory, U32 address, VkAttachmentDescriptionStencilLayout* s);
    ~MarshalVkAttachmentDescriptionStencilLayout();
};

class MarshalVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR {
public:
    MarshalVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR() {}
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR s;
    MarshalVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* s);
    ~MarshalVkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR();
};

class MarshalVkPipelineInfoKHR {
public:
    MarshalVkPipelineInfoKHR() {}
    VkPipelineInfoKHR s;
    MarshalVkPipelineInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkPipelineInfoKHR* s);
    ~MarshalVkPipelineInfoKHR();
};

class MarshalVkPipelineExecutablePropertiesKHR {
public:
    MarshalVkPipelineExecutablePropertiesKHR() {}
    VkPipelineExecutablePropertiesKHR s;
    MarshalVkPipelineExecutablePropertiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineExecutablePropertiesKHR* s);
    static void write(KMemory* memory, U32 address, VkPipelineExecutablePropertiesKHR* s);
    ~MarshalVkPipelineExecutablePropertiesKHR();
};

class MarshalVkPipelineExecutableInfoKHR {
public:
    MarshalVkPipelineExecutableInfoKHR() {}
    VkPipelineExecutableInfoKHR s;
    MarshalVkPipelineExecutableInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineExecutableInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkPipelineExecutableInfoKHR* s);
    ~MarshalVkPipelineExecutableInfoKHR();
};

class MarshalVkPipelineExecutableStatisticKHR {
public:
    MarshalVkPipelineExecutableStatisticKHR() {}
    VkPipelineExecutableStatisticKHR s;
    MarshalVkPipelineExecutableStatisticKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineExecutableStatisticKHR* s);
    static void write(KMemory* memory, U32 address, VkPipelineExecutableStatisticKHR* s);
    ~MarshalVkPipelineExecutableStatisticKHR();
};

class MarshalVkPipelineExecutableInternalRepresentationKHR {
public:
    MarshalVkPipelineExecutableInternalRepresentationKHR() {}
    VkPipelineExecutableInternalRepresentationKHR s;
    MarshalVkPipelineExecutableInternalRepresentationKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineExecutableInternalRepresentationKHR* s);
    static void write(KMemory* memory, U32 address, VkPipelineExecutableInternalRepresentationKHR* s);
    ~MarshalVkPipelineExecutableInternalRepresentationKHR();
};

class MarshalVkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT() {}
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT s;
    MarshalVkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT();
};

class MarshalVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT {
public:
    MarshalVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT() {}
    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT s;
    MarshalVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceTexelBufferAlignmentFeaturesEXT();
};

class MarshalVkPhysicalDeviceTexelBufferAlignmentPropertiesEXT {
public:
    MarshalVkPhysicalDeviceTexelBufferAlignmentPropertiesEXT() {}
    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT s;
    MarshalVkPhysicalDeviceTexelBufferAlignmentPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceTexelBufferAlignmentPropertiesEXT();
};

class MarshalVkPhysicalDeviceSubgroupSizeControlFeaturesEXT {
public:
    MarshalVkPhysicalDeviceSubgroupSizeControlFeaturesEXT() {}
    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT s;
    MarshalVkPhysicalDeviceSubgroupSizeControlFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSubgroupSizeControlFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSubgroupSizeControlFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceSubgroupSizeControlFeaturesEXT();
};

class MarshalVkPhysicalDeviceSubgroupSizeControlPropertiesEXT {
public:
    MarshalVkPhysicalDeviceSubgroupSizeControlPropertiesEXT() {}
    VkPhysicalDeviceSubgroupSizeControlPropertiesEXT s;
    MarshalVkPhysicalDeviceSubgroupSizeControlPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSubgroupSizeControlPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSubgroupSizeControlPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceSubgroupSizeControlPropertiesEXT();
};

class MarshalVkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT {
public:
    MarshalVkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT() {}
    VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT s;
    MarshalVkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT* s);
    ~MarshalVkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT();
};

class MarshalVkSubpassShadingPipelineCreateInfoHUAWEI {
public:
    MarshalVkSubpassShadingPipelineCreateInfoHUAWEI() {}
    VkSubpassShadingPipelineCreateInfoHUAWEI s;
    MarshalVkSubpassShadingPipelineCreateInfoHUAWEI(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubpassShadingPipelineCreateInfoHUAWEI* s);
    static void write(KMemory* memory, U32 address, VkSubpassShadingPipelineCreateInfoHUAWEI* s);
    ~MarshalVkSubpassShadingPipelineCreateInfoHUAWEI();
};

class MarshalVkPhysicalDeviceSubpassShadingPropertiesHUAWEI {
public:
    MarshalVkPhysicalDeviceSubpassShadingPropertiesHUAWEI() {}
    VkPhysicalDeviceSubpassShadingPropertiesHUAWEI s;
    MarshalVkPhysicalDeviceSubpassShadingPropertiesHUAWEI(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSubpassShadingPropertiesHUAWEI* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSubpassShadingPropertiesHUAWEI* s);
    ~MarshalVkPhysicalDeviceSubpassShadingPropertiesHUAWEI();
};

class MarshalVkMemoryOpaqueCaptureAddressAllocateInfo {
public:
    MarshalVkMemoryOpaqueCaptureAddressAllocateInfo() {}
    VkMemoryOpaqueCaptureAddressAllocateInfo s;
    MarshalVkMemoryOpaqueCaptureAddressAllocateInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryOpaqueCaptureAddressAllocateInfo* s);
    static void write(KMemory* memory, U32 address, VkMemoryOpaqueCaptureAddressAllocateInfo* s);
    ~MarshalVkMemoryOpaqueCaptureAddressAllocateInfo();
};

class MarshalVkDeviceMemoryOpaqueCaptureAddressInfo {
public:
    MarshalVkDeviceMemoryOpaqueCaptureAddressInfo() {}
    VkDeviceMemoryOpaqueCaptureAddressInfo s;
    MarshalVkDeviceMemoryOpaqueCaptureAddressInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceMemoryOpaqueCaptureAddressInfo* s);
    static void write(KMemory* memory, U32 address, VkDeviceMemoryOpaqueCaptureAddressInfo* s);
    ~MarshalVkDeviceMemoryOpaqueCaptureAddressInfo();
};

class MarshalVkPhysicalDeviceLineRasterizationFeaturesEXT {
public:
    MarshalVkPhysicalDeviceLineRasterizationFeaturesEXT() {}
    VkPhysicalDeviceLineRasterizationFeaturesEXT s;
    MarshalVkPhysicalDeviceLineRasterizationFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceLineRasterizationFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceLineRasterizationFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceLineRasterizationFeaturesEXT();
};

class MarshalVkPhysicalDeviceLineRasterizationPropertiesEXT {
public:
    MarshalVkPhysicalDeviceLineRasterizationPropertiesEXT() {}
    VkPhysicalDeviceLineRasterizationPropertiesEXT s;
    MarshalVkPhysicalDeviceLineRasterizationPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceLineRasterizationPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceLineRasterizationPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceLineRasterizationPropertiesEXT();
};

class MarshalVkPipelineRasterizationLineStateCreateInfoEXT {
public:
    MarshalVkPipelineRasterizationLineStateCreateInfoEXT() {}
    VkPipelineRasterizationLineStateCreateInfoEXT s;
    MarshalVkPipelineRasterizationLineStateCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineRasterizationLineStateCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineRasterizationLineStateCreateInfoEXT* s);
    ~MarshalVkPipelineRasterizationLineStateCreateInfoEXT();
};

class MarshalVkPhysicalDevicePipelineCreationCacheControlFeaturesEXT {
public:
    MarshalVkPhysicalDevicePipelineCreationCacheControlFeaturesEXT() {}
    VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT s;
    MarshalVkPhysicalDevicePipelineCreationCacheControlFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT* s);
    ~MarshalVkPhysicalDevicePipelineCreationCacheControlFeaturesEXT();
};

class MarshalVkPhysicalDeviceVulkan11Features {
public:
    MarshalVkPhysicalDeviceVulkan11Features() {}
    VkPhysicalDeviceVulkan11Features s;
    MarshalVkPhysicalDeviceVulkan11Features(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceVulkan11Features* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceVulkan11Features* s);
    ~MarshalVkPhysicalDeviceVulkan11Features();
};

class MarshalVkPhysicalDeviceVulkan11Properties {
public:
    MarshalVkPhysicalDeviceVulkan11Properties() {}
    VkPhysicalDeviceVulkan11Properties s;
    MarshalVkPhysicalDeviceVulkan11Properties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceVulkan11Properties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceVulkan11Properties* s);
    ~MarshalVkPhysicalDeviceVulkan11Properties();
};

class MarshalVkPhysicalDeviceVulkan12Features {
public:
    MarshalVkPhysicalDeviceVulkan12Features() {}
    VkPhysicalDeviceVulkan12Features s;
    MarshalVkPhysicalDeviceVulkan12Features(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceVulkan12Features* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceVulkan12Features* s);
    ~MarshalVkPhysicalDeviceVulkan12Features();
};

class MarshalVkPhysicalDeviceVulkan12Properties {
public:
    MarshalVkPhysicalDeviceVulkan12Properties() {}
    VkPhysicalDeviceVulkan12Properties s;
    MarshalVkPhysicalDeviceVulkan12Properties(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceVulkan12Properties* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceVulkan12Properties* s);
    ~MarshalVkPhysicalDeviceVulkan12Properties();
};

class MarshalVkPipelineCompilerControlCreateInfoAMD {
public:
    MarshalVkPipelineCompilerControlCreateInfoAMD() {}
    VkPipelineCompilerControlCreateInfoAMD s;
    MarshalVkPipelineCompilerControlCreateInfoAMD(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineCompilerControlCreateInfoAMD* s);
    static void write(KMemory* memory, U32 address, VkPipelineCompilerControlCreateInfoAMD* s);
    ~MarshalVkPipelineCompilerControlCreateInfoAMD();
};

class MarshalVkPhysicalDeviceCoherentMemoryFeaturesAMD {
public:
    MarshalVkPhysicalDeviceCoherentMemoryFeaturesAMD() {}
    VkPhysicalDeviceCoherentMemoryFeaturesAMD s;
    MarshalVkPhysicalDeviceCoherentMemoryFeaturesAMD(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceCoherentMemoryFeaturesAMD* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceCoherentMemoryFeaturesAMD* s);
    ~MarshalVkPhysicalDeviceCoherentMemoryFeaturesAMD();
};

class MarshalVkPhysicalDeviceToolPropertiesEXT {
public:
    MarshalVkPhysicalDeviceToolPropertiesEXT() {}
    VkPhysicalDeviceToolPropertiesEXT s;
    MarshalVkPhysicalDeviceToolPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceToolPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceToolPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceToolPropertiesEXT();
};

class MarshalVkSamplerCustomBorderColorCreateInfoEXT {
public:
    MarshalVkSamplerCustomBorderColorCreateInfoEXT() {}
    VkSamplerCustomBorderColorCreateInfoEXT s;
    MarshalVkSamplerCustomBorderColorCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSamplerCustomBorderColorCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkSamplerCustomBorderColorCreateInfoEXT* s);
    ~MarshalVkSamplerCustomBorderColorCreateInfoEXT();
};

class MarshalVkPhysicalDeviceCustomBorderColorPropertiesEXT {
public:
    MarshalVkPhysicalDeviceCustomBorderColorPropertiesEXT() {}
    VkPhysicalDeviceCustomBorderColorPropertiesEXT s;
    MarshalVkPhysicalDeviceCustomBorderColorPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceCustomBorderColorPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceCustomBorderColorPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceCustomBorderColorPropertiesEXT();
};

class MarshalVkPhysicalDeviceCustomBorderColorFeaturesEXT {
public:
    MarshalVkPhysicalDeviceCustomBorderColorFeaturesEXT() {}
    VkPhysicalDeviceCustomBorderColorFeaturesEXT s;
    MarshalVkPhysicalDeviceCustomBorderColorFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceCustomBorderColorFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceCustomBorderColorFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceCustomBorderColorFeaturesEXT();
};

class MarshalVkAccelerationStructureGeometryTrianglesDataKHR {
public:
    MarshalVkAccelerationStructureGeometryTrianglesDataKHR() {}
    VkAccelerationStructureGeometryTrianglesDataKHR s;
    MarshalVkAccelerationStructureGeometryTrianglesDataKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureGeometryTrianglesDataKHR* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureGeometryTrianglesDataKHR* s);
    ~MarshalVkAccelerationStructureGeometryTrianglesDataKHR();
};

class MarshalVkAccelerationStructureGeometryAabbsDataKHR {
public:
    MarshalVkAccelerationStructureGeometryAabbsDataKHR() {}
    VkAccelerationStructureGeometryAabbsDataKHR s;
    MarshalVkAccelerationStructureGeometryAabbsDataKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureGeometryAabbsDataKHR* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureGeometryAabbsDataKHR* s);
    ~MarshalVkAccelerationStructureGeometryAabbsDataKHR();
};

class MarshalVkAccelerationStructureGeometryInstancesDataKHR {
public:
    MarshalVkAccelerationStructureGeometryInstancesDataKHR() {}
    VkAccelerationStructureGeometryInstancesDataKHR s;
    MarshalVkAccelerationStructureGeometryInstancesDataKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureGeometryInstancesDataKHR* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureGeometryInstancesDataKHR* s);
    ~MarshalVkAccelerationStructureGeometryInstancesDataKHR();
};

class MarshalVkAccelerationStructureGeometryKHR {
public:
    MarshalVkAccelerationStructureGeometryKHR() {}
    VkAccelerationStructureGeometryKHR s;
    MarshalVkAccelerationStructureGeometryKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureGeometryKHR* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureGeometryKHR* s);
    ~MarshalVkAccelerationStructureGeometryKHR();
};

class MarshalVkAccelerationStructureCreateInfoKHR {
public:
    MarshalVkAccelerationStructureCreateInfoKHR() {}
    VkAccelerationStructureCreateInfoKHR s;
    MarshalVkAccelerationStructureCreateInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureCreateInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureCreateInfoKHR* s);
    ~MarshalVkAccelerationStructureCreateInfoKHR();
};

class MarshalVkAccelerationStructureDeviceAddressInfoKHR {
public:
    MarshalVkAccelerationStructureDeviceAddressInfoKHR() {}
    VkAccelerationStructureDeviceAddressInfoKHR s;
    MarshalVkAccelerationStructureDeviceAddressInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureDeviceAddressInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureDeviceAddressInfoKHR* s);
    ~MarshalVkAccelerationStructureDeviceAddressInfoKHR();
};

class MarshalVkAccelerationStructureVersionInfoKHR {
public:
    MarshalVkAccelerationStructureVersionInfoKHR() {}
    VkAccelerationStructureVersionInfoKHR s;
    MarshalVkAccelerationStructureVersionInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureVersionInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureVersionInfoKHR* s);
    ~MarshalVkAccelerationStructureVersionInfoKHR();
};

class MarshalVkCopyAccelerationStructureInfoKHR {
public:
    MarshalVkCopyAccelerationStructureInfoKHR() {}
    VkCopyAccelerationStructureInfoKHR s;
    MarshalVkCopyAccelerationStructureInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCopyAccelerationStructureInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkCopyAccelerationStructureInfoKHR* s);
    ~MarshalVkCopyAccelerationStructureInfoKHR();
};

class MarshalVkCopyAccelerationStructureToMemoryInfoKHR {
public:
    MarshalVkCopyAccelerationStructureToMemoryInfoKHR() {}
    VkCopyAccelerationStructureToMemoryInfoKHR s;
    MarshalVkCopyAccelerationStructureToMemoryInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCopyAccelerationStructureToMemoryInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkCopyAccelerationStructureToMemoryInfoKHR* s);
    ~MarshalVkCopyAccelerationStructureToMemoryInfoKHR();
};

class MarshalVkCopyMemoryToAccelerationStructureInfoKHR {
public:
    MarshalVkCopyMemoryToAccelerationStructureInfoKHR() {}
    VkCopyMemoryToAccelerationStructureInfoKHR s;
    MarshalVkCopyMemoryToAccelerationStructureInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCopyMemoryToAccelerationStructureInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkCopyMemoryToAccelerationStructureInfoKHR* s);
    ~MarshalVkCopyMemoryToAccelerationStructureInfoKHR();
};

class MarshalVkPhysicalDeviceExtendedDynamicStateFeaturesEXT {
public:
    MarshalVkPhysicalDeviceExtendedDynamicStateFeaturesEXT() {}
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT s;
    MarshalVkPhysicalDeviceExtendedDynamicStateFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceExtendedDynamicStateFeaturesEXT();
};

class MarshalVkPhysicalDeviceExtendedDynamicState2FeaturesEXT {
public:
    MarshalVkPhysicalDeviceExtendedDynamicState2FeaturesEXT() {}
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT s;
    MarshalVkPhysicalDeviceExtendedDynamicState2FeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceExtendedDynamicState2FeaturesEXT();
};

class MarshalVkRenderPassTransformBeginInfoQCOM {
public:
    MarshalVkRenderPassTransformBeginInfoQCOM() {}
    VkRenderPassTransformBeginInfoQCOM s;
    MarshalVkRenderPassTransformBeginInfoQCOM(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkRenderPassTransformBeginInfoQCOM* s);
    static void write(KMemory* memory, U32 address, VkRenderPassTransformBeginInfoQCOM* s);
    ~MarshalVkRenderPassTransformBeginInfoQCOM();
};

class MarshalVkCopyCommandTransformInfoQCOM {
public:
    MarshalVkCopyCommandTransformInfoQCOM() {}
    VkCopyCommandTransformInfoQCOM s;
    MarshalVkCopyCommandTransformInfoQCOM(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCopyCommandTransformInfoQCOM* s);
    static void write(KMemory* memory, U32 address, VkCopyCommandTransformInfoQCOM* s);
    ~MarshalVkCopyCommandTransformInfoQCOM();
};

class MarshalVkCommandBufferInheritanceRenderPassTransformInfoQCOM {
public:
    MarshalVkCommandBufferInheritanceRenderPassTransformInfoQCOM() {}
    VkCommandBufferInheritanceRenderPassTransformInfoQCOM s;
    MarshalVkCommandBufferInheritanceRenderPassTransformInfoQCOM(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCommandBufferInheritanceRenderPassTransformInfoQCOM* s);
    static void write(KMemory* memory, U32 address, VkCommandBufferInheritanceRenderPassTransformInfoQCOM* s);
    ~MarshalVkCommandBufferInheritanceRenderPassTransformInfoQCOM();
};

class MarshalVkPhysicalDeviceDiagnosticsConfigFeaturesNV {
public:
    MarshalVkPhysicalDeviceDiagnosticsConfigFeaturesNV() {}
    VkPhysicalDeviceDiagnosticsConfigFeaturesNV s;
    MarshalVkPhysicalDeviceDiagnosticsConfigFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDiagnosticsConfigFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDiagnosticsConfigFeaturesNV* s);
    ~MarshalVkPhysicalDeviceDiagnosticsConfigFeaturesNV();
};

class MarshalVkDeviceDiagnosticsConfigCreateInfoNV {
public:
    MarshalVkDeviceDiagnosticsConfigCreateInfoNV() {}
    VkDeviceDiagnosticsConfigCreateInfoNV s;
    MarshalVkDeviceDiagnosticsConfigCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDeviceDiagnosticsConfigCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkDeviceDiagnosticsConfigCreateInfoNV* s);
    ~MarshalVkDeviceDiagnosticsConfigCreateInfoNV();
};

class MarshalVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR {
public:
    MarshalVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR() {}
    VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR s;
    MarshalVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR();
};

class MarshalVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR {
public:
    MarshalVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR() {}
    VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR s;
    MarshalVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR();
};

class MarshalVkPhysicalDeviceRobustness2FeaturesEXT {
public:
    MarshalVkPhysicalDeviceRobustness2FeaturesEXT() {}
    VkPhysicalDeviceRobustness2FeaturesEXT s;
    MarshalVkPhysicalDeviceRobustness2FeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceRobustness2FeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceRobustness2FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceRobustness2FeaturesEXT();
};

class MarshalVkPhysicalDeviceRobustness2PropertiesEXT {
public:
    MarshalVkPhysicalDeviceRobustness2PropertiesEXT() {}
    VkPhysicalDeviceRobustness2PropertiesEXT s;
    MarshalVkPhysicalDeviceRobustness2PropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceRobustness2PropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceRobustness2PropertiesEXT* s);
    ~MarshalVkPhysicalDeviceRobustness2PropertiesEXT();
};

class MarshalVkPhysicalDeviceImageRobustnessFeaturesEXT {
public:
    MarshalVkPhysicalDeviceImageRobustnessFeaturesEXT() {}
    VkPhysicalDeviceImageRobustnessFeaturesEXT s;
    MarshalVkPhysicalDeviceImageRobustnessFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceImageRobustnessFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceImageRobustnessFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceImageRobustnessFeaturesEXT();
};

class MarshalVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR {
public:
    MarshalVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR() {}
    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR s;
    MarshalVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR();
};

class MarshalVkPhysicalDevice4444FormatsFeaturesEXT {
public:
    MarshalVkPhysicalDevice4444FormatsFeaturesEXT() {}
    VkPhysicalDevice4444FormatsFeaturesEXT s;
    MarshalVkPhysicalDevice4444FormatsFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDevice4444FormatsFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDevice4444FormatsFeaturesEXT* s);
    ~MarshalVkPhysicalDevice4444FormatsFeaturesEXT();
};

class MarshalVkPhysicalDeviceSubpassShadingFeaturesHUAWEI {
public:
    MarshalVkPhysicalDeviceSubpassShadingFeaturesHUAWEI() {}
    VkPhysicalDeviceSubpassShadingFeaturesHUAWEI s;
    MarshalVkPhysicalDeviceSubpassShadingFeaturesHUAWEI(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSubpassShadingFeaturesHUAWEI* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSubpassShadingFeaturesHUAWEI* s);
    ~MarshalVkPhysicalDeviceSubpassShadingFeaturesHUAWEI();
};

class MarshalVkBufferCopy2KHR {
public:
    MarshalVkBufferCopy2KHR() {}
    VkBufferCopy2KHR s;
    MarshalVkBufferCopy2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferCopy2KHR* s);
    static void write(KMemory* memory, U32 address, VkBufferCopy2KHR* s);
    ~MarshalVkBufferCopy2KHR();
};

class MarshalVkImageCopy2KHR {
public:
    MarshalVkImageCopy2KHR() {}
    VkImageCopy2KHR s;
    MarshalVkImageCopy2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageCopy2KHR* s);
    static void write(KMemory* memory, U32 address, VkImageCopy2KHR* s);
    ~MarshalVkImageCopy2KHR();
};

class MarshalVkImageBlit2KHR {
public:
    MarshalVkImageBlit2KHR() {}
    VkImageBlit2KHR s;
    MarshalVkImageBlit2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageBlit2KHR* s);
    static void write(KMemory* memory, U32 address, VkImageBlit2KHR* s);
    ~MarshalVkImageBlit2KHR();
};

class MarshalVkBufferImageCopy2KHR {
public:
    MarshalVkBufferImageCopy2KHR() {}
    VkBufferImageCopy2KHR s;
    MarshalVkBufferImageCopy2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferImageCopy2KHR* s);
    static void write(KMemory* memory, U32 address, VkBufferImageCopy2KHR* s);
    ~MarshalVkBufferImageCopy2KHR();
};

class MarshalVkImageResolve2KHR {
public:
    MarshalVkImageResolve2KHR() {}
    VkImageResolve2KHR s;
    MarshalVkImageResolve2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageResolve2KHR* s);
    static void write(KMemory* memory, U32 address, VkImageResolve2KHR* s);
    ~MarshalVkImageResolve2KHR();
};

class MarshalVkCopyBufferInfo2KHR {
public:
    MarshalVkCopyBufferInfo2KHR() {}
    VkCopyBufferInfo2KHR s;
    MarshalVkCopyBufferInfo2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCopyBufferInfo2KHR* s);
    static void write(KMemory* memory, U32 address, VkCopyBufferInfo2KHR* s);
    ~MarshalVkCopyBufferInfo2KHR();
};

class MarshalVkCopyImageInfo2KHR {
public:
    MarshalVkCopyImageInfo2KHR() {}
    VkCopyImageInfo2KHR s;
    MarshalVkCopyImageInfo2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCopyImageInfo2KHR* s);
    static void write(KMemory* memory, U32 address, VkCopyImageInfo2KHR* s);
    ~MarshalVkCopyImageInfo2KHR();
};

class MarshalVkBlitImageInfo2KHR {
public:
    MarshalVkBlitImageInfo2KHR() {}
    VkBlitImageInfo2KHR s;
    MarshalVkBlitImageInfo2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBlitImageInfo2KHR* s);
    static void write(KMemory* memory, U32 address, VkBlitImageInfo2KHR* s);
    ~MarshalVkBlitImageInfo2KHR();
};

class MarshalVkCopyBufferToImageInfo2KHR {
public:
    MarshalVkCopyBufferToImageInfo2KHR() {}
    VkCopyBufferToImageInfo2KHR s;
    MarshalVkCopyBufferToImageInfo2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCopyBufferToImageInfo2KHR* s);
    static void write(KMemory* memory, U32 address, VkCopyBufferToImageInfo2KHR* s);
    ~MarshalVkCopyBufferToImageInfo2KHR();
};

class MarshalVkCopyImageToBufferInfo2KHR {
public:
    MarshalVkCopyImageToBufferInfo2KHR() {}
    VkCopyImageToBufferInfo2KHR s;
    MarshalVkCopyImageToBufferInfo2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCopyImageToBufferInfo2KHR* s);
    static void write(KMemory* memory, U32 address, VkCopyImageToBufferInfo2KHR* s);
    ~MarshalVkCopyImageToBufferInfo2KHR();
};

class MarshalVkResolveImageInfo2KHR {
public:
    MarshalVkResolveImageInfo2KHR() {}
    VkResolveImageInfo2KHR s;
    MarshalVkResolveImageInfo2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkResolveImageInfo2KHR* s);
    static void write(KMemory* memory, U32 address, VkResolveImageInfo2KHR* s);
    ~MarshalVkResolveImageInfo2KHR();
};

class MarshalVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT {
public:
    MarshalVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT() {}
    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT s;
    MarshalVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* s);
    ~MarshalVkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT();
};

class MarshalVkFragmentShadingRateAttachmentInfoKHR {
public:
    MarshalVkFragmentShadingRateAttachmentInfoKHR() {}
    VkFragmentShadingRateAttachmentInfoKHR s;
    MarshalVkFragmentShadingRateAttachmentInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkFragmentShadingRateAttachmentInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkFragmentShadingRateAttachmentInfoKHR* s);
    ~MarshalVkFragmentShadingRateAttachmentInfoKHR();
};

class MarshalVkPipelineFragmentShadingRateStateCreateInfoKHR {
public:
    MarshalVkPipelineFragmentShadingRateStateCreateInfoKHR() {}
    VkPipelineFragmentShadingRateStateCreateInfoKHR s;
    MarshalVkPipelineFragmentShadingRateStateCreateInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineFragmentShadingRateStateCreateInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkPipelineFragmentShadingRateStateCreateInfoKHR* s);
    ~MarshalVkPipelineFragmentShadingRateStateCreateInfoKHR();
};

class MarshalVkPhysicalDeviceFragmentShadingRateFeaturesKHR {
public:
    MarshalVkPhysicalDeviceFragmentShadingRateFeaturesKHR() {}
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR s;
    MarshalVkPhysicalDeviceFragmentShadingRateFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRateFeaturesKHR();
};

class MarshalVkPhysicalDeviceFragmentShadingRatePropertiesKHR {
public:
    MarshalVkPhysicalDeviceFragmentShadingRatePropertiesKHR() {}
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR s;
    MarshalVkPhysicalDeviceFragmentShadingRatePropertiesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRatePropertiesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRatePropertiesKHR* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRatePropertiesKHR();
};

class MarshalVkPhysicalDeviceFragmentShadingRateKHR {
public:
    MarshalVkPhysicalDeviceFragmentShadingRateKHR() {}
    VkPhysicalDeviceFragmentShadingRateKHR s;
    MarshalVkPhysicalDeviceFragmentShadingRateKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateKHR* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRateKHR();
};

class MarshalVkPhysicalDeviceShaderTerminateInvocationFeaturesKHR {
public:
    MarshalVkPhysicalDeviceShaderTerminateInvocationFeaturesKHR() {}
    VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR s;
    MarshalVkPhysicalDeviceShaderTerminateInvocationFeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR* s);
    ~MarshalVkPhysicalDeviceShaderTerminateInvocationFeaturesKHR();
};

class MarshalVkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV {
public:
    MarshalVkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV() {}
    VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV s;
    MarshalVkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV();
};

class MarshalVkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV {
public:
    MarshalVkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV() {}
    VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV s;
    MarshalVkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV* s);
    ~MarshalVkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV();
};

class MarshalVkPipelineFragmentShadingRateEnumStateCreateInfoNV {
public:
    MarshalVkPipelineFragmentShadingRateEnumStateCreateInfoNV() {}
    VkPipelineFragmentShadingRateEnumStateCreateInfoNV s;
    MarshalVkPipelineFragmentShadingRateEnumStateCreateInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineFragmentShadingRateEnumStateCreateInfoNV* s);
    static void write(KMemory* memory, U32 address, VkPipelineFragmentShadingRateEnumStateCreateInfoNV* s);
    ~MarshalVkPipelineFragmentShadingRateEnumStateCreateInfoNV();
};

class MarshalVkAccelerationStructureBuildSizesInfoKHR {
public:
    MarshalVkAccelerationStructureBuildSizesInfoKHR() {}
    VkAccelerationStructureBuildSizesInfoKHR s;
    MarshalVkAccelerationStructureBuildSizesInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureBuildSizesInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureBuildSizesInfoKHR* s);
    ~MarshalVkAccelerationStructureBuildSizesInfoKHR();
};

class MarshalVkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE {
public:
    MarshalVkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE() {}
    VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE s;
    MarshalVkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE* s);
    ~MarshalVkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE();
};

class MarshalVkMutableDescriptorTypeListVALVE {
public:
    MarshalVkMutableDescriptorTypeListVALVE() {}
    VkMutableDescriptorTypeListVALVE s;
    MarshalVkMutableDescriptorTypeListVALVE(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMutableDescriptorTypeListVALVE* s);
    ~MarshalVkMutableDescriptorTypeListVALVE();
};

class MarshalVkMutableDescriptorTypeCreateInfoVALVE {
public:
    MarshalVkMutableDescriptorTypeCreateInfoVALVE() {}
    VkMutableDescriptorTypeCreateInfoVALVE s;
    MarshalVkMutableDescriptorTypeCreateInfoVALVE(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMutableDescriptorTypeCreateInfoVALVE* s);
    static void write(KMemory* memory, U32 address, VkMutableDescriptorTypeCreateInfoVALVE* s);
    ~MarshalVkMutableDescriptorTypeCreateInfoVALVE();
};

class MarshalVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT {
public:
    MarshalVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT() {}
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT s;
    MarshalVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceVertexInputDynamicStateFeaturesEXT();
};

class MarshalVkVertexInputBindingDescription2EXT {
public:
    MarshalVkVertexInputBindingDescription2EXT() {}
    VkVertexInputBindingDescription2EXT s;
    MarshalVkVertexInputBindingDescription2EXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkVertexInputBindingDescription2EXT* s);
    static void write(KMemory* memory, U32 address, VkVertexInputBindingDescription2EXT* s);
    ~MarshalVkVertexInputBindingDescription2EXT();
};

class MarshalVkVertexInputAttributeDescription2EXT {
public:
    MarshalVkVertexInputAttributeDescription2EXT() {}
    VkVertexInputAttributeDescription2EXT s;
    MarshalVkVertexInputAttributeDescription2EXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkVertexInputAttributeDescription2EXT* s);
    static void write(KMemory* memory, U32 address, VkVertexInputAttributeDescription2EXT* s);
    ~MarshalVkVertexInputAttributeDescription2EXT();
};

class MarshalVkPhysicalDeviceColorWriteEnableFeaturesEXT {
public:
    MarshalVkPhysicalDeviceColorWriteEnableFeaturesEXT() {}
    VkPhysicalDeviceColorWriteEnableFeaturesEXT s;
    MarshalVkPhysicalDeviceColorWriteEnableFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceColorWriteEnableFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceColorWriteEnableFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceColorWriteEnableFeaturesEXT();
};

class MarshalVkPipelineColorWriteCreateInfoEXT {
public:
    MarshalVkPipelineColorWriteCreateInfoEXT() {}
    VkPipelineColorWriteCreateInfoEXT s;
    MarshalVkPipelineColorWriteCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineColorWriteCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineColorWriteCreateInfoEXT* s);
    ~MarshalVkPipelineColorWriteCreateInfoEXT();
};

class MarshalVkMemoryBarrier2KHR {
public:
    MarshalVkMemoryBarrier2KHR() {}
    VkMemoryBarrier2KHR s;
    MarshalVkMemoryBarrier2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkMemoryBarrier2KHR* s);
    static void write(KMemory* memory, U32 address, VkMemoryBarrier2KHR* s);
    ~MarshalVkMemoryBarrier2KHR();
};

class MarshalVkImageMemoryBarrier2KHR {
public:
    MarshalVkImageMemoryBarrier2KHR() {}
    VkImageMemoryBarrier2KHR s;
    MarshalVkImageMemoryBarrier2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkImageMemoryBarrier2KHR* s);
    static void write(KMemory* memory, U32 address, VkImageMemoryBarrier2KHR* s);
    ~MarshalVkImageMemoryBarrier2KHR();
};

class MarshalVkBufferMemoryBarrier2KHR {
public:
    MarshalVkBufferMemoryBarrier2KHR() {}
    VkBufferMemoryBarrier2KHR s;
    MarshalVkBufferMemoryBarrier2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkBufferMemoryBarrier2KHR* s);
    static void write(KMemory* memory, U32 address, VkBufferMemoryBarrier2KHR* s);
    ~MarshalVkBufferMemoryBarrier2KHR();
};

class MarshalVkDependencyInfoKHR {
public:
    MarshalVkDependencyInfoKHR() {}
    VkDependencyInfoKHR s;
    MarshalVkDependencyInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDependencyInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkDependencyInfoKHR* s);
    ~MarshalVkDependencyInfoKHR();
};

class MarshalVkSemaphoreSubmitInfoKHR {
public:
    MarshalVkSemaphoreSubmitInfoKHR() {}
    VkSemaphoreSubmitInfoKHR s;
    MarshalVkSemaphoreSubmitInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSemaphoreSubmitInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkSemaphoreSubmitInfoKHR* s);
    ~MarshalVkSemaphoreSubmitInfoKHR();
};

class MarshalVkCommandBufferSubmitInfoKHR {
public:
    MarshalVkCommandBufferSubmitInfoKHR() {}
    VkCommandBufferSubmitInfoKHR s;
    MarshalVkCommandBufferSubmitInfoKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCommandBufferSubmitInfoKHR* s);
    static void write(KMemory* memory, U32 address, VkCommandBufferSubmitInfoKHR* s);
    ~MarshalVkCommandBufferSubmitInfoKHR();
};

class MarshalVkSubmitInfo2KHR {
public:
    MarshalVkSubmitInfo2KHR() {}
    VkSubmitInfo2KHR s;
    MarshalVkSubmitInfo2KHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkSubmitInfo2KHR* s);
    static void write(KMemory* memory, U32 address, VkSubmitInfo2KHR* s);
    ~MarshalVkSubmitInfo2KHR();
};

class MarshalVkQueueFamilyCheckpointProperties2NV {
public:
    MarshalVkQueueFamilyCheckpointProperties2NV() {}
    VkQueueFamilyCheckpointProperties2NV s;
    MarshalVkQueueFamilyCheckpointProperties2NV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkQueueFamilyCheckpointProperties2NV* s);
    static void write(KMemory* memory, U32 address, VkQueueFamilyCheckpointProperties2NV* s);
    ~MarshalVkQueueFamilyCheckpointProperties2NV();
};

class MarshalVkCheckpointData2NV {
public:
    MarshalVkCheckpointData2NV() {}
    VkCheckpointData2NV s;
    static void write(KMemory* memory, U32 address, VkCheckpointData2NV* s);
};

class MarshalVkPhysicalDeviceSynchronization2FeaturesKHR {
public:
    MarshalVkPhysicalDeviceSynchronization2FeaturesKHR() {}
    VkPhysicalDeviceSynchronization2FeaturesKHR s;
    MarshalVkPhysicalDeviceSynchronization2FeaturesKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceSynchronization2FeaturesKHR* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceSynchronization2FeaturesKHR* s);
    ~MarshalVkPhysicalDeviceSynchronization2FeaturesKHR();
};

class MarshalVkPhysicalDeviceInheritedViewportScissorFeaturesNV {
public:
    MarshalVkPhysicalDeviceInheritedViewportScissorFeaturesNV() {}
    VkPhysicalDeviceInheritedViewportScissorFeaturesNV s;
    MarshalVkPhysicalDeviceInheritedViewportScissorFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceInheritedViewportScissorFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceInheritedViewportScissorFeaturesNV* s);
    ~MarshalVkPhysicalDeviceInheritedViewportScissorFeaturesNV();
};

class MarshalVkCommandBufferInheritanceViewportScissorInfoNV {
public:
    MarshalVkCommandBufferInheritanceViewportScissorInfoNV() {}
    VkCommandBufferInheritanceViewportScissorInfoNV s;
    MarshalVkCommandBufferInheritanceViewportScissorInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCommandBufferInheritanceViewportScissorInfoNV* s);
    static void write(KMemory* memory, U32 address, VkCommandBufferInheritanceViewportScissorInfoNV* s);
    ~MarshalVkCommandBufferInheritanceViewportScissorInfoNV();
};

class MarshalVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT {
public:
    MarshalVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT() {}
    VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT s;
    MarshalVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT();
};

class MarshalVkPhysicalDeviceProvokingVertexFeaturesEXT {
public:
    MarshalVkPhysicalDeviceProvokingVertexFeaturesEXT() {}
    VkPhysicalDeviceProvokingVertexFeaturesEXT s;
    MarshalVkPhysicalDeviceProvokingVertexFeaturesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceProvokingVertexFeaturesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceProvokingVertexFeaturesEXT* s);
    ~MarshalVkPhysicalDeviceProvokingVertexFeaturesEXT();
};

class MarshalVkPhysicalDeviceProvokingVertexPropertiesEXT {
public:
    MarshalVkPhysicalDeviceProvokingVertexPropertiesEXT() {}
    VkPhysicalDeviceProvokingVertexPropertiesEXT s;
    MarshalVkPhysicalDeviceProvokingVertexPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceProvokingVertexPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceProvokingVertexPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceProvokingVertexPropertiesEXT();
};

class MarshalVkPipelineRasterizationProvokingVertexStateCreateInfoEXT {
public:
    MarshalVkPipelineRasterizationProvokingVertexStateCreateInfoEXT() {}
    VkPipelineRasterizationProvokingVertexStateCreateInfoEXT s;
    MarshalVkPipelineRasterizationProvokingVertexStateCreateInfoEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPipelineRasterizationProvokingVertexStateCreateInfoEXT* s);
    static void write(KMemory* memory, U32 address, VkPipelineRasterizationProvokingVertexStateCreateInfoEXT* s);
    ~MarshalVkPipelineRasterizationProvokingVertexStateCreateInfoEXT();
};

class MarshalVkCuFunctionCreateInfoNVX {
public:
    MarshalVkCuFunctionCreateInfoNVX() {}
    VkCuFunctionCreateInfoNVX s;
    MarshalVkCuFunctionCreateInfoNVX(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkCuFunctionCreateInfoNVX* s);
    static void write(KMemory* memory, U32 address, VkCuFunctionCreateInfoNVX* s);
    ~MarshalVkCuFunctionCreateInfoNVX();
};

class MarshalVkPhysicalDeviceDrmPropertiesEXT {
public:
    MarshalVkPhysicalDeviceDrmPropertiesEXT() {}
    VkPhysicalDeviceDrmPropertiesEXT s;
    MarshalVkPhysicalDeviceDrmPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceDrmPropertiesEXT* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceDrmPropertiesEXT* s);
    ~MarshalVkPhysicalDeviceDrmPropertiesEXT();
};

class MarshalVkPhysicalDeviceRayTracingMotionBlurFeaturesNV {
public:
    MarshalVkPhysicalDeviceRayTracingMotionBlurFeaturesNV() {}
    VkPhysicalDeviceRayTracingMotionBlurFeaturesNV s;
    MarshalVkPhysicalDeviceRayTracingMotionBlurFeaturesNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceRayTracingMotionBlurFeaturesNV* s);
    static void write(KMemory* memory, U32 address, VkPhysicalDeviceRayTracingMotionBlurFeaturesNV* s);
    ~MarshalVkPhysicalDeviceRayTracingMotionBlurFeaturesNV();
};

class MarshalVkAccelerationStructureGeometryMotionTrianglesDataNV {
public:
    MarshalVkAccelerationStructureGeometryMotionTrianglesDataNV() {}
    VkAccelerationStructureGeometryMotionTrianglesDataNV s;
    MarshalVkAccelerationStructureGeometryMotionTrianglesDataNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureGeometryMotionTrianglesDataNV* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureGeometryMotionTrianglesDataNV* s);
    ~MarshalVkAccelerationStructureGeometryMotionTrianglesDataNV();
};

class MarshalVkAccelerationStructureMotionInfoNV {
public:
    MarshalVkAccelerationStructureMotionInfoNV() {}
    VkAccelerationStructureMotionInfoNV s;
    MarshalVkAccelerationStructureMotionInfoNV(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkAccelerationStructureMotionInfoNV* s);
    static void write(KMemory* memory, U32 address, VkAccelerationStructureMotionInfoNV* s);
    ~MarshalVkAccelerationStructureMotionInfoNV();
};

class MarshalVkDrmFormatModifierPropertiesEXT {
public:
    MarshalVkDrmFormatModifierPropertiesEXT() {}
    VkDrmFormatModifierPropertiesEXT s;
    MarshalVkDrmFormatModifierPropertiesEXT(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkDrmFormatModifierPropertiesEXT* s);
};

class MarshalVkPhysicalDeviceFeatures {
public:
    MarshalVkPhysicalDeviceFeatures() {}
    VkPhysicalDeviceFeatures s;
    MarshalVkPhysicalDeviceFeatures(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceFeatures* s);
};

class MarshalVkFramebufferAttachmentImageInfo {
public:
    MarshalVkFramebufferAttachmentImageInfo() {}
    VkFramebufferAttachmentImageInfo s;
    MarshalVkFramebufferAttachmentImageInfo(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkFramebufferAttachmentImageInfo* s);
    ~MarshalVkFramebufferAttachmentImageInfo();
};

class MarshalVkPresentRegionKHR {
public:
    MarshalVkPresentRegionKHR() {}
    VkPresentRegionKHR s;
    MarshalVkPresentRegionKHR(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPresentRegionKHR* s);
    ~MarshalVkPresentRegionKHR();
};

class MarshalVkPhysicalDeviceLimits {
public:
    MarshalVkPhysicalDeviceLimits() {}
    VkPhysicalDeviceLimits s;
    MarshalVkPhysicalDeviceLimits(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPhysicalDeviceLimits* s);
};

class MarshalVkPresentTimeGOOGLE {
public:
    MarshalVkPresentTimeGOOGLE() {}
    VkPresentTimeGOOGLE s;
    MarshalVkPresentTimeGOOGLE(KMemory* memory, U32 address) {read(memory, address, &this->s);}
    static void read(KMemory* memory, U32 address, VkPresentTimeGOOGLE* s);
};

