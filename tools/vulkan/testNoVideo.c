/* Linux regression: optional Vulkan probes must not terminate a GDI guest
 * when -novideo leaves SDL video uninitialized. Use the full Wine filesystem.
 * From this directory:
 *   gcc -m32 -O2 -Wall -Wextra -Werror testNoVideo.c -ldl -o testNoVideo
 *   mkdir -p /tmp/vulkan-no-video/root/home/username
 *   cp testNoVideo /tmp/vulkan-no-video/root/home/username/
 *   boxedwine -novideo -nosound -root /tmp/vulkan-no-video/root \
 *     -zip /path/to/TinyCore15Wine11.0.zip /home/username/testNoVideo
 * Require VULKAN_NO_VIDEO_PASS in the output; Boxedwine's normal host exit is 1.
 */
#include <dlfcn.h>
#include <stdio.h>
#define VK_NO_PROTOTYPES
#include "../../source/vulkan/vk/vulkan.h"

#define LOAD(name) PFN_vk##name p##name = (PFN_vk##name)dlsym(library, "vk" #name); \
    if (!p##name) { fprintf(stderr, "Missing vk%s\n", #name); return 1; }
#define CHECK(expression) do { if (!(expression)) { \
    fprintf(stderr, "FAIL: %s\n", #expression); return 1; } } while (0)

int main(void)
{
    void *library = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_GLOBAL);
    if (!library) { fprintf(stderr, "%s\n", dlerror()); return 1; }
    LOAD(GetInstanceProcAddr);
    LOAD(EnumerateInstanceVersion);
    LOAD(EnumerateInstanceExtensionProperties);
    LOAD(EnumerateInstanceLayerProperties);
    LOAD(CreateInstance);
    for (int attempt = 0; attempt < 2; ++attempt) {
        uint32_t count = 0, version = 0;
        VkInstance instance = VK_NULL_HANDLE;
        VkInstanceCreateInfo info = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        /* Match Wine's display probe: resolve the global function, then call it. */
        PFN_vkCreateInstance create = (PFN_vkCreateInstance)pGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
        CHECK(create == pCreateInstance);
        CHECK(pGetInstanceProcAddr(VK_NULL_HANDLE, "vkGetInstanceProcAddr") == (PFN_vkVoidFunction)pGetInstanceProcAddr);
        CHECK(pGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion") == (PFN_vkVoidFunction)pEnumerateInstanceVersion);
        CHECK(pGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties") == (PFN_vkVoidFunction)pEnumerateInstanceExtensionProperties);
        CHECK(pGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceLayerProperties") == (PFN_vkVoidFunction)pEnumerateInstanceLayerProperties);
        CHECK(pGetInstanceProcAddr(VK_NULL_HANDLE, "vkDestroyInstance") == NULL);
        CHECK(pGetInstanceProcAddr(VK_NULL_HANDLE, "vkUnknownFunction") == NULL);
        CHECK(pEnumerateInstanceVersion(&version) == VK_ERROR_INITIALIZATION_FAILED);
        CHECK(pEnumerateInstanceExtensionProperties(NULL, &count, NULL) == VK_ERROR_INITIALIZATION_FAILED);
        CHECK(pEnumerateInstanceLayerProperties(&count, NULL) == VK_ERROR_INITIALIZATION_FAILED);
        CHECK(create(&info, NULL, &instance) == VK_ERROR_INITIALIZATION_FAILED);
    }
    dlclose(library);
    puts("VULKAN_NO_VIDEO_PASS");
    return 0;
}
