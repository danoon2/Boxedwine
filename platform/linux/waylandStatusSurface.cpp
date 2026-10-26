/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "waylandStatusSurface.h"
#if defined(__linux__) && defined(SDL_VIDEO_DRIVER_WAYLAND) && !defined(__TEST)
#include <wayland-client.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <limits>
#include <vector>

namespace {
bool windowInfo(SDL_Window* window, SDL_SysWMinfo& info) {
    SDL_VERSION(&info.version);
    return window && SDL_GetWindowWMInfo(window, &info) && info.subsystem == SDL_SYSWM_WAYLAND;
}

struct CompatScaling {
    // sdl2-compat can expose pixel coordinates even though Wayland surfaces
    // use logical coordinates. SDL2 has no query for that emulation scale.
    // Use SDL3's public API only after verifying it owns the same window.
    void* library = SDL_LoadObject("libSDL3.so.0");
    SDL_Window* (*windowFromId)(Uint32) = library ?
        reinterpret_cast<decltype(windowFromId)>(SDL_LoadFunction(library, "SDL_GetWindowFromID")) : nullptr;
    float (*displayScale)(SDL_Window*) = library ?
        reinterpret_cast<decltype(displayScale)>(SDL_LoadFunction(library, "SDL_GetWindowDisplayScale")) : nullptr;
    ~CompatScaling() { if (library) SDL_UnloadObject(library); }
};

struct Connection {
    wl_display* display = nullptr; // Owned by SDL.
    wl_registry* registry = nullptr;
    wl_subcompositor* compositor = nullptr;
    wl_compositor* surfaceCompositor = nullptr;
    wl_shm* shm = nullptr;
    static void global(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t) {
        auto& self = *static_cast<Connection*>(data);
        if (!strcmp(interface, wl_compositor_interface.name)) {
            self.surfaceCompositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 1));
        } else if (!strcmp(interface, wl_subcompositor_interface.name)) {
            self.compositor = static_cast<wl_subcompositor*>(wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
        } else if (!strcmp(interface, wl_shm_interface.name)) {
            self.shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
        }
    }
    static void removed(void*, wl_registry*, uint32_t) {}
    bool init(wl_display* value) {
        display = value;
        registry = wl_display_get_registry(display);
        static const wl_registry_listener listener = {global, removed};
        wl_registry_add_listener(registry, &listener, this);
        return wl_display_roundtrip(display) >= 0 && compositor && shm;
    }
    ~Connection() {
        if (compositor) wl_subcompositor_destroy(compositor);
        if (surfaceCompositor) wl_compositor_destroy(surfaceCompositor);
        if (shm) wl_shm_destroy(shm);
        if (registry) wl_registry_destroy(registry);
    }
};
std::weak_ptr<Connection> connection;

struct Buffer {
    wl_buffer* buffer = nullptr;
    void* pixels = MAP_FAILED;
    size_t size = 0;
    int width = 0, height = 0;
    bool busy = false;
    static void released(void* data, wl_buffer*) { static_cast<Buffer*>(data)->busy = false; }
    bool init(wl_shm* shm, int w, int h) {
        if (w <= 0 || h <= 0 || w > std::numeric_limits<int>::max() / 4 / h) return false;
        size = static_cast<size_t>(w) * h * 4;
        int fd = memfd_create("boxedwine-status-bar", MFD_CLOEXEC);
        if (fd < 0) return false;
        if (ftruncate(fd, size) < 0) { close(fd); return false; }
        pixels = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (pixels == MAP_FAILED) { close(fd); return false; }
        wl_shm_pool* pool = wl_shm_create_pool(shm, fd, static_cast<int>(size));
        buffer = wl_shm_pool_create_buffer(pool, 0, w, h, w * 4, WL_SHM_FORMAT_ARGB8888);
        wl_shm_pool_destroy(pool);
        close(fd);
        if (!buffer) return false;
        static const wl_buffer_listener listener = {released};
        wl_buffer_add_listener(buffer, &listener, this);
        width = w; height = h;
        return true;
    }
    ~Buffer() {
        if (buffer) wl_buffer_destroy(buffer);
        if (pixels != MAP_FAILED) munmap(pixels, size);
    }
};
}

struct WaylandStatusSurface::Impl {
    std::shared_ptr<Connection> connection;
    wl_surface* surface = nullptr; // Owned by the hidden SDL window.
    wl_surface* parent = nullptr;
    wl_subsurface* subsurface = nullptr;
    std::vector<std::unique_ptr<Buffer>> buffers;
    int x = -1, y = -1;
    bool visible = false;
    ~Impl() {
        if (subsurface) {
            wl_surface_attach(surface, nullptr, 0, 0);
            wl_surface_commit(surface);
            wl_subsurface_destroy(subsurface);
            wl_display_flush(connection->display);
        }
    }
};

WaylandStatusSurface::WaylandStatusSurface() = default;
WaylandStatusSurface::~WaylandStatusSurface() = default;

bool WaylandStatusSurface::supported(SDL_Window* window) {
    SDL_SysWMinfo info = {};
    return windowInfo(window, info);
}

void WaylandStatusSurface::toSurfaceCoordinates(SDL_Window* window, int& x, int& y) {
    if (!SDL_GetHintBoolean("SDL_VIDEO_WAYLAND_SCALE_TO_DISPLAY", SDL_FALSE)) return;
    static CompatScaling compat;
    if (!compat.windowFromId || !compat.displayScale || compat.windowFromId(SDL_GetWindowID(window)) != window) return;
    float scale = compat.displayScale(window);
    if (scale > 0) {
        x = static_cast<int>(std::lround(x / scale));
        y = static_cast<int>(std::lround(y / scale));
    }
}

bool WaylandStatusSurface::attach(SDL_Window* window, SDL_Window* parent) {
    SDL_SysWMinfo childInfo = {}, parentInfo = {};
    if (!windowInfo(window, childInfo) || !windowInfo(parent, parentInfo) ||
        childInfo.info.wl.display != parentInfo.info.wl.display || childInfo.info.wl.xdg_surface) return false;
    impl.reset();
    auto shared = connection.lock();
    if (!shared || shared->display != childInfo.info.wl.display) {
        shared = std::make_shared<Connection>();
        if (!shared->init(childInfo.info.wl.display)) return false;
        connection = shared;
    }
    impl = std::make_unique<Impl>();
    impl->connection = shared;
    impl->surface = childInfo.info.wl.surface;
    impl->parent = parentInfo.info.wl.surface;
    impl->subsurface = wl_subcompositor_get_subsurface(shared->compositor, impl->surface, impl->parent);
    wl_subsurface_set_desync(impl->subsurface);
    return true;
}

void WaylandStatusSurface::position(int x, int y) {
    if (!impl || (x == impl->x && y == impl->y)) return;
    wl_subsurface_set_position(impl->subsurface, x, y);
    wl_surface_commit(impl->parent); // Subsurface positions are parent-buffered.
    wl_display_flush(impl->connection->display);
    impl->x = x; impl->y = y;
}

void WaylandStatusSurface::setInputTransparent() {
    if (!impl || !impl->connection->surfaceCompositor) return;
    wl_region* empty = wl_compositor_create_region(impl->connection->surfaceCompositor);
    wl_surface_set_input_region(impl->surface, empty);
    wl_region_destroy(empty);
    wl_surface_commit(impl->surface);
}

bool WaylandStatusSurface::present(SDL_Surface* pixels) {
    if (!impl || !pixels || pixels->format->format != SDL_PIXELFORMAT_ARGB8888) return false;
    Buffer* next = nullptr;
    for (auto it = impl->buffers.begin(); it != impl->buffers.end();) {
        Buffer& buffer = **it;
        if (!buffer.busy && (buffer.width != pixels->w || buffer.height != pixels->h)) {
            it = impl->buffers.erase(it);
        } else {
            if (!buffer.busy) next = &buffer;
            ++it;
        }
    }
    if (!next) {
        // Do not overwrite buffers still read by the compositor. The caller
        // retries on its next tick if all three are in flight.
        if (impl->buffers.size() >= 3) return false;
        auto buffer = std::make_unique<Buffer>();
        if (!buffer->init(impl->connection->shm, pixels->w, pixels->h)) return false;
        next = buffer.get();
        impl->buffers.push_back(std::move(buffer));
    }
    for (int y = 0; y < pixels->h; ++y) {
        memcpy(static_cast<char*>(next->pixels) + y * pixels->w * 4,
            static_cast<char*>(pixels->pixels) + y * pixels->pitch, pixels->w * 4);
    }
    next->busy = true;
    wl_surface_set_buffer_scale(impl->surface, 1);
    wl_surface_attach(impl->surface, next->buffer, 0, 0);
    wl_surface_damage(impl->surface, 0, 0, pixels->w, pixels->h);
    wl_surface_commit(impl->surface);
    // Let the shell's default bounds include newly mapped subsurfaces. Do not
    // override SDL's shell geometry (fullscreen and client decorations own it).
    if (!impl->visible) wl_surface_commit(impl->parent);
    wl_display_flush(impl->connection->display);
    impl->visible = true;
    return true;
}

void WaylandStatusSurface::hide() {
    if (!impl || !impl->visible) return;
    wl_surface_attach(impl->surface, nullptr, 0, 0);
    wl_surface_commit(impl->surface);
    wl_surface_commit(impl->parent);
    wl_display_flush(impl->connection->display);
    impl->visible = false;
}
#endif
