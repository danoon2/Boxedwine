#include "boxedwine.h"
#ifdef BOXEDWINE_VULKAN
#include <SDL.h>
#include <SDL_vulkan.h>
#define VK_NO_PROTOTYPES
#include "vk/vulkan.h"
#include "vk/vulkan_core.h"
#define BOXED_VK_EXTERN
#include "vk_host.h"

void initVulkan();
void* getVulkanPtr(U32 address);
U32 createVulkanPtr(U64 value, BoxedVulkanInfo* info);
BoxedVulkanInfo* getInfoFromHandle(U32 address);
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
    MarshalVkPhysicalDeviceProperties(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceProperties* s) {
        s->apiVersion = (uint32_t)readd(address);address+=4;
        s->driverVersion = (uint32_t)readd(address);address+=4;
        s->vendorID = (uint32_t)readd(address);address+=4;
        s->deviceID = (uint32_t)readd(address);address+=4;
        s->deviceType = (VkPhysicalDeviceType)readd(address);address+=4;
        memcopyToNative(address, &s->deviceName, 256);address+=256;
        memcopyToNative(address, &s->pipelineCacheUUID, 16);address+=16;
        memcopyToNative(address, &s->limits, 488);address+=488;
        memcopyToNative(address, &s->sparseProperties, 20);address+=20;
    }
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
                ppEnabledLayerNames[i] = (char*)getPhysicalAddress(readd(paramAddress + i*4), 0);
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
                ppEnabledExtensionNames[i] = (char*)getPhysicalAddress(readd(paramAddress + i*4), 0);
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
                ppEnabledLayerNames[i] = (char*)getPhysicalAddress(readd(paramAddress + i*4), 0);
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
                ppEnabledExtensionNames[i] = (char*)getPhysicalAddress(readd(paramAddress + i*4), 0);
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
        memcopyToNative(address, &s->extent, 12);address+=12;
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
        memcopyToNative(address, &s->components, 16);address+=16;
        memcopyToNative(address, &s->subresourceRange, 20);address+=20;
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
        memcopyToNative(address, &s->renderArea, 16);address+=16;
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
    MarshalVkPhysicalDeviceFeatures2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceFeatures2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        memcopyToNative(address, &s->features, 220);address+=220;
    }
    static void write(U32 address, VkPhysicalDeviceFeatures2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
        }
        memcopyFromNative(address, &s->features, 220); address+=220;
    }
};

class MarshalVkPhysicalDeviceProperties2 {
public:
    MarshalVkPhysicalDeviceProperties2() {}
    VkPhysicalDeviceProperties2 s;
    MarshalVkPhysicalDeviceProperties2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceProperties2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        memcopyToNative(address, &s->properties, 800);address+=800;
    }
    static void write(U32 address, VkPhysicalDeviceProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
        }
        memcopyFromNative(address, &s->properties, 800); address+=800;
    }
};

class MarshalVkFormatProperties2 {
public:
    MarshalVkFormatProperties2() {}
    VkFormatProperties2 s;
    MarshalVkFormatProperties2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkFormatProperties2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        memcopyToNative(address, &s->formatProperties, 12);address+=12;
    }
    static void write(U32 address, VkFormatProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
        }
        memcopyFromNative(address, &s->formatProperties, 12); address+=12;
    }
};

class MarshalVkImageFormatProperties2 {
public:
    MarshalVkImageFormatProperties2() {}
    VkImageFormatProperties2 s;
    MarshalVkImageFormatProperties2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkImageFormatProperties2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        memcopyToNative(address, &s->imageFormatProperties, 32);address+=32;
    }
    static void write(U32 address, VkImageFormatProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
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
            delete s->pNext;
        }
        memcopyFromNative(address, &s->queueFamilyProperties, 24); address+=24;
    }
};

class MarshalVkPhysicalDeviceMemoryProperties2 {
public:
    MarshalVkPhysicalDeviceMemoryProperties2() {}
    VkPhysicalDeviceMemoryProperties2 s;
    MarshalVkPhysicalDeviceMemoryProperties2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceMemoryProperties2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        memcopyToNative(address, &s->memoryProperties, 456);address+=456;
    }
    static void write(U32 address, VkPhysicalDeviceMemoryProperties2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
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
            delete s->pNext;
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
    MarshalVkExternalBufferProperties(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkExternalBufferProperties* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        memcopyToNative(address, &s->externalMemoryProperties, 12);address+=12;
    }
    static void write(U32 address, VkExternalBufferProperties* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
        }
        memcopyFromNative(address, &s->externalMemoryProperties, 12); address+=12;
    }
};

class MarshalVkPhysicalDeviceIDProperties {
public:
    MarshalVkPhysicalDeviceIDProperties() {}
    VkPhysicalDeviceIDProperties s;
    MarshalVkPhysicalDeviceIDProperties(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceIDProperties* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        memcopyToNative(address, &s->deviceUUID, 16);address+=16;
        memcopyToNative(address, &s->driverUUID, 16);address+=16;
        memcopyToNative(address, &s->deviceLUID, 8);address+=8;
        s->deviceNodeMask = (uint32_t)readd(address);address+=4;
        s->deviceLUIDValid = (VkBool32)readd(address);address+=4;
    }
    static void write(U32 address, VkPhysicalDeviceIDProperties* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
        }
        memcopyFromNative(address, s->deviceUUID, 16); address+=16;
        memcopyFromNative(address, s->driverUUID, 16); address+=16;
        memcopyFromNative(address, s->deviceLUID, 8); address+=8;
        writed(address, s->deviceNodeMask);address+=4;
        writed(address, s->deviceLUIDValid);address+=4;
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
    MarshalVkExternalSemaphoreProperties(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkExternalSemaphoreProperties* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->exportFromImportedHandleTypes = (VkExternalSemaphoreHandleTypeFlags)readd(address);address+=4;
        s->compatibleHandleTypes = (VkExternalSemaphoreHandleTypeFlags)readd(address);address+=4;
        s->externalSemaphoreFeatures = (VkExternalSemaphoreFeatureFlags)readd(address);address+=4;
    }
    static void write(U32 address, VkExternalSemaphoreProperties* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
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
    MarshalVkExternalFenceProperties(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkExternalFenceProperties* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->exportFromImportedHandleTypes = (VkExternalFenceHandleTypeFlags)readd(address);address+=4;
        s->compatibleHandleTypes = (VkExternalFenceHandleTypeFlags)readd(address);address+=4;
        s->externalFenceFeatures = (VkExternalFenceFeatureFlags)readd(address);address+=4;
    }
    static void write(U32 address, VkExternalFenceProperties* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
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
            delete s->pNext;
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
    MarshalVkMemoryRequirements2(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkMemoryRequirements2* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        memcopyToNative(address, &s->memoryRequirements, 20);address+=20;
    }
    static void write(U32 address, VkMemoryRequirements2* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
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
            delete s->pNext;
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
        memcopyToNative(address, &s->components, 16);address+=16;
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
    MarshalVkDescriptorSetLayoutSupport(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkDescriptorSetLayoutSupport* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->supported = (VkBool32)readd(address);address+=4;
    }
    static void write(U32 address, VkDescriptorSetLayoutSupport* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
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

class MarshalVkPhysicalDeviceVertexAttributeDivisorFeaturesEXT {
public:
    MarshalVkPhysicalDeviceVertexAttributeDivisorFeaturesEXT() {}
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT s;
    MarshalVkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->vertexAttributeInstanceRateDivisor = (VkBool32)readd(address);address+=4;
        s->vertexAttributeInstanceRateZeroDivisor = (VkBool32)readd(address);address+=4;
    }
    static void write(U32 address, VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
        }
        writed(address, s->vertexAttributeInstanceRateDivisor);address+=4;
        writed(address, s->vertexAttributeInstanceRateZeroDivisor);address+=4;
    }
};

class MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT {
public:
    MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT() {}
    VkPhysicalDeviceTransformFeedbackFeaturesEXT s;
    MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT(U32 address) {read(address, &this->s);}
    static void read(U32 address, VkPhysicalDeviceTransformFeedbackFeaturesEXT* s) {
        s->sType = (VkStructureType)readd(address);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress == 0) {
            s->pNext = NULL;
        } else {
            s->pNext = vulkanGetNextPtr(paramAddress);
        }
        s->transformFeedback = (VkBool32)readd(address);address+=4;
        s->geometryStreams = (VkBool32)readd(address);address+=4;
    }
    static void write(U32 address, VkPhysicalDeviceTransformFeedbackFeaturesEXT* s) {
        writed(address, s->sType);address+=4;
        U32 paramAddress = readd(address);address+=4;
        if (paramAddress != 0) {
            vulkanWriteNextPtr(paramAddress, s->pNext);
            delete s->pNext;
        }
        writed(address, s->transformFeedback);address+=4;
        writed(address, s->geometryStreams);address+=4;
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

// return type: VkResult(4 bytes)
void vk_CreateInstance(CPU* cpu) {
    initVulkan();
    MarshalVkInstanceCreateInfo local_pCreateInfo(ARG1);
    VkInstanceCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG2) { klog("vkCreateInstance:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkInstance pInstance;
    EAX = pvkCreateInstance(pCreateInfo, pAllocator, &pInstance);
    writed(ARG3, createVulkanPtr((U64)pInstance, NULL));
}
void vk_DestroyInstance(CPU* cpu) {
    VkInstance instance = (VkInstance)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    static bool shown; if (!shown && ARG2) { klog("vkDestroyInstance:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyInstance(instance, pAllocator);
    freeVulkanPtr(ARG1);
}
// return type: VkResult(4 bytes)
void vk_EnumeratePhysicalDevices(CPU* cpu) {
    VkInstance instance = (VkInstance)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t* pPhysicalDeviceCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkPhysicalDevice* pPhysicalDevices = NULL;
    if (ARG3) {
        pPhysicalDevices = new VkPhysicalDevice[*pPhysicalDeviceCount];
    }
    EAX = pBoxedInfo->pvkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    if (ARG3) {
        for (U32 i=0;i<*pPhysicalDeviceCount;i++) {
            writed(ARG3 + i*4, createVulkanPtr((U64)pPhysicalDevices[i], pBoxedInfo));
        }
        delete[] pPhysicalDevices;
    }
}
void vk_GetPhysicalDeviceProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPhysicalDeviceProperties pProperties(ARG2);
    pBoxedInfo->pvkGetPhysicalDeviceProperties(physicalDevice, &pProperties.s);
    MarshalVkPhysicalDeviceProperties::write(ARG2, &pProperties.s);
}
void vk_GetPhysicalDeviceQueueFamilyProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t* pQueueFamilyPropertyCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkQueueFamilyProperties* pQueueFamilyProperties = (VkQueueFamilyProperties*)getPhysicalAddress(ARG3, (U32)(pQueueFamilyPropertyCount ? *pQueueFamilyPropertyCount : 0));
    pBoxedInfo->pvkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
void vk_GetPhysicalDeviceMemoryProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPhysicalDeviceMemoryProperties* pMemoryProperties = (VkPhysicalDeviceMemoryProperties*)getPhysicalAddress(ARG2, 4);
    pBoxedInfo->pvkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}
void vk_GetPhysicalDeviceFeatures(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPhysicalDeviceFeatures* pFeatures = (VkPhysicalDeviceFeatures*)getPhysicalAddress(ARG2, 4);
    pBoxedInfo->pvkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}
void vk_GetPhysicalDeviceFormatProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkFormat format = (VkFormat)ARG2;
    VkFormatProperties* pFormatProperties = (VkFormatProperties*)getPhysicalAddress(ARG3, 4);
    pBoxedInfo->pvkGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}
// return type: VkResult(4 bytes)
void vk_GetPhysicalDeviceImageFormatProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkFormat format = (VkFormat)ARG2;
    VkImageType type = (VkImageType)ARG3;
    VkImageTiling tiling = (VkImageTiling)ARG4;
    VkImageUsageFlags usage = (VkImageUsageFlags)ARG5;
    VkImageCreateFlags flags = (VkImageCreateFlags)ARG6;
    VkImageFormatProperties* pImageFormatProperties = (VkImageFormatProperties*)getPhysicalAddress(ARG7, 4);
    EAX = pBoxedInfo->pvkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}
// return type: VkResult(4 bytes)
void vk_CreateDevice(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkDeviceCreateInfo local_pCreateInfo(ARG2);
    VkDeviceCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateDevice:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDevice pDevice;
    EAX = pBoxedInfo->pvkCreateDevice(physicalDevice, pCreateInfo, pAllocator, &pDevice);
    writed(ARG4, createVulkanPtr((U64)pDevice, pBoxedInfo));
}
void vk_DestroyDevice(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    static bool shown; if (!shown && ARG2) { klog("vkDestroyDevice:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyDevice(device, pAllocator);
    freeVulkanPtr(ARG1);
}
// return type: VkResult(4 bytes)
void vk_EnumerateInstanceVersion(CPU* cpu) {
    initVulkan();
    uint32_t* pApiVersion = (uint32_t*)getPhysicalAddress(ARG1, 4);
    EAX = pvkEnumerateInstanceVersion(pApiVersion);
}
// return type: VkResult(4 bytes)
void vk_EnumerateInstanceLayerProperties(CPU* cpu) {
    initVulkan();
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG1, 4);
    VkLayerProperties* pProperties = (VkLayerProperties*)getPhysicalAddress(ARG2, (U32)(pPropertyCount ? *pPropertyCount : 0));
    EAX = pvkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}
// return type: VkResult(4 bytes)
void vk_EnumerateInstanceExtensionProperties(CPU* cpu) {
    initVulkan();
    char* pLayerName = (char*)getPhysicalAddress(ARG1, 4);
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkExtensionProperties* pProperties = (VkExtensionProperties*)getPhysicalAddress(ARG3, (U32)(pPropertyCount ? *pPropertyCount : 0));
    EAX = pvkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}
// return type: VkResult(4 bytes)
void vk_EnumerateDeviceLayerProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkLayerProperties* pProperties = (VkLayerProperties*)getPhysicalAddress(ARG3, (U32)(pPropertyCount ? *pPropertyCount : 0));
    EAX = pBoxedInfo->pvkEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}
// return type: VkResult(4 bytes)
void vk_EnumerateDeviceExtensionProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    char* pLayerName = (char*)getPhysicalAddress(ARG2, 4);
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG3, 4);
    VkExtensionProperties* pProperties = (VkExtensionProperties*)getPhysicalAddress(ARG4, (U32)(pPropertyCount ? *pPropertyCount : 0));
    EAX = pBoxedInfo->pvkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}
void vk_GetDeviceQueue(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t queueFamilyIndex = (uint32_t)ARG2;
    uint32_t queueIndex = (uint32_t)ARG3;
    VkQueue pQueue;
    pBoxedInfo->pvkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &pQueue);
    writed(ARG4, createVulkanPtr((U64)pQueue, pBoxedInfo));
}
// return type: VkResult(4 bytes)
void vk_QueueSubmit(CPU* cpu) {
    VkQueue queue = (VkQueue)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t submitCount = (uint32_t)ARG2;
    VkSubmitInfo* pSubmits = NULL;
    if (ARG3) {
        pSubmits = new VkSubmitInfo[submitCount];
    }
    VkFence fence = *(VkFence*)getPhysicalAddress(ARG4, 8);
;
    EAX = pBoxedInfo->pvkQueueSubmit(queue, submitCount, pSubmits, fence);
    if (pSubmits) {
        delete[] pSubmits;
    }
}
// return type: VkResult(4 bytes)
void vk_QueueWaitIdle(CPU* cpu) {
    VkQueue queue = (VkQueue)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    EAX = pBoxedInfo->pvkQueueWaitIdle(queue);
}
// return type: VkResult(4 bytes)
void vk_DeviceWaitIdle(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    EAX = pBoxedInfo->pvkDeviceWaitIdle(device);
}
// return type: VkResult(4 bytes)
void vk_AllocateMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkMemoryAllocateInfo local_pAllocateInfo(ARG2);
    VkMemoryAllocateInfo* pAllocateInfo = &local_pAllocateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkAllocateMemory:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDeviceMemory* pMemory = (VkDeviceMemory*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}
void vk_FreeMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkFreeMemory:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkFreeMemory(device, memory, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_MapMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize size = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    VkMemoryMapFlags flags = (VkMemoryMapFlags)ARG5;
    void **ppData = NULL;
    kpanic("vkMapMemory not implemented");
    EAX = pBoxedInfo->pvkMapMemory(device, memory, offset, size, flags, ppData);
}
void vk_UnmapMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG2, 8);
;
    pBoxedInfo->pvkUnmapMemory(device, memory);
}
// return type: VkResult(4 bytes)
void vk_FlushMappedMemoryRanges(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t memoryRangeCount = (uint32_t)ARG2;
    VkMappedMemoryRange* pMemoryRanges = NULL;
    if (ARG3) {
        pMemoryRanges = new VkMappedMemoryRange[memoryRangeCount];
    }
    EAX = pBoxedInfo->pvkFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    if (pMemoryRanges) {
        delete[] pMemoryRanges;
    }
}
// return type: VkResult(4 bytes)
void vk_InvalidateMappedMemoryRanges(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t memoryRangeCount = (uint32_t)ARG2;
    VkMappedMemoryRange* pMemoryRanges = NULL;
    if (ARG3) {
        pMemoryRanges = new VkMappedMemoryRange[memoryRangeCount];
    }
    EAX = pBoxedInfo->pvkInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    if (pMemoryRanges) {
        delete[] pMemoryRanges;
    }
}
void vk_GetDeviceMemoryCommitment(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize* pCommittedMemoryInBytes = (VkDeviceSize*)getPhysicalAddress(ARG3, 4);
    pBoxedInfo->pvkGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}
void vk_GetBufferMemoryRequirements(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkMemoryRequirements* pMemoryRequirements = (VkMemoryRequirements*)getPhysicalAddress(ARG3, 4);
    pBoxedInfo->pvkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}
// return type: VkResult(4 bytes)
void vk_BindBufferMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize memoryOffset = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    EAX = pBoxedInfo->pvkBindBufferMemory(device, buffer, memory, memoryOffset);
}
void vk_GetImageMemoryRequirements(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkMemoryRequirements* pMemoryRequirements = (VkMemoryRequirements*)getPhysicalAddress(ARG3, 4);
    pBoxedInfo->pvkGetImageMemoryRequirements(device, image, pMemoryRequirements);
}
// return type: VkResult(4 bytes)
void vk_BindImageMemory(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceMemory memory = *(VkDeviceMemory*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize memoryOffset = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    EAX = pBoxedInfo->pvkBindImageMemory(device, image, memory, memoryOffset);
}
void vk_GetImageSparseMemoryRequirements(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    uint32_t* pSparseMemoryRequirementCount = (uint32_t*)getPhysicalAddress(ARG3, 4);
    VkSparseImageMemoryRequirements* pSparseMemoryRequirements = (VkSparseImageMemoryRequirements*)getPhysicalAddress(ARG4, (U32)(pSparseMemoryRequirementCount ? *pSparseMemoryRequirementCount : 0));
    pBoxedInfo->pvkGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
void vk_GetPhysicalDeviceSparseImageFormatProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkFormat format = (VkFormat)ARG2;
    VkImageType type = (VkImageType)ARG3;
    VkSampleCountFlagBits samples = (VkSampleCountFlagBits)ARG4;
    VkImageUsageFlags usage = (VkImageUsageFlags)ARG5;
    VkImageTiling tiling = (VkImageTiling)ARG6;
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG7, 4);
    VkSparseImageFormatProperties* pProperties = (VkSparseImageFormatProperties*)getPhysicalAddress(ARG8, (U32)(pPropertyCount ? *pPropertyCount : 0));
    pBoxedInfo->pvkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}
// return type: VkResult(4 bytes)
void vk_QueueBindSparse(CPU* cpu) {
    VkQueue queue = (VkQueue)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t bindInfoCount = (uint32_t)ARG2;
    VkBindSparseInfo* pBindInfo = NULL;
    if (ARG3) {
        pBindInfo = new VkBindSparseInfo[bindInfoCount];
    }
    VkFence fence = *(VkFence*)getPhysicalAddress(ARG4, 8);
;
    EAX = pBoxedInfo->pvkQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    if (pBindInfo) {
        delete[] pBindInfo;
    }
}
// return type: VkResult(4 bytes)
void vk_CreateFence(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkFenceCreateInfo local_pCreateInfo(ARG2);
    VkFenceCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateFence:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkFence* pFence = (VkFence*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateFence(device, pCreateInfo, pAllocator, pFence);
}
void vk_DestroyFence(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkFence fence = *(VkFence*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyFence:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyFence(device, fence, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_ResetFences(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t fenceCount = (uint32_t)ARG2;
    VkFence* pFences = (VkFence*)getPhysicalAddress(ARG3, (U32)fenceCount * 4);
    EAX = pBoxedInfo->pvkResetFences(device, fenceCount, pFences);
}
// return type: VkResult(4 bytes)
void vk_GetFenceStatus(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkFence fence = *(VkFence*)getPhysicalAddress(ARG2, 8);
;
    EAX = pBoxedInfo->pvkGetFenceStatus(device, fence);
}
// return type: VkResult(4 bytes)
void vk_WaitForFences(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t fenceCount = (uint32_t)ARG2;
    VkFence* pFences = (VkFence*)getPhysicalAddress(ARG3, (U32)fenceCount * 4);
    VkBool32 waitAll = (VkBool32)ARG4;
    uint64_t timeout = *(uint64_t*)getPhysicalAddress(ARG5, 8);
;
    EAX = pBoxedInfo->pvkWaitForFences(device, fenceCount, pFences, waitAll, timeout);
}
// return type: VkResult(4 bytes)
void vk_CreateSemaphore(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkSemaphoreCreateInfo local_pCreateInfo(ARG2);
    VkSemaphoreCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateSemaphore:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkSemaphore* pSemaphore = (VkSemaphore*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}
void vk_DestroySemaphore(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkSemaphore semaphore = *(VkSemaphore*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroySemaphore:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroySemaphore(device, semaphore, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreateEvent(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkEventCreateInfo local_pCreateInfo(ARG2);
    VkEventCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateEvent:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkEvent* pEvent = (VkEvent*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateEvent(device, pCreateInfo, pAllocator, pEvent);
}
void vk_DestroyEvent(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyEvent:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyEvent(device, event, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_GetEventStatus(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    EAX = pBoxedInfo->pvkGetEventStatus(device, event);
}
// return type: VkResult(4 bytes)
void vk_SetEvent(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    EAX = pBoxedInfo->pvkSetEvent(device, event);
}
// return type: VkResult(4 bytes)
void vk_ResetEvent(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    EAX = pBoxedInfo->pvkResetEvent(device, event);
}
// return type: VkResult(4 bytes)
void vk_CreateQueryPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkQueryPoolCreateInfo local_pCreateInfo(ARG2);
    VkQueryPoolCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateQueryPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkQueryPool* pQueryPool = (VkQueryPool*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}
void vk_DestroyQueryPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyQueryPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyQueryPool(device, queryPool, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_GetQueryPoolResults(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t firstQuery = (uint32_t)ARG3;
    uint32_t queryCount = (uint32_t)ARG4;
    size_t dataSize = (size_t)ARG5;
    void* pData = (void*)getPhysicalAddress(ARG6, (U32)dataSize * 4);
    VkDeviceSize stride = *(VkDeviceSize*)getPhysicalAddress(ARG7, 8);
;
    VkQueryResultFlags flags = (VkQueryResultFlags)ARG8;
    EAX = pBoxedInfo->pvkGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}
void vk_ResetQueryPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t firstQuery = (uint32_t)ARG3;
    uint32_t queryCount = (uint32_t)ARG4;
    pBoxedInfo->pvkResetQueryPool(device, queryPool, firstQuery, queryCount);
}
// return type: VkResult(4 bytes)
void vk_CreateBuffer(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkBufferCreateInfo local_pCreateInfo(ARG2);
    VkBufferCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateBuffer:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkBuffer* pBuffer = (VkBuffer*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}
void vk_DestroyBuffer(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyBuffer:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyBuffer(device, buffer, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreateBufferView(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkBufferViewCreateInfo local_pCreateInfo(ARG2);
    VkBufferViewCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateBufferView:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkBufferView* pView = (VkBufferView*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateBufferView(device, pCreateInfo, pAllocator, pView);
}
void vk_DestroyBufferView(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBufferView bufferView = *(VkBufferView*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyBufferView:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyBufferView(device, bufferView, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreateImage(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkImageCreateInfo local_pCreateInfo(ARG2);
    VkImageCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateImage:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkImage* pImage = (VkImage*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateImage(device, pCreateInfo, pAllocator, pImage);
}
void vk_DestroyImage(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyImage:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyImage(device, image, pAllocator);
}
void vk_GetImageSubresourceLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageSubresource* pSubresource = (VkImageSubresource*)getPhysicalAddress(ARG3, 4);
    VkSubresourceLayout* pLayout = (VkSubresourceLayout*)getPhysicalAddress(ARG4, 4);
    pBoxedInfo->pvkGetImageSubresourceLayout(device, image, pSubresource, pLayout);
}
// return type: VkResult(4 bytes)
void vk_CreateImageView(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkImageViewCreateInfo local_pCreateInfo(ARG2);
    VkImageViewCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateImageView:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkImageView* pView = (VkImageView*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateImageView(device, pCreateInfo, pAllocator, pView);
}
void vk_DestroyImageView(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImageView imageView = *(VkImageView*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyImageView:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyImageView(device, imageView, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreateShaderModule(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkShaderModuleCreateInfo local_pCreateInfo(ARG2);
    VkShaderModuleCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateShaderModule:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkShaderModule* pShaderModule = (VkShaderModule*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}
void vk_DestroyShaderModule(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkShaderModule shaderModule = *(VkShaderModule*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyShaderModule:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyShaderModule(device, shaderModule, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreatePipelineCache(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPipelineCacheCreateInfo local_pCreateInfo(ARG2);
    VkPipelineCacheCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreatePipelineCache:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkPipelineCache* pPipelineCache = (VkPipelineCache*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}
void vk_DestroyPipelineCache(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPipelineCache pipelineCache = *(VkPipelineCache*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyPipelineCache:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyPipelineCache(device, pipelineCache, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_GetPipelineCacheData(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPipelineCache pipelineCache = *(VkPipelineCache*)getPhysicalAddress(ARG2, 8);
;
    size_t* pDataSize = (size_t*)getPhysicalAddress(ARG3, 4);
    void* pData = (void*)getPhysicalAddress(ARG4, (U32)(pDataSize ? *pDataSize : 0));
    EAX = pBoxedInfo->pvkGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}
// return type: VkResult(4 bytes)
void vk_MergePipelineCaches(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPipelineCache dstCache = *(VkPipelineCache*)getPhysicalAddress(ARG2, 8);
;
    uint32_t srcCacheCount = (uint32_t)ARG3;
    VkPipelineCache* pSrcCaches = (VkPipelineCache*)getPhysicalAddress(ARG4, (U32)srcCacheCount * 4);
    EAX = pBoxedInfo->pvkMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}
// return type: VkResult(4 bytes)
void vk_CreateGraphicsPipelines(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
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
    EAX = pBoxedInfo->pvkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    if (pCreateInfos) {
        delete[] pCreateInfos;
    }
}
// return type: VkResult(4 bytes)
void vk_CreateComputePipelines(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
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
    EAX = pBoxedInfo->pvkCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    if (pCreateInfos) {
        delete[] pCreateInfos;
    }
}
void vk_DestroyPipeline(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPipeline pipeline = *(VkPipeline*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyPipeline:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyPipeline(device, pipeline, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreatePipelineLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPipelineLayoutCreateInfo local_pCreateInfo(ARG2);
    VkPipelineLayoutCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreatePipelineLayout:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkPipelineLayout* pPipelineLayout = (VkPipelineLayout*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}
void vk_DestroyPipelineLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPipelineLayout pipelineLayout = *(VkPipelineLayout*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyPipelineLayout:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreateSampler(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkSamplerCreateInfo local_pCreateInfo(ARG2);
    VkSamplerCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateSampler:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkSampler* pSampler = (VkSampler*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateSampler(device, pCreateInfo, pAllocator, pSampler);
}
void vk_DestroySampler(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkSampler sampler = *(VkSampler*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroySampler:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroySampler(device, sampler, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreateDescriptorSetLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkDescriptorSetLayoutCreateInfo local_pCreateInfo(ARG2);
    VkDescriptorSetLayoutCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateDescriptorSetLayout:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDescriptorSetLayout* pSetLayout = (VkDescriptorSetLayout*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}
void vk_DestroyDescriptorSetLayout(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDescriptorSetLayout descriptorSetLayout = *(VkDescriptorSetLayout*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyDescriptorSetLayout:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreateDescriptorPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkDescriptorPoolCreateInfo local_pCreateInfo(ARG2);
    VkDescriptorPoolCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateDescriptorPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDescriptorPool* pDescriptorPool = (VkDescriptorPool*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}
void vk_DestroyDescriptorPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDescriptorPool descriptorPool = *(VkDescriptorPool*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyDescriptorPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyDescriptorPool(device, descriptorPool, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_ResetDescriptorPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDescriptorPool descriptorPool = *(VkDescriptorPool*)getPhysicalAddress(ARG2, 8);
;
    VkDescriptorPoolResetFlags flags = (VkDescriptorPoolResetFlags)ARG3;
    EAX = pBoxedInfo->pvkResetDescriptorPool(device, descriptorPool, flags);
}
// return type: VkResult(4 bytes)
void vk_AllocateDescriptorSets(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkDescriptorSetAllocateInfo local_pAllocateInfo(ARG2);
    VkDescriptorSetAllocateInfo* pAllocateInfo = &local_pAllocateInfo.s;
    VkDescriptorSet* pDescriptorSets = (VkDescriptorSet*)getPhysicalAddress(ARG3, (U32)pAllocateInfo->descriptorSetCount * 4);
    EAX = pBoxedInfo->pvkAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}
// return type: VkResult(4 bytes)
void vk_FreeDescriptorSets(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDescriptorPool descriptorPool = *(VkDescriptorPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t descriptorSetCount = (uint32_t)ARG3;
    VkDescriptorSet* pDescriptorSets = (VkDescriptorSet*)getPhysicalAddress(ARG4, (U32)descriptorSetCount * 4);
    EAX = pBoxedInfo->pvkFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}
void vk_UpdateDescriptorSets(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
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
    pBoxedInfo->pvkUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    if (pDescriptorWrites) {
        delete[] pDescriptorWrites;
    }
    if (pDescriptorCopies) {
        delete[] pDescriptorCopies;
    }
}
// return type: VkResult(4 bytes)
void vk_CreateFramebuffer(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkFramebufferCreateInfo local_pCreateInfo(ARG2);
    VkFramebufferCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateFramebuffer:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkFramebuffer* pFramebuffer = (VkFramebuffer*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}
void vk_DestroyFramebuffer(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkFramebuffer framebuffer = *(VkFramebuffer*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyFramebuffer:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyFramebuffer(device, framebuffer, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_CreateRenderPass(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkRenderPassCreateInfo local_pCreateInfo(ARG2);
    VkRenderPassCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateRenderPass:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkRenderPass* pRenderPass = (VkRenderPass*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}
void vk_DestroyRenderPass(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkRenderPass renderPass = *(VkRenderPass*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyRenderPass:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyRenderPass(device, renderPass, pAllocator);
}
void vk_GetRenderAreaGranularity(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkRenderPass renderPass = *(VkRenderPass*)getPhysicalAddress(ARG2, 8);
;
    VkExtent2D* pGranularity = (VkExtent2D*)getPhysicalAddress(ARG3, 4);
    pBoxedInfo->pvkGetRenderAreaGranularity(device, renderPass, pGranularity);
}
// return type: VkResult(4 bytes)
void vk_CreateCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkCommandPoolCreateInfo local_pCreateInfo(ARG2);
    VkCommandPoolCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateCommandPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkCommandPool* pCommandPool = (VkCommandPool*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}
void vk_DestroyCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkCommandPool commandPool = *(VkCommandPool*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyCommandPool:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyCommandPool(device, commandPool, pAllocator);
}
// return type: VkResult(4 bytes)
void vk_ResetCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkCommandPool commandPool = *(VkCommandPool*)getPhysicalAddress(ARG2, 8);
;
    VkCommandPoolResetFlags flags = (VkCommandPoolResetFlags)ARG3;
    EAX = pBoxedInfo->pvkResetCommandPool(device, commandPool, flags);
}
// return type: VkResult(4 bytes)
void vk_AllocateCommandBuffers(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkCommandBufferAllocateInfo local_pAllocateInfo(ARG2);
    VkCommandBufferAllocateInfo* pAllocateInfo = &local_pAllocateInfo.s;
    VkCommandBuffer* pCommandBuffers = NULL;
    if (ARG3) {
        pCommandBuffers = new VkCommandBuffer[pAllocateInfo->commandBufferCount];
    }
    EAX = pBoxedInfo->pvkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    if (ARG3) {
        for (U32 i=0;i<pAllocateInfo->commandBufferCount;i++) {
            writed(ARG3 + i*4, createVulkanPtr((U64)pCommandBuffers[i], pBoxedInfo));
        }
        delete[] pCommandBuffers;
    }
}
void vk_FreeCommandBuffers(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkCommandPool commandPool = *(VkCommandPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t commandBufferCount = (uint32_t)ARG3;
    VkCommandBuffer* pCommandBuffers = new VkCommandBuffer[commandBufferCount];
    for (U32 i=0;i<commandBufferCount;i++) {
        pCommandBuffers[i] = (VkCommandBuffer)getVulkanPtr(readd(ARG4 + i*4));
    }
    pBoxedInfo->pvkFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    delete[] pCommandBuffers;
}
// return type: VkResult(4 bytes)
void vk_BeginCommandBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkCommandBufferBeginInfo local_pBeginInfo(ARG2);
    VkCommandBufferBeginInfo* pBeginInfo = &local_pBeginInfo.s;
    EAX = pBoxedInfo->pvkBeginCommandBuffer(commandBuffer, pBeginInfo);
}
// return type: VkResult(4 bytes)
void vk_EndCommandBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    EAX = pBoxedInfo->pvkEndCommandBuffer(commandBuffer);
}
// return type: VkResult(4 bytes)
void vk_ResetCommandBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkCommandBufferResetFlags flags = (VkCommandBufferResetFlags)ARG2;
    EAX = pBoxedInfo->pvkResetCommandBuffer(commandBuffer, flags);
}
void vk_CmdBindPipeline(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPipelineBindPoint pipelineBindPoint = (VkPipelineBindPoint)ARG2;
    VkPipeline pipeline = *(VkPipeline*)getPhysicalAddress(ARG3, 8);
;
    pBoxedInfo->pvkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}
void vk_CmdSetViewport(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t firstViewport = (uint32_t)ARG2;
    uint32_t viewportCount = (uint32_t)ARG3;
    VkViewport* pViewports = (VkViewport*)getPhysicalAddress(ARG4, (U32)viewportCount * 4);
    pBoxedInfo->pvkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}
void vk_CmdSetScissor(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t firstScissor = (uint32_t)ARG2;
    uint32_t scissorCount = (uint32_t)ARG3;
    VkRect2D* pScissors = (VkRect2D*)getPhysicalAddress(ARG4, (U32)scissorCount * 4);
    pBoxedInfo->pvkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}
void vk_CmdSetLineWidth(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    float lineWidth = (float)ARG2;
    pBoxedInfo->pvkCmdSetLineWidth(commandBuffer, lineWidth);
}
void vk_CmdSetDepthBias(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    float depthBiasConstantFactor = (float)ARG2;
    float depthBiasClamp = (float)ARG3;
    float depthBiasSlopeFactor = (float)ARG4;
    pBoxedInfo->pvkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
void vk_CmdSetBlendConstants(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    float* blendConstants = (float*)getPhysicalAddress(ARG2, 16);
    pBoxedInfo->pvkCmdSetBlendConstants(commandBuffer, blendConstants);
}
void vk_CmdSetDepthBounds(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    float minDepthBounds = (float)ARG2;
    float maxDepthBounds = (float)ARG3;
    pBoxedInfo->pvkCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}
void vk_CmdSetStencilCompareMask(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkStencilFaceFlags faceMask = (VkStencilFaceFlags)ARG2;
    uint32_t compareMask = (uint32_t)ARG3;
    pBoxedInfo->pvkCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}
void vk_CmdSetStencilWriteMask(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkStencilFaceFlags faceMask = (VkStencilFaceFlags)ARG2;
    uint32_t writeMask = (uint32_t)ARG3;
    pBoxedInfo->pvkCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}
void vk_CmdSetStencilReference(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkStencilFaceFlags faceMask = (VkStencilFaceFlags)ARG2;
    uint32_t reference = (uint32_t)ARG3;
    pBoxedInfo->pvkCmdSetStencilReference(commandBuffer, faceMask, reference);
}
void vk_CmdBindDescriptorSets(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPipelineBindPoint pipelineBindPoint = (VkPipelineBindPoint)ARG2;
    VkPipelineLayout layout = *(VkPipelineLayout*)getPhysicalAddress(ARG3, 8);
;
    uint32_t firstSet = (uint32_t)ARG4;
    uint32_t descriptorSetCount = (uint32_t)ARG5;
    VkDescriptorSet* pDescriptorSets = (VkDescriptorSet*)getPhysicalAddress(ARG6, (U32)descriptorSetCount * 4);
    uint32_t dynamicOffsetCount = (uint32_t)ARG7;
    uint32_t* pDynamicOffsets = (uint32_t*)getPhysicalAddress(ARG8, (U32)dynamicOffsetCount * 4);
    pBoxedInfo->pvkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}
void vk_CmdBindIndexBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkIndexType indexType = (VkIndexType)ARG4;
    pBoxedInfo->pvkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}
void vk_CmdBindVertexBuffers(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t firstBinding = (uint32_t)ARG2;
    uint32_t bindingCount = (uint32_t)ARG3;
    VkBuffer* pBuffers = (VkBuffer*)getPhysicalAddress(ARG4, (U32)bindingCount * 4);
    VkDeviceSize* pOffsets = (VkDeviceSize*)getPhysicalAddress(ARG5, (U32)bindingCount * 4);
    pBoxedInfo->pvkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}
void vk_CmdDraw(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t vertexCount = (uint32_t)ARG2;
    uint32_t instanceCount = (uint32_t)ARG3;
    uint32_t firstVertex = (uint32_t)ARG4;
    uint32_t firstInstance = (uint32_t)ARG5;
    pBoxedInfo->pvkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
void vk_CmdDrawIndexed(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t indexCount = (uint32_t)ARG2;
    uint32_t instanceCount = (uint32_t)ARG3;
    uint32_t firstIndex = (uint32_t)ARG4;
    int32_t vertexOffset = (int32_t)ARG5;
    uint32_t firstInstance = (uint32_t)ARG6;
    pBoxedInfo->pvkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
void vk_CmdDrawIndirect(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    uint32_t drawCount = (uint32_t)ARG4;
    uint32_t stride = (uint32_t)ARG5;
    pBoxedInfo->pvkCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}
void vk_CmdDrawIndexedIndirect(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    uint32_t drawCount = (uint32_t)ARG4;
    uint32_t stride = (uint32_t)ARG5;
    pBoxedInfo->pvkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}
void vk_CmdDispatch(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t groupCountX = (uint32_t)ARG2;
    uint32_t groupCountY = (uint32_t)ARG3;
    uint32_t groupCountZ = (uint32_t)ARG4;
    pBoxedInfo->pvkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}
void vk_CmdDispatchIndirect(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer buffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize offset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    pBoxedInfo->pvkCmdDispatchIndirect(commandBuffer, buffer, offset);
}
void vk_CmdCopyBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer srcBuffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkBuffer dstBuffer = *(VkBuffer*)getPhysicalAddress(ARG3, 8);
;
    uint32_t regionCount = (uint32_t)ARG4;
    VkBufferCopy* pRegions = (VkBufferCopy*)getPhysicalAddress(ARG5, (U32)regionCount * 4);
    pBoxedInfo->pvkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}
void vk_CmdCopyImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage srcImage = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG3;
    VkImage dstImage = *(VkImage*)getPhysicalAddress(ARG4, 8);
;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG5;
    uint32_t regionCount = (uint32_t)ARG6;
    VkImageCopy* pRegions = (VkImageCopy*)getPhysicalAddress(ARG7, (U32)regionCount * 4);
    pBoxedInfo->pvkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
void vk_CmdBlitImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage srcImage = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG3;
    VkImage dstImage = *(VkImage*)getPhysicalAddress(ARG4, 8);
;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG5;
    uint32_t regionCount = (uint32_t)ARG6;
    VkImageBlit* pRegions = (VkImageBlit*)getPhysicalAddress(ARG7, (U32)regionCount * 4);
    VkFilter filter = (VkFilter)ARG8;
    pBoxedInfo->pvkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}
void vk_CmdCopyBufferToImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer srcBuffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkImage dstImage = *(VkImage*)getPhysicalAddress(ARG3, 8);
;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG4;
    uint32_t regionCount = (uint32_t)ARG5;
    VkBufferImageCopy* pRegions = (VkBufferImageCopy*)getPhysicalAddress(ARG6, (U32)regionCount * 4);
    pBoxedInfo->pvkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}
void vk_CmdCopyImageToBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage srcImage = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG3;
    VkBuffer dstBuffer = *(VkBuffer*)getPhysicalAddress(ARG4, 8);
;
    uint32_t regionCount = (uint32_t)ARG5;
    VkBufferImageCopy* pRegions = (VkBufferImageCopy*)getPhysicalAddress(ARG6, (U32)regionCount * 4);
    pBoxedInfo->pvkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}
void vk_CmdUpdateBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer dstBuffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize dstOffset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize dataSize = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    void* pData = (void*)getPhysicalAddress(ARG5, (U32)dataSize * 8);
    pBoxedInfo->pvkCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}
void vk_CmdFillBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkBuffer dstBuffer = *(VkBuffer*)getPhysicalAddress(ARG2, 8);
;
    VkDeviceSize dstOffset = *(VkDeviceSize*)getPhysicalAddress(ARG3, 8);
;
    VkDeviceSize size = *(VkDeviceSize*)getPhysicalAddress(ARG4, 8);
;
    uint32_t data = (uint32_t)ARG5;
    pBoxedInfo->pvkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}
void vk_CmdClearColorImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout imageLayout = (VkImageLayout)ARG3;
    VkClearColorValue* pColor = (VkClearColorValue*)getPhysicalAddress(ARG4, 4);
    uint32_t rangeCount = (uint32_t)ARG5;
    VkImageSubresourceRange* pRanges = (VkImageSubresourceRange*)getPhysicalAddress(ARG6, (U32)rangeCount * 4);
    pBoxedInfo->pvkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}
void vk_CmdClearDepthStencilImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage image = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout imageLayout = (VkImageLayout)ARG3;
    VkClearDepthStencilValue* pDepthStencil = (VkClearDepthStencilValue*)getPhysicalAddress(ARG4, 4);
    uint32_t rangeCount = (uint32_t)ARG5;
    VkImageSubresourceRange* pRanges = (VkImageSubresourceRange*)getPhysicalAddress(ARG6, (U32)rangeCount * 4);
    pBoxedInfo->pvkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}
void vk_CmdClearAttachments(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t attachmentCount = (uint32_t)ARG2;
    VkClearAttachment* pAttachments = (VkClearAttachment*)getPhysicalAddress(ARG3, (U32)attachmentCount * 4);
    uint32_t rectCount = (uint32_t)ARG4;
    VkClearRect* pRects = (VkClearRect*)getPhysicalAddress(ARG5, (U32)rectCount * 4);
    pBoxedInfo->pvkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}
void vk_CmdResolveImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkImage srcImage = *(VkImage*)getPhysicalAddress(ARG2, 8);
;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG3;
    VkImage dstImage = *(VkImage*)getPhysicalAddress(ARG4, 8);
;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG5;
    uint32_t regionCount = (uint32_t)ARG6;
    VkImageResolve* pRegions = (VkImageResolve*)getPhysicalAddress(ARG7, (U32)regionCount * 4);
    pBoxedInfo->pvkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
void vk_CmdSetEvent(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    VkPipelineStageFlags stageMask = (VkPipelineStageFlags)ARG3;
    pBoxedInfo->pvkCmdSetEvent(commandBuffer, event, stageMask);
}
void vk_CmdResetEvent(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkEvent event = *(VkEvent*)getPhysicalAddress(ARG2, 8);
;
    VkPipelineStageFlags stageMask = (VkPipelineStageFlags)ARG3;
    pBoxedInfo->pvkCmdResetEvent(commandBuffer, event, stageMask);
}
void vk_CmdWaitEvents(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
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
    pBoxedInfo->pvkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    if (pMemoryBarriers) {
        delete[] pMemoryBarriers;
    }
    if (pBufferMemoryBarriers) {
        delete[] pBufferMemoryBarriers;
    }
    if (pImageMemoryBarriers) {
        delete[] pImageMemoryBarriers;
    }
}
void vk_CmdPipelineBarrier(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
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
    pBoxedInfo->pvkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    if (pMemoryBarriers) {
        delete[] pMemoryBarriers;
    }
    if (pBufferMemoryBarriers) {
        delete[] pBufferMemoryBarriers;
    }
    if (pImageMemoryBarriers) {
        delete[] pImageMemoryBarriers;
    }
}
void vk_CmdBeginQuery(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t query = (uint32_t)ARG3;
    VkQueryControlFlags flags = (VkQueryControlFlags)ARG4;
    pBoxedInfo->pvkCmdBeginQuery(commandBuffer, queryPool, query, flags);
}
void vk_CmdEndQuery(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t query = (uint32_t)ARG3;
    pBoxedInfo->pvkCmdEndQuery(commandBuffer, queryPool, query);
}
void vk_CmdResetQueryPool(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG2, 8);
;
    uint32_t firstQuery = (uint32_t)ARG3;
    uint32_t queryCount = (uint32_t)ARG4;
    pBoxedInfo->pvkCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}
void vk_CmdWriteTimestamp(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPipelineStageFlagBits pipelineStage = (VkPipelineStageFlagBits)ARG2;
    VkQueryPool queryPool = *(VkQueryPool*)getPhysicalAddress(ARG3, 8);
;
    uint32_t query = (uint32_t)ARG4;
    pBoxedInfo->pvkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}
void vk_CmdCopyQueryPoolResults(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
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
    pBoxedInfo->pvkCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}
void vk_CmdPushConstants(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkPipelineLayout layout = *(VkPipelineLayout*)getPhysicalAddress(ARG2, 8);
;
    VkShaderStageFlags stageFlags = (VkShaderStageFlags)ARG3;
    uint32_t offset = (uint32_t)ARG4;
    uint32_t size = (uint32_t)ARG5;
    void* pValues = (void*)getPhysicalAddress(ARG6, (U32)size * 4);
    pBoxedInfo->pvkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}
void vk_CmdBeginRenderPass(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkRenderPassBeginInfo local_pRenderPassBegin(ARG2);
    VkRenderPassBeginInfo* pRenderPassBegin = &local_pRenderPassBegin.s;
    VkSubpassContents contents = (VkSubpassContents)ARG3;
    pBoxedInfo->pvkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}
void vk_CmdNextSubpass(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkSubpassContents contents = (VkSubpassContents)ARG2;
    pBoxedInfo->pvkCmdNextSubpass(commandBuffer, contents);
}
void vk_CmdEndRenderPass(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    pBoxedInfo->pvkCmdEndRenderPass(commandBuffer);
}
void vk_CmdExecuteCommands(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t commandBufferCount = (uint32_t)ARG2;
    VkCommandBuffer* pCommandBuffers = new VkCommandBuffer[commandBufferCount];
    for (U32 i=0;i<commandBufferCount;i++) {
        pCommandBuffers[i] = (VkCommandBuffer)getVulkanPtr(readd(ARG3 + i*4));
    }
    pBoxedInfo->pvkCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    delete[] pCommandBuffers;
}
void vk_GetPhysicalDeviceFeatures2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPhysicalDeviceFeatures2 pFeatures(ARG2);
    pBoxedInfo->pvkGetPhysicalDeviceFeatures2(physicalDevice, &pFeatures.s);
    MarshalVkPhysicalDeviceFeatures2::write(ARG2, &pFeatures.s);
}
void vk_GetPhysicalDeviceProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPhysicalDeviceProperties2 pProperties(ARG2);
    pBoxedInfo->pvkGetPhysicalDeviceProperties2(physicalDevice, &pProperties.s);
    MarshalVkPhysicalDeviceProperties2::write(ARG2, &pProperties.s);
}
void vk_GetPhysicalDeviceFormatProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkFormat format = (VkFormat)ARG2;
    MarshalVkFormatProperties2 pFormatProperties(ARG3);
    pBoxedInfo->pvkGetPhysicalDeviceFormatProperties2(physicalDevice, format, &pFormatProperties.s);
    MarshalVkFormatProperties2::write(ARG3, &pFormatProperties.s);
}
// return type: VkResult(4 bytes)
void vk_GetPhysicalDeviceImageFormatProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPhysicalDeviceImageFormatInfo2 local_pImageFormatInfo(ARG2);
    VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo = &local_pImageFormatInfo.s;
    MarshalVkImageFormatProperties2 pImageFormatProperties(ARG3);
    EAX = pBoxedInfo->pvkGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, &pImageFormatProperties.s);
    MarshalVkImageFormatProperties2::write(ARG3, &pImageFormatProperties.s);
}
void vk_GetPhysicalDeviceQueueFamilyProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t* pQueueFamilyPropertyCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkQueueFamilyProperties2* pQueueFamilyProperties = NULL;
    if (ARG3) {
        pQueueFamilyProperties = new VkQueueFamilyProperties2[*pQueueFamilyPropertyCount];
    }
    pBoxedInfo->pvkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (U32 i=0;i<*pQueueFamilyPropertyCount;i++) {
        MarshalVkQueueFamilyProperties2::write(ARG3 + i * 32, &pQueueFamilyProperties[i]);
    }
    if (ARG3) {
        delete[] pQueueFamilyProperties;
    }
}
void vk_GetPhysicalDeviceMemoryProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPhysicalDeviceMemoryProperties2 pMemoryProperties(ARG2);
    pBoxedInfo->pvkGetPhysicalDeviceMemoryProperties2(physicalDevice, &pMemoryProperties.s);
    MarshalVkPhysicalDeviceMemoryProperties2::write(ARG2, &pMemoryProperties.s);
}
void vk_GetPhysicalDeviceSparseImageFormatProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPhysicalDeviceSparseImageFormatInfo2 local_pFormatInfo(ARG2);
    VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo = &local_pFormatInfo.s;
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG3, 4);
    VkSparseImageFormatProperties2* pProperties = NULL;
    if (ARG4) {
        pProperties = new VkSparseImageFormatProperties2[*pPropertyCount];
    }
    pBoxedInfo->pvkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    for (U32 i=0;i<*pPropertyCount;i++) {
        MarshalVkSparseImageFormatProperties2::write(ARG4 + i * 28, &pProperties[i]);
    }
    if (ARG4) {
        delete[] pProperties;
    }
}
void vk_TrimCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkCommandPool commandPool = *(VkCommandPool*)getPhysicalAddress(ARG2, 8);
;
    VkCommandPoolTrimFlags flags = (VkCommandPoolTrimFlags)ARG3;
    pBoxedInfo->pvkTrimCommandPool(device, commandPool, flags);
}
void vk_GetPhysicalDeviceExternalBufferProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPhysicalDeviceExternalBufferInfo local_pExternalBufferInfo(ARG2);
    VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo = &local_pExternalBufferInfo.s;
    MarshalVkExternalBufferProperties pExternalBufferProperties(ARG3);
    pBoxedInfo->pvkGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, &pExternalBufferProperties.s);
    MarshalVkExternalBufferProperties::write(ARG3, &pExternalBufferProperties.s);
}
void vk_GetPhysicalDeviceExternalSemaphoreProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPhysicalDeviceExternalSemaphoreInfo local_pExternalSemaphoreInfo(ARG2);
    VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo = &local_pExternalSemaphoreInfo.s;
    MarshalVkExternalSemaphoreProperties pExternalSemaphoreProperties(ARG3);
    pBoxedInfo->pvkGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, &pExternalSemaphoreProperties.s);
    MarshalVkExternalSemaphoreProperties::write(ARG3, &pExternalSemaphoreProperties.s);
}
void vk_GetPhysicalDeviceExternalFenceProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkPhysicalDeviceExternalFenceInfo local_pExternalFenceInfo(ARG2);
    VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo = &local_pExternalFenceInfo.s;
    MarshalVkExternalFenceProperties pExternalFenceProperties(ARG3);
    pBoxedInfo->pvkGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, &pExternalFenceProperties.s);
    MarshalVkExternalFenceProperties::write(ARG3, &pExternalFenceProperties.s);
}
// return type: VkResult(4 bytes)
void vk_EnumeratePhysicalDeviceGroups(CPU* cpu) {
    VkInstance instance = (VkInstance)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t* pPhysicalDeviceGroupCount = (uint32_t*)getPhysicalAddress(ARG2, 4);
    VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties = NULL;
    if (ARG3) {
        pPhysicalDeviceGroupProperties = new VkPhysicalDeviceGroupProperties[*pPhysicalDeviceGroupCount];
    }
    EAX = pBoxedInfo->pvkEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    for (U32 i=0;i<*pPhysicalDeviceGroupCount;i++) {
        MarshalVkPhysicalDeviceGroupProperties::write(ARG3 + i * 144, &pPhysicalDeviceGroupProperties[i]);
    }
    if (ARG3) {
        delete[] pPhysicalDeviceGroupProperties;
    }
}
void vk_GetDeviceGroupPeerMemoryFeatures(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t heapIndex = (uint32_t)ARG2;
    uint32_t localDeviceIndex = (uint32_t)ARG3;
    uint32_t remoteDeviceIndex = (uint32_t)ARG4;
    VkPeerMemoryFeatureFlags* pPeerMemoryFeatures = (VkPeerMemoryFeatureFlags*)getPhysicalAddress(ARG5, 4);
    pBoxedInfo->pvkGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
// return type: VkResult(4 bytes)
void vk_BindBufferMemory2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t bindInfoCount = (uint32_t)ARG2;
    VkBindBufferMemoryInfo* pBindInfos = NULL;
    if (ARG3) {
        pBindInfos = new VkBindBufferMemoryInfo[bindInfoCount];
    }
    EAX = pBoxedInfo->pvkBindBufferMemory2(device, bindInfoCount, pBindInfos);
    if (pBindInfos) {
        delete[] pBindInfos;
    }
}
// return type: VkResult(4 bytes)
void vk_BindImageMemory2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t bindInfoCount = (uint32_t)ARG2;
    VkBindImageMemoryInfo* pBindInfos = NULL;
    if (ARG3) {
        pBindInfos = new VkBindImageMemoryInfo[bindInfoCount];
    }
    EAX = pBoxedInfo->pvkBindImageMemory2(device, bindInfoCount, pBindInfos);
    if (pBindInfos) {
        delete[] pBindInfos;
    }
}
void vk_CmdSetDeviceMask(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t deviceMask = (uint32_t)ARG2;
    pBoxedInfo->pvkCmdSetDeviceMask(commandBuffer, deviceMask);
}
void vk_CmdDispatchBase(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    uint32_t baseGroupX = (uint32_t)ARG2;
    uint32_t baseGroupY = (uint32_t)ARG3;
    uint32_t baseGroupZ = (uint32_t)ARG4;
    uint32_t groupCountX = (uint32_t)ARG5;
    uint32_t groupCountY = (uint32_t)ARG6;
    uint32_t groupCountZ = (uint32_t)ARG7;
    pBoxedInfo->pvkCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
// return type: VkResult(4 bytes)
void vk_CreateDescriptorUpdateTemplate(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkDescriptorUpdateTemplateCreateInfo local_pCreateInfo(ARG2);
    VkDescriptorUpdateTemplateCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateDescriptorUpdateTemplate:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate = (VkDescriptorUpdateTemplate*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
void vk_DestroyDescriptorUpdateTemplate(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDescriptorUpdateTemplate descriptorUpdateTemplate = *(VkDescriptorUpdateTemplate*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroyDescriptorUpdateTemplate:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}
void vk_UpdateDescriptorSetWithTemplate(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkDescriptorSet descriptorSet = *(VkDescriptorSet*)getPhysicalAddress(ARG2, 8);
;
    VkDescriptorUpdateTemplate descriptorUpdateTemplate = *(VkDescriptorUpdateTemplate*)getPhysicalAddress(ARG3, 8);
;
    void* pData = (void*)getPhysicalAddress(ARG4, 4);
    pBoxedInfo->pvkUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}
void vk_GetBufferMemoryRequirements2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkBufferMemoryRequirementsInfo2 local_pInfo(ARG2);
    VkBufferMemoryRequirementsInfo2* pInfo = &local_pInfo.s;
    MarshalVkMemoryRequirements2 pMemoryRequirements(ARG3);
    pBoxedInfo->pvkGetBufferMemoryRequirements2(device, pInfo, &pMemoryRequirements.s);
    MarshalVkMemoryRequirements2::write(ARG3, &pMemoryRequirements.s);
}
void vk_GetImageMemoryRequirements2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkImageMemoryRequirementsInfo2 local_pInfo(ARG2);
    VkImageMemoryRequirementsInfo2* pInfo = &local_pInfo.s;
    MarshalVkMemoryRequirements2 pMemoryRequirements(ARG3);
    pBoxedInfo->pvkGetImageMemoryRequirements2(device, pInfo, &pMemoryRequirements.s);
    MarshalVkMemoryRequirements2::write(ARG3, &pMemoryRequirements.s);
}
void vk_GetImageSparseMemoryRequirements2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkImageSparseMemoryRequirementsInfo2 local_pInfo(ARG2);
    VkImageSparseMemoryRequirementsInfo2* pInfo = &local_pInfo.s;
    uint32_t* pSparseMemoryRequirementCount = (uint32_t*)getPhysicalAddress(ARG3, 4);
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements = NULL;
    if (ARG4) {
        pSparseMemoryRequirements = new VkSparseImageMemoryRequirements2[*pSparseMemoryRequirementCount];
    }
    pBoxedInfo->pvkGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (U32 i=0;i<*pSparseMemoryRequirementCount;i++) {
        MarshalVkSparseImageMemoryRequirements2::write(ARG4 + i * 56, &pSparseMemoryRequirements[i]);
    }
    if (ARG4) {
        delete[] pSparseMemoryRequirements;
    }
}
// return type: VkResult(4 bytes)
void vk_CreateSamplerYcbcrConversion(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkSamplerYcbcrConversionCreateInfo local_pCreateInfo(ARG2);
    VkSamplerYcbcrConversionCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateSamplerYcbcrConversion:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkSamplerYcbcrConversion* pYcbcrConversion = (VkSamplerYcbcrConversion*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}
void vk_DestroySamplerYcbcrConversion(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkSamplerYcbcrConversion ycbcrConversion = *(VkSamplerYcbcrConversion*)getPhysicalAddress(ARG2, 8);
;
    static bool shown; if (!shown && ARG3) { klog("vkDestroySamplerYcbcrConversion:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    pBoxedInfo->pvkDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}
void vk_GetDeviceQueue2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkDeviceQueueInfo2 local_pQueueInfo(ARG2);
    VkDeviceQueueInfo2* pQueueInfo = &local_pQueueInfo.s;
    VkQueue pQueue;
    pBoxedInfo->pvkGetDeviceQueue2(device, pQueueInfo, &pQueue);
    writed(ARG3, createVulkanPtr((U64)pQueue, pBoxedInfo));
}
void vk_GetDescriptorSetLayoutSupport(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkDescriptorSetLayoutCreateInfo local_pCreateInfo(ARG2);
    VkDescriptorSetLayoutCreateInfo* pCreateInfo = &local_pCreateInfo.s;
    MarshalVkDescriptorSetLayoutSupport pSupport(ARG3);
    pBoxedInfo->pvkGetDescriptorSetLayoutSupport(device, pCreateInfo, &pSupport.s);
    MarshalVkDescriptorSetLayoutSupport::write(ARG3, &pSupport.s);
}
// return type: VkResult(4 bytes)
void vk_CreateRenderPass2(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkRenderPassCreateInfo2 local_pCreateInfo(ARG2);
    VkRenderPassCreateInfo2* pCreateInfo = &local_pCreateInfo.s;
    static bool shown; if (!shown && ARG3) { klog("vkCreateRenderPass2:VkAllocationCallbacks not implemented"); shown = true;}
    VkAllocationCallbacks* pAllocator = NULL;
    VkRenderPass* pRenderPass = (VkRenderPass*)getPhysicalAddress(ARG4, 4);
    EAX = pBoxedInfo->pvkCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
}
void vk_CmdBeginRenderPass2(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkRenderPassBeginInfo local_pRenderPassBegin(ARG2);
    VkRenderPassBeginInfo* pRenderPassBegin = &local_pRenderPassBegin.s;
    MarshalVkSubpassBeginInfo local_pSubpassBeginInfo(ARG3);
    VkSubpassBeginInfo* pSubpassBeginInfo = &local_pSubpassBeginInfo.s;
    pBoxedInfo->pvkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
void vk_CmdNextSubpass2(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkSubpassBeginInfo local_pSubpassBeginInfo(ARG2);
    VkSubpassBeginInfo* pSubpassBeginInfo = &local_pSubpassBeginInfo.s;
    MarshalVkSubpassEndInfo local_pSubpassEndInfo(ARG3);
    VkSubpassEndInfo* pSubpassEndInfo = &local_pSubpassEndInfo.s;
    pBoxedInfo->pvkCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
void vk_CmdEndRenderPass2(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkSubpassEndInfo local_pSubpassEndInfo(ARG2);
    VkSubpassEndInfo* pSubpassEndInfo = &local_pSubpassEndInfo.s;
    pBoxedInfo->pvkCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}
// return type: VkResult(4 bytes)
void vk_GetSemaphoreCounterValue(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    VkSemaphore semaphore = *(VkSemaphore*)getPhysicalAddress(ARG2, 8);
;
    uint64_t* pValue = (uint64_t*)getPhysicalAddress(ARG3, 4);
    EAX = pBoxedInfo->pvkGetSemaphoreCounterValue(device, semaphore, pValue);
}
// return type: VkResult(4 bytes)
void vk_WaitSemaphores(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkSemaphoreWaitInfo local_pWaitInfo(ARG2);
    VkSemaphoreWaitInfo* pWaitInfo = &local_pWaitInfo.s;
    uint64_t timeout = *(uint64_t*)getPhysicalAddress(ARG3, 8);
;
    EAX = pBoxedInfo->pvkWaitSemaphores(device, pWaitInfo, timeout);
}
// return type: VkResult(4 bytes)
void vk_SignalSemaphore(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkSemaphoreSignalInfo local_pSignalInfo(ARG2);
    VkSemaphoreSignalInfo* pSignalInfo = &local_pSignalInfo.s;
    EAX = pBoxedInfo->pvkSignalSemaphore(device, pSignalInfo);
}
void vk_CmdDrawIndirectCount(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
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
    pBoxedInfo->pvkCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
void vk_CmdDrawIndexedIndirectCount(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
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
    pBoxedInfo->pvkCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
// return type: uint64_t(8 bytes)
void vk_GetBufferOpaqueCaptureAddress(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkBufferDeviceAddressInfo local_pInfo(ARG2);
    VkBufferDeviceAddressInfo* pInfo = &local_pInfo.s;
    uint64_t result = pBoxedInfo->pvkGetBufferOpaqueCaptureAddress(device, pInfo);
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}
// return type: VkDeviceAddress(8 bytes)
void vk_GetBufferDeviceAddress(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkBufferDeviceAddressInfo local_pInfo(ARG2);
    VkBufferDeviceAddressInfo* pInfo = &local_pInfo.s;
    VkDeviceAddress result = pBoxedInfo->pvkGetBufferDeviceAddress(device, pInfo);
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}
// return type: uint64_t(8 bytes)
void vk_GetDeviceMemoryOpaqueCaptureAddress(CPU* cpu) {
    VkDevice device = (VkDevice)getVulkanPtr(ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(ARG1);
    MarshalVkDeviceMemoryOpaqueCaptureAddressInfo local_pInfo(ARG2);
    VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo = &local_pInfo.s;
    uint64_t result = pBoxedInfo->pvkGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}
void* vulkanGetNextPtr(U32 address) {
    if (address == 0) {
        return NULL;
    }
    VkStructureType type = (VkStructureType)readd(address);
    switch (type) {
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT: {
            VkPhysicalDeviceTransformFeedbackFeaturesEXT* p = new VkPhysicalDeviceTransformFeedbackFeaturesEXT();
            MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT::read(address, p);
            return p;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT: {
            VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* p = new VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT();
            MarshalVkPhysicalDeviceVertexAttributeDivisorFeaturesEXT::read(address, p);
            return p;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES: {
            VkPhysicalDeviceIDProperties* p = new VkPhysicalDeviceIDProperties();
            MarshalVkPhysicalDeviceIDProperties::read(address, p);
            return p;
        }
       default:
            kpanic("vulkanGetNextPtr not implemented for %d", type);
    }
}
void vulkanWriteNextPtr(U32 address, void* p) {
    if (address == 0) {
        return;
    }
    VkStructureType type = (VkStructureType)readd(address);
    switch (type) {
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT: {
            MarshalVkPhysicalDeviceTransformFeedbackFeaturesEXT::write(address, (VkPhysicalDeviceTransformFeedbackFeaturesEXT*)p);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT: {
            MarshalVkPhysicalDeviceVertexAttributeDivisorFeaturesEXT::write(address, (VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT*)p);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES: {
            MarshalVkPhysicalDeviceIDProperties::write(address, (VkPhysicalDeviceIDProperties*)p);
            break;
        }
       default:
            kpanic("vulkanWriteNextPtr not implemented for %d", type);
    }
}
#endif

