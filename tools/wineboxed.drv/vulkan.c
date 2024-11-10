#if 0
MAKE_DEP_UNIX
#endif

#include "wineboxed.h"
#include "wine/gdi_driver.h"
#include "wine/debug.h"

#include <dlfcn.h>

#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "windef.h"
#include "winbase.h"

#ifdef BOXEDWINE_VULKAN
#include "wine/vulkan.h"
#include "wine/vulkan_driver.h"
#endif

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

#if WINE_VULKAN_DRIVER_VERSION >= 7

// keep in sync with winedrv.cpp
#define BOXED_BASE 0

#define BOXED_VK_CREATE_INSTANCE                    (BOXED_BASE+89)
#define BOXED_VK_CREATE_SWAPCHAIN                   (BOXED_BASE+90)
#define BOXED_VK_CREATE_SURFACE                     (BOXED_BASE+91)
#define BOXED_VK_DESTROY_INSTANCE                   (BOXED_BASE+92)
#define BOXED_VK_DESTROY_SURFACE                    (BOXED_BASE+93)
#define BOXED_VK_DESTROY_SWAPCHAIN                  (BOXED_BASE+94)
#define BOXED_VK_ENUMERATE_INSTANCE_EXTENSION_PROPERTIES (BOXED_BASE+95)
#define BOXED_VK_GET_DEVICE_GROUP_SURFACE_PRESENT_MODES  (BOXED_BASE+96)
#define BOXED_VK_GET_PHYSICAL_DEVICE_PRESENT_RECTANGLES  (BOXED_BASE+97)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_CAPABILITIES   (BOXED_BASE+98)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_FORMATS (BOXED_BASE+99)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_PRESENT_MODES (BOXED_BASE+100)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_SUPPORT (BOXED_BASE+101)
#define BOXED_VK_GET_PHYSICAL_DEVICE_WIN32_PRESENTATION_SUPPORT (BOXED_BASE+102)
#define BOXED_VK_GET_SWAPCHAIN_IMAGES                (BOXED_BASE+103)
#define BOXED_VK_QUEUE_PRESENT                       (BOXED_BASE+104)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_CAPABILITIES2   (BOXED_BASE+105)
#define BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_FORMATS2 (BOXED_BASE+106)
#define BOXED_VK_GET_NATIVE_SURFACE                  (BOXED_BASE+107)
#define BOXED_VK_GET_HOST_EXTENSION				(BOXED_BASE+110)
#define BOXED_VK_QUEUE_PRESENT2				(BOXED_BASE+111)
#define BOXED_VK_VULKAN_SURFACE_PRESENTED   (BOXED_BASE+112)
#define BOXED_VK_DESTROY_SURFACE2           (BOXED_BASE+113)
#define BOXED_VK_CREATE_SURFACE2            (BOXED_BASE+114)
#define BOXED_VK_DETATCH_SURFACE            (BOXED_BASE+115)

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
    CALL_3(BOXED_VK_CREATE_INSTANCE, create_info, allocator, instance);
    return (VkResult)result;
}

static VkResult boxedwine_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* create_info, const VkAllocationCallbacks* allocator, VkSwapchainKHR* swapchain)
{
    int result;
    TRACE("%p %p %p %p\n", device, create_info, allocator, swapchain);
    CALL_4(BOXED_VK_CREATE_SWAPCHAIN, device, create_info, allocator, swapchain);
    return (VkResult)result;
}

static VkResult boxedwine_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* create_info, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
{
    int result;
    TRACE("%p %p %p %p\n", instance, create_info, allocator, surface);
    CALL_4(BOXED_VK_CREATE_SURFACE, instance, create_info, allocator, surface);
    return (VkResult)result;
}

#if WINE_VULKAN_DRIVER_VERSION >= 33
static VkResult boxedwine_vulkan_surface_create(HWND hwnd, VkInstance instance, VkSurfaceKHR* surface, void** privateData)
{
    int result;
    TRACE("%p %p %p %p\n", (int)hwnd, instance, surface, privateData);
    CALL_4(BOXED_VK_CREATE_SURFACE2, hwnd, instance, surface, privateData);
    return (VkResult)result;
}
#elif WINE_VULKAN_DRIVER_VERSION >= 31
static VkResult boxedwine_vulkan_surface_create(HWND hwnd, VkInstance instance, VkSurfaceKHR* surface)
{
    int result;
    TRACE("%p %p %p\n", hwnd, instance, surface);
    CALL_4(BOXED_VK_CREATE_SURFACE, instance, 0, 0, surface);
    return (VkResult)result;
}
#else
static VkResult boxedwine_vulkan_surface_create(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* create_info, VkSurfaceKHR* surface)
{
    int result;
    TRACE("%p %p %p\n", instance, create_info, surface);
    CALL_4(BOXED_VK_CREATE_SURFACE, instance, create_info, 0, surface);
    return (VkResult)result;
}
#endif

static void boxedwine_vulkan_surface_detach(HWND hwnd, void* privateData) {
    TRACE("%p %p\n", (int)hwnd, privateData);
    CALL_NORETURN_2(BOXED_VK_DETATCH_SURFACE, (int)hwnd, privateData);
}

static void boxedwine_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* allocator)
{
    TRACE("%p %p\n", instance, allocator);
    CALL_NORETURN_2(BOXED_VK_DESTROY_INSTANCE, instance, allocator);
}

static void boxedwine_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* allocator) {
    TRACE("%p, 0x%s %p\n", instance, wine_dbgstr_longlong(surface), allocator);
    CALL_NORETURN_3(BOXED_VK_DESTROY_SURFACE, instance, &surface, allocator);
}

#if WINE_VULKAN_DRIVER_VERSION >= 33
static void boxedwine_vulkan_surface_destroy(HWND hwnd, void* privateData)
{
    TRACE("%p %p\n", (int)hwnd, privateData);
    CALL_NORETURN_2(BOXED_VK_DESTROY_SURFACE2, (int)hwnd, privateData);
}
#elif WINE_VULKAN_DRIVER_VERSION >= 32
static void boxedwine_vulkan_surface_destroy(HWND hwnd, VkSurfaceKHR surface)
{
    TRACE("%p 0x%s\n", hwnd, wine_dbgstr_longlong(surface));
    CALL_NORETURN_3(BOXED_VK_DESTROY_SURFACE, 0, &surface, 0);
}
#elif WINE_VULKAN_DRIVER_VERSION == 31
static void boxedwine_vulkan_surface_destroy(HWND hwnd, VkInstance instance, VkSurfaceKHR surface)
{
    TRACE("%p %p 0x%s\n", hwnd, instance, wine_dbgstr_longlong(surface));
    CALL_NORETURN_3(BOXED_VK_DESTROY_SURFACE, instance, &surface, 0);
}
#else
static void boxedwine_vulkan_surface_destroy(VkInstance instance, VkSurfaceKHR surface)
{
    TRACE("%p 0x%s\n", instance, wine_dbgstr_longlong(surface));
    CALL_NORETURN_3(BOXED_VK_DESTROY_SURFACE, instance, &surface, 0);
}
#endif

static void boxedwine_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* allocator)
{
    TRACE("%p, 0x%s %p\n", device, wine_dbgstr_longlong(swapchain), allocator);
    CALL_NORETURN_3(BOXED_VK_DESTROY_SWAPCHAIN, device, &swapchain, allocator);
}

static VkResult boxedwine_vkEnumerateInstanceExtensionProperties(const char* layer_name, uint32_t* count, VkExtensionProperties* properties)
{
    int result;
    TRACE("layer_name %s, count %p, properties %p\n", debugstr_a(layer_name), count, properties);
    CALL_3(BOXED_VK_ENUMERATE_INSTANCE_EXTENSION_PROPERTIES, layer_name, count, properties);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* flags)
{
    int result;
    TRACE("%p, 0x%s, %p\n", device, wine_dbgstr_longlong(surface), flags);
    CALL_3(BOXED_VK_GET_DEVICE_GROUP_SURFACE_PRESENT_MODES, device, &surface, flags);
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
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_PRESENT_RECTANGLES, phys_dev, &surface, count, rects);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* capabilities)
{
    int result;
    TRACE("%p, 0x%s, %p\n", phys_dev, wine_dbgstr_longlong(surface), capabilities);
    CALL_3(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_CAPABILITIES, phys_dev, &surface, capabilities);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice phys_dev, const VkPhysicalDeviceSurfaceInfo2KHR* surface_info, VkSurfaceCapabilities2KHR* capabilities)
{
    int result;
    TRACE("%p, %p, %p\n", phys_dev, surface_info, capabilities);
    CALL_3(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_CAPABILITIES2, phys_dev, surface_info, capabilities);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, uint32_t* count, VkSurfaceFormatKHR* formats)
{
    int result;
    TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, formats);
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_FORMATS, phys_dev, &surface, count, formats);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice phys_dev, const VkPhysicalDeviceSurfaceInfo2KHR* surface_info, uint32_t* count, VkSurfaceFormat2KHR* formats)
{
    int result;
    TRACE("%p, %p, %p, %p\n", phys_dev, surface_info, count, formats);
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_FORMATS2, phys_dev, surface_info, count, formats);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice phys_dev, VkSurfaceKHR surface, uint32_t* count, VkPresentModeKHR* modes)
{
    int result;
    TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, modes);
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_PRESENT_MODES, phys_dev, &surface, count, modes);
    return (VkResult)result;
}

static VkResult boxedwine_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice phys_dev, uint32_t index, VkSurfaceKHR surface, VkBool32* supported)
{
    int result;
    TRACE("%p, %u, 0x%s, %p\n", phys_dev, index, wine_dbgstr_longlong(surface), supported);
    CALL_4(BOXED_VK_GET_PHYSICAL_DEVICE_SURFACE_SUPPORT, phys_dev, index, &surface, supported);
    return (VkResult)result;
}

static VkBool32 boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice phys_dev, uint32_t index)
{
    int result;
    TRACE("%p %u\n", phys_dev, index);
    CALL_2(BOXED_VK_GET_PHYSICAL_DEVICE_WIN32_PRESENTATION_SUPPORT, phys_dev, index);
    return (VkBool32)result;
}

static VkResult boxedwine_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* count, VkImage* images)
{
    int result;
    TRACE("%p, 0x%s %p %p\n", device, wine_dbgstr_longlong(swapchain), count, images);
    CALL_4(BOXED_VK_GET_SWAPCHAIN_IMAGES, device, &swapchain, count, images);
    return (VkResult)result;
}

#if WINE_VULKAN_DRIVER_VERSION >= 24
static VkResult boxedwine_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* present_info, HWND* surfaces)
{
    int result;
    TRACE("%p, %p, %p\n", queue, present_info, surfaces);
    CALL_3(BOXED_VK_QUEUE_PRESENT2, queue, present_info, surfaces);
    return (VkResult)result;
}
#else
static VkResult boxedwine_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* present_info)
{
    int result;
    TRACE("%p, %p\n", queue, present_info);
    CALL_2(BOXED_VK_QUEUE_PRESENT, queue, present_info);
    return (VkResult)result;
}
#endif

#if WINE_VULKAN_DRIVER_VERSION >= 10
static VkSurfaceKHR boxedwine_wine_get_native_surface(VkSurfaceKHR surface)
{
    int result;
    TRACE("%s\n", wine_dbgstr_longlong(surface));
    CALL_1(BOXED_VK_GET_NATIVE_SURFACE, surface);
    return (VkSurfaceKHR)result;
}
#endif

#if WINE_VULKAN_DRIVER_VERSION >= 20
static const char* boxedwine_get_host_surface_extension() {
    int result;
    TRACE("\n");
    CALL_0(BOXED_VK_GET_HOST_EXTENSION);
    return (const char*)result;
}
#endif

#if WINE_VULKAN_DRIVER_VERSION >= 25
static void boxedwine_vulkan_surface_presented(HWND hwnd, VkResult result) {
    TRACE("%p, %p\n", (int)hwnd, result);
    CALL_NORETURN_2(BOXED_VK_VULKAN_SURFACE_PRESENTED, (int)hwnd, result);
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
#elif WINE_VULKAN_DRIVER_VERSION == 10
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

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 10
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

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 11
// no change, removed CDECL from __wine_get_vulkan_driver
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

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 12
// removed vkGetPhysicalDeviceSurfaceSupportKHR
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
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 13
// removed vkGetPhysicalDeviceSurfacePresentModesKHR
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
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 14
// removed vkGetDeviceGroupSurfacePresentModesKHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilities2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 15
// removed vkGetDeviceGroupSurfacePresentModesKHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 16
// removed vkGetPhysicalDeviceSurfaceCapabilitiesKHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDevicePresentRectanglesKHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 17
// removed vkGetPhysicalDevicePresentRectanglesKHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceSurfaceFormats2KHR,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 18
// removed vkGetPhysicalDeviceSurfaceFormats2KHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceSurfaceFormatsKHR,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 19
// removed vkGetPhysicalDeviceSurfaceFormatsKHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkEnumerateInstanceExtensionProperties,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 20
// removed vkEnumerateInstanceExtensionProperties
// added get_host_surface_extension
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateInstance,
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_get_host_surface_extension,
    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 21
// removed vkCreateInstance
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroyInstance,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_get_host_surface_extension,
    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 22
// removed vkDestroyInstance
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkGetSwapchainImagesKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_get_host_surface_extension,
    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 23
// removed vkGetSwapchainImagesKHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_get_host_surface_extension,
    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 24
// added parameter to boxedwine_vkQueuePresentKHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_get_host_surface_extension,
    boxedwine_wine_get_native_surface
};
#elif WINE_VULKAN_DRIVER_VERSION == 25
// added boxedwine_vulkan_surface_presented
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkDestroySwapchainKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_get_host_surface_extension,
    boxedwine_wine_get_native_surface,
    boxedwine_vulkan_surface_presented
};
#elif WINE_VULKAN_DRIVER_VERSION == 26
// removed vkDestroySwapchainKHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateSwapchainKHR,
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_get_host_surface_extension,
    boxedwine_wine_get_native_surface,
    boxedwine_vulkan_surface_presented
};
#elif WINE_VULKAN_DRIVER_VERSION == 27
// removed vkCreateSwapchainKHR
static const struct vulkan_funcs vulkan_funcs =
{
    boxedwine_vkCreateWin32SurfaceKHR,
    boxedwine_vkDestroySurfaceKHR,
    boxedwine_vkGetDeviceProcAddr,
    boxedwine_vkGetInstanceProcAddr,
    boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    boxedwine_vkQueuePresentKHR,

    boxedwine_get_host_surface_extension,
    boxedwine_wine_get_native_surface,
    boxedwine_vulkan_surface_presented
};
#elif WINE_VULKAN_DRIVER_VERSION == 28 || WINE_VULKAN_DRIVER_VERSION == 29
static const struct vulkan_driver_funcs vulkan_funcs =
{
    .p_vkCreateWin32SurfaceKHR = boxedwine_vkCreateWin32SurfaceKHR,
    .p_vkDestroySurfaceKHR = boxedwine_vkDestroySurfaceKHR,
    .p_vulkan_surface_presented = boxedwine_vulkan_surface_presented,

    .p_vkGetPhysicalDeviceWin32PresentationSupportKHR = boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    .p_get_host_surface_extension = boxedwine_get_host_surface_extension,
    .p_wine_get_host_surface = boxedwine_wine_get_native_surface,
};
#elif WINE_VULKAN_DRIVER_VERSION >= 30 && WINE_VULKAN_DRIVER_VERSION <= 32
static const struct vulkan_driver_funcs vulkan_funcs =
{
    .p_vulkan_surface_create = boxedwine_vulkan_surface_create,
    .p_vulkan_surface_destroy = boxedwine_vulkan_surface_destroy,
    .p_vulkan_surface_presented = boxedwine_vulkan_surface_presented,

    .p_vkGetPhysicalDeviceWin32PresentationSupportKHR = boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    .p_get_host_surface_extension = boxedwine_get_host_surface_extension,
    .p_wine_get_host_surface = boxedwine_wine_get_native_surface,
};
#elif WINE_VULKAN_DRIVER_VERSION == 33
static const struct vulkan_driver_funcs vulkan_funcs =
{
    .p_vulkan_surface_create = boxedwine_vulkan_surface_create,
    .p_vulkan_surface_destroy = boxedwine_vulkan_surface_destroy,
    .p_vulkan_surface_presented = boxedwine_vulkan_surface_presented,

    .p_vkGetPhysicalDeviceWin32PresentationSupportKHR = boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    .p_get_host_surface_extension = boxedwine_get_host_surface_extension,
};
#elif WINE_VULKAN_DRIVER_VERSION == 34
static const struct vulkan_driver_funcs vulkan_funcs =
{
    .p_vulkan_surface_create = boxedwine_vulkan_surface_create,
    .p_vulkan_surface_destroy = boxedwine_vulkan_surface_destroy,
    .p_vulkan_surface_detach = boxedwine_vulkan_surface_detach,
    .p_vulkan_surface_presented = boxedwine_vulkan_surface_presented,

    .p_vkGetPhysicalDeviceWin32PresentationSupportKHR = boxedwine_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    .p_get_host_surface_extension = boxedwine_get_host_surface_extension,
};
#endif

// WINE_VULKAN_DRIVER_VERSION
// 7  Jul 16, 2018 (Wine 3.13)
// 8  Apr 7, 2020 (Wine 6.0)
// 9  Jan 22, 2021 (Wine 6.1)
// 10 Jan 26, 2021 (Wine 6.1)
// 11 Jul 27, 2022 (Wine 7.14)
// 12 Feb 1, 2024 (Wine 9.2)
// 13 Feb 1, 2024 (Wine 9.2)
// 14 Feb 1, 2024 (Wine 9.2)
// 15 Feb 19, 2024 (Wine 9.3)
// 16 Feb 19, 2024 (Wine 9.3)
// 17 Feb 21, 2024 (Wine 9.3)
// 18 Feb 22, 2024 (Wine 9.3)
// 19 Feb 22, 2024 (Wine 9.3)
// 20 Mar 28, 2024 (Wine 9.6)
// 21 Mar 28, 2024 (Wine 9.6)
// 22 Mar 28, 2024 (Wine 9.6)
// 23 Apr 9, 2024 (Wine 9.7)
// 24 Apr 11, 2024 (Wine 9.7)
// 25 Apr 11, 2024 (Wine 9.7)
// 26 Apr 11, 2024 (Wine 9.7)
// 27 Apr 11, 2024 (Wine 9.7)
// 28 Apr 25, 2024 (Wine 9.8)
// 29 Apr 25, 2024 (Wine 9.8) // no changes in this file
// 30 Apr 25, 2024 (Wine 9.8) // p_vkCreateWin32SurfaceKHR -> p_vulkan_surface_create, p_vkDestroySurfaceKHR -> p_vulkan_surface_destroy
// 31 Apr 25, 2024 (Wine 9.8) // added HWND to p_vulkan_surface_create and p_vulkan_surface_destroy
// 32 Apr 25, 2024 (Wine 9.8) // removed VkInstance instance from p_vulkan_surface_destroy
// 33 May 1, 2024 (Wine 9.8) // added privateData to p_vulkan_surface_create and p_vulkan_surface_destroy, removed p_wine_get_host_surface
// 34 May 1, 2024 (Wine 9.8) // added p_vulkan_surface_detach

#if BOXED_WINE_VERSION >= 9050
#if WINE_VULKAN_DRIVER_VERSION >= 28
UINT boxeddrv_VulkanInit(UINT version, void* vulkan_handle, const struct vulkan_driver_funcs** driver_funcs) {
    TRACE("version %d\n", version);
    if (version != WINE_VULKAN_DRIVER_VERSION) {
        ERR("version mismatch, win32u wants %u but boxeddrv has %u\n", version, WINE_VULKAN_DRIVER_VERSION);
        return STATUS_INVALID_PARAMETER;
    }

    if (!wine_vk_init()) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }
    *driver_funcs = &vulkan_funcs;
    return STATUS_SUCCESS;
}
#else
UINT boxeddrv_VulkanInit(UINT version, void* vulkan_handle, struct vulkan_funcs* driver_funcs) {
    TRACE("version %d\n", version);
    if (version != WINE_VULKAN_DRIVER_VERSION) {
        ERR("version mismatch, win32u wants %u but boxeddrv has %u\n", version, WINE_VULKAN_DRIVER_VERSION);
        return STATUS_INVALID_PARAMETER;
    }

    if (!wine_vk_init()) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }
    *driver_funcs = vulkan_funcs;
    return STATUS_SUCCESS;
}
#endif

#else

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

#endif