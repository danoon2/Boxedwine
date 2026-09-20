/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#ifdef BOXEDWINE_VULKAN
#include "../../source/x11/x11.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include "kvulkanSDL.h"
#include <unordered_map>

class KVulkdanSDLImpl : public KVulkan {
public:
	KVulkdanSDLImpl(const KNativeScreenSDLPtr& screen) : screen(screen) {}
    ~KVulkdanSDLImpl() override;
	KNativeScreenSDLPtr screen;

	void* createVulkanSurface(const XWindowPtr& wnd, void* instance) override;
    void resizeWindow(const XWindowPtr& wnd) override;
    void showWindow(const XWindowPtr& wnd, bool show) override;
    void destroyVulkanSurface(void* surface) override;
    void focusWindow(U32 nativeId) override;
    bool warpMouse(S32 x, S32 y) override;
    void restoreInput();
    U32 savedWidth = 0, savedHeight = 0, savedScaleX = 0, savedScaleY = 0, savedOffsetX = 0, savedOffsetY = 0;
    std::weak_ptr<XWindow> inputWindow;
    struct NativeWindow {
        SDL_Window* window;
        std::weak_ptr<XWindow> guest;
        ~NativeWindow() { SDL_DestroyWindow(window); }
    };
    // All window/map access runs on SDL's thread, including surface creation on
    // macOS. Several VkSurfaces for one drawable share its native window.
    std::unordered_map<void*, std::shared_ptr<NativeWindow>> surfaces;
};

KVulkdanSDLImpl::~KVulkdanSDLImpl() {
    screen->input->runOnUiThread([this]() { surfaces.clear(); });
}

void KVulkdanSDLImpl::resizeWindow(const XWindowPtr& wnd) {
    screen->input->runOnUiThread([this, wnd]() {
        for (const auto& entry : surfaces) if (entry.second->guest.lock() == wnd) {
            SDL_SetWindowSize(entry.second->window, std::max(1u, wnd->width()), std::max(1u, wnd->height()));
            if (SDL_GetKeyboardFocus() == entry.second->window) focusWindow(SDL_GetWindowID(entry.second->window));
            break;
        }
    });
}

void KVulkdanSDLImpl::showWindow(const XWindowPtr& wnd, bool show) {
    screen->input->runOnUiThread([this, wnd, show]() {
        for (const auto& entry : surfaces) if (entry.second->guest.lock() == wnd) {
            if (show) SDL_ShowWindow(entry.second->window);
            else SDL_HideWindow(entry.second->window);
            break;
        }
    });
}

void KVulkdanSDLImpl::focusWindow(U32 nativeId) {
    // Called from the SDL focus event, or another operation already on its thread.
    for (const auto& entry : surfaces) if (SDL_GetWindowID(entry.second->window) == nativeId) {
        auto wnd = entry.second->guest.lock();
        if (wnd && XServer::getServer(true)) {
            XServer::getServer()->setFakeFullScreenWindow(wnd);
            if (!savedWidth) {
                savedWidth = screen->input->screenWidth(); savedHeight = screen->input->screenHeight();
                savedScaleX = screen->input->scaleX; savedScaleY = screen->input->scaleY;
                savedOffsetX = screen->input->scaleXOffset; savedOffsetY = screen->input->scaleYOffset;
            }
            inputWindow = wnd;
            screen->input->scaleX = screen->input->scaleY = 100;
            screen->input->scaleXOffset = screen->input->scaleYOffset = 0;
            screen->input->setScreenSize(std::max(1u, wnd->width()), std::max(1u, wnd->height()));
        }
        return;
    }
    restoreInput();
}

void KVulkdanSDLImpl::restoreInput() {
    if (!savedWidth) return;
    if (XServer::getServer(true)) XServer::getServer()->clearFakeFullScreenWindow(inputWindow.lock());
    inputWindow.reset();
    screen->input->scaleX = savedScaleX; screen->input->scaleY = savedScaleY;
    screen->input->scaleXOffset = savedOffsetX; screen->input->scaleYOffset = savedOffsetY;
    screen->input->setScreenSize(savedWidth, savedHeight);
    savedWidth = 0;
}

bool KVulkdanSDLImpl::warpMouse(S32 x, S32 y) {
    bool handled = false;
    screen->input->runOnUiThread([this, x, y, &handled]() {
        auto wnd = inputWindow.lock();
        if (!wnd) return;
        for (const auto& entry : surfaces) if (entry.second->guest.lock() == wnd) {
            SDL_WarpMouseInWindow(entry.second->window, x, y);
            handled = true;
            break;
        }
    });
    return handled;
}

void KVulkdanSDLImpl::destroyVulkanSurface(void* surface) {
    screen->input->runOnUiThread([this, surface]() {
        auto found = surfaces.find(surface);
        if (found == surfaces.end()) return;
        auto window = found->second;
        surfaces.erase(found);
        if (window.use_count() == 1 && window->guest.lock() == inputWindow.lock()) restoreInput();
        if (window.use_count() == 1 && XServer::getServer(true))
            XServer::getServer()->clearFakeFullScreenWindow(window->guest.lock());
        window.reset();
        if (surfaces.empty()) screen->showWindow(true);
    });
}

void* KVulkdanSDLImpl::createVulkanSurface(const XWindowPtr& wnd, void* instance) {
    VkSurfaceKHR result = 0;
    screen->input->runOnUiThread([this, wnd, instance, &result]() {
        std::shared_ptr<NativeWindow> window;
        for (const auto& entry : surfaces) if (entry.second->guest.lock() == wnd) {
            window = entry.second;
            break;
        }
        if (!window) {
            S32 x = 0, y = 0;
            screen->getPos(x, y);
            const char* title = KSystem::title.length() ? KSystem::title.c_str() : "BoxedWine Vulkan";
            U32 flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN;
            SDL_DisplayMode display = {};
            if (SDL_GetDesktopDisplayMode(0, &display) == 0 && wnd->width() == (U32)display.w && wnd->height() == (U32)display.h)
                flags |= SDL_WINDOW_BORDERLESS;
            SDL_Window* native = SDL_CreateWindow(title, x, y, std::max(1u, wnd->width()), std::max(1u, wnd->height()), flags);
            if (!native) { kwarn_fmt("Failed to create Vulkan window: %s", SDL_GetError()); return; }
            window = std::make_shared<NativeWindow>();
            window->window = native;
            window->guest = wnd;
        }
        if (!SDL_Vulkan_CreateSurface(window->window, (VkInstance)instance, &result)) {
            kwarn_fmt("Failed to create Vulkan surface: %s", SDL_GetError());
            result = 0;
            return;
        }
        surfaces[(void*)result] = window;
        screen->showWindow(false);
        SDL_ShowWindow(window->window);
        focusWindow(SDL_GetWindowID(window->window));
    });
    return (void*)result;
}

KVulkanPtr KVulkanSDL::create(const KNativeScreenSDLPtr& screen) {
	return std::make_shared<KVulkdanSDLImpl>(screen);
}

#endif
