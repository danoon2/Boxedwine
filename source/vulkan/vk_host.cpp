#include "boxedwine.h"
#ifdef BOXEDWINE_VULKAN
#include <SDL.h>
#include <SDL_vulkan.h>
#define VK_NO_PROTOTYPES
#include "vk/vulkan.h"
#include "vk/vulkan_core.h"

void* getVulkanPtr(U32 address);
U32 createVulkanPtr(U64 value);
void freeVulkanPtr(U32 p);

#define ARG1 cpu->peek32(1)
#define ARG2 cpu->peek32(2)
#define ARG3 cpu->peek32(3)
#define ARG4 cpu->peek32(4)
#define ARG5 cpu->peek32(5)
#define ARG6 cpu->peek32(6)
#define ARG7 cpu->peek32(7)
#define ARG8 cpu->peek32(8)
#define ARG9 cpu->peek32(9)
#define ARG10 cpu->peek32(10)
#define ARG11 cpu->peek32(11)
#define ARG12 cpu->peek32(12)
#define ARG13 cpu->peek32(13)
#define ARG14 cpu->peek32(14)
#define ARG15 cpu->peek32(15)
void* vulkanGetNextPtr(U32 address);
void vulkanWriteNextPtr(U32 address, void* pNext);
class MarshalVkPhysicalDeviceFeatures {
public:
    MarshalVkPhysicalDeviceFeatures() {}
    VkPhysicalDeviceFeatures s;
    MarshalVkPhysicalDeviceFeatures(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceFeatures* s) {
        s->robustBufferAccess = (VkBool32)readd(address);address+=4;
        s->fullDrawIndexUint32 = (VkBool32)readd(address);address+=4;
        s->imageCubeArray = (VkBool32)readd(address);address+=4;
        s->independentBlend = (VkBool32)readd(address);address+=4;
        s->geometryShader = (VkBool32)readd(address);address+=4;
        s->tessellationShader = (VkBool32)readd(address);address+=4;
        s->sampleRateShading = (VkBool32)readd(address);address+=4;
        s->dualSrcBlend = (VkBool32)readd(address);address+=4;
        s->logicOp = (VkBool32)readd(address);address+=4;
        s->multiDrawIndirect = (VkBool32)readd(address);address+=4;
        s->drawIndirectFirstInstance = (VkBool32)readd(address);address+=4;
        s->depthClamp = (VkBool32)readd(address);address+=4;
        s->depthBiasClamp = (VkBool32)readd(address);address+=4;
        s->fillModeNonSolid = (VkBool32)readd(address);address+=4;
        s->depthBounds = (VkBool32)readd(address);address+=4;
        s->wideLines = (VkBool32)readd(address);address+=4;
        s->largePoints = (VkBool32)readd(address);address+=4;
        s->alphaToOne = (VkBool32)readd(address);address+=4;
        s->multiViewport = (VkBool32)readd(address);address+=4;
        s->samplerAnisotropy = (VkBool32)readd(address);address+=4;
        s->textureCompressionETC2 = (VkBool32)readd(address);address+=4;
        s->textureCompressionASTC_LDR = (VkBool32)readd(address);address+=4;
        s->textureCompressionBC = (VkBool32)readd(address);address+=4;
        s->occlusionQueryPrecise = (VkBool32)readd(address);address+=4;
        s->pipelineStatisticsQuery = (VkBool32)readd(address);address+=4;
        s->vertexPipelineStoresAndAtomics = (VkBool32)readd(address);address+=4;
        s->fragmentStoresAndAtomics = (VkBool32)readd(address);address+=4;
        s->shaderTessellationAndGeometryPointSize = (VkBool32)readd(address);address+=4;
        s->shaderImageGatherExtended = (VkBool32)readd(address);address+=4;
        s->shaderStorageImageExtendedFormats = (VkBool32)readd(address);address+=4;
        s->shaderStorageImageMultisample = (VkBool32)readd(address);address+=4;
        s->shaderStorageImageReadWithoutFormat = (VkBool32)readd(address);address+=4;
        s->shaderStorageImageWriteWithoutFormat = (VkBool32)readd(address);address+=4;
        s->shaderUniformBufferArrayDynamicIndexing = (VkBool32)readd(address);address+=4;
        s->shaderSampledImageArrayDynamicIndexing = (VkBool32)readd(address);address+=4;
        s->shaderStorageBufferArrayDynamicIndexing = (VkBool32)readd(address);address+=4;
        s->shaderStorageImageArrayDynamicIndexing = (VkBool32)readd(address);address+=4;
        s->shaderClipDistance = (VkBool32)readd(address);address+=4;
        s->shaderCullDistance = (VkBool32)readd(address);address+=4;
        s->shaderFloat64 = (VkBool32)readd(address);address+=4;
        s->shaderInt64 = (VkBool32)readd(address);address+=4;
        s->shaderInt16 = (VkBool32)readd(address);address+=4;
        s->shaderResourceResidency = (VkBool32)readd(address);address+=4;
        s->shaderResourceMinLod = (VkBool32)readd(address);address+=4;
        s->sparseBinding = (VkBool32)readd(address);address+=4;
        s->sparseResidencyBuffer = (VkBool32)readd(address);address+=4;
        s->sparseResidencyImage2D = (VkBool32)readd(address);address+=4;
        s->sparseResidencyImage3D = (VkBool32)readd(address);address+=4;
        s->sparseResidency2Samples = (VkBool32)readd(address);address+=4;
        s->sparseResidency4Samples = (VkBool32)readd(address);address+=4;
        s->sparseResidency8Samples = (VkBool32)readd(address);address+=4;
        s->sparseResidency16Samples = (VkBool32)readd(address);address+=4;
        s->sparseResidencyAliased = (VkBool32)readd(address);address+=4;
        s->variableMultisampleRate = (VkBool32)readd(address);address+=4;
        s->inheritedQueries = (VkBool32)readd(address);address+=4;
    }
};

class MarshalVkPhysicalDeviceProperties {
public:
    MarshalVkPhysicalDeviceProperties() {}
    VkPhysicalDeviceProperties s;
    static void write(U32 address, VkPhysicalDeviceProperties* s) {
        writed(address, s->apiVersion);address+=4;
        writed(address, s->driverVersion);address+=4;
        writed(address, s->vendorID);address+=4;
        writed(address, s->deviceID);address+=4;
        writed(address, s->deviceType);address+=4;
        memcopyFromNative(address, s->deviceName, 256); address+=256;
        memcopyFromNative(address, s->pipelineCacheUUID, 16); address+=16;
        memcopyFromNative(address, &s->limits, 488); address+=488;
        memcopyFromNative(address, &s->sparseProperties, 20); address+=20;
    }
};

class MarshalVkApplicationInfo {
public:
    MarshalVkApplicationInfo() {}
    VkApplicationInfo s;
    MarshalVkApplicationInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkApplicationInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pApplicationName = NULL;
        } else {
            s->pApplicationName = (char*)getPhysicalAddress(paramAddress, 0 * sizeof(char));
        }
        s->applicationVersion = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pEngineName = NULL;
        } else {
            s->pEngineName = (char*)getPhysicalAddress(paramAddress, 0 * sizeof(char));
        }
        s->engineVersion = (uint32_t)readd(address);address+=4;
        s->apiVersion = (uint32_t)readd(address);address+=4;
    }
};

class MarshalVkDeviceQueueCreateInfo {
public:
    MarshalVkDeviceQueueCreateInfo() {}
    VkDeviceQueueCreateInfo s;
    MarshalVkDeviceQueueCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDeviceQueueCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkDeviceQueueCreateFlags)readd(address);address+=4;
        s->queueFamilyIndex = (uint32_t)readd(address);address+=4;
        s->queueCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pQueuePriorities = NULL;
        } else {
            s->pQueuePriorities = (float*)getPhysicalAddress(paramAddress, s->queueCount * sizeof(float));
        }
    }
};

class MarshalVkDeviceCreateInfo {
public:
    MarshalVkDeviceCreateInfo() {}
    VkDeviceCreateInfo s;
    MarshalVkDeviceCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDeviceCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkDeviceCreateFlags)readd(address);address+=4;
        s->queueCreateInfoCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pQueueCreateInfos = NULL;
        } else {
            VkDeviceQueueCreateInfo* pQueueCreateInfos = new VkDeviceQueueCreateInfo();
            MarshalVkDeviceQueueCreateInfo::read(paramAddress, pQueueCreateInfos);
            s->pQueueCreateInfos = pQueueCreateInfos;
        }
        s->enabledLayerCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->ppEnabledLayerNames = NULL;
        } else {
            char** ppEnabledLayerNames = new char*[s->enabledLayerCount];
            for (int i=0;i<(int)s->enabledLayerCount;i++) {
                ppEnabledLayerNames[i] = (char*)getPhysicalAddress(paramAddress + i*4, 0);
            }
            s->ppEnabledLayerNames = ppEnabledLayerNames;
        }
        s->enabledExtensionCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->ppEnabledExtensionNames = NULL;
        } else {
            char** ppEnabledExtensionNames = new char*[s->enabledExtensionCount];
            for (int i=0;i<(int)s->enabledExtensionCount;i++) {
                ppEnabledExtensionNames[i] = (char*)getPhysicalAddress(paramAddress + i*4, 0);
            }
            s->ppEnabledExtensionNames = ppEnabledExtensionNames;
        }
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pEnabledFeatures = NULL;
        } else {
            VkPhysicalDeviceFeatures* pEnabledFeatures = new VkPhysicalDeviceFeatures();
            MarshalVkPhysicalDeviceFeatures::read(paramAddress, pEnabledFeatures);
            s->pEnabledFeatures = pEnabledFeatures;
        }
    }
};

class MarshalVkInstanceCreateInfo {
public:
    MarshalVkInstanceCreateInfo() {}
    VkInstanceCreateInfo s;
    MarshalVkInstanceCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkInstanceCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkInstanceCreateFlags)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pApplicationInfo = NULL;
        } else {
            VkApplicationInfo* pApplicationInfo = new VkApplicationInfo();
            MarshalVkApplicationInfo::read(paramAddress, pApplicationInfo);
            s->pApplicationInfo = pApplicationInfo;
        }
        s->enabledLayerCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->ppEnabledLayerNames = NULL;
        } else {
            char** ppEnabledLayerNames = new char*[s->enabledLayerCount];
            for (int i=0;i<(int)s->enabledLayerCount;i++) {
                ppEnabledLayerNames[i] = (char*)getPhysicalAddress(paramAddress + i*4, 0);
            }
            s->ppEnabledLayerNames = ppEnabledLayerNames;
        }
        s->enabledExtensionCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->ppEnabledExtensionNames = NULL;
        } else {
            char** ppEnabledExtensionNames = new char*[s->enabledExtensionCount];
            for (int i=0;i<(int)s->enabledExtensionCount;i++) {
                ppEnabledExtensionNames[i] = (char*)getPhysicalAddress(paramAddress + i*4, 0);
            }
            s->ppEnabledExtensionNames = ppEnabledExtensionNames;
        }
    }
};

class MarshalVkMemoryAllocateInfo {
public:
    MarshalVkMemoryAllocateInfo() {}
    VkMemoryAllocateInfo s;
    MarshalVkMemoryAllocateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkMemoryAllocateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->allocationSize = (VkDeviceSize)readq(address);address+=8;
        s->memoryTypeIndex = (uint32_t)readd(address);address+=4;
    }
};

class MarshalVkBufferCreateInfo {
public:
    MarshalVkBufferCreateInfo() {}
    VkBufferCreateInfo s;
    MarshalVkBufferCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkBufferCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkBufferCreateFlags)readd(address);address+=4;
        s->size = (VkDeviceSize)readq(address);address+=8;
        s->usage = (VkBufferUsageFlags)readd(address);address+=4;
        s->sharingMode = (VkSharingMode)readd(address);address+=4;
        s->queueFamilyIndexCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pQueueFamilyIndices = NULL;
        } else {
            s->pQueueFamilyIndices = (uint32_t*)getPhysicalAddress(paramAddress, s->queueFamilyIndexCount * sizeof(uint32_t));
        }
    }
};

class MarshalVkBufferViewCreateInfo {
public:
    MarshalVkBufferViewCreateInfo() {}
    VkBufferViewCreateInfo s;
    MarshalVkBufferViewCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkBufferViewCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkBufferViewCreateFlags)readd(address);address+=4;
        s->buffer = (VkBuffer)readq(address);address+=8;
        s->format = (VkFormat)readd(address);address+=4;
        s->offset = (VkDeviceSize)readq(address);address+=8;
        s->range = (VkDeviceSize)readq(address);address+=8;
    }
};

class MarshalVkImageCreateInfo {
public:
    MarshalVkImageCreateInfo() {}
    VkImageCreateInfo s;
    MarshalVkImageCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkImageCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkImageCreateFlags)readd(address);address+=4;
        s->imageType = (VkImageType)readd(address);address+=4;
        s->format = (VkFormat)readd(address);address+=4;
        memcopyToNative(paramAddress, &s->extent, 12);
        s->mipLevels = (uint32_t)readd(address);address+=4;
        s->arrayLayers = (uint32_t)readd(address);address+=4;
        s->samples = (VkSampleCountFlagBits)readd(address);address+=4;
        s->tiling = (VkImageTiling)readd(address);address+=4;
        s->usage = (VkImageUsageFlags)readd(address);address+=4;
        s->sharingMode = (VkSharingMode)readd(address);address+=4;
        s->queueFamilyIndexCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pQueueFamilyIndices = NULL;
        } else {
            s->pQueueFamilyIndices = (uint32_t*)getPhysicalAddress(paramAddress, s->queueFamilyIndexCount * sizeof(uint32_t));
        }
        s->initialLayout = (VkImageLayout)readd(address);address+=4;
    }
};

class MarshalVkImageViewCreateInfo {
public:
    MarshalVkImageViewCreateInfo() {}
    VkImageViewCreateInfo s;
    MarshalVkImageViewCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkImageViewCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkImageViewCreateFlags)readd(address);address+=4;
        s->image = (VkImage)readq(address);address+=8;
        s->viewType = (VkImageViewType)readd(address);address+=4;
        s->format = (VkFormat)readd(address);address+=4;
        memcopyToNative(paramAddress, &s->components, 16);
        memcopyToNative(paramAddress, &s->subresourceRange, 20);
    }
};

class MarshalVkShaderModuleCreateInfo {
public:
    MarshalVkShaderModuleCreateInfo() {}
    VkShaderModuleCreateInfo s;
    MarshalVkShaderModuleCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkShaderModuleCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkShaderModuleCreateFlags)readd(address);address+=4;
        s->codeSize = (size_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pCode = NULL;
        } else {
            s->pCode = (uint32_t*)getPhysicalAddress(paramAddress, s->codeSize / 4 * sizeof(uint32_t));
        }
    }
};

class MarshalVkDescriptorSetLayoutBinding {
public:
    MarshalVkDescriptorSetLayoutBinding() {}
    VkDescriptorSetLayoutBinding s;
    MarshalVkDescriptorSetLayoutBinding(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDescriptorSetLayoutBinding* s) {
        s->binding = (uint32_t)readd(address);address+=4;
        s->descriptorType = (VkDescriptorType)readd(address);address+=4;
        s->descriptorCount = (uint32_t)readd(address);address+=4;
        s->stageFlags = (VkShaderStageFlags)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pImmutableSamplers = NULL;
        } else {
            s->pImmutableSamplers = (VkSampler*)getPhysicalAddress(paramAddress, s->descriptorCount * sizeof(VkSampler));
        }
    }
};

class MarshalVkDescriptorSetLayoutCreateInfo {
public:
    MarshalVkDescriptorSetLayoutCreateInfo() {}
    VkDescriptorSetLayoutCreateInfo s;
    MarshalVkDescriptorSetLayoutCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDescriptorSetLayoutCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkDescriptorSetLayoutCreateFlags)readd(address);address+=4;
        s->bindingCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pBindings = NULL;
        } else {
            VkDescriptorSetLayoutBinding* pBindings = new VkDescriptorSetLayoutBinding();
            MarshalVkDescriptorSetLayoutBinding::read(paramAddress, pBindings);
            s->pBindings = pBindings;
        }
    }
};

class MarshalVkDescriptorPoolSize {
public:
    MarshalVkDescriptorPoolSize() {}
    VkDescriptorPoolSize s;
    MarshalVkDescriptorPoolSize(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDescriptorPoolSize* s) {
        s->type = (VkDescriptorType)readd(address);address+=4;
        s->descriptorCount = (uint32_t)readd(address);address+=4;
    }
};

class MarshalVkDescriptorPoolCreateInfo {
public:
    MarshalVkDescriptorPoolCreateInfo() {}
    VkDescriptorPoolCreateInfo s;
    MarshalVkDescriptorPoolCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDescriptorPoolCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkDescriptorPoolCreateFlags)readd(address);address+=4;
        s->maxSets = (uint32_t)readd(address);address+=4;
        s->poolSizeCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pPoolSizes = NULL;
        } else {
            VkDescriptorPoolSize* pPoolSizes = new VkDescriptorPoolSize();
            MarshalVkDescriptorPoolSize::read(paramAddress, pPoolSizes);
            s->pPoolSizes = pPoolSizes;
        }
    }
};

class MarshalVkDescriptorSetAllocateInfo {
public:
    MarshalVkDescriptorSetAllocateInfo() {}
    VkDescriptorSetAllocateInfo s;
    MarshalVkDescriptorSetAllocateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDescriptorSetAllocateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->descriptorPool = (VkDescriptorPool)readq(address);address+=8;
        s->descriptorSetCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pSetLayouts = NULL;
        } else {
            s->pSetLayouts = (VkDescriptorSetLayout*)getPhysicalAddress(paramAddress, s->descriptorSetCount * sizeof(VkDescriptorSetLayout));
        }
    }
};

class MarshalVkPipelineCacheCreateInfo {
public:
    MarshalVkPipelineCacheCreateInfo() {}
    VkPipelineCacheCreateInfo s;
    MarshalVkPipelineCacheCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPipelineCacheCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkPipelineCacheCreateFlags)readd(address);address+=4;
        s->initialDataSize = (size_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pInitialData = NULL;
        } else {
            s->pInitialData = (void*)getPhysicalAddress(paramAddress, s->initialDataSize);
        }
    }
};

class MarshalVkPipelineLayoutCreateInfo {
public:
    MarshalVkPipelineLayoutCreateInfo() {}
    VkPipelineLayoutCreateInfo s;
    MarshalVkPipelineLayoutCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPipelineLayoutCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkPipelineLayoutCreateFlags)readd(address);address+=4;
        s->setLayoutCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pSetLayouts = NULL;
        } else {
            s->pSetLayouts = (VkDescriptorSetLayout*)getPhysicalAddress(paramAddress, s->setLayoutCount * sizeof(VkDescriptorSetLayout));
        }
        s->pushConstantRangeCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pPushConstantRanges = NULL;
        } else {
            s->pPushConstantRanges = (VkPushConstantRange*)getPhysicalAddress(paramAddress, s->pushConstantRangeCount * sizeof(VkPushConstantRange));
        }
    }
};

class MarshalVkSamplerCreateInfo {
public:
    MarshalVkSamplerCreateInfo() {}
    VkSamplerCreateInfo s;
    MarshalVkSamplerCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSamplerCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkSamplerCreateFlags)readd(address);address+=4;
        s->magFilter = (VkFilter)readd(address);address+=4;
        s->minFilter = (VkFilter)readd(address);address+=4;
        s->mipmapMode = (VkSamplerMipmapMode)readd(address);address+=4;
        s->addressModeU = (VkSamplerAddressMode)readd(address);address+=4;
        s->addressModeV = (VkSamplerAddressMode)readd(address);address+=4;
        s->addressModeW = (VkSamplerAddressMode)readd(address);address+=4;
        s->mipLodBias = (float)readd(address);address+=4;
        s->anisotropyEnable = (VkBool32)readd(address);address+=4;
        s->maxAnisotropy = (float)readd(address);address+=4;
        s->compareEnable = (VkBool32)readd(address);address+=4;
        s->compareOp = (VkCompareOp)readd(address);address+=4;
        s->minLod = (float)readd(address);address+=4;
        s->maxLod = (float)readd(address);address+=4;
        s->borderColor = (VkBorderColor)readd(address);address+=4;
        s->unnormalizedCoordinates = (VkBool32)readd(address);address+=4;
    }
};

class MarshalVkCommandPoolCreateInfo {
public:
    MarshalVkCommandPoolCreateInfo() {}
    VkCommandPoolCreateInfo s;
    MarshalVkCommandPoolCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkCommandPoolCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkCommandPoolCreateFlags)readd(address);address+=4;
        s->queueFamilyIndex = (uint32_t)readd(address);address+=4;
    }
};

class MarshalVkCommandBufferAllocateInfo {
public:
    MarshalVkCommandBufferAllocateInfo() {}
    VkCommandBufferAllocateInfo s;
    MarshalVkCommandBufferAllocateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkCommandBufferAllocateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->commandPool = (VkCommandPool)readq(address);address+=8;
        s->level = (VkCommandBufferLevel)readd(address);address+=4;
        s->commandBufferCount = (uint32_t)readd(address);address+=4;
    }
};

class MarshalVkCommandBufferInheritanceInfo {
public:
    MarshalVkCommandBufferInheritanceInfo() {}
    VkCommandBufferInheritanceInfo s;
    MarshalVkCommandBufferInheritanceInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkCommandBufferInheritanceInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->renderPass = (VkRenderPass)readq(address);address+=8;
        s->subpass = (uint32_t)readd(address);address+=4;
        s->framebuffer = (VkFramebuffer)readq(address);address+=8;
        s->occlusionQueryEnable = (VkBool32)readd(address);address+=4;
        s->queryFlags = (VkQueryControlFlags)readd(address);address+=4;
        s->pipelineStatistics = (VkQueryPipelineStatisticFlags)readd(address);address+=4;
    }
};

class MarshalVkCommandBufferBeginInfo {
public:
    MarshalVkCommandBufferBeginInfo() {}
    VkCommandBufferBeginInfo s;
    MarshalVkCommandBufferBeginInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkCommandBufferBeginInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkCommandBufferUsageFlags)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pInheritanceInfo = NULL;
        } else {
            VkCommandBufferInheritanceInfo* pInheritanceInfo = new VkCommandBufferInheritanceInfo();
            MarshalVkCommandBufferInheritanceInfo::read(paramAddress, pInheritanceInfo);
            s->pInheritanceInfo = pInheritanceInfo;
        }
    }
};

class MarshalVkRenderPassBeginInfo {
public:
    MarshalVkRenderPassBeginInfo() {}
    VkRenderPassBeginInfo s;
    MarshalVkRenderPassBeginInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkRenderPassBeginInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->renderPass = (VkRenderPass)readq(address);address+=8;
        s->framebuffer = (VkFramebuffer)readq(address);address+=8;
        memcopyToNative(paramAddress, &s->renderArea, 16);
        s->clearValueCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pClearValues = NULL;
        } else {
            s->pClearValues = (VkClearValue*)getPhysicalAddress(paramAddress, s->clearValueCount * sizeof(VkClearValue));
        }
    }
};

class MarshalVkAttachmentDescription {
public:
    MarshalVkAttachmentDescription() {}
    VkAttachmentDescription s;
    MarshalVkAttachmentDescription(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkAttachmentDescription* s) {
        s->flags = (VkAttachmentDescriptionFlags)readd(address);address+=4;
        s->format = (VkFormat)readd(address);address+=4;
        s->samples = (VkSampleCountFlagBits)readd(address);address+=4;
        s->loadOp = (VkAttachmentLoadOp)readd(address);address+=4;
        s->storeOp = (VkAttachmentStoreOp)readd(address);address+=4;
        s->stencilLoadOp = (VkAttachmentLoadOp)readd(address);address+=4;
        s->stencilStoreOp = (VkAttachmentStoreOp)readd(address);address+=4;
        s->initialLayout = (VkImageLayout)readd(address);address+=4;
        s->finalLayout = (VkImageLayout)readd(address);address+=4;
    }
};

class MarshalVkAttachmentReference {
public:
    MarshalVkAttachmentReference() {}
    VkAttachmentReference s;
    MarshalVkAttachmentReference(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkAttachmentReference* s) {
        s->attachment = (uint32_t)readd(address);address+=4;
        s->layout = (VkImageLayout)readd(address);address+=4;
    }
};

class MarshalVkSubpassDescription {
public:
    MarshalVkSubpassDescription() {}
    VkSubpassDescription s;
    MarshalVkSubpassDescription(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSubpassDescription* s) {
        s->flags = (VkSubpassDescriptionFlags)readd(address);address+=4;
        s->pipelineBindPoint = (VkPipelineBindPoint)readd(address);address+=4;
        s->inputAttachmentCount = (uint32_t)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pInputAttachments = NULL;
        } else {
            VkAttachmentReference* pInputAttachments = new VkAttachmentReference();
            MarshalVkAttachmentReference::read(paramAddress, pInputAttachments);
            s->pInputAttachments = pInputAttachments;
        }
        s->colorAttachmentCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pColorAttachments = NULL;
        } else {
            VkAttachmentReference* pColorAttachments = new VkAttachmentReference();
            MarshalVkAttachmentReference::read(paramAddress, pColorAttachments);
            s->pColorAttachments = pColorAttachments;
        }
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pResolveAttachments = NULL;
        } else {
            VkAttachmentReference* pResolveAttachments = new VkAttachmentReference();
            MarshalVkAttachmentReference::read(paramAddress, pResolveAttachments);
            s->pResolveAttachments = pResolveAttachments;
        }
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pDepthStencilAttachment = NULL;
        } else {
            VkAttachmentReference* pDepthStencilAttachment = new VkAttachmentReference();
            MarshalVkAttachmentReference::read(paramAddress, pDepthStencilAttachment);
            s->pDepthStencilAttachment = pDepthStencilAttachment;
        }
        s->preserveAttachmentCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pPreserveAttachments = NULL;
        } else {
            s->pPreserveAttachments = (uint32_t*)getPhysicalAddress(paramAddress, s->preserveAttachmentCount * sizeof(uint32_t));
        }
    }
};

class MarshalVkRenderPassCreateInfo {
public:
    MarshalVkRenderPassCreateInfo() {}
    VkRenderPassCreateInfo s;
    MarshalVkRenderPassCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkRenderPassCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkRenderPassCreateFlags)readd(address);address+=4;
        s->attachmentCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pAttachments = NULL;
        } else {
            VkAttachmentDescription* pAttachments = new VkAttachmentDescription();
            MarshalVkAttachmentDescription::read(paramAddress, pAttachments);
            s->pAttachments = pAttachments;
        }
        s->subpassCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pSubpasses = NULL;
        } else {
            VkSubpassDescription* pSubpasses = new VkSubpassDescription();
            MarshalVkSubpassDescription::read(paramAddress, pSubpasses);
            s->pSubpasses = pSubpasses;
        }
        s->dependencyCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pDependencies = NULL;
        } else {
            s->pDependencies = (VkSubpassDependency*)getPhysicalAddress(paramAddress, s->dependencyCount * sizeof(VkSubpassDependency));
        }
    }
};

class MarshalVkEventCreateInfo {
public:
    MarshalVkEventCreateInfo() {}
    VkEventCreateInfo s;
    MarshalVkEventCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkEventCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkEventCreateFlags)readd(address);address+=4;
    }
};

class MarshalVkFenceCreateInfo {
public:
    MarshalVkFenceCreateInfo() {}
    VkFenceCreateInfo s;
    MarshalVkFenceCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkFenceCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkFenceCreateFlags)readd(address);address+=4;
    }
};

class MarshalVkSemaphoreCreateInfo {
public:
    MarshalVkSemaphoreCreateInfo() {}
    VkSemaphoreCreateInfo s;
    MarshalVkSemaphoreCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSemaphoreCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkSemaphoreCreateFlags)readd(address);address+=4;
    }
};

class MarshalVkQueryPoolCreateInfo {
public:
    MarshalVkQueryPoolCreateInfo() {}
    VkQueryPoolCreateInfo s;
    MarshalVkQueryPoolCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkQueryPoolCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkQueryPoolCreateFlags)readd(address);address+=4;
        s->queryType = (VkQueryType)readd(address);address+=4;
        s->queryCount = (uint32_t)readd(address);address+=4;
        s->pipelineStatistics = (VkQueryPipelineStatisticFlags)readd(address);address+=4;
    }
};

class MarshalVkFramebufferCreateInfo {
public:
    MarshalVkFramebufferCreateInfo() {}
    VkFramebufferCreateInfo s;
    MarshalVkFramebufferCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkFramebufferCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkFramebufferCreateFlags)readd(address);address+=4;
        s->renderPass = (VkRenderPass)readq(address);address+=8;
        s->attachmentCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pAttachments = NULL;
        } else {
            s->pAttachments = (VkImageView*)getPhysicalAddress(paramAddress, s->attachmentCount * sizeof(VkImageView));
        }
        s->width = (uint32_t)readd(address);address+=4;
        s->height = (uint32_t)readd(address);address+=4;
        s->layers = (uint32_t)readd(address);address+=4;
    }
};

class MarshalVkPhysicalDeviceFeatures2 {
public:
    MarshalVkPhysicalDeviceFeatures2() {}
    VkPhysicalDeviceFeatures2 s;
    static void write(U32 address, VkPhysicalDeviceFeatures2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->features, 220); address+=220;
    }
};

class MarshalVkPhysicalDeviceProperties2 {
public:
    MarshalVkPhysicalDeviceProperties2() {}
    VkPhysicalDeviceProperties2 s;
    static void write(U32 address, VkPhysicalDeviceProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->properties, 800); address+=800;
    }
};

class MarshalVkFormatProperties2 {
public:
    MarshalVkFormatProperties2() {}
    VkFormatProperties2 s;
    static void write(U32 address, VkFormatProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->formatProperties, 12); address+=12;
    }
};

class MarshalVkImageFormatProperties2 {
public:
    MarshalVkImageFormatProperties2() {}
    VkImageFormatProperties2 s;
    static void write(U32 address, VkImageFormatProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->imageFormatProperties, 32); address+=32;
    }
};

class MarshalVkPhysicalDeviceImageFormatInfo2 {
public:
    MarshalVkPhysicalDeviceImageFormatInfo2() {}
    VkPhysicalDeviceImageFormatInfo2 s;
    MarshalVkPhysicalDeviceImageFormatInfo2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceImageFormatInfo2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->format = (VkFormat)readd(address);address+=4;
        s->type = (VkImageType)readd(address);address+=4;
        s->tiling = (VkImageTiling)readd(address);address+=4;
        s->usage = (VkImageUsageFlags)readd(address);address+=4;
        s->flags = (VkImageCreateFlags)readd(address);address+=4;
    }
};

class MarshalVkQueueFamilyProperties2 {
public:
    MarshalVkQueueFamilyProperties2() {}
    VkQueueFamilyProperties2 s;
    static void write(U32 address, VkQueueFamilyProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->queueFamilyProperties, 24); address+=24;
    }
};

class MarshalVkPhysicalDeviceMemoryProperties2 {
public:
    MarshalVkPhysicalDeviceMemoryProperties2() {}
    VkPhysicalDeviceMemoryProperties2 s;
    static void write(U32 address, VkPhysicalDeviceMemoryProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->memoryProperties, 456); address+=456;
    }
};

class MarshalVkSparseImageFormatProperties2 {
public:
    MarshalVkSparseImageFormatProperties2() {}
    VkSparseImageFormatProperties2 s;
    static void write(U32 address, VkSparseImageFormatProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->properties, 20); address+=20;
    }
};

class MarshalVkPhysicalDeviceSparseImageFormatInfo2 {
public:
    MarshalVkPhysicalDeviceSparseImageFormatInfo2() {}
    VkPhysicalDeviceSparseImageFormatInfo2 s;
    MarshalVkPhysicalDeviceSparseImageFormatInfo2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceSparseImageFormatInfo2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->format = (VkFormat)readd(address);address+=4;
        s->type = (VkImageType)readd(address);address+=4;
        s->samples = (VkSampleCountFlagBits)readd(address);address+=4;
        s->usage = (VkImageUsageFlags)readd(address);address+=4;
        s->tiling = (VkImageTiling)readd(address);address+=4;
    }
};

class MarshalVkPhysicalDeviceExternalBufferInfo {
public:
    MarshalVkPhysicalDeviceExternalBufferInfo() {}
    VkPhysicalDeviceExternalBufferInfo s;
    MarshalVkPhysicalDeviceExternalBufferInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceExternalBufferInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkBufferCreateFlags)readd(address);address+=4;
        s->usage = (VkBufferUsageFlags)readd(address);address+=4;
        s->handleType = (VkExternalMemoryHandleTypeFlagBits)readd(address);address+=4;
    }
};

class MarshalVkExternalBufferProperties {
public:
    MarshalVkExternalBufferProperties() {}
    VkExternalBufferProperties s;
    static void write(U32 address, VkExternalBufferProperties* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->externalMemoryProperties, 12); address+=12;
    }
};

class MarshalVkPhysicalDeviceExternalSemaphoreInfo {
public:
    MarshalVkPhysicalDeviceExternalSemaphoreInfo() {}
    VkPhysicalDeviceExternalSemaphoreInfo s;
    MarshalVkPhysicalDeviceExternalSemaphoreInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceExternalSemaphoreInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->handleType = (VkExternalSemaphoreHandleTypeFlagBits)readd(address);address+=4;
    }
};

class MarshalVkExternalSemaphoreProperties {
public:
    MarshalVkExternalSemaphoreProperties() {}
    VkExternalSemaphoreProperties s;
    static void write(U32 address, VkExternalSemaphoreProperties* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        writed(address, s->exportFromImportedHandleTypes);address+=4;
        writed(address, s->compatibleHandleTypes);address+=4;
        writed(address, s->externalSemaphoreFeatures);address+=4;
    }
};

class MarshalVkPhysicalDeviceExternalFenceInfo {
public:
    MarshalVkPhysicalDeviceExternalFenceInfo() {}
    VkPhysicalDeviceExternalFenceInfo s;
    MarshalVkPhysicalDeviceExternalFenceInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceExternalFenceInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->handleType = (VkExternalFenceHandleTypeFlagBits)readd(address);address+=4;
    }
};

class MarshalVkExternalFenceProperties {
public:
    MarshalVkExternalFenceProperties() {}
    VkExternalFenceProperties s;
    static void write(U32 address, VkExternalFenceProperties* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        writed(address, s->exportFromImportedHandleTypes);address+=4;
        writed(address, s->compatibleHandleTypes);address+=4;
        writed(address, s->externalFenceFeatures);address+=4;
    }
};

class MarshalVkPhysicalDeviceGroupProperties {
public:
    MarshalVkPhysicalDeviceGroupProperties() {}
    VkPhysicalDeviceGroupProperties s;
    static void write(U32 address, VkPhysicalDeviceGroupProperties* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        writed(address, s->physicalDeviceCount);address+=4;
        memcopyFromNative(address, s->physicalDevices, 128); address+=128;
        writed(address, s->subsetAllocation);address+=4;
    }
};

class MarshalVkDescriptorUpdateTemplateEntry {
public:
    MarshalVkDescriptorUpdateTemplateEntry() {}
    VkDescriptorUpdateTemplateEntry s;
    MarshalVkDescriptorUpdateTemplateEntry(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDescriptorUpdateTemplateEntry* s) {
        s->dstBinding = (uint32_t)readd(address);address+=4;
        s->dstArrayElement = (uint32_t)readd(address);address+=4;
        s->descriptorCount = (uint32_t)readd(address);address+=4;
        s->descriptorType = (VkDescriptorType)readd(address);address+=4;
        s->offset = (size_t)readd(address);address+=4;
        s->stride = (size_t)readd(address);address+=4;
    }
};

class MarshalVkDescriptorUpdateTemplateCreateInfo {
public:
    MarshalVkDescriptorUpdateTemplateCreateInfo() {}
    VkDescriptorUpdateTemplateCreateInfo s;
    MarshalVkDescriptorUpdateTemplateCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDescriptorUpdateTemplateCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkDescriptorUpdateTemplateCreateFlags)readd(address);address+=4;
        s->descriptorUpdateEntryCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pDescriptorUpdateEntries = NULL;
        } else {
            VkDescriptorUpdateTemplateEntry* pDescriptorUpdateEntries = new VkDescriptorUpdateTemplateEntry();
            MarshalVkDescriptorUpdateTemplateEntry::read(paramAddress, pDescriptorUpdateEntries);
            s->pDescriptorUpdateEntries = pDescriptorUpdateEntries;
        }
        s->templateType = (VkDescriptorUpdateTemplateType)readd(address);address+=4;
        s->descriptorSetLayout = (VkDescriptorSetLayout)readq(address);address+=8;
        s->pipelineBindPoint = (VkPipelineBindPoint)readd(address);address+=4;
        s->pipelineLayout = (VkPipelineLayout)readq(address);address+=8;
        s->set = (uint32_t)readd(address);address+=4;
    }
};

class MarshalVkBufferMemoryRequirementsInfo2 {
public:
    MarshalVkBufferMemoryRequirementsInfo2() {}
    VkBufferMemoryRequirementsInfo2 s;
    MarshalVkBufferMemoryRequirementsInfo2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkBufferMemoryRequirementsInfo2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->buffer = (VkBuffer)readq(address);address+=8;
    }
};

class MarshalVkImageMemoryRequirementsInfo2 {
public:
    MarshalVkImageMemoryRequirementsInfo2() {}
    VkImageMemoryRequirementsInfo2 s;
    MarshalVkImageMemoryRequirementsInfo2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkImageMemoryRequirementsInfo2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->image = (VkImage)readq(address);address+=8;
    }
};

class MarshalVkImageSparseMemoryRequirementsInfo2 {
public:
    MarshalVkImageSparseMemoryRequirementsInfo2() {}
    VkImageSparseMemoryRequirementsInfo2 s;
    MarshalVkImageSparseMemoryRequirementsInfo2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkImageSparseMemoryRequirementsInfo2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->image = (VkImage)readq(address);address+=8;
    }
};

class MarshalVkMemoryRequirements2 {
public:
    MarshalVkMemoryRequirements2() {}
    VkMemoryRequirements2 s;
    static void write(U32 address, VkMemoryRequirements2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->memoryRequirements, 20); address+=20;
    }
};

class MarshalVkSparseImageMemoryRequirements2 {
public:
    MarshalVkSparseImageMemoryRequirements2() {}
    VkSparseImageMemoryRequirements2 s;
    static void write(U32 address, VkSparseImageMemoryRequirements2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        memcopyFromNative(address, &s->memoryRequirements, 48); address+=48;
    }
};

class MarshalVkSamplerYcbcrConversionCreateInfo {
public:
    MarshalVkSamplerYcbcrConversionCreateInfo() {}
    VkSamplerYcbcrConversionCreateInfo s;
    MarshalVkSamplerYcbcrConversionCreateInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSamplerYcbcrConversionCreateInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->format = (VkFormat)readd(address);address+=4;
        s->ycbcrModel = (VkSamplerYcbcrModelConversion)readd(address);address+=4;
        s->ycbcrRange = (VkSamplerYcbcrRange)readd(address);address+=4;
        memcopyToNative(paramAddress, &s->components, 16);
        s->xChromaOffset = (VkChromaLocation)readd(address);address+=4;
        s->yChromaOffset = (VkChromaLocation)readd(address);address+=4;
        s->chromaFilter = (VkFilter)readd(address);address+=4;
        s->forceExplicitReconstruction = (VkBool32)readd(address);address+=4;
    }
};

class MarshalVkDeviceQueueInfo2 {
public:
    MarshalVkDeviceQueueInfo2() {}
    VkDeviceQueueInfo2 s;
    MarshalVkDeviceQueueInfo2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDeviceQueueInfo2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkDeviceQueueCreateFlags)readd(address);address+=4;
        s->queueFamilyIndex = (uint32_t)readd(address);address+=4;
        s->queueIndex = (uint32_t)readd(address);address+=4;
    }
};

class MarshalVkDescriptorSetLayoutSupport {
public:
    MarshalVkDescriptorSetLayoutSupport() {}
    VkDescriptorSetLayoutSupport s;
    static void write(U32 address, VkDescriptorSetLayoutSupport* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
        }
        writed(address, s->supported);address+=4;
    }
};

class MarshalVkAttachmentDescription2 {
public:
    MarshalVkAttachmentDescription2() {}
    VkAttachmentDescription2 s;
    MarshalVkAttachmentDescription2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkAttachmentDescription2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkAttachmentDescriptionFlags)readd(address);address+=4;
        s->format = (VkFormat)readd(address);address+=4;
        s->samples = (VkSampleCountFlagBits)readd(address);address+=4;
        s->loadOp = (VkAttachmentLoadOp)readd(address);address+=4;
        s->storeOp = (VkAttachmentStoreOp)readd(address);address+=4;
        s->stencilLoadOp = (VkAttachmentLoadOp)readd(address);address+=4;
        s->stencilStoreOp = (VkAttachmentStoreOp)readd(address);address+=4;
        s->initialLayout = (VkImageLayout)readd(address);address+=4;
        s->finalLayout = (VkImageLayout)readd(address);address+=4;
    }
};

class MarshalVkAttachmentReference2 {
public:
    MarshalVkAttachmentReference2() {}
    VkAttachmentReference2 s;
    MarshalVkAttachmentReference2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkAttachmentReference2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->attachment = (uint32_t)readd(address);address+=4;
        s->layout = (VkImageLayout)readd(address);address+=4;
        s->aspectMask = (VkImageAspectFlags)readd(address);address+=4;
    }
};

class MarshalVkSubpassDescription2 {
public:
    MarshalVkSubpassDescription2() {}
    VkSubpassDescription2 s;
    MarshalVkSubpassDescription2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSubpassDescription2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkSubpassDescriptionFlags)readd(address);address+=4;
        s->pipelineBindPoint = (VkPipelineBindPoint)readd(address);address+=4;
        s->viewMask = (uint32_t)readd(address);address+=4;
        s->inputAttachmentCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pInputAttachments = NULL;
        } else {
            VkAttachmentReference2* pInputAttachments = new VkAttachmentReference2();
            MarshalVkAttachmentReference2::read(paramAddress, pInputAttachments);
            s->pInputAttachments = pInputAttachments;
        }
        s->colorAttachmentCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pColorAttachments = NULL;
        } else {
            VkAttachmentReference2* pColorAttachments = new VkAttachmentReference2();
            MarshalVkAttachmentReference2::read(paramAddress, pColorAttachments);
            s->pColorAttachments = pColorAttachments;
        }
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pResolveAttachments = NULL;
        } else {
            VkAttachmentReference2* pResolveAttachments = new VkAttachmentReference2();
            MarshalVkAttachmentReference2::read(paramAddress, pResolveAttachments);
            s->pResolveAttachments = pResolveAttachments;
        }
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pDepthStencilAttachment = NULL;
        } else {
            VkAttachmentReference2* pDepthStencilAttachment = new VkAttachmentReference2();
            MarshalVkAttachmentReference2::read(paramAddress, pDepthStencilAttachment);
            s->pDepthStencilAttachment = pDepthStencilAttachment;
        }
        s->preserveAttachmentCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pPreserveAttachments = NULL;
        } else {
            s->pPreserveAttachments = (uint32_t*)getPhysicalAddress(paramAddress, s->preserveAttachmentCount * sizeof(uint32_t));
        }
    }
};

class MarshalVkSubpassDependency2 {
public:
    MarshalVkSubpassDependency2() {}
    VkSubpassDependency2 s;
    MarshalVkSubpassDependency2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSubpassDependency2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->srcSubpass = (uint32_t)readd(address);address+=4;
        s->dstSubpass = (uint32_t)readd(address);address+=4;
        s->srcStageMask = (VkPipelineStageFlags)readd(address);address+=4;
        s->dstStageMask = (VkPipelineStageFlags)readd(address);address+=4;
        s->srcAccessMask = (VkAccessFlags)readd(address);address+=4;
        s->dstAccessMask = (VkAccessFlags)readd(address);address+=4;
        s->dependencyFlags = (VkDependencyFlags)readd(address);address+=4;
        s->viewOffset = (int32_t)readd(address);address+=4;
    }
};

class MarshalVkRenderPassCreateInfo2 {
public:
    MarshalVkRenderPassCreateInfo2() {}
    VkRenderPassCreateInfo2 s;
    MarshalVkRenderPassCreateInfo2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkRenderPassCreateInfo2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkRenderPassCreateFlags)readd(address);address+=4;
        s->attachmentCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pAttachments = NULL;
        } else {
            VkAttachmentDescription2* pAttachments = new VkAttachmentDescription2();
            MarshalVkAttachmentDescription2::read(paramAddress, pAttachments);
            s->pAttachments = pAttachments;
        }
        s->subpassCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pSubpasses = NULL;
        } else {
            VkSubpassDescription2* pSubpasses = new VkSubpassDescription2();
            MarshalVkSubpassDescription2::read(paramAddress, pSubpasses);
            s->pSubpasses = pSubpasses;
        }
        s->dependencyCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pDependencies = NULL;
        } else {
            VkSubpassDependency2* pDependencies = new VkSubpassDependency2();
            MarshalVkSubpassDependency2::read(paramAddress, pDependencies);
            s->pDependencies = pDependencies;
        }
        s->correlatedViewMaskCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pCorrelatedViewMasks = NULL;
        } else {
            s->pCorrelatedViewMasks = (uint32_t*)getPhysicalAddress(paramAddress, s->correlatedViewMaskCount * sizeof(uint32_t));
        }
    }
};

class MarshalVkSubpassBeginInfo {
public:
    MarshalVkSubpassBeginInfo() {}
    VkSubpassBeginInfo s;
    MarshalVkSubpassBeginInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSubpassBeginInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->contents = (VkSubpassContents)readd(address);address+=4;
    }
};

class MarshalVkSubpassEndInfo {
public:
    MarshalVkSubpassEndInfo() {}
    VkSubpassEndInfo s;
    MarshalVkSubpassEndInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSubpassEndInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
    }
};

class MarshalVkSemaphoreWaitInfo {
public:
    MarshalVkSemaphoreWaitInfo() {}
    VkSemaphoreWaitInfo s;
    MarshalVkSemaphoreWaitInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSemaphoreWaitInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->flags = (VkSemaphoreWaitFlags)readd(address);address+=4;
        s->semaphoreCount = (uint32_t)readd(address);address+=4;
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pSemaphores = NULL;
        } else {
            s->pSemaphores = (VkSemaphore*)getPhysicalAddress(paramAddress, s->semaphoreCount * sizeof(VkSemaphore));
        }
        paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pValues = NULL;
        } else {
            s->pValues = (uint64_t*)getPhysicalAddress(paramAddress, s->semaphoreCount * sizeof(uint64_t));
        }
    }
};

class MarshalVkSemaphoreSignalInfo {
public:
    MarshalVkSemaphoreSignalInfo() {}
    VkSemaphoreSignalInfo s;
    MarshalVkSemaphoreSignalInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkSemaphoreSignalInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->semaphore = (VkSemaphore)readq(address);address+=8;
        s->value = (uint64_t)readq(address);address+=8;
    }
};

class MarshalVkBufferDeviceAddressInfo {
public:
    MarshalVkBufferDeviceAddressInfo() {}
    VkBufferDeviceAddressInfo s;
    MarshalVkBufferDeviceAddressInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkBufferDeviceAddressInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->buffer = (VkBuffer)readq(address);address+=8;
    }
};

class MarshalVkDeviceMemoryOpaqueCaptureAddressInfo {
public:
    MarshalVkDeviceMemoryOpaqueCaptureAddressInfo() {}
    VkDeviceMemoryOpaqueCaptureAddressInfo s;
    MarshalVkDeviceMemoryOpaqueCaptureAddressInfo(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDeviceMemoryOpaqueCaptureAddressInfo* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->memory = (VkDeviceMemory)readq(address);address+=8;
    }
};

PFN_vkCreateInstance vkCreateInstance;
// return type: VkResult(4 bytes)
void vk_CreateInstance(CPU* cpu) {
    MarshalVkInstanceCreateInfo local_pCreateInfo(ARG1);
    VkInstanceCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG2) { klog("vkCreateInstance:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkInstance pInstance;
    EAX = vkCreateInstance(pCreateInfo, pAllocator, &pInstance);
}
PFN_vkDestroyInstance vkDestroyInstance;
void vk_DestroyInstance(CPU* cpu) {
    VkInstance instance = (VkInstance)getVulkanPtr(ARG1);
    static bool shown; if (!shown && ARG2) { klog("vkDestroyInstance:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyInstance(instance, pAllocator);
    freeVulkanPtr(ARG1);
}
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
// return type: VkResult(4 bytes)
void vk_EnumeratePhysicalDevices(CPU* cpu) {
    VkInstance instance = (VkInstance)getVulkanPtr(ARG1);
    uint32_t* pPhysicalDeviceCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkPhysicalDevice* pPhysicalDevices = NULL;
    if (ARG3) {
        pPhysicalDevices = new VkPhysicalDevice[*pPhysicalDeviceCount];
    }
    EAX = vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    if (ARG3) {
        for (U32 i=0;i<*pPhysicalDeviceCount;i++) {
            writed(ARG3 + i*4, createVulkanPtr((U64)pPhysicalDevices[i]));
        }
    }
}
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
void vk_GetPhysicalDeviceProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkPhysicalDeviceProperties pProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &pProperties.s);
    MarshalVkPhysicalDeviceProperties::write(ARG2, &pProperties.s);
}
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
void vk_GetPhysicalDeviceQueueFamilyProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    uint32_t* pQueueFamilyPropertyCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkQueueFamilyProperties* pQueueFamilyProperties = (VkQueueFamilyProperties*)getPhysicalAddress(ARG3, (U32)(pQueueFamilyPropertyCount ? *pQueueFamilyPropertyCount : 0));
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
void vk_GetPhysicalDeviceMemoryProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    VkPhysicalDeviceMemoryProperties* pMemoryProperties = (VkPhysicalDeviceMemoryProperties*)getPhysicalAddress(ARG2, 4);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
void vk_GetPhysicalDeviceFeatures(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    VkPhysicalDeviceFeatures* pFeatures = (VkPhysicalDeviceFeatures*)getPhysicalAddress(ARG2, 4);
    vkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
void vk_GetPhysicalDeviceFormatProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    VkFormat format = (VkFormat)ARG2;
    VkFormatProperties* pFormatProperties = (VkFormatProperties*)getPhysicalAddress(ARG3, 4);
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}
PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
// return type: VkResult(4 bytes)
void vk_GetPhysicalDeviceImageFormatProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    VkFormat format = (VkFormat)ARG2;
    VkImageType type = (VkImageType)ARG3;
    VkImageTiling tiling = (VkImageTiling)ARG4;
    VkImageUsageFlags usage = (VkImageUsageFlags)ARG5;
    VkImageCreateFlags flags = (VkImageCreateFlags)ARG6;
    VkImageFormatProperties* pImageFormatProperties = (VkImageFormatProperties*)getPhysicalAddress(ARG7, 4);
    EAX = vkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}
PFN_vkCreateDevice vkCreateDevice;
// return type: VkResult(4 bytes)
void vk_CreateDevice(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkDeviceCreateInfo local_pCreateInfo(ARG2);
    VkDeviceCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateDevice:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDevice pDevice;
    EAX = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, &pDevice);
}
PFN_vkDestroyDevice vkDestroyDevice;
void vk_DestroyDevice(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    static bool shown; if (!shown && ARG2) { klog("vkDestroyDevice:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyDevice(device, pAllocator);
    freeVulkanPtr(ARG1);
}
PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
// return type: VkResult(4 bytes)
void vk_EnumerateInstanceVersion(CPU* cpu) {
    uint32_t* pApiVersion = (uint32_t*)getPhysicalAddress(ARG1, 4);
    EAX = vkEnumerateInstanceVersion(pApiVersion);
}
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
// return type: VkResult(4 bytes)
void vk_EnumerateInstanceLayerProperties(CPU* cpu) {
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG1, 4);
    VkLayerProperties* pProperties = (VkLayerProperties*)getPhysicalAddress(ARG2, (U32)(pPropertyCount ? *pPropertyCount : 0));
    EAX = vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
// return type: VkResult(4 bytes)
void vk_EnumerateInstanceExtensionProperties(CPU* cpu) {
    char* pLayerName = (char*)getPhysicalAddress(ARG1, 4);
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkExtensionProperties* pProperties = (VkExtensionProperties*)getPhysicalAddress(ARG3, (U32)(pPropertyCount ? *pPropertyCount : 0));
    EAX = vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
// return type: VkResult(4 bytes)
void vk_EnumerateDeviceLayerProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkLayerProperties* pProperties = (VkLayerProperties*)getPhysicalAddress(ARG3, (U32)(pPropertyCount ? *pPropertyCount : 0));
    EAX = vkEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
// return type: VkResult(4 bytes)
void vk_EnumerateDeviceExtensionProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    char* pLayerName = (char*)getPhysicalAddress(ARG2, 4);
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG3, 4);
    VkExtensionProperties* pProperties = (VkExtensionProperties*)getPhysicalAddress(ARG4, (U32)(pPropertyCount ? *pPropertyCount : 0));
    EAX = vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}
PFN_vkGetDeviceQueue vkGetDeviceQueue;
void vk_GetDeviceQueue(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    uint32_t queueFamilyIndex = (uint32_t)ARG2;
    uint32_t queueIndex = (uint32_t)ARG3;
    VkQueue pQueue;
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &pQueue);
}
PFN_vkQueueSubmit vkQueueSubmit;
// return type: VkResult(4 bytes)
void vk_QueueSubmit(CPU* cpu) {
    VkQueue queue = (VkQueue)getVulkanPtr(ARG1);
    uint32_t submitCount = (uint32_t)ARG2;
    VkSubmitInfo* pSubmits = NULL;
    if (ARG3) {
        pSubmits = new VkSubmitInfo[submitCount];
    }
    VkFence fence = *(VkFence*)getPhysicalAddress(ARG4, 8);
;
    EAX = vkQueueSubmit(queue, submitCount, pSubmits, fence);
}
PFN_vkQueueWaitIdle vkQueueWaitIdle;
// return type: VkResult(4 bytes)
void vk_QueueWaitIdle(CPU* cpu) {
    VkQueue queue = (VkQueue)getVulkanPtr(ARG1);
    EAX = vkQueueWaitIdle(queue);
}
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
// return type: VkResult(4 bytes)
void vk_DeviceWaitIdle(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    EAX = vkDeviceWaitIdle(device);
}
PFN_vkAllocateMemory vkAllocateMemory;
// return type: VkResult(4 bytes)
void vk_AllocateMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkMemoryAllocateInfo local_pAllocateInfo(ARG2);
    VkMemoryAllocateInfo* pAllocateInfo = &local_pAllocateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkAllocateMemory:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDeviceMemory* pMemory = (VkDeviceMemory*)getPhysicalAddress(ARG4, 4);
    EAX = vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}
PFN_vkFreeMemory vkFreeMemory;
void vk_FreeMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkFreeMemory:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkFreeMemory(device, memory, pAllocator);
}
PFN_vkMapMemory vkMapMemory;
// return type: VkResult(4 bytes)
void vk_MapMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize size = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    VkMemoryMapFlags flags = (VkMemoryMapFlags)ARG5;
    void **ppData = NULL;
    kpanic("vkMapMemory not implemented");
    EAX = vkMapMemory(device, memory, offset, size, flags, ppData);
}
PFN_vkUnmapMemory vkUnmapMemory;
void vk_UnmapMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG2, 8);
;
    vkUnmapMemory(device, memory);
}
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
// return type: VkResult(4 bytes)
void vk_FlushMappedMemoryRanges(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    uint32_t memoryRangeCount = (uint32_t)ARG2;
    VkMappedMemoryRange* pMemoryRanges = NULL;
    if (ARG3) {
        pMemoryRanges = new VkMappedMemoryRange[memoryRangeCount];
    }
    EAX = vkFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
// return type: VkResult(4 bytes)
void vk_InvalidateMappedMemoryRanges(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    uint32_t memoryRangeCount = (uint32_t)ARG2;
    VkMappedMemoryRange* pMemoryRanges = NULL;
    if (ARG3) {
        pMemoryRanges = new VkMappedMemoryRange[memoryRangeCount];
    }
    EAX = vkInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
void vk_GetDeviceMemoryCommitment(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize* pCommittedMemoryInBytes = (VkDeviceSize*)getPhysicalAddress(ARG3, 4);
    vkGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
void vk_GetBufferMemoryRequirements(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkMemoryRequirements* pMemoryRequirements = (VkMemoryRequirements*)getPhysicalAddress(ARG3, 4);
    vkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}
PFN_vkBindBufferMemory vkBindBufferMemory;
// return type: VkResult(4 bytes)
void vk_BindBufferMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize memoryOffset = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    EAX = vkBindBufferMemory(device, buffer, memory, memoryOffset);
}
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
void vk_GetImageMemoryRequirements(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkMemoryRequirements* pMemoryRequirements = (VkMemoryRequirements*)getPhysicalAddress(ARG3, 4);
    vkGetImageMemoryRequirements(device, image, pMemoryRequirements);
}
PFN_vkBindImageMemory vkBindImageMemory;
// return type: VkResult(4 bytes)
void vk_BindImageMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize memoryOffset = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    EAX = vkBindImageMemory(device, image, memory, memoryOffset);
}
PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
void vk_GetImageSparseMemoryRequirements(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    uint32_t* pSparseMemoryRequirementCount = (uint32_t*)getPhysicalAddress(ARG3, 4);
    VkSparseImageMemoryRequirements* pSparseMemoryRequirements = (VkSparseImageMemoryRequirements*)getPhysicalAddress(ARG4, (U32)(pSparseMemoryRequirementCount ? *pSparseMemoryRequirementCount : 0));
    vkGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
void vk_GetPhysicalDeviceSparseImageFormatProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    VkFormat format = (VkFormat)ARG2;
    VkImageType type = (VkImageType)ARG3;
    VkSampleCountFlagBits samples = (VkSampleCountFlagBits)ARG4;
    VkImageUsageFlags usage = (VkImageUsageFlags)ARG5;
    VkImageTiling tiling = (VkImageTiling)ARG6;
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG7, 4);
    VkSparseImageFormatProperties* pProperties = (VkSparseImageFormatProperties*)getPhysicalAddress(ARG8, (U32)(pPropertyCount ? *pPropertyCount : 0));
    vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}
PFN_vkQueueBindSparse vkQueueBindSparse;
// return type: VkResult(4 bytes)
void vk_QueueBindSparse(CPU* cpu) {
    VkQueue queue = (VkQueue)getVulkanPtr(ARG1);
    uint32_t bindInfoCount = (uint32_t)ARG2;
    VkBindSparseInfo* pBindInfo = NULL;
    if (ARG3) {
        pBindInfo = new VkBindSparseInfo[bindInfoCount];
    }
    VkFence fence = *(VkFence*)getPhysicalAddress(ARG4, 8);
;
    EAX = vkQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}
PFN_vkCreateFence vkCreateFence;
// return type: VkResult(4 bytes)
void vk_CreateFence(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkFenceCreateInfo local_pCreateInfo(ARG2);
    VkFenceCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateFence:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkFence* pFence = (VkFence*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateFence(device, pCreateInfo, pAllocator, pFence);
}
PFN_vkDestroyFence vkDestroyFence;
void vk_DestroyFence(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkFence fence = *(VkFence*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyFence:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyFence(device, fence, pAllocator);
}
PFN_vkResetFences vkResetFences;
// return type: VkResult(4 bytes)
void vk_ResetFences(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    uint32_t fenceCount = (uint32_t)ARG2;
    VkFence* pFences = (VkFence*)getPhysicalAddress(ARG3, (U32)fenceCount * 4);
    EAX = vkResetFences(device, fenceCount, pFences);
}
PFN_vkGetFenceStatus vkGetFenceStatus;
// return type: VkResult(4 bytes)
void vk_GetFenceStatus(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkFence fence = *(VkFence*)getPhysicalAddress(ARG2, 8);
;
    EAX = vkGetFenceStatus(device, fence);
}
PFN_vkWaitForFences vkWaitForFences;
// return type: VkResult(4 bytes)
void vk_WaitForFences(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    uint32_t fenceCount = (uint32_t)ARG2;
    VkFence* pFences = (VkFence*)getPhysicalAddress(ARG3, (U32)fenceCount * 4);
    VkBool32 waitAll = (VkBool32)ARG4;
    uint64_t timeout = *(uint64_t*)getPhysicalAddress(ARG5, 8);
;
    EAX = vkWaitForFences(device, fenceCount, pFences, waitAll, timeout);
}
PFN_vkCreateSemaphore vkCreateSemaphore;
// return type: VkResult(4 bytes)
void vk_CreateSemaphore(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkSemaphoreCreateInfo local_pCreateInfo(ARG2);
    VkSemaphoreCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateSemaphore:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkSemaphore* pSemaphore = (VkSemaphore*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}
PFN_vkDestroySemaphore vkDestroySemaphore;
void vk_DestroySemaphore(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkSemaphore semaphore = *(VkSemaphore*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroySemaphore:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroySemaphore(device, semaphore, pAllocator);
}
PFN_vkCreateEvent vkCreateEvent;
// return type: VkResult(4 bytes)
void vk_CreateEvent(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkEventCreateInfo local_pCreateInfo(ARG2);
    VkEventCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateEvent:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkEvent* pEvent = (VkEvent*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateEvent(device, pCreateInfo, pAllocator, pEvent);
}
PFN_vkDestroyEvent vkDestroyEvent;
void vk_DestroyEvent(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyEvent:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyEvent(device, event, pAllocator);
}
PFN_vkGetEventStatus vkGetEventStatus;
// return type: VkResult(4 bytes)
void vk_GetEventStatus(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    EAX = vkGetEventStatus(device, event);
}
PFN_vkSetEvent vkSetEvent;
// return type: VkResult(4 bytes)
void vk_SetEvent(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    EAX = vkSetEvent(device, event);
}
PFN_vkResetEvent vkResetEvent;
// return type: VkResult(4 bytes)
void vk_ResetEvent(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    EAX = vkResetEvent(device, event);
}
PFN_vkCreateQueryPool vkCreateQueryPool;
// return type: VkResult(4 bytes)
void vk_CreateQueryPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkQueryPoolCreateInfo local_pCreateInfo(ARG2);
    VkQueryPoolCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateQueryPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkQueryPool* pQueryPool = (VkQueryPool*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}
PFN_vkDestroyQueryPool vkDestroyQueryPool;
void vk_DestroyQueryPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyQueryPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyQueryPool(device, queryPool, pAllocator);
}
PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
// return type: VkResult(4 bytes)
void vk_GetQueryPoolResults(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t firstQuery = (uint32_t)ARG3;
    uint32_t queryCount = (uint32_t)ARG4;
    size_t dataSize = (size_t)ARG5;
    void* pData = (void*)getPhysicalAddress(ARG6, (U32)dataSize * 4);
    VkDeviceSize stride = *(VkDeviceSize*)getPhysicalAddress(ARG7, 8);
;
    VkQueryResultFlags flags = (VkQueryResultFlags)ARG8;
    EAX = vkGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}
PFN_vkResetQueryPool vkResetQueryPool;
void vk_ResetQueryPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t firstQuery = (uint32_t)ARG3;
    uint32_t queryCount = (uint32_t)ARG4;
    vkResetQueryPool(device, queryPool, firstQuery, queryCount);
}
PFN_vkCreateBuffer vkCreateBuffer;
// return type: VkResult(4 bytes)
void vk_CreateBuffer(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkBufferCreateInfo local_pCreateInfo(ARG2);
    VkBufferCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateBuffer:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkBuffer* pBuffer = (VkBuffer*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}
PFN_vkDestroyBuffer vkDestroyBuffer;
void vk_DestroyBuffer(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyBuffer:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyBuffer(device, buffer, pAllocator);
}
PFN_vkCreateBufferView vkCreateBufferView;
// return type: VkResult(4 bytes)
void vk_CreateBufferView(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkBufferViewCreateInfo local_pCreateInfo(ARG2);
    VkBufferViewCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateBufferView:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkBufferView* pView = (VkBufferView*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateBufferView(device, pCreateInfo, pAllocator, pView);
}
PFN_vkDestroyBufferView vkDestroyBufferView;
void vk_DestroyBufferView(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkBufferView bufferView = *(VkBufferView*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyBufferView:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyBufferView(device, bufferView, pAllocator);
}
PFN_vkCreateImage vkCreateImage;
// return type: VkResult(4 bytes)
void vk_CreateImage(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkImageCreateInfo local_pCreateInfo(ARG2);
    VkImageCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateImage:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkImage* pImage = (VkImage*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateImage(device, pCreateInfo, pAllocator, pImage);
}
PFN_vkDestroyImage vkDestroyImage;
void vk_DestroyImage(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyImage:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyImage(device, image, pAllocator);
}
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
void vk_GetImageSubresourceLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageSubresource* pSubresource = (VkImageSubresource*)getPhysicalAddress(ARG3, 4);
    VkSubresourceLayout* pLayout = (VkSubresourceLayout*)getPhysicalAddress(ARG4, 4);
    vkGetImageSubresourceLayout(device, image, pSubresource, pLayout);
}
PFN_vkCreateImageView vkCreateImageView;
// return type: VkResult(4 bytes)
void vk_CreateImageView(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkImageViewCreateInfo local_pCreateInfo(ARG2);
    VkImageViewCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateImageView:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkImageView* pView = (VkImageView*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateImageView(device, pCreateInfo, pAllocator, pView);
}
PFN_vkDestroyImageView vkDestroyImageView;
void vk_DestroyImageView(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkImageView imageView = *(VkImageView*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyImageView:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyImageView(device, imageView, pAllocator);
}
PFN_vkCreateShaderModule vkCreateShaderModule;
// return type: VkResult(4 bytes)
void vk_CreateShaderModule(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkShaderModuleCreateInfo local_pCreateInfo(ARG2);
    VkShaderModuleCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateShaderModule:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkShaderModule* pShaderModule = (VkShaderModule*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}
PFN_vkDestroyShaderModule vkDestroyShaderModule;
void vk_DestroyShaderModule(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkShaderModule shaderModule = *(VkShaderModule*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyShaderModule:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyShaderModule(device, shaderModule, pAllocator);
}
PFN_vkCreatePipelineCache vkCreatePipelineCache;
// return type: VkResult(4 bytes)
void vk_CreatePipelineCache(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkPipelineCacheCreateInfo local_pCreateInfo(ARG2);
    VkPipelineCacheCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreatePipelineCache:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkPipelineCache* pPipelineCache = (VkPipelineCache*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
void vk_DestroyPipelineCache(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkPipelineCache pipelineCache = *(VkPipelineCache*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyPipelineCache:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyPipelineCache(device, pipelineCache, pAllocator);
}
PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
// return type: VkResult(4 bytes)
void vk_GetPipelineCacheData(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkPipelineCache pipelineCache = *(VkPipelineCache*)getPhysicalAddress(ARG2, 8);
;
    size_t* pDataSize = (size_t*)getPhysicalAddress(ARG3, 4);
    void* pData = (void*)getPhysicalAddress(ARG4, (U32)(pDataSize ? *pDataSize : 0));
    EAX = vkGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}
PFN_vkMergePipelineCaches vkMergePipelineCaches;
// return type: VkResult(4 bytes)
void vk_MergePipelineCaches(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkPipelineCache dstCache = *(VkPipelineCache*)getPhysicalAddress(ARG2, 8);
;
    uint32_t srcCacheCount = (uint32_t)ARG3;
    VkPipelineCache* pSrcCaches = (VkPipelineCache*)getPhysicalAddress(ARG4, (U32)srcCacheCount * 4);
    EAX = vkMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
// return type: VkResult(4 bytes)
void vk_CreateGraphicsPipelines(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkPipelineCache pipelineCache = *(VkPipelineCache*)getPhysicalAddress(ARG2, 8);
;
    uint32_t createInfoCount = (uint32_t)ARG3;
    VkGraphicsPipelineCreateInfo* pCreateInfos = NULL;
    if (ARG4) {
        pCreateInfos = new VkGraphicsPipelineCreateInfo[createInfoCount];
    }
    static bool shown; if (!shown && ARG5) { klog("vkCreateGraphicsPipelines:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkPipeline* pPipelines = (VkPipeline*)getPhysicalAddress(ARG6, (U32)createInfoCount * 4);
    EAX = vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
PFN_vkCreateComputePipelines vkCreateComputePipelines;
// return type: VkResult(4 bytes)
void vk_CreateComputePipelines(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkPipelineCache pipelineCache = *(VkPipelineCache*)getPhysicalAddress(ARG2, 8);
;
    uint32_t createInfoCount = (uint32_t)ARG3;
    VkComputePipelineCreateInfo* pCreateInfos = NULL;
    if (ARG4) {
        pCreateInfos = new VkComputePipelineCreateInfo[createInfoCount];
    }
    static bool shown; if (!shown && ARG5) { klog("vkCreateComputePipelines:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkPipeline* pPipelines = (VkPipeline*)getPhysicalAddress(ARG6, (U32)createInfoCount * 4);
    EAX = vkCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
PFN_vkDestroyPipeline vkDestroyPipeline;
void vk_DestroyPipeline(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkPipeline pipeline = *(VkPipeline*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyPipeline:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyPipeline(device, pipeline, pAllocator);
}
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
// return type: VkResult(4 bytes)
void vk_CreatePipelineLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkPipelineLayoutCreateInfo local_pCreateInfo(ARG2);
    VkPipelineLayoutCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreatePipelineLayout:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkPipelineLayout* pPipelineLayout = (VkPipelineLayout*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
void vk_DestroyPipelineLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkPipelineLayout pipelineLayout = *(VkPipelineLayout*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyPipelineLayout:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}
PFN_vkCreateSampler vkCreateSampler;
// return type: VkResult(4 bytes)
void vk_CreateSampler(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkSamplerCreateInfo local_pCreateInfo(ARG2);
    VkSamplerCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateSampler:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkSampler* pSampler = (VkSampler*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateSampler(device, pCreateInfo, pAllocator, pSampler);
}
PFN_vkDestroySampler vkDestroySampler;
void vk_DestroySampler(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkSampler sampler = *(VkSampler*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroySampler:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroySampler(device, sampler, pAllocator);
}
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
// return type: VkResult(4 bytes)
void vk_CreateDescriptorSetLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkDescriptorSetLayoutCreateInfo local_pCreateInfo(ARG2);
    VkDescriptorSetLayoutCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateDescriptorSetLayout:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDescriptorSetLayout* pSetLayout = (VkDescriptorSetLayout*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
void vk_DestroyDescriptorSetLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDescriptorSetLayout descriptorSetLayout = *(VkDescriptorSetLayout*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyDescriptorSetLayout:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
// return type: VkResult(4 bytes)
void vk_CreateDescriptorPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkDescriptorPoolCreateInfo local_pCreateInfo(ARG2);
    VkDescriptorPoolCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateDescriptorPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDescriptorPool* pDescriptorPool = (VkDescriptorPool*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
void vk_DestroyDescriptorPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDescriptorPool descriptorPool = *(VkDescriptorPool*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyDescriptorPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyDescriptorPool(device, descriptorPool, pAllocator);
}
PFN_vkResetDescriptorPool vkResetDescriptorPool;
// return type: VkResult(4 bytes)
void vk_ResetDescriptorPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDescriptorPool descriptorPool = *(VkDescriptorPool*)getPhysicalAddress(ARG2, 8);
;
    VkDescriptorPoolResetFlags flags = (VkDescriptorPoolResetFlags)ARG3;
    EAX = vkResetDescriptorPool(device, descriptorPool, flags);
}
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
// return type: VkResult(4 bytes)
void vk_AllocateDescriptorSets(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkDescriptorSetAllocateInfo local_pAllocateInfo(ARG2);
    VkDescriptorSetAllocateInfo* pAllocateInfo = &local_pAllocateInfo.s;
    VkDescriptorSet* pDescriptorSets = (VkDescriptorSet*)getPhysicalAddress(ARG3, (U32)pAllocateInfo->descriptorSetCount * 4);
    EAX = vkAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}
PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
// return type: VkResult(4 bytes)
void vk_FreeDescriptorSets(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDescriptorPool descriptorPool = *(VkDescriptorPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t descriptorSetCount = (uint32_t)ARG3;
    VkDescriptorSet* pDescriptorSets = (VkDescriptorSet*)getPhysicalAddress(ARG4, (U32)descriptorSetCount * 4);
    EAX = vkFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
void vk_UpdateDescriptorSets(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    uint32_t descriptorWriteCount = (uint32_t)ARG2;
    VkWriteDescriptorSet* pDescriptorWrites = NULL;
    if (ARG3) {
        pDescriptorWrites = new VkWriteDescriptorSet[descriptorWriteCount];
    }
    uint32_t descriptorCopyCount = (uint32_t)ARG4;
    VkCopyDescriptorSet* pDescriptorCopies = NULL;
    if (ARG5) {
        pDescriptorCopies = new VkCopyDescriptorSet[descriptorCopyCount];
    }
    vkUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}
PFN_vkCreateFramebuffer vkCreateFramebuffer;
// return type: VkResult(4 bytes)
void vk_CreateFramebuffer(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkFramebufferCreateInfo local_pCreateInfo(ARG2);
    VkFramebufferCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateFramebuffer:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkFramebuffer* pFramebuffer = (VkFramebuffer*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
void vk_DestroyFramebuffer(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkFramebuffer framebuffer = *(VkFramebuffer*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyFramebuffer:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyFramebuffer(device, framebuffer, pAllocator);
}
PFN_vkCreateRenderPass vkCreateRenderPass;
// return type: VkResult(4 bytes)
void vk_CreateRenderPass(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkRenderPassCreateInfo local_pCreateInfo(ARG2);
    VkRenderPassCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateRenderPass:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkRenderPass* pRenderPass = (VkRenderPass*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}
PFN_vkDestroyRenderPass vkDestroyRenderPass;
void vk_DestroyRenderPass(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkRenderPass renderPass = *(VkRenderPass*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyRenderPass:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyRenderPass(device, renderPass, pAllocator);
}
PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
void vk_GetRenderAreaGranularity(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkRenderPass renderPass = *(VkRenderPass*)getPhysicalAddress(ARG2, 8);
;
    VkExtent2D* pGranularity = (VkExtent2D*)getPhysicalAddress(ARG3, 4);
    vkGetRenderAreaGranularity(device, renderPass, pGranularity);
}
PFN_vkCreateCommandPool vkCreateCommandPool;
// return type: VkResult(4 bytes)
void vk_CreateCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkCommandPoolCreateInfo local_pCreateInfo(ARG2);
    VkCommandPoolCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateCommandPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkCommandPool* pCommandPool = (VkCommandPool*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}
PFN_vkDestroyCommandPool vkDestroyCommandPool;
void vk_DestroyCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkCommandPool commandPool = *(VkCommandPool*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyCommandPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyCommandPool(device, commandPool, pAllocator);
}
PFN_vkResetCommandPool vkResetCommandPool;
// return type: VkResult(4 bytes)
void vk_ResetCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkCommandPool commandPool = *(VkCommandPool*)getPhysicalAddress(ARG2, 8);
;
    VkCommandPoolResetFlags flags = (VkCommandPoolResetFlags)ARG3;
    EAX = vkResetCommandPool(device, commandPool, flags);
}
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
// return type: VkResult(4 bytes)
void vk_AllocateCommandBuffers(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkCommandBufferAllocateInfo local_pAllocateInfo(ARG2);
    VkCommandBufferAllocateInfo* pAllocateInfo = &local_pAllocateInfo.s;
    VkCommandBuffer* pCommandBuffers = NULL;
    if (ARG3) {
        pCommandBuffers = new VkCommandBuffer[pAllocateInfo->commandBufferCount];
    }
    EAX = vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    if (ARG3) {
        for (U32 i=0;i<pAllocateInfo->commandBufferCount;i++) {
            writed(ARG3 + i*4, createVulkanPtr((U64)pCommandBuffers[i]));
        }
    }
}
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
void vk_FreeCommandBuffers(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkCommandPool commandPool = *(VkCommandPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t commandBufferCount = (uint32_t)ARG3;
    VkCommandBuffer* pCommandBuffers = new VkCommandBuffer[commandBufferCount];
    for (U32 i=0;i<commandBufferCount;i++) {
        pCommandBuffers[i] = (VkCommandBuffer)getVulkanPtr(readd(ARG4 + i*4));
    }
    vkFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
// return type: VkResult(4 bytes)
void vk_BeginCommandBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    MarshalVkCommandBufferBeginInfo local_pBeginInfo(ARG2);
    VkCommandBufferBeginInfo* pBeginInfo = &local_pBeginInfo.s;
    EAX = vkBeginCommandBuffer(commandBuffer, pBeginInfo);
}
PFN_vkEndCommandBuffer vkEndCommandBuffer;
// return type: VkResult(4 bytes)
void vk_EndCommandBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    EAX = vkEndCommandBuffer(commandBuffer);
}
PFN_vkResetCommandBuffer vkResetCommandBuffer;
// return type: VkResult(4 bytes)
void vk_ResetCommandBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkCommandBufferResetFlags flags = (VkCommandBufferResetFlags)ARG2;
    EAX = vkResetCommandBuffer(commandBuffer, flags);
}
PFN_vkCmdBindPipeline vkCmdBindPipeline;
void vk_CmdBindPipeline(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkPipelineBindPoint pipelineBindPoint = (VkPipelineBindPoint)ARG2;
    VkPipeline pipeline = *(VkPipeline*)getPhysicalAddress(ARG3, 8);
;
    vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}
PFN_vkCmdSetViewport vkCmdSetViewport;
void vk_CmdSetViewport(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t firstViewport = (uint32_t)ARG2;
    uint32_t viewportCount = (uint32_t)ARG3;
    VkViewport* pViewports = (VkViewport*)getPhysicalAddress(ARG4, (U32)viewportCount * 4);
    vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}
PFN_vkCmdSetScissor vkCmdSetScissor;
void vk_CmdSetScissor(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t firstScissor = (uint32_t)ARG2;
    uint32_t scissorCount = (uint32_t)ARG3;
    VkRect2D* pScissors = (VkRect2D*)getPhysicalAddress(ARG4, (U32)scissorCount * 4);
    vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
void vk_CmdSetLineWidth(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    float lineWidth = (float)ARG2;
    vkCmdSetLineWidth(commandBuffer, lineWidth);
}
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
void vk_CmdSetDepthBias(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    float depthBiasConstantFactor = (float)ARG2;
    float depthBiasClamp = (float)ARG3;
    float depthBiasSlopeFactor = (float)ARG4;
    vkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
void vk_CmdSetBlendConstants(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    float* blendConstants = (float*)getPhysicalAddress(ARG2, 16);
    vkCmdSetBlendConstants(commandBuffer, blendConstants);
}
PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
void vk_CmdSetDepthBounds(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    float minDepthBounds = (float)ARG2;
    float maxDepthBounds = (float)ARG3;
    vkCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}
PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
void vk_CmdSetStencilCompareMask(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkStencilFaceFlags faceMask = (VkStencilFaceFlags)ARG2;
    uint32_t compareMask = (uint32_t)ARG3;
    vkCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}
PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
void vk_CmdSetStencilWriteMask(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkStencilFaceFlags faceMask = (VkStencilFaceFlags)ARG2;
    uint32_t writeMask = (uint32_t)ARG3;
    vkCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}
PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
void vk_CmdSetStencilReference(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkStencilFaceFlags faceMask = (VkStencilFaceFlags)ARG2;
    uint32_t reference = (uint32_t)ARG3;
    vkCmdSetStencilReference(commandBuffer, faceMask, reference);
}
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
void vk_CmdBindDescriptorSets(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkPipelineBindPoint pipelineBindPoint = (VkPipelineBindPoint)ARG2;
    VkPipelineLayout layout = *(VkPipelineLayout*)getPhysicalAddress(ARG3, 8);
;
    uint32_t firstSet = (uint32_t)ARG4;
    uint32_t descriptorSetCount = (uint32_t)ARG5;
    VkDescriptorSet* pDescriptorSets = (VkDescriptorSet*)getPhysicalAddress(ARG6, (U32)descriptorSetCount * 4);
    uint32_t dynamicOffsetCount = (uint32_t)ARG7;
    uint32_t* pDynamicOffsets = (uint32_t*)getPhysicalAddress(ARG8, (U32)dynamicOffsetCount * 4);
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
void vk_CmdBindIndexBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkIndexType indexType = (VkIndexType)ARG4;
    vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
void vk_CmdBindVertexBuffers(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t firstBinding = (uint32_t)ARG2;
    uint32_t bindingCount = (uint32_t)ARG3;
    VkBuffer* pBuffers = (VkBuffer*)getPhysicalAddress(ARG4, (U32)bindingCount * 4);
    VkDeviceSize* pOffsets = (VkDeviceSize*)getPhysicalAddress(ARG5, (U32)bindingCount * 4);
    vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}
PFN_vkCmdDraw vkCmdDraw;
void vk_CmdDraw(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t vertexCount = (uint32_t)ARG2;
    uint32_t instanceCount = (uint32_t)ARG3;
    uint32_t firstVertex = (uint32_t)ARG4;
    uint32_t firstInstance = (uint32_t)ARG5;
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
void vk_CmdDrawIndexed(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t indexCount = (uint32_t)ARG2;
    uint32_t instanceCount = (uint32_t)ARG3;
    uint32_t firstIndex = (uint32_t)ARG4;
    int32_t vertexOffset = (int32_t)ARG5;
    uint32_t firstInstance = (uint32_t)ARG6;
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
void vk_CmdDrawIndirect(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    uint32_t drawCount = (uint32_t)ARG4;
    uint32_t stride = (uint32_t)ARG5;
    vkCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
void vk_CmdDrawIndexedIndirect(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    uint32_t drawCount = (uint32_t)ARG4;
    uint32_t stride = (uint32_t)ARG5;
    vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}
PFN_vkCmdDispatch vkCmdDispatch;
void vk_CmdDispatch(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t groupCountX = (uint32_t)ARG2;
    uint32_t groupCountY = (uint32_t)ARG3;
    uint32_t groupCountZ = (uint32_t)ARG4;
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}
PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
void vk_CmdDispatchIndirect(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    vkCmdDispatchIndirect(commandBuffer, buffer, offset);
}
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
void vk_CmdCopyBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer srcBuffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkBuffer dstBuffer = *(VkBuffer*)getPhysicalAddress(ARG3, 8);
;
    uint32_t regionCount = (uint32_t)ARG4;
    VkBufferCopy* pRegions = (VkBufferCopy*)getPhysicalAddress(ARG5, (U32)regionCount * 4);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}
PFN_vkCmdCopyImage vkCmdCopyImage;
void vk_CmdCopyImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkImage srcImage = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG3;
    VkImage dstImage = *(VkImage*)getPhysicalAddress(ARG4, 8);
;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG5;
    uint32_t regionCount = (uint32_t)ARG6;
    VkImageCopy* pRegions = (VkImageCopy*)getPhysicalAddress(ARG7, (U32)regionCount * 4);
    vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
PFN_vkCmdBlitImage vkCmdBlitImage;
void vk_CmdBlitImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkImage srcImage = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG3;
    VkImage dstImage = *(VkImage*)getPhysicalAddress(ARG4, 8);
;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG5;
    uint32_t regionCount = (uint32_t)ARG6;
    VkImageBlit* pRegions = (VkImageBlit*)getPhysicalAddress(ARG7, (U32)regionCount * 4);
    VkFilter filter = (VkFilter)ARG8;
    vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
void vk_CmdCopyBufferToImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer srcBuffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkImage dstImage = *(VkImage*)getPhysicalAddress(ARG3, 8);
;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG4;
    uint32_t regionCount = (uint32_t)ARG5;
    VkBufferImageCopy* pRegions = (VkBufferImageCopy*)getPhysicalAddress(ARG6, (U32)regionCount * 4);
    vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}
PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
void vk_CmdCopyImageToBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkImage srcImage = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG3;
    VkBuffer dstBuffer = *(VkBuffer*)getPhysicalAddress(ARG4, 8);
;
    uint32_t regionCount = (uint32_t)ARG5;
    VkBufferImageCopy* pRegions = (VkBufferImageCopy*)getPhysicalAddress(ARG6, (U32)regionCount * 4);
    vkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}
PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
void vk_CmdUpdateBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer dstBuffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize dstOffset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize dataSize = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    void* pData = (void*)getPhysicalAddress(ARG5, (U32)dataSize * 8);
    vkCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}
PFN_vkCmdFillBuffer vkCmdFillBuffer;
void vk_CmdFillBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer dstBuffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize dstOffset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize size = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    uint32_t data = (uint32_t)ARG5;
    vkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}
PFN_vkCmdClearColorImage vkCmdClearColorImage;
void vk_CmdClearColorImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout imageLayout = (VkImageLayout)ARG3;
    VkClearColorValue* pColor = (VkClearColorValue*)getPhysicalAddress(ARG4, 4);
    uint32_t rangeCount = (uint32_t)ARG5;
    VkImageSubresourceRange* pRanges = (VkImageSubresourceRange*)getPhysicalAddress(ARG6, (U32)rangeCount * 4);
    vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}
PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
void vk_CmdClearDepthStencilImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout imageLayout = (VkImageLayout)ARG3;
    VkClearDepthStencilValue* pDepthStencil = (VkClearDepthStencilValue*)getPhysicalAddress(ARG4, 4);
    uint32_t rangeCount = (uint32_t)ARG5;
    VkImageSubresourceRange* pRanges = (VkImageSubresourceRange*)getPhysicalAddress(ARG6, (U32)rangeCount * 4);
    vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}
PFN_vkCmdClearAttachments vkCmdClearAttachments;
void vk_CmdClearAttachments(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t attachmentCount = (uint32_t)ARG2;
    VkClearAttachment* pAttachments = (VkClearAttachment*)getPhysicalAddress(ARG3, (U32)attachmentCount * 4);
    uint32_t rectCount = (uint32_t)ARG4;
    VkClearRect* pRects = (VkClearRect*)getPhysicalAddress(ARG5, (U32)rectCount * 4);
    vkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}
PFN_vkCmdResolveImage vkCmdResolveImage;
void vk_CmdResolveImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkImage srcImage = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG3;
    VkImage dstImage = *(VkImage*)getPhysicalAddress(ARG4, 8);
;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG5;
    uint32_t regionCount = (uint32_t)ARG6;
    VkImageResolve* pRegions = (VkImageResolve*)getPhysicalAddress(ARG7, (U32)regionCount * 4);
    vkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
PFN_vkCmdSetEvent vkCmdSetEvent;
void vk_CmdSetEvent(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    VkPipelineStageFlags stageMask = (VkPipelineStageFlags)ARG3;
    vkCmdSetEvent(commandBuffer, event, stageMask);
}
PFN_vkCmdResetEvent vkCmdResetEvent;
void vk_CmdResetEvent(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    VkPipelineStageFlags stageMask = (VkPipelineStageFlags)ARG3;
    vkCmdResetEvent(commandBuffer, event, stageMask);
}
PFN_vkCmdWaitEvents vkCmdWaitEvents;
void vk_CmdWaitEvents(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t eventCount = (uint32_t)ARG2;
    VkEvent* pEvents = (VkEvent*)getPhysicalAddress(ARG3, (U32)eventCount * 4);
    VkPipelineStageFlags srcStageMask = (VkPipelineStageFlags)ARG4;
    VkPipelineStageFlags dstStageMask = (VkPipelineStageFlags)ARG5;
    uint32_t memoryBarrierCount = (uint32_t)ARG6;
    VkMemoryBarrier* pMemoryBarriers = NULL;
    if (ARG7) {
        pMemoryBarriers = new VkMemoryBarrier[memoryBarrierCount];
    }
    uint32_t bufferMemoryBarrierCount = (uint32_t)ARG8;
    VkBufferMemoryBarrier* pBufferMemoryBarriers = NULL;
    if (ARG9) {
        pBufferMemoryBarriers = new VkBufferMemoryBarrier[bufferMemoryBarrierCount];
    }
    uint32_t imageMemoryBarrierCount = (uint32_t)ARG10;
    VkImageMemoryBarrier* pImageMemoryBarriers = NULL;
    if (ARG11) {
        pImageMemoryBarriers = new VkImageMemoryBarrier[imageMemoryBarrierCount];
    }
    vkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
void vk_CmdPipelineBarrier(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkPipelineStageFlags srcStageMask = (VkPipelineStageFlags)ARG2;
    VkPipelineStageFlags dstStageMask = (VkPipelineStageFlags)ARG3;
    VkDependencyFlags dependencyFlags = (VkDependencyFlags)ARG4;
    uint32_t memoryBarrierCount = (uint32_t)ARG5;
    VkMemoryBarrier* pMemoryBarriers = NULL;
    if (ARG6) {
        pMemoryBarriers = new VkMemoryBarrier[memoryBarrierCount];
    }
    uint32_t bufferMemoryBarrierCount = (uint32_t)ARG7;
    VkBufferMemoryBarrier* pBufferMemoryBarriers = NULL;
    if (ARG8) {
        pBufferMemoryBarriers = new VkBufferMemoryBarrier[bufferMemoryBarrierCount];
    }
    uint32_t imageMemoryBarrierCount = (uint32_t)ARG9;
    VkImageMemoryBarrier* pImageMemoryBarriers = NULL;
    if (ARG10) {
        pImageMemoryBarriers = new VkImageMemoryBarrier[imageMemoryBarrierCount];
    }
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
PFN_vkCmdBeginQuery vkCmdBeginQuery;
void vk_CmdBeginQuery(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t query = (uint32_t)ARG3;
    VkQueryControlFlags flags = (VkQueryControlFlags)ARG4;
    vkCmdBeginQuery(commandBuffer, queryPool, query, flags);
}
PFN_vkCmdEndQuery vkCmdEndQuery;
void vk_CmdEndQuery(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t query = (uint32_t)ARG3;
    vkCmdEndQuery(commandBuffer, queryPool, query);
}
PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
void vk_CmdResetQueryPool(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t firstQuery = (uint32_t)ARG3;
    uint32_t queryCount = (uint32_t)ARG4;
    vkCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}
PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
void vk_CmdWriteTimestamp(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkPipelineStageFlagBits pipelineStage = (VkPipelineStageFlagBits)ARG2;
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG3, 8);
;
    uint32_t query = (uint32_t)ARG4;
    vkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
void vk_CmdCopyQueryPoolResults(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t firstQuery = (uint32_t)ARG3;
    uint32_t queryCount = (uint32_t)ARG4;
    VkBuffer dstBuffer = *(VkBuffer*)getPhysicalAddress(ARG5, 8);
;
    VkDeviceSize dstOffset = *(VkDeviceSize*)getPhysicalAddress(ARG6, 8);
;
    VkDeviceSize stride = *(VkDeviceSize*)getPhysicalAddress(ARG7, 8);
;
    VkQueryResultFlags flags = (VkQueryResultFlags)ARG8;
    vkCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}
PFN_vkCmdPushConstants vkCmdPushConstants;
void vk_CmdPushConstants(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkPipelineLayout layout = *(VkPipelineLayout*)getPhysicalAddress(ARG2, 8);
;
    VkShaderStageFlags stageFlags = (VkShaderStageFlags)ARG3;
    uint32_t offset = (uint32_t)ARG4;
    uint32_t size = (uint32_t)ARG5;
    void* pValues = (void*)getPhysicalAddress(ARG6, (U32)size * 4);
    vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
void vk_CmdBeginRenderPass(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    MarshalVkRenderPassBeginInfo local_pRenderPassBegin(ARG2);
    VkRenderPassBeginInfo* pRenderPassBegin = &local_pRenderPassBegin.s;
    VkSubpassContents contents = (VkSubpassContents)ARG3;
    vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}
PFN_vkCmdNextSubpass vkCmdNextSubpass;
void vk_CmdNextSubpass(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkSubpassContents contents = (VkSubpassContents)ARG2;
    vkCmdNextSubpass(commandBuffer, contents);
}
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
void vk_CmdEndRenderPass(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    vkCmdEndRenderPass(commandBuffer);
}
PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
void vk_CmdExecuteCommands(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t commandBufferCount = (uint32_t)ARG2;
    VkCommandBuffer* pCommandBuffers = new VkCommandBuffer[commandBufferCount];
    for (U32 i=0;i<commandBufferCount;i++) {
        pCommandBuffers[i] = (VkCommandBuffer)getVulkanPtr(readd(ARG3 + i*4));
    }
    vkCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}
PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
void vk_GetPhysicalDeviceFeatures2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkPhysicalDeviceFeatures2 pFeatures;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &pFeatures.s);
    MarshalVkPhysicalDeviceFeatures2::write(ARG2, &pFeatures.s);
}
PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
void vk_GetPhysicalDeviceProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkPhysicalDeviceProperties2 pProperties;
    vkGetPhysicalDeviceProperties2(physicalDevice, &pProperties.s);
    MarshalVkPhysicalDeviceProperties2::write(ARG2, &pProperties.s);
}
PFN_vkGetPhysicalDeviceFormatProperties2 vkGetPhysicalDeviceFormatProperties2;
void vk_GetPhysicalDeviceFormatProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    VkFormat format = (VkFormat)ARG2;
    MarshalVkFormatProperties2 pFormatProperties;
    vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, &pFormatProperties.s);
    MarshalVkFormatProperties2::write(ARG3, &pFormatProperties.s);
}
PFN_vkGetPhysicalDeviceImageFormatProperties2 vkGetPhysicalDeviceImageFormatProperties2;
// return type: VkResult(4 bytes)
void vk_GetPhysicalDeviceImageFormatProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkPhysicalDeviceImageFormatInfo2 local_pImageFormatInfo(ARG2);
    VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo = &local_pImageFormatInfo.s;
    MarshalVkImageFormatProperties2 pImageFormatProperties;
    EAX = vkGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, &pImageFormatProperties.s);
    MarshalVkImageFormatProperties2::write(ARG3, &pImageFormatProperties.s);
}
PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2;
void vk_GetPhysicalDeviceQueueFamilyProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    uint32_t* pQueueFamilyPropertyCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkQueueFamilyProperties2* pQueueFamilyProperties = NULL;
    if (ARG3) {
        pQueueFamilyProperties = new VkQueueFamilyProperties2[*pQueueFamilyPropertyCount];
    }
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (U32 i=0;i<*pQueueFamilyPropertyCount;i++) {
        MarshalVkQueueFamilyProperties2::write(ARG3 + i * 32, &pQueueFamilyProperties[i]);
    }
}
PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2;
void vk_GetPhysicalDeviceMemoryProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkPhysicalDeviceMemoryProperties2 pMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &pMemoryProperties.s);
    MarshalVkPhysicalDeviceMemoryProperties2::write(ARG2, &pMemoryProperties.s);
}
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 vkGetPhysicalDeviceSparseImageFormatProperties2;
void vk_GetPhysicalDeviceSparseImageFormatProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkPhysicalDeviceSparseImageFormatInfo2 local_pFormatInfo(ARG2);
    VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo = &local_pFormatInfo.s;
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG3, 4);
    VkSparseImageFormatProperties2* pProperties = NULL;
    if (ARG4) {
        pProperties = new VkSparseImageFormatProperties2[*pPropertyCount];
    }
    vkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    for (U32 i=0;i<*pPropertyCount;i++) {
        MarshalVkSparseImageFormatProperties2::write(ARG4 + i * 28, &pProperties[i]);
    }
}
PFN_vkTrimCommandPool vkTrimCommandPool;
void vk_TrimCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkCommandPool commandPool = *(VkCommandPool*)getPhysicalAddress(ARG2, 8);
;
    VkCommandPoolTrimFlags flags = (VkCommandPoolTrimFlags)ARG3;
    vkTrimCommandPool(device, commandPool, flags);
}
PFN_vkGetPhysicalDeviceExternalBufferProperties vkGetPhysicalDeviceExternalBufferProperties;
void vk_GetPhysicalDeviceExternalBufferProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkPhysicalDeviceExternalBufferInfo local_pExternalBufferInfo(ARG2);
    VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo = &local_pExternalBufferInfo.s;
    MarshalVkExternalBufferProperties pExternalBufferProperties;
    vkGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, &pExternalBufferProperties.s);
    MarshalVkExternalBufferProperties::write(ARG3, &pExternalBufferProperties.s);
}
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties vkGetPhysicalDeviceExternalSemaphoreProperties;
void vk_GetPhysicalDeviceExternalSemaphoreProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkPhysicalDeviceExternalSemaphoreInfo local_pExternalSemaphoreInfo(ARG2);
    VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo = &local_pExternalSemaphoreInfo.s;
    MarshalVkExternalSemaphoreProperties pExternalSemaphoreProperties;
    vkGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, &pExternalSemaphoreProperties.s);
    MarshalVkExternalSemaphoreProperties::write(ARG3, &pExternalSemaphoreProperties.s);
}
PFN_vkGetPhysicalDeviceExternalFenceProperties vkGetPhysicalDeviceExternalFenceProperties;
void vk_GetPhysicalDeviceExternalFenceProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    MarshalVkPhysicalDeviceExternalFenceInfo local_pExternalFenceInfo(ARG2);
    VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo = &local_pExternalFenceInfo.s;
    MarshalVkExternalFenceProperties pExternalFenceProperties;
    vkGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, &pExternalFenceProperties.s);
    MarshalVkExternalFenceProperties::write(ARG3, &pExternalFenceProperties.s);
}
PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups;
// return type: VkResult(4 bytes)
void vk_EnumeratePhysicalDeviceGroups(CPU* cpu) {
    VkInstance instance = (VkInstance)getVulkanPtr(ARG1);
    uint32_t* pPhysicalDeviceGroupCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties = NULL;
    if (ARG3) {
        pPhysicalDeviceGroupProperties = new VkPhysicalDeviceGroupProperties[*pPhysicalDeviceGroupCount];
    }
    EAX = vkEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    for (U32 i=0;i<*pPhysicalDeviceGroupCount;i++) {
        MarshalVkPhysicalDeviceGroupProperties::write(ARG3 + i * 144, &pPhysicalDeviceGroupProperties[i]);
    }
}
PFN_vkGetDeviceGroupPeerMemoryFeatures vkGetDeviceGroupPeerMemoryFeatures;
void vk_GetDeviceGroupPeerMemoryFeatures(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    uint32_t heapIndex = (uint32_t)ARG2;
    uint32_t localDeviceIndex = (uint32_t)ARG3;
    uint32_t remoteDeviceIndex = (uint32_t)ARG4;
    VkPeerMemoryFeatureFlags* pPeerMemoryFeatures = (VkPeerMemoryFeatureFlags*)getPhysicalAddress(ARG5, 4);
    vkGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
PFN_vkBindBufferMemory2 vkBindBufferMemory2;
// return type: VkResult(4 bytes)
void vk_BindBufferMemory2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    uint32_t bindInfoCount = (uint32_t)ARG2;
    VkBindBufferMemoryInfo* pBindInfos = NULL;
    if (ARG3) {
        pBindInfos = new VkBindBufferMemoryInfo[bindInfoCount];
    }
    EAX = vkBindBufferMemory2(device, bindInfoCount, pBindInfos);
}
PFN_vkBindImageMemory2 vkBindImageMemory2;
// return type: VkResult(4 bytes)
void vk_BindImageMemory2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    uint32_t bindInfoCount = (uint32_t)ARG2;
    VkBindImageMemoryInfo* pBindInfos = NULL;
    if (ARG3) {
        pBindInfos = new VkBindImageMemoryInfo[bindInfoCount];
    }
    EAX = vkBindImageMemory2(device, bindInfoCount, pBindInfos);
}
PFN_vkCmdSetDeviceMask vkCmdSetDeviceMask;
void vk_CmdSetDeviceMask(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t deviceMask = (uint32_t)ARG2;
    vkCmdSetDeviceMask(commandBuffer, deviceMask);
}
PFN_vkCmdDispatchBase vkCmdDispatchBase;
void vk_CmdDispatchBase(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    uint32_t baseGroupX = (uint32_t)ARG2;
    uint32_t baseGroupY = (uint32_t)ARG3;
    uint32_t baseGroupZ = (uint32_t)ARG4;
    uint32_t groupCountX = (uint32_t)ARG5;
    uint32_t groupCountY = (uint32_t)ARG6;
    uint32_t groupCountZ = (uint32_t)ARG7;
    vkCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
PFN_vkCreateDescriptorUpdateTemplate vkCreateDescriptorUpdateTemplate;
// return type: VkResult(4 bytes)
void vk_CreateDescriptorUpdateTemplate(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkDescriptorUpdateTemplateCreateInfo local_pCreateInfo(ARG2);
    VkDescriptorUpdateTemplateCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateDescriptorUpdateTemplate:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate = (VkDescriptorUpdateTemplate*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
PFN_vkDestroyDescriptorUpdateTemplate vkDestroyDescriptorUpdateTemplate;
void vk_DestroyDescriptorUpdateTemplate(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDescriptorUpdateTemplate descriptorUpdateTemplate = *(VkDescriptorUpdateTemplate*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyDescriptorUpdateTemplate:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}
PFN_vkUpdateDescriptorSetWithTemplate vkUpdateDescriptorSetWithTemplate;
void vk_UpdateDescriptorSetWithTemplate(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkDescriptorSet descriptorSet = *(VkDescriptorSet*)getPhysicalAddress(ARG2, 8);
;
    VkDescriptorUpdateTemplate descriptorUpdateTemplate = *(VkDescriptorUpdateTemplate*)getPhysicalAddress(ARG3, 8);
;
    void* pData = (void*)getPhysicalAddress(ARG4, 4);
    vkUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}
PFN_vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2;
void vk_GetBufferMemoryRequirements2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkBufferMemoryRequirementsInfo2 local_pInfo(ARG2);
    VkBufferMemoryRequirementsInfo2* pInfo = &local_pInfo.s;
    MarshalVkMemoryRequirements2 pMemoryRequirements;
    vkGetBufferMemoryRequirements2(device, pInfo, &pMemoryRequirements.s);
    MarshalVkMemoryRequirements2::write(ARG3, &pMemoryRequirements.s);
}
PFN_vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2;
void vk_GetImageMemoryRequirements2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkImageMemoryRequirementsInfo2 local_pInfo(ARG2);
    VkImageMemoryRequirementsInfo2* pInfo = &local_pInfo.s;
    MarshalVkMemoryRequirements2 pMemoryRequirements;
    vkGetImageMemoryRequirements2(device, pInfo, &pMemoryRequirements.s);
    MarshalVkMemoryRequirements2::write(ARG3, &pMemoryRequirements.s);
}
PFN_vkGetImageSparseMemoryRequirements2 vkGetImageSparseMemoryRequirements2;
void vk_GetImageSparseMemoryRequirements2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkImageSparseMemoryRequirementsInfo2 local_pInfo(ARG2);
    VkImageSparseMemoryRequirementsInfo2* pInfo = &local_pInfo.s;
    uint32_t* pSparseMemoryRequirementCount = (uint32_t*)getPhysicalAddress(ARG3, 4);
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements = NULL;
    if (ARG4) {
        pSparseMemoryRequirements = new VkSparseImageMemoryRequirements2[*pSparseMemoryRequirementCount];
    }
    vkGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (U32 i=0;i<*pSparseMemoryRequirementCount;i++) {
        MarshalVkSparseImageMemoryRequirements2::write(ARG4 + i * 56, &pSparseMemoryRequirements[i]);
    }
}
PFN_vkCreateSamplerYcbcrConversion vkCreateSamplerYcbcrConversion;
// return type: VkResult(4 bytes)
void vk_CreateSamplerYcbcrConversion(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkSamplerYcbcrConversionCreateInfo local_pCreateInfo(ARG2);
    VkSamplerYcbcrConversionCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateSamplerYcbcrConversion:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkSamplerYcbcrConversion* pYcbcrConversion = (VkSamplerYcbcrConversion*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}
PFN_vkDestroySamplerYcbcrConversion vkDestroySamplerYcbcrConversion;
void vk_DestroySamplerYcbcrConversion(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkSamplerYcbcrConversion ycbcrConversion = *(VkSamplerYcbcrConversion*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroySamplerYcbcrConversion:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    vkDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}
PFN_vkGetDeviceQueue2 vkGetDeviceQueue2;
void vk_GetDeviceQueue2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkDeviceQueueInfo2 local_pQueueInfo(ARG2);
    VkDeviceQueueInfo2* pQueueInfo = &local_pQueueInfo.s;
    VkQueue pQueue;
    vkGetDeviceQueue2(device, pQueueInfo, &pQueue);
}
PFN_vkGetDescriptorSetLayoutSupport vkGetDescriptorSetLayoutSupport;
void vk_GetDescriptorSetLayoutSupport(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkDescriptorSetLayoutCreateInfo local_pCreateInfo(ARG2);
    VkDescriptorSetLayoutCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    MarshalVkDescriptorSetLayoutSupport pSupport;
    vkGetDescriptorSetLayoutSupport(device, pCreateInfo, &pSupport.s);
    MarshalVkDescriptorSetLayoutSupport::write(ARG3, &pSupport.s);
}
PFN_vkCreateRenderPass2 vkCreateRenderPass2;
// return type: VkResult(4 bytes)
void vk_CreateRenderPass2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkRenderPassCreateInfo2 local_pCreateInfo(ARG2);
    VkRenderPassCreateInfo2* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateRenderPass2:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkRenderPass* pRenderPass = (VkRenderPass*)getPhysicalAddress(ARG4, 4);
    EAX = vkCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
}
PFN_vkCmdBeginRenderPass2 vkCmdBeginRenderPass2;
void vk_CmdBeginRenderPass2(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    MarshalVkRenderPassBeginInfo local_pRenderPassBegin(ARG2);
    VkRenderPassBeginInfo* pRenderPassBegin = &local_pRenderPassBegin.s;
    MarshalVkSubpassBeginInfo local_pSubpassBeginInfo(ARG3);
    VkSubpassBeginInfo* pSubpassBeginInfo = &local_pSubpassBeginInfo.s;
    vkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
PFN_vkCmdNextSubpass2 vkCmdNextSubpass2;
void vk_CmdNextSubpass2(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    MarshalVkSubpassBeginInfo local_pSubpassBeginInfo(ARG2);
    VkSubpassBeginInfo* pSubpassBeginInfo = &local_pSubpassBeginInfo.s;
    MarshalVkSubpassEndInfo local_pSubpassEndInfo(ARG3);
    VkSubpassEndInfo* pSubpassEndInfo = &local_pSubpassEndInfo.s;
    vkCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
PFN_vkCmdEndRenderPass2 vkCmdEndRenderPass2;
void vk_CmdEndRenderPass2(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    MarshalVkSubpassEndInfo local_pSubpassEndInfo(ARG2);
    VkSubpassEndInfo* pSubpassEndInfo = &local_pSubpassEndInfo.s;
    vkCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}
PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue;
// return type: VkResult(4 bytes)
void vk_GetSemaphoreCounterValue(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    VkSemaphore semaphore = *(VkSemaphore*)getPhysicalAddress(ARG2, 8);
;
    uint64_t* pValue = (uint64_t*)getPhysicalAddress(ARG3, 4);
    EAX = vkGetSemaphoreCounterValue(device, semaphore, pValue);
}
PFN_vkWaitSemaphores vkWaitSemaphores;
// return type: VkResult(4 bytes)
void vk_WaitSemaphores(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkSemaphoreWaitInfo local_pWaitInfo(ARG2);
    VkSemaphoreWaitInfo* pWaitInfo = &local_pWaitInfo.s;
    uint64_t timeout = *(uint64_t*)getPhysicalAddress(ARG3, 8);
;
    EAX = vkWaitSemaphores(device, pWaitInfo, timeout);
}
PFN_vkSignalSemaphore vkSignalSemaphore;
// return type: VkResult(4 bytes)
void vk_SignalSemaphore(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkSemaphoreSignalInfo local_pSignalInfo(ARG2);
    VkSemaphoreSignalInfo* pSignalInfo = &local_pSignalInfo.s;
    EAX = vkSignalSemaphore(device, pSignalInfo);
}
PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount;
void vk_CmdDrawIndirectCount(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkBuffer countBuffer = *(VkBuffer*)getPhysicalAddress(ARG4, 8);
;
    VkDeviceSize countBufferOffset = *(VkDeviceSize*)getPhysicalAddress(ARG5, 8);
;
    uint32_t maxDrawCount = (uint32_t)ARG6;
    uint32_t stride = (uint32_t)ARG7;
    vkCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount;
void vk_CmdDrawIndexedIndirectCount(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkBuffer countBuffer = *(VkBuffer*)getPhysicalAddress(ARG4, 8);
;
    VkDeviceSize countBufferOffset = *(VkDeviceSize*)getPhysicalAddress(ARG5, 8);
;
    uint32_t maxDrawCount = (uint32_t)ARG6;
    uint32_t stride = (uint32_t)ARG7;
    vkCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
PFN_vkGetBufferOpaqueCaptureAddress vkGetBufferOpaqueCaptureAddress;
// return type: uint64_t(8 bytes)
void vk_GetBufferOpaqueCaptureAddress(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkBufferDeviceAddressInfo local_pInfo(ARG2);
    VkBufferDeviceAddressInfo* pInfo = &local_pInfo.s;
    uint64_t result = vkGetBufferOpaqueCaptureAddress(device, pInfo);
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}
PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;
// return type: VkDeviceAddress(8 bytes)
void vk_GetBufferDeviceAddress(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkBufferDeviceAddressInfo local_pInfo(ARG2);
    VkBufferDeviceAddressInfo* pInfo = &local_pInfo.s;
    VkDeviceAddress result = vkGetBufferDeviceAddress(device, pInfo);
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}
PFN_vkGetDeviceMemoryOpaqueCaptureAddress vkGetDeviceMemoryOpaqueCaptureAddress;
// return type: uint64_t(8 bytes)
void vk_GetDeviceMemoryOpaqueCaptureAddress(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    MarshalVkDeviceMemoryOpaqueCaptureAddressInfo local_pInfo(ARG2);
    VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo = &local_pInfo.s;
    uint64_t result = vkGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}
#endif

