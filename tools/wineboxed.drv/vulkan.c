#if 0
MAKE_DEP_UNIX
#endif

#include "wineboxed.h"
#include "wine/gdi_driver.h"
#include "wine/debug.h"

#include <dlfcn.h>

#ifdef BOXEDWINE_VULKAN
#include "wine/vulkan.h"
#include "wine/vulkan_driver.h"
#endif

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

#if WINE_VULKAN_DRIVER_VERSION >= 7
static void* (*pvkGetDeviceProcAddr)(VkDevice, const char*);
static void* (*pvkGetInstanceProcAddr)(VkInstance, const char*);

static void* vulkan_handle;

static BOOL wine_vk_init(void)
{
    if (!(vulkan_handle = dlopen("/lib/libvulkan.so.1", RTLD_NOW)))
    {
        ERR("Failed to load libvulkan.so.1.\n");
        return TRUE;
    }
#define LOAD_FUNCPTR(f) p##f = dlsym(vulkan_handle, #f);
    LOAD_FUNCPTR(vkGetDeviceProcAddr)
        LOAD_FUNCPTR(vkGetInstanceProcAddr)
#undef LOAD_FUNCPTR
#undef LOAD_OPTIONAL_FUNCPTR
        return TRUE;

}

static VkResult boxedwine_vkCreateInstance(const VkInstanceCreateInfo* create_info, const VkAllocationCallbacks* allocator, VkInstance* instance)
{
    int result;
    TRACE("create_info %p, allocator %p, instance %p\n", create_info, allocator, instance);
    CALL_3(1, create_info, allocator, instance);
    return (VkResult)result;
}

static VkResult boxedwine_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* create_info, const VkAllocationCallbacks* allocator, VkSwapchainKHR* swapchain)
{
    int result;
    TRACE("%p %p %p %p\n", device, create_info, allocator, swapchain);
    CALL_4(160, device, create_info, allocator, swapchain);
    return (VkResult)result;
}

static VkResult boxedwine_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* create_info, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
{
    int result;
    TRACE("%p %p %p %p\n", instance, create_info, allocator, surface);
    CALL_4(168, instance, create_info, allocator, surface);
    return (VkResult)result;
}

static void boxedwine_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* allocator)
{
    TRACE("%p %p\n", instance, allocator);
    CALL_NORETURN_2(2, instance, allocator);
}

static void boxedwine_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* allocator)
{
    TRACE("%p 0x%s %p\n", instance, wine_dbgstr_longlong(surface), allocator);
    CALL_NORETURN_3(155, instance, &surface, allocator);
}

static void boxedwine_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* allocator)
{
    TRACE("%p, 0x%s %p\n", device, wine_dbgstr_longlong(swapchain), allocator);
    CALL_NORETURN_3(161, device, &swapchain, allocator);
}

static VkResult boxedwine_vkEnumerateInstanceExtensionProperties(const char* layer_name, uint32_t* count, VkExtensionProperties* properties)
{
    int result;
    TRACE("layer_name %s, count %p, properties %p\n", debugstr_a(layer_name), count, properties);
    CALL_3(16, layer_name, count, properties);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* flags)
{
    int result;
    TRACE("%p, 0x%s, %p\n", device, wine_dbgstr_longlong(surface), flags);
    CALL_3(240, device, &surface, flags);
    return (VkResult)result;
}

static void* boxedwine_vkGetDeviceProcAddr(VkDevice device, const char* name)
{
    TRACE("%p, %s\n", device, debugstr_a(name));
    return pvkGetDeviceProcAddr(device, name);
}

static void* boxedwine_vkGetInstanceProcAddr(VkInstance instance, const char* name)
{
    TRACE("%p, %s\n", instance, debugstr_a(name));
    return pvkGetInstanceProcAddr(instance, name);
}

static VkResult boxedwine_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, uint32_t* count, VkRect2D* rects)
{
    int result;
    TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, rects);
    CALL_4(243, phys_dev, &surface, count, rects);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* capabilities)
{
    int result;
    TRACE("%p, 0x%s, %p\n", phys_dev, wine_dbgstr_longlong(surface), capabilities);
    CALL_3(157, phys_dev, &surface, capabilities);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice phys_dev, const VkPhysicalDeviceSurfaceInfo2KHR* surface_info, VkSurfaceCapabilities2KHR* capabilities)
{
    int result;
    TRACE("%p, %p, %p\n", phys_dev, surface_info, capabilities);
    CALL_3(259, phys_dev, surface_info, capabilities);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, uint32_t* count, VkSurfaceFormatKHR* formats)
{
    int result;
    TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, formats);
    CALL_4(158, phys_dev, &surface, count, formats);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice phys_dev, const VkPhysicalDeviceSurfaceInfo2KHR* surface_info, uint32_t* count, VkSurfaceFormat2KHR* formats)
{
    int result;
    TRACE("%p, %p, %p, %p\n", phys_dev, surface_info, count, formats);
    CALL_4(260, phys_dev, surface_info, count, formats);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, uint32_t* count, VkPresentModeKHR* modes)
{
    int result;
    TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, modes);
    CALL_4(159, phys_dev, &surface, count, modes);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice phys_dev, uint32_t index, VkSurfaceKHR surface, VkBool32* supported)
{
    int result;
    TRACE("%p, %u, 0x%s, %p\n", phys_dev, index, wine_dbgstr_longlong(surface), supported);
    CALL_4(156, phys_dev, index, &surface, supported);
    return (VkResult)result;
}

static VkBool32 boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice phys_dev, uint32_t index)
{
    int result;
    TRACE("%p %u\n", phys_dev, index);
    CALL_2(169, phys_dev, index);
    return (VkBool32)result;
}

static VkResult boxedwine_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* count, VkImage* images)
{
    int result;
    TRACE("%p, 0x%s %p %p\n", device, wine_dbgstr_longlong(swapchain), count, images);
    CALL_4(162, device, &swapchain, count, images);
    return (VkResult)result;
}

static VkResult boxedwine_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* present_info)
{
    int result;
    TRACE("%p, %p\n", queue, present_info);
    CALL_2(164, queue, present_info);
    return (VkResult)result;
}

#if WINE_VULKAN_DRIVER_VERSION >= 10
static VkSurfaceKHR boxedwine_wine_get_native_surface(VkSurfaceKHR surface)
{
    TRACE("0x%s\n", wine_dbgstr_longlong(surface));
    return surface;
}
#endif

#if WINE_VULKAN_DRIVER_VERSION == 7
static const struct vulkan_funcs vulkan_funcs =
{
    boxeddrv_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceGroupSurfacePresentModesKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,
};
#elif WINE_VULKAN_DRIVER_VERSION == 8
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceGroupSurfacePresentModesKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,
};
#elif WINE_VULKAN_DRIVER_VERSION == 9
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceGroupSurfacePresentModesKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,
};
#else
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceGroupSurfacePresentModesKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface,
};
#endif

// WINE_VULKAN_DRIVER_VERSION
// 7  Jul 16, 2018 (Wine 3.13)
// 8  Apr 7, 2020 (Wine 6.0)
// 9  Jan 22, 2021 (Wine 6.1)
// 10 Jan 26, 2021 (Wine 6.1)

#if WINE_VULKAN_DRIVER_VERSION >= 11
const struct vulkan_funcs* boxeddrv_wine_get_vulkan_driver(UINT version)
#elif WINE_GDI_DRIVER_VERSION >= 74
const struct vulkan_funcs* CDECL boxeddrv_wine_get_vulkan_driver(UINT version)
#else
const struct vulkan_funcs* CDECL boxeddrv_wine_get_vulkan_driver(PHYSDEV hdc, UINT version)
#endif
{
    TRACE("version %d\n", version);
    if (version != WINE_VULKAN_DRIVER_VERSION)
    {
        ERR("version mismatch, vulkan wants %u but boxeddrv has %u\n", version, WINE_VULKAN_DRIVER_VERSION);
        return NULL;
    }

    if (wine_vk_init())
        return &vulkan_funcs;
    return NULL;
}
#endif