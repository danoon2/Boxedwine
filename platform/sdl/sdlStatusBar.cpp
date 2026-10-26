/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "boxedwine.h"
#include "sdlStatusBar.h"
#if !defined(__EMSCRIPTEN__) && !defined(__TEST) && !defined(__ANDROID__)
#include "devcdrom.h"
#undef ID // x86 EFLAGS bit name conflicts with ImGui's public structures.
#include "imgui.h"
#include "imgui_internal.h"
#include "addon/imguitinyfiledialogs.h"
#include <SDL_syswm.h>
#include <algorithm>
#include <cmath>
#include "../linux/waylandStatusSurface.h"
#ifdef __linux__
#include "../linux/linuxImagePicker.h"
#endif
#ifdef __APPLE__
#include "../mac/macStatusBar.h"
#endif
#ifdef BOXEDWINE_MSVC
#include <commdlg.h>
#include "../windows/windowsAppIdentity.h"
#endif
#if defined(BOXEDWINE_NATIVE_RUNTIME) && defined(__linux__)
#include "../../project/linux/nativeAppIdentity.h"
#endif

namespace {
constexpr int barHeight = 32, rowHeight = 32, menuWidth = 460, maxDiscs = 8;
struct Surface {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* fontTexture = nullptr;
    SDL_Surface* pixels = nullptr;
    std::unique_ptr<WaylandStatusSurface> subsurface;
    void destroy() {
        subsurface.reset();
        SDL_DestroyTexture(fontTexture);
        SDL_DestroyRenderer(renderer);
        SDL_FreeSurface(pixels);
        SDL_DestroyWindow(window);
        window = nullptr;
        renderer = nullptr;
        fontTexture = nullptr;
        pixels = nullptr;
    }
    void hide() { if (subsurface) subsurface->hide(); else if (window) SDL_HideWindow(window); }
    bool present() {
        SDL_RenderPresent(renderer);
        return !subsurface || subsurface->present(pixels);
    }
};
Surface bar, menu;
std::unique_ptr<ImFontAtlas> atlas;
ImFont* font = nullptr;
int atlasWidth = 0, atlasHeight = 0;
U32 ownerId = 0, nextRefresh = 0;
bool opened = false, dirty = true;
bool barPressed = false;
int pressedMenuRow = -1;
bool pressedMenuRemove = false;
U32 dismissedButtons = 0;
bool enabled = false;
int hover = -1, firstDisc = 0;
int guestHeight = 0;
bool embedded = false;
bool wayland = false, waylandBarShown = false;
#ifdef BOXEDWINE_MSVC
LONG_PTR guestStyle = 0, guestExStyle = 0;
#endif
CdromState state;
#ifdef __linux__
LinuxImagePicker imagePicker;
#endif

SDL_Window* owner() { return SDL_GetWindowFromID(ownerId); }
#if defined(__linux__) && defined(SDL_VIDEO_DRIVER_X11)
struct X11Embedding {
    void* library = SDL_LoadObject("libX11.so.6");
    int (*reparent)(Display*, Window, Window, int, int) = library ?
        (decltype(reparent))SDL_LoadFunction(library, "XReparentWindow") : nullptr;
    int (*focus)(Display*, Window, int, Time) = library ?
        (decltype(focus))SDL_LoadFunction(library, "XSetInputFocus") : nullptr;
    int (*transient)(Display*, Window, Window) = library ?
        (decltype(transient))SDL_LoadFunction(library, "XSetTransientForHint") : nullptr;
    int (*attributes)(Display*, Window, unsigned long, XSetWindowAttributes*) = library ?
        (decltype(attributes))SDL_LoadFunction(library, "XChangeWindowAttributes") : nullptr;
    int (*flush)(Display*) = library ? (decltype(flush))SDL_LoadFunction(library, "XFlush") : nullptr;
};
X11Embedding& x11() { static X11Embedding api; return api; }
#endif
bool fullscreen(SDL_Window* window) {
    return window && (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN);
}

// Owned windows stay with the game in the Windows task switcher and minimize
// with it. X11 popups also need an explicit transient parent.
void setOwner(SDL_Window* child, SDL_Window* parent, bool passive) {
#ifdef __APPLE__
    macStatusBarSetOwner(child, parent);
#elif defined(BOXEDWINE_MSVC)
    SDL_SysWMinfo c = {}, p = {};
    SDL_VERSION(&c.version);
    SDL_VERSION(&p.version);
    if (SDL_GetWindowWMInfo(child, &c) && SDL_GetWindowWMInfo(parent, &p)) {
        SetWindowLongPtr(c.info.win.window, GWLP_HWNDPARENT, (LONG_PTR)p.info.win.window);
        LONG_PTR style = GetWindowLongPtr(c.info.win.window, GWL_EXSTYLE);
        style = (style | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW;
        if (passive) style |= WS_EX_NOACTIVATE;
        SetWindowLongPtr(c.info.win.window, GWL_EXSTYLE, style);
    }
#elif defined(__linux__) && defined(SDL_VIDEO_DRIVER_X11)
    SDL_SysWMinfo c = {}, p = {};
    SDL_VERSION(&c.version); SDL_VERSION(&p.version);
    if (SDL_GetWindowWMInfo(child, &c) && SDL_GetWindowWMInfo(parent, &p) &&
            c.subsystem == SDL_SYSWM_X11 && p.subsystem == SDL_SYSWM_X11 && x11().transient && x11().flush) {
        x11().transient(c.info.x11.display, c.info.x11.window, p.info.x11.window);
        if (!passive && x11().attributes) {
            // Keep the popup's pixels and input region together under WMs
            // such as WSLg. Set this natively: sdl2-compat treats SDL's popup
            // flag as an SDL3 child of whichever window happens to have focus.
            XSetWindowAttributes attributes = {};
            attributes.override_redirect = True;
            x11().attributes(c.info.x11.display, c.info.x11.window, CWOverrideRedirect, &attributes);
        }
        x11().flush(c.info.x11.display);
    }
#endif
}

void focusGuest() {
#ifdef __APPLE__
    macStatusBarFocus(owner());
#elif defined(BOXEDWINE_MSVC)
    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    if (owner() && SDL_GetWindowWMInfo(owner(), &info)) SetFocus(info.info.win.window);
#elif defined(__linux__) && defined(SDL_VIDEO_DRIVER_X11)
    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    if (owner() && SDL_GetWindowWMInfo(owner(), &info) && info.subsystem == SDL_SYSWM_X11 && x11().focus && x11().flush) {
        x11().focus(info.info.x11.display, info.info.x11.window, RevertToParent, CurrentTime);
        x11().flush(info.info.x11.display);
    }
#endif
}

bool embedGuest(SDL_Window* child, SDL_Window* parent, bool attach) {
#ifdef __APPLE__
    return macStatusBarEmbedGuest(child, parent, attach);
#elif defined(BOXEDWINE_MSVC)
    SDL_SysWMinfo c = {}, p = {};
    SDL_VERSION(&c.version); SDL_VERSION(&p.version);
    if (!SDL_GetWindowWMInfo(child, &c) || !SDL_GetWindowWMInfo(parent, &p)) return false;
    HWND hwnd = c.info.win.window;
    if (attach) {
        guestStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
        guestExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        SetWindowLongPtr(hwnd, GWL_STYLE, (guestStyle & ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)) | WS_CHILD | WS_CLIPSIBLINGS);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, guestExStyle & ~(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE));
        SetWindowLongPtr(p.info.win.window, GWL_STYLE, GetWindowLongPtr(p.info.win.window, GWL_STYLE) | WS_CLIPCHILDREN);
        SetParent(hwnd, p.info.win.window);
        int w, h;
        SDL_GetWindowSize(child, &w, &h);
        SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    } else {
        SDL_HideWindow(child);
        SetParent(hwnd, nullptr);
        SetWindowLongPtr(hwnd, GWL_STYLE, guestStyle);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, guestExStyle);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    return true;
#elif defined(__linux__) && defined(SDL_VIDEO_DRIVER_X11)
    SDL_SysWMinfo c = {}, p = {};
    SDL_VERSION(&c.version); SDL_VERSION(&p.version);
    if (!SDL_GetWindowWMInfo(child, &c) || !SDL_GetWindowWMInfo(parent, &p) ||
            c.subsystem != SDL_SYSWM_X11 || p.subsystem != SDL_SYSWM_X11 || !x11().reparent || !x11().flush) return false;
    SDL_HideWindow(child);
    x11().reparent(c.info.x11.display, c.info.x11.window,
        attach ? p.info.x11.window : DefaultRootWindow(c.info.x11.display), 0, 0);
    x11().flush(c.info.x11.display);
    // SDL may apply a pending top-level position when the child is shown
    // (notably through sdl2-compat). Keep its cached position at the origin too.
    if (attach) SDL_SetWindowPosition(child, 0, 0);
    return true;
#else
    return false;
#endif
}

bool createSurface(Surface& surface, const char* title, int width, int height, bool passive) {
    if (surface.window) {
        if (!surface.pixels || (surface.pixels->w == width && surface.pixels->h == height)) return true;
        SDL_SetWindowSize(surface.window, width, height);
        SDL_DestroyTexture(surface.fontTexture);
        SDL_DestroyRenderer(surface.renderer);
        SDL_FreeSurface(surface.pixels);
        surface.fontTexture = nullptr;
        surface.renderer = nullptr;
        surface.pixels = nullptr;
        dirty = true;
    } else {
        U32 flags = SDL_WINDOW_HIDDEN;
        if (!passive || wayland) flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_UTILITY;
        surface.window = SDL_CreateWindow(title, 0, 0, width, height, flags);
        if (!surface.window) return false;
        if (wayland) {
            surface.subsurface = std::make_unique<WaylandStatusSurface>();
            if (!surface.subsurface->attach(surface.window, owner())) { surface.destroy(); return false; }
        }
    }
#ifdef BOXEDWINE_MSVC
    windowsAppSetWindowIcon(surface.window);
#endif
#if defined(BOXEDWINE_NATIVE_RUNTIME) && defined(__linux__)
    linuxAppSetWindowIcon(surface.window);
#endif
    // Never select a GL renderer here: the guest may have a context current on
    // this thread. In particular, do not change its read/draw framebuffer.
    if (surface.subsurface) {
        surface.pixels = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
        if (surface.pixels) surface.renderer = SDL_CreateSoftwareRenderer(surface.pixels);
    } else {
#ifdef __linux__
        // A software renderer can still get a GL-backed window surface on Linux,
        // including with sdl2-compat. Use the native framebuffer for these controls.
        const char* acceleration = SDL_GetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION);
        bool hadAcceleration = acceleration != nullptr;
        BString previousAcceleration = acceleration ? BString::copy(acceleration) : BString::empty;
        SDL_SetHintWithPriority(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0", SDL_HINT_OVERRIDE);
#endif
        surface.renderer = SDL_CreateRenderer(surface.window, -1, SDL_RENDERER_SOFTWARE);
#ifdef __linux__
        SDL_SetHintWithPriority(SDL_HINT_FRAMEBUFFER_ACCELERATION,
            hadAcceleration ? previousAcceleration.c_str() : nullptr, SDL_HINT_OVERRIDE);
#endif
    }
    if (!surface.renderer) { surface.destroy(); return false; }
    if (!atlas) {
        atlas = std::make_unique<ImFontAtlas>();
        const char* paths[] = {
#ifdef BOXEDWINE_MSVC
            "C:/Windows/Fonts/segoeui.ttf",
#elif defined(__APPLE__)
            "/System/Library/Fonts/Supplemental/Arial.ttf",
#else
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
#endif
        };
        for (const char* path : paths) {
            if (Fs::doesNativePathExist(BString::copy(path))) {
                font = atlas->AddFontFromFileTTF(path, 15);
                if (font) break;
            }
        }
        if (!font) font = atlas->AddFontDefault();
    }
    unsigned char* pixels;
    atlas->GetTexDataAsRGBA32(&pixels, &atlasWidth, &atlasHeight);
    surface.fontTexture = SDL_CreateTexture(surface.renderer, SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STATIC, atlasWidth, atlasHeight);
    if (!surface.fontTexture) { surface.destroy(); return false; }
    SDL_UpdateTexture(surface.fontTexture, nullptr, pixels, atlasWidth * 4);
    SDL_SetTextureBlendMode(surface.fontTexture, SDL_BLENDMODE_BLEND);
    if (!passive && !wayland && owner()) setOwner(surface.window, embedded ? bar.window : owner(), false);
    return true;
}

void fill(Surface& surface, int x, int y, int w, int h, U8 r, U8 g, U8 b) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(surface.renderer, r, g, b, 255);
    SDL_RenderFillRect(surface.renderer, &rect);
}

void text(Surface& surface, const BString& value, int x, int y, int maxWidth, U8 gray = 40) {
    SDL_Rect clip = {x, y, std::max(0, maxWidth), rowHeight};
    SDL_RenderSetClipRect(surface.renderer, &clip);
    SDL_SetTextureColorMod(surface.fontTexture, gray, gray, gray);
    const char* current = value.c_str();
    float pen = (float)x;
    while (*current && pen < clip.x + clip.w) {
        unsigned int codepoint;
        int length = ImTextCharFromUtf8(&codepoint, current, nullptr);
        if (!length) break;
        current += length;
        const ImFontGlyph* glyph = font->FindGlyph((ImWchar)codepoint);
        if (!glyph) continue;
        SDL_Rect src = {(int)std::round(glyph->U0 * atlasWidth), (int)std::round(glyph->V0 * atlasHeight),
            (int)std::round((glyph->U1 - glyph->U0) * atlasWidth), (int)std::round((glyph->V1 - glyph->V0) * atlasHeight)};
        // Atlas glyphs are horizontally oversampled; their UV rectangle is
        // wider than their display rectangle. Use font metrics for the latter.
        SDL_Rect dst = {(int)std::round(pen + glyph->X0), y + (int)glyph->Y0,
            (int)std::round(glyph->X1 - glyph->X0), (int)std::round(glyph->Y1 - glyph->Y0)};
        if (src.w && src.h) SDL_RenderCopy(surface.renderer, surface.fontTexture, &src, &dst);
        pen += glyph->AdvanceX;
    }
    SDL_RenderSetClipRect(surface.renderer, nullptr);
}

int visibleDiscs() { return std::min(maxDiscs, std::max(0, (int)state.discs.size() - firstDisc)); }
int rows() { return visibleDiscs() + 2; }
bool removeControl(int row, int x) {
    return row > 0 && row <= visibleDiscs() && firstDisc + row - 1 != state.index && x >= menuWidth - 80;
}
BString discName(int index) {
    return B("Disc ") + BString::valueOf(index + 1) + " - " + Fs::getFileNameFromNativePath(state.discs[index]);
}

void closeMenu(bool restoreFocus) {
    opened = false;
    barPressed = false;
    pressedMenuRow = -1;
    hover = -1;
    if (menu.window) {
        menu.hide();
#ifdef __APPLE__
        // AppKit restores child windows when showing their parent. A closed
        // popup must stay detached across owner recreation and fullscreen.
        macStatusBarSetOwner(menu.window, nullptr);
#endif
    }
    if (restoreFocus && owner() && !wayland) { SDL_RaiseWindow(embedded ? bar.window : owner()); focusGuest(); }
    dirty = true;
}

void releaseWayland() {
    if (!wayland) return;
    menu.destroy();
    bar.destroy();
    wayland = waylandBarShown = false;
}

void positionSurfaces() {
    SDL_Window* parent = owner();
    if (!parent) return;
    int x, y, w, h;
    SDL_GetWindowPosition(embedded ? bar.window : parent, &x, &y);
    SDL_GetWindowSize(parent, &w, &h);
    guestHeight = h;
    if (wayland) {
        int surfaceW = w, surfaceH = h;
        WaylandStatusSurface::toSurfaceCoordinates(parent, surfaceW, surfaceH);
        bool showBar = state.configured && !fullscreen(parent) && (SDL_GetWindowFlags(parent) & SDL_WINDOW_SHOWN);
        if (showBar) {
            showBar = createSurface(bar, SDL_GetWindowTitle(parent), w, barHeight, true);
            if (showBar) {
                bar.subsurface->position(0, surfaceH);
            }
        }
        if (!showBar) bar.hide();
        if (showBar != waylandBarShown) dirty = true;
        waylandBarShown = showBar;
        if (opened) {
            int height = rows() * rowHeight + 8;
            if (createSurface(menu, "Boxedwine CD-ROM menu", std::min(menuWidth, w), height, false)) {
                int menuW = 0;
                WaylandStatusSurface::toSurfaceCoordinates(menu.window, menuW, height);
                menu.subsurface->position(0, std::max(0, surfaceH - height));
            } else closeMenu(false);
        }
        return;
    }
    if (embedded) {
        int oldW, oldH;
        SDL_GetWindowSize(bar.window, &oldW, &oldH);
        if (oldW != w || oldH != h + barHeight) {
            SDL_SetWindowSize(bar.window, w, h + barHeight);
            dirty = true;
        }
        // Native child coordinates are independent of the outer window's move.
        const char* title = SDL_GetWindowTitle(parent);
        if (strcmp(SDL_GetWindowTitle(bar.window), title)) SDL_SetWindowTitle(bar.window, title);
#ifdef __APPLE__
        macStatusBarPositionGuest(parent, bar.window);
#endif
    }
    if (opened && menu.window) {
        int height = rows() * rowHeight + 8;
        int width = menuWidth;
        int mx = x, my = y + h - height;
        SDL_Rect bounds = {};
        if (SDL_GetDisplayUsableBounds(SDL_GetWindowDisplayIndex(parent), &bounds) == 0) {
            width = std::min(width, bounds.w);
            mx = std::max(bounds.x, std::min(mx, bounds.x + bounds.w - width));
            my = std::max(bounds.y, std::min(my, bounds.y + bounds.h - height));
        }
        int oldW, oldH, oldX, oldY;
        SDL_GetWindowSize(menu.window, &oldW, &oldH);
        SDL_GetWindowPosition(menu.window, &oldX, &oldY);
        if (oldW != width || oldH != height) SDL_SetWindowSize(menu.window, width, height);
        if (oldX != mx || oldY != my) SDL_SetWindowPosition(menu.window, mx, my);
    }
}

void draw() {
    bool presented = true;
    if (bar.window && (embedded || waylandBarShown)) {
        int width, height;
        SDL_GetWindowSize(bar.window, &width, &height);
        SDL_SetRenderDrawColor(bar.renderer, 0, 0, 0, 255);
        SDL_RenderClear(bar.renderer);
        SDL_Rect viewport = {0, wayland ? 0 : guestHeight, width, barHeight};
        SDL_RenderSetViewport(bar.renderer, &viewport);
        height = barHeight;
        fill(bar, 0, 0, width, height, 242, 243, 245);
        fill(bar, 0, 0, width, 1, 190, 194, 200);
        if (opened) fill(bar, 1, 1, std::min(width - 2, 285), height - 2, 218, 233, 250);
        // Optical disc icon, drawn without any font/icon dependency.
        SDL_SetRenderDrawColor(bar.renderer, 92, 105, 122, 255);
        for (int radius : {8, 2}) {
            for (int angle = 0; angle < 64; ++angle) {
                double a = angle * 6.28318530718 / 64;
                SDL_RenderDrawPoint(bar.renderer, 18 + (int)std::round(std::cos(a) * radius),
                    16 + (int)std::round(std::sin(a) * radius));
            }
        }
        BString label = B("CD-ROM");
        if (!state.drive.isEmpty()) label += " (" + state.drive + ")";
        label += state.index >= 0 ? B("  -  Disc ") + BString::valueOf(state.index + 1) : B("  -  Empty");
        text(bar, label, 36, 7, std::min(width - 45, 235));
        for (int i = 0; i < 4; ++i) fill(bar, 267 + i, 14 + i, 7 - i * 2, 1, 80, 86, 96);
        if (width > 660) text(bar, B("Ctrl+Shift+F12: Next disc"), width - 193, 8, 185, 110);
        presented &= bar.present();
    }
    if (opened && menu.window) {
        int width, height;
        SDL_GetWindowSize(menu.window, &width, &height);
        fill(menu, 0, 0, width, height, 180, 186, 195);
        fill(menu, 1, 1, width - 2, height - 2, 252, 252, 253);
        for (int row = 0; row < rows(); ++row) {
            int y = 4 + row * rowHeight;
            if (row == hover) fill(menu, 4, y, width - 8, rowHeight, 222, 237, 253);
            BString label;
            if (!row) label = B("Choose disc image...");
            else if (row <= visibleDiscs()) {
                int index = firstDisc + row - 1;
                label = discName(index);
                if (index == state.index) {
                    SDL_SetRenderDrawColor(menu.renderer, 48, 109, 177, 255);
                    SDL_RenderDrawLine(menu.renderer, 12, y + 16, 16, y + 20);
                    SDL_RenderDrawLine(menu.renderer, 16, y + 20, 23, y + 12);
                }
                if (index != state.index) text(menu, B("Remove"), width - 68, y + 7, 60, 110);
            } else label = B("Eject disc");
            text(menu, label, 34, y + 7, width - (removeControl(row, width) ? 110 : 46),
                row == visibleDiscs() + 1 && state.index < 0 ? 160 : 40);
            if (row == 0 || row == visibleDiscs()) fill(menu, 8, y + rowHeight - 1, width - 16, 1, 224, 226, 230);
        }
        if (state.discs.size() > maxDiscs) {
            text(menu, B("Scroll for more"), width - 115, 11, 107, 120);
        }
        presented &= menu.present();
    }
    dirty = !presented;
}

#ifndef __linux__
BString chooseImage() {
#ifdef __APPLE__
    const char* selected = macStatusBarChooseImage();
    return selected ? BString::copy(selected) : BString::empty;
#elif defined(BOXEDWINE_MSVC)
    // Use the Unicode common dialog; the older bundled tinyfiledialogs uses
    // ANSI paths on Windows. SDL/filesystem paths are UTF-8.
    wchar_t path[32768] = {};
    OPENFILENAMEW dialog = {};
    dialog.lStructSize = sizeof(dialog);
    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    if (owner() && SDL_GetWindowWMInfo(embedded ? bar.window : owner(), &info)) dialog.hwndOwner = info.info.win.window;
    dialog.lpstrTitle = L"Choose CD-ROM disc image";
    dialog.lpstrFilter = L"Disc images (*.iso;*.cue)\0*.iso;*.cue\0All files\0*.*\0";
    dialog.lpstrFile = path;
    dialog.nMaxFile = 32768;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&dialog)) return BString::empty;
    int size = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> utf8(size);
    WideCharToMultiByte(CP_UTF8, 0, path, -1, utf8.data(), size, nullptr, nullptr);
    return BString::copy(utf8.data());
#else
    const char* filters[] = {"*.iso", "*.cue", "*.ISO", "*.CUE"};
    const char* selected = tfd::openFileDialog("Choose CD-ROM disc image", "", 4, filters, "Disc images", 0);
    return selected ? BString::copy(selected) : BString::empty;
#endif
}
#endif

void refreshDiscState(bool success) {
    if (!success) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not insert disc",
        "The image could not be opened as an ISO or CUE disc.\nThe current disc has been kept in the drive.", owner());
    state = cdromGetState();
    firstDisc = std::min(firstDisc, std::max(0, (int)state.discs.size() - maxDiscs));
    dirty = true;
    positionSurfaces();
}

void finishImageSelection(const BString& path) {
    bool success = path.isEmpty() || cdromInsertPath(path);
    if (owner()) { SDL_RaiseWindow(embedded ? bar.window : owner()); focusGuest(); }
    refreshDiscState(success);
}

#ifdef __linux__
void pickerError() {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open image picker",
        "The file picker could not be opened. Install zenity or kdialog and try again.\nThe current disc has been kept in the drive.", owner());
}
#endif

void activate(int row, int x) {
    if (row < 0 || row >= rows()) return;
    bool success = true;
    if (row == 0) {
        closeMenu(false);
#ifdef __linux__
        if (!imagePicker.open(wayland)) pickerError();
#else
        finishImageSelection(chooseImage());
#endif
        return;
    } else if (row <= visibleDiscs()) {
        int index = firstDisc + row - 1;
        if (removeControl(row, x)) {
            cdromRemoveDisc(index);
        } else {
            success = cdromInsert(index);
            closeMenu(true);
        }
    } else if (row == visibleDiscs() + 1) {
        cdromEject();
        closeMenu(true);
    }
    refreshDiscState(success);
}
} // namespace

bool sdlStatusBarOwns(SDL_Window* window) {
    return window && (window == bar.window || window == menu.window);
}

void sdlStatusBarTick() {
    if (!enabled || !owner()) return;
#ifdef __linux__
    std::string path;
    bool failed;
    if (imagePicker.poll(path, failed)) {
        if (failed) pickerError();
        else finishImageSelection(BString::copy(path.c_str()));
    }
#endif
    if ((S32)(SDL_GetTicks() - nextRefresh) >= 0) {
        CdromState latest = cdromGetState();
        if (state.index != latest.index || state.drive != latest.drive || state.discs != latest.discs) dirty = true;
        state = std::move(latest);
        nextRefresh = SDL_GetTicks() + 100;
    }
    positionSurfaces();
    if (dirty) draw();
}

void sdlStatusBarShowWindow(SDL_Window* window, bool show) {
    if (!enabled) {
        if (show) SDL_ShowWindow(window); else SDL_HideWindow(window);
        return;
    }
    if (!show) {
        if (owner() == window) {
            closeMenu(false);
            if (embedded || wayland) bar.hide();
            waylandBarShown = false;
        }
        SDL_HideWindow(window);
        return;
    }
    if (owner() != window) {
#ifdef __linux__
        imagePicker.cancel();
#endif
        closeMenu(false);
        releaseWayland();
        if (embedded && owner()) embedGuest(owner(), bar.window, false);
        embedded = false;
        if (bar.window) SDL_HideWindow(bar.window);
        ownerId = SDL_GetWindowID(window);
        wayland = WaylandStatusSurface::supported(window);
        state = cdromGetState();
        int w, h, x, y;
        SDL_GetWindowSize(window, &w, &h);
        SDL_GetWindowPosition(window, &x, &y);
        if (!wayland && !fullscreen(window) && state.configured && createSurface(bar, SDL_GetWindowTitle(window), w, h + barHeight, true)) {
            SDL_SetWindowPosition(bar.window, x, y);
            embedded = embedGuest(window, bar.window, true);
        }
#ifndef __APPLE__
        if (!wayland && menu.window) setOwner(menu.window, embedded ? bar.window : window, false);
#endif
    }
    if (embedded) {
        positionSurfaces();
        SDL_ShowWindow(bar.window);
        // X11 cannot focus a mapped child until its parent is viewable.
        SDL_ShowWindow(window);
        SDL_RaiseWindow(bar.window);
        focusGuest();
    } else SDL_ShowWindow(window);
    if (wayland) positionSurfaces();
    dirty = true;
}

void sdlStatusBarSetWindowSize(SDL_Window* window, int width, int height) {
    SDL_SetWindowSize(window, width, height);
#ifdef BOXEDWINE_MSVC
    if (embedded && window == owner() && !fullscreen(window)) {
        // SDL's Windows backend uses a top-level z-order band when resizing.
        // We reparented this window, so apply its size without changing z-order
        // before event pumping can replace SDL's size with the old native size.
        SDL_SysWMinfo info = {};
        SDL_VERSION(&info.version);
        if (SDL_GetWindowWMInfo(window, &info)) {
            SDL_GetWindowSize(window, &width, &height);
            RECT rect = {};
            HWND hwnd = info.info.win.window;
            if (GetClientRect(hwnd, &rect) && (rect.right != width || rect.bottom != height)) {
                if (!SetWindowPos(hwnd, nullptr, 0, 0, width, height,
                        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE)) {
                    kwarn_fmt("Could not resize embedded guest window: %lu", GetLastError());
                }
            }
        }
    }
#endif
}

void sdlStatusBarRaiseWindow(SDL_Window* window) {
    if (embedded && window == owner()) {
        SDL_RaiseWindow(bar.window);
        focusGuest();
    } else SDL_RaiseWindow(window);
}

void sdlStatusBarOpen() {
    sdlStatusBarTick();
#ifdef __linux__
    if (imagePicker.isOpen()) return;
#endif
    if (!owner() || !state.configured) return;
    if (opened) { closeMenu(true); return; }
    if (!createSurface(menu, "Boxedwine CD-ROM menu", menuWidth, 200, false)) return;
#ifdef __APPLE__
    macStatusBarSetOwner(menu.window, embedded ? bar.window : owner());
#endif
    firstDisc = std::max(0, state.index - maxDiscs + 1);
    opened = true;
    hover = 0;
    positionSurfaces();
    if (!wayland) {
        SDL_ShowWindow(menu.window);
        SDL_RaiseWindow(menu.window);
    }
#ifdef __APPLE__
    macStatusBarFocus(menu.window);
#endif
#if defined(__linux__) && defined(SDL_VIDEO_DRIVER_X11)
    // Override-redirect menus do not get keyboard focus from the WM.
    // sdl2-compat implements SDL_SetWindowInputFocus as just a window raise.
    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(menu.window, &info) && info.subsystem == SDL_SYSWM_X11 && x11().focus && x11().flush) {
        x11().focus(info.info.x11.display, info.info.x11.window, RevertToParent, CurrentTime);
        x11().flush(info.info.x11.display);
    }
#endif
    dirty = true;
    draw();
}

SDL_Window* sdlStatusBarInputWindow(SDL_Window* window) {
    if (window && window == menu.window) return nullptr;
    return (embedded || wayland) && window == bar.window ? owner() : window;
}

void sdlStatusBarConstrainPointer(SDL_Event& event) {
    if (!embedded || opened || !owner() || guestHeight <= 0) return;
    bool motion = event.type == SDL_MOUSEMOTION;
    bool button = event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP;
    if (!motion && !button && event.type != SDL_MOUSEWHEEL) return;
    SDL_Window* target = SDL_GetWindowFromID(event.window.windowID);
    if (target != bar.window && target != owner()) return;
    // SDL confines the focused outer window, including its status bar. While
    // a game captures the pointer, that strip must not activate host controls
    // or release capture. Return it to the guest before routing the event.
    if (motion || button) {
        int& x = motion ? event.motion.x : event.button.x;
        int& y = motion ? event.motion.y : event.button.y;
        int width;
        SDL_GetWindowSize(owner(), &width, nullptr);
        if (width <= 0) return;
        int clippedX = std::max(0, std::min(x, width - 1));
        int clippedY = std::max(0, std::min(y, guestHeight - 1));
        if (x != clippedX || y != clippedY) {
            x = clippedX; y = clippedY;
            SDL_WarpMouseInWindow(owner(), x, y);
        }
    }
    event.window.windowID = ownerId;
}

bool sdlStatusBarEvent(SDL_Event& event) {
    // These SDL event types all place windowID after timestamp. Do not inspect
    // that union field for user events, controller events, or SDL_QUIT.
    bool windowEvent = event.type == SDL_WINDOWEVENT || event.type == SDL_KEYDOWN || event.type == SDL_KEYUP ||
        event.type == SDL_TEXTINPUT || event.type == SDL_TEXTEDITING || event.type == SDL_MOUSEMOTION ||
        event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEWHEEL ||
        event.type == SDL_DROPFILE || event.type == SDL_DROPTEXT || event.type == SDL_DROPBEGIN || event.type == SDL_DROPCOMPLETE;
    if (!windowEvent) return false;
    U32 id = (event.type >= SDL_DROPFILE && event.type <= SDL_DROPCOMPLETE) ? event.drop.windowID : event.window.windowID;
    SDL_Window* target = SDL_GetWindowFromID(id);
    if (wayland && opened) {
        // A subsurface shares the toplevel's keyboard focus. Route its menu
        // keys here without changing the guest's drawable or native focus.
        if ((target == owner() || target == bar.window) &&
            (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP || event.type == SDL_TEXTINPUT || event.type == SDL_TEXTEDITING)) {
            target = menu.window;
        } else if (target == owner() && event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
            closeMenu(false);
        }
    }
    const bool wasBarPressed = barPressed;
    const int wasMenuRow = pressedMenuRow;
    const bool wasMenuRemove = pressedMenuRemove;
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        U32 button = SDL_BUTTON(event.button.button);
        if (event.button.button == SDL_BUTTON_LEFT) {
            barPressed = false;
            pressedMenuRow = -1;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            // A new press supersedes any release lost when another app took focus.
            dismissedButtons &= ~button;
            int menuW = 0, menuH = 0;
            if (opened && target == menu.window) SDL_GetWindowSize(menu.window, &menuW, &menuH);
            bool outside = target != menu.window || event.button.x < 0 || event.button.y < 0 ||
                event.button.x >= menuW || event.button.y >= menuH;
            if (opened && outside) {
                closeMenu(target == owner() || target == bar.window);
                dismissedButtons |= button;
                return true;
            }
        } else if (dismissedButtons & button) {
            // The click that dismisses the menu belongs entirely to the host UI.
            dismissedButtons &= ~button;
            return true;
        }
    }
    if (!sdlStatusBarOwns(target)) {
        // Alt+F4 can target the focused child. SDL no longer synthesizes QUIT
        // automatically because the host and popup are additional windows.
        if ((embedded || wayland) && target == owner() && event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
            SDL_Event quit = {}; quit.type = SDL_QUIT; SDL_PushEvent(&quit);
            return true;
        }
        if (target == owner() && event.type == SDL_WINDOWEVENT) dirty = true;
        return false;
    }
    // Ignore queued input for the popup after it has been hidden.
    if (target == menu.window && !opened && event.type != SDL_WINDOWEVENT) {
        if (event.type == SDL_DROPFILE || event.type == SDL_DROPTEXT) SDL_free(event.drop.file);
        return true;
    }
    // SDL 2's Windows backend tracks top-level WM_ACTIVATE, not child
    // WM_SETFOCUS. Route host activation/guest-area input to the embedded view.
    int mouseY = 0;
    if (event.type == SDL_MOUSEWHEEL) SDL_GetMouseState(nullptr, &mouseY);
    bool guestEvent = !opened && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP ||
        event.type == SDL_TEXTINPUT || event.type == SDL_TEXTEDITING ||
        (event.type == SDL_MOUSEMOTION && event.motion.y < guestHeight) ||
        (event.type == SDL_MOUSEWHEEL && mouseY < guestHeight) ||
        ((event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) && event.button.y < guestHeight) ||
        (event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED ||
            event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)));
    if (embedded && target == bar.window && guestEvent) {
        event.window.windowID = ownerId;
        return false;
    }
    if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_EXPOSED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) dirty = true;
        if (target == menu.window && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) closeMenu(false);
        if (target == bar.window && event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED && !opened) focusGuest();
        if (target == bar.window && event.window.event == SDL_WINDOWEVENT_CLOSE) {
            SDL_Event quit = {}; quit.type = SDL_QUIT; SDL_PushEvent(&quit);
        }
    } else if ((event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) && event.button.button == SDL_BUTTON_LEFT) {
        int width, height;
        SDL_GetWindowSize(target, &width, &height);
        if (target == bar.window) {
            int top = wayland ? 0 : guestHeight;
            bool inside = event.button.x >= 0 && event.button.x < std::min(width, 285) &&
                event.button.y >= top && event.button.y < top + barHeight;
            if (event.type == SDL_MOUSEBUTTONDOWN) barPressed = inside;
            else {
                if (wasBarPressed && inside) sdlStatusBarOpen();
            }
        } else {
            int row = event.button.x >= 4 && event.button.x < width - 4 &&
                event.button.y >= 4 && event.button.y < height - 4 ? (event.button.y - 4) / rowHeight : -1;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                pressedMenuRow = row;
                pressedMenuRemove = removeControl(row, event.button.x);
            }
            else {
                if (row >= 0 && row == wasMenuRow && wasMenuRemove == removeControl(row, event.button.x)) activate(row, event.button.x);
            }
        }
    } else if (event.type == SDL_MOUSEMOTION && target == menu.window) {
        hover = (event.motion.y - 4) / rowHeight;
        dirty = true;
    } else if (event.type == SDL_MOUSEWHEEL && target == menu.window) {
        firstDisc = std::max(0, std::min(firstDisc - event.wheel.y, std::max(0, (int)state.discs.size() - maxDiscs)));
        dirty = true;
    } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) closeMenu(true);
        else if (event.key.keysym.sym == SDLK_DOWN) {
            if (hover == visibleDiscs() && firstDisc + visibleDiscs() < (int)state.discs.size()) ++firstDisc;
            else hover = (hover + 1) % rows();
        } else if (event.key.keysym.sym == SDLK_UP) {
            if (hover == 1 && firstDisc) --firstDisc;
            else hover = (hover + rows() - 1) % rows();
        } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) activate(hover, 0);
        else if ((event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_BACKSPACE) && removeControl(hover, menuWidth)) activate(hover, menuWidth);
        dirty = true;
    } else if (event.type == SDL_DROPFILE || event.type == SDL_DROPTEXT) {
        SDL_free(event.drop.file);
    }
    return true;
}

void sdlStatusBarShutdown() {
#ifdef __linux__
    imagePicker.cancel();
#endif
    releaseWayland();
    if (embedded && owner()) embedGuest(owner(), bar.window, false);
    embedded = false;
    menu.destroy();
    bar.destroy();
    atlas.reset();
    font = nullptr;
    ownerId = 0;
    opened = false;
    barPressed = false;
    pressedMenuRow = -1;
    dismissedButtons = 0;
    state = {};
    nextRefresh = 0;
    dirty = true;
#ifdef __APPLE__
    macStatusBarReleaseImages();
#endif
}

void sdlStatusBarConfigure(bool value) {
    sdlStatusBarShutdown();
    enabled = value;
}

void sdlStatusBarWindowDestroyed(SDL_Window* window) {
    if (window && SDL_GetWindowID(window) == ownerId) {
#ifdef __linux__
        imagePicker.cancel();
#endif
        closeMenu(false);
        releaseWayland();
        if (embedded) { embedGuest(window, bar.window, false); SDL_HideWindow(bar.window); }
        embedded = false;
        ownerId = 0;
    }
}
#endif
