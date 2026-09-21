/* Execute the generated i386 exports on Linux, intercepting int 9a locally.
 * gcc -m32 -O2 -Wall -Wextra -Werror testGuestABI.c -ldl -o testGuestABI
 * ./testGuestABI ./libvulkan.so.1
 * No Vulkan driver or BoxedWine is used: this tests the compiled guest ABI. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>
#define VK_NO_PROTOTYPES
#include "../../source/vulkan/vk/vulkan.h"
#include "vkdef.h"

static uint32_t command, words[12], word_count, calls;

static void trap(int signal_number, siginfo_t *info, void *context)
{
    ucontext_t *u = context;
    const unsigned char *code = (void *)u->uc_mcontext.gregs[REG_EIP];
    const uint32_t *stack = (void *)u->uc_mcontext.gregs[REG_ESP];
    uint32_t actual_command;
    (void)signal_number;
    (void)info;
    memcpy(&actual_command, code + 2, 4);
    if (code[0] != 0xcd || code[1] != 0x9a || code[6] != 0xc3 ||
        actual_command != command || !stack[0] || memcmp(stack + 1, words, word_count * 4))
        _exit(2);
    ++calls;
    u->uc_mcontext.gregs[REG_EIP] += 6;
    u->uc_mcontext.gregs[REG_EAX] = 0x55667788;
    u->uc_mcontext.gregs[REG_EDX] = 0x12345678;
}

static void expect(uint32_t index, const uint32_t *args, uint32_t count)
{
    command = index;
    word_count = count;
    memcpy(words, args, count * 4);
}

#define LOAD(name) PFN_vk##name p##name = (PFN_vk##name)dlsym(library, "vk" #name); \
    if (!p##name) { fprintf(stderr, "Missing vk%s\n", #name); return 1; }
#define EXPECT(name, ...) do { const uint32_t args[] = {__VA_ARGS__}; \
    expect(name, args, sizeof(args) / sizeof(args[0])); } while (0)

int main(int argc, char **argv)
{
    struct sigaction action = {0};
    if (argc != 2) return 1;
    void *library = dlopen(argv[1], RTLD_NOW | RTLD_GLOBAL);
    if (!library) { fprintf(stderr, "%s\n", dlerror()); return 1; }
    action.sa_sigaction = trap;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGSEGV, &action, NULL)) return 1;
    LOAD(MapMemory);
    LOAD(CmdCopyBuffer);
    LOAD(CmdSetDepthBounds);
    LOAD(CmdSetBlendConstants);
    LOAD(GetBufferDeviceAddress);
    LOAD(GetInstanceProcAddr);
    LOAD(GetDeviceProcAddr);
    void *mapped;
    EXPECT(MapMemory, 0x11223344, 0x89abcdef, 0x76543210, 0xfedcba98, 0xabcdef01,
           0xffffffff, 0xffffffff, 7, (uint32_t)&mapped);
    if ((uint32_t)pMapMemory((VkDevice)0x11223344, 0x7654321089abcdefULL,
                            0xabcdef01fedcba98ULL, VK_WHOLE_SIZE, 7, &mapped) != 0x55667788) return 3;
    VkBufferCopy copy = {0};
    EXPECT(CmdCopyBuffer, 0x1234, 0x87654321, 0x12345678, 0xfedcba98, 0xabcdef01, 1, (uint32_t)&copy);
    pCmdCopyBuffer((VkCommandBuffer)0x1234, 0x1234567887654321ULL, 0xabcdef01fedcba98ULL, 1, &copy);
    EXPECT(CmdSetDepthBounds, 0x1234, 0x3e800000, 0x3f400000);
    pCmdSetDepthBounds((VkCommandBuffer)0x1234, 0.25f, 0.75f);
    float blend[4] = {0};
    EXPECT(CmdSetBlendConstants, 0x1234, (uint32_t)blend);
    pCmdSetBlendConstants((VkCommandBuffer)0x1234, blend);
    VkBufferDeviceAddressInfo address = {0};
    EXPECT(GetBufferDeviceAddress, 0x1234, (uint32_t)&address);
    if (pGetBufferDeviceAddress((VkDevice)0x1234, &address) != 0x1234567855667788ULL) return 4;
    const char *name = "vkMapMemory";
    EXPECT(GetInstanceProcAddr, 0, (uint32_t)name);
    if (pGetInstanceProcAddr(VK_NULL_HANDLE, name) != (PFN_vkVoidFunction)pMapMemory) return 5;
    EXPECT(GetDeviceProcAddr, 0x1234, (uint32_t)name);
    if (pGetDeviceProcAddr((VkDevice)0x1234, name) != (PFN_vkVoidFunction)pMapMemory) return 6;
    if (calls != 7) return 7;
    dlclose(library);
    puts("PASS: 7 guest ABI calls, inline 64-bit values, floats, arrays, EDX:EAX, proc lookup");
    return 0;
}
