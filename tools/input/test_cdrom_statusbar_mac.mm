/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#import <Cocoa/Cocoa.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>
#include "vulkan.h"
#include "sdlStatusBar.h"
#include <cassert>
#include <cstdio>
#include <initializer_list>
#include <vector>

static NSWindow* native(SDL_Window* window) {
    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    assert(SDL_GetWindowWMInfo(window, &info) && info.subsystem == SDL_SYSWM_COCOA);
    return info.info.cocoa.window;
}

void testCocoaStatusBar(SDL_Window* guest, SDL_Window* host, SDL_Window* popup) {
    bool sawQuit = false;
    auto pump = [&]() {
        Uint32 until = SDL_GetTicks() + 100;
        do {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) sawQuit = true;
                if (getenv("BOXEDWINE_STATUSBAR_TEST_TRACE")) {
                    if (event.type == SDL_MOUSEMOTION) fprintf(stderr, "motion %u %d,%d\n", event.motion.windowID, event.motion.x, event.motion.y);
                    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) fprintf(stderr, "button %u %u %d,%d\n", event.type, event.button.windowID, event.button.x, event.button.y);
                    if (event.type == SDL_WINDOWEVENT) fprintf(stderr, "window %u %u\n", event.window.windowID, event.window.event);
                }
                sdlStatusBarEvent(event);
            }
            sdlStatusBarTick();
            SDL_Delay(5);
        } while ((Sint32)(SDL_GetTicks() - until) < 0);
    };
    auto visible = [&]() { return (SDL_GetWindowFlags(popup) & SDL_WINDOW_SHOWN) != 0; };
    auto click = [&](SDL_Window* window, int x, int y) {
        NSWindow* target = native(window);
        NSPoint position = NSMakePoint(x, NSHeight(target.contentView.bounds) - y);
        for (NSEventType type : {NSEventTypeMouseMoved, NSEventTypeLeftMouseDown, NSEventTypeLeftMouseUp}) {
            NSEvent* event = [NSEvent mouseEventWithType:type location:position modifierFlags:0
                timestamp:NSProcessInfo.processInfo.systemUptime windowNumber:target.windowNumber
                context:nil eventNumber:0 clickCount:1 pressure:type == NSEventTypeLeftMouseDown ? 1 : 0];
            [NSApp postEvent:event atStart:NO];
            pump();
        }
    };
    NSWindow* child = native(guest);
    NSWindow* parent = native(host);
    assert(child.parentWindow == parent && native(popup).parentWindow == parent);
    assert(!(child.styleMask & NSWindowStyleMaskTitled));
    assert(!child.hasShadow && child.excludedFromWindowsMenu);
    NSRect content = [parent contentRectForFrameRect:parent.frame];
    assert(NSWidth(child.frame) == 640 && NSHeight(child.frame) == 480);
    assert(NSMinX(child.frame) == NSMinX(content) && NSMaxY(child.frame) == NSMaxY(content));
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    assert(visible());
    click(popup, 100, 52); assert(!visible()); // Eject
    click(host, 100, 495); assert(visible());
    click(guest, 200, 150); assert(!visible());
    assert(SDL_GetKeyboardFocus() == guest);
    NSPoint old = child.frame.origin;
    [parent setFrameOrigin:NSMakePoint(parent.frame.origin.x + 37, parent.frame.origin.y - 23)];
    pump();
    assert(child.frame.origin.x == old.x + 37 && child.frame.origin.y == old.y - 23);
    NSMenuItem* minimize = nil;
    NSMenuItem* close = nil;
    for (NSMenuItem* item in NSApp.windowsMenu.itemArray) {
        if (item.action == @selector(performMiniaturize:)) minimize = item;
        if (item.action == @selector(performClose:)) close = item;
    }
    assert(minimize && close);
    [NSApp sendAction:minimize.action to:minimize.target from:minimize];
    pump();
    assert(parent.isMiniaturized && !child.isVisible);
    [parent deminiaturize:nil];
    pump();
    assert(!parent.isMiniaturized && child.isVisible);
    [NSApp sendAction:close.action to:close.target from:close];
    pump();
    assert(sawQuit);
    click(host, 100, 495); assert(visible());
    puts("Cocoa popup input, hierarchy, focus, movement, and standard Close/Minimize checks passed");
}

void testCocoaVulkanStatusBar() {
    assert(SDL_Vulkan_LoadLibrary("../../lib/mac/vulkan/lib/libMoltenVK.dylib") == 0);
    SDL_Window* guest = SDL_CreateWindow("Vulkan status bar harness", 20, 20, 640, 480,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);
    assert(guest);
    unsigned count = 0;
    assert(SDL_Vulkan_GetInstanceExtensions(guest, &count, nullptr));
    std::vector<const char*> extensions(count);
    assert(SDL_Vulkan_GetInstanceExtensions(guest, &count, extensions.data()));
    VkInstanceCreateInfo info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    info.enabledExtensionCount = count;
    info.ppEnabledExtensionNames = extensions.data();
    VkInstance instance;
    assert(vkCreateInstance(&info, nullptr, &instance) == VK_SUCCESS);
    VkSurfaceKHR surface;
    assert(SDL_Vulkan_CreateSurface(guest, instance, &surface));
    assert(vkEnumeratePhysicalDevices(instance, &count, nullptr) == VK_SUCCESS && count);
    std::vector<VkPhysicalDevice> devices(count);
    assert(vkEnumeratePhysicalDevices(instance, &count, devices.data()) == VK_SUCCESS);
    VkSurfaceCapabilitiesKHR before, after;
    assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[0], surface, &before) == VK_SUCCESS);
    sdlStatusBarShowWindow(guest, true);
    sdlStatusBarOpen();
    sdlStatusBarTick();
    SDL_Window* host = nullptr;
    SDL_Window* popup = nullptr;
    for (unsigned id = 1; id < 32; ++id) {
        SDL_Window* candidate = SDL_GetWindowFromID(id);
        if (!candidate || !sdlStatusBarOwns(candidate)) continue;
        if (!strcmp(SDL_GetWindowTitle(candidate), "Vulkan status bar harness")) host = candidate;
        if (!strcmp(SDL_GetWindowTitle(candidate), "Boxedwine CD-ROM menu")) popup = candidate;
    }
    assert(host && popup);
    testCocoaStatusBar(guest, host, popup);
    assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[0], surface, &after) == VK_SUCCESS);
    assert(before.currentExtent.width == after.currentExtent.width && before.currentExtent.height == after.currentExtent.height);
    int width, height;
    SDL_Vulkan_GetDrawableSize(guest, &width, &height);
    assert(width == (int)after.currentExtent.width && height == (int)after.currentExtent.height);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    sdlStatusBarWindowDestroyed(guest);
    SDL_DestroyWindow(guest);
    vkDestroyInstance(instance, nullptr);
    SDL_Vulkan_UnloadLibrary();
    puts("Cocoa Vulkan surface, drawable size, and status bar lifecycle checks passed");
}
