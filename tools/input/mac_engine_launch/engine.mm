/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#import <Cocoa/Cocoa.h>
#include <SDL.h>
#include "macLaunch.h"
#include "macStatusBar.h"
#include <cassert>
#include <cstdio>
#include <unistd.h>
extern "C" void MacPlatformSetRuntimeDockIcon(void);
int probe(int argc, const char** argv) {
    assert(argc == 4);
    assert(!strcmp(argv[3], "literal $(no-shell) 日本語"));
    assert(!strcmp(getenv("BOXEDWINE_TEST_ENV"), "environment passed"));
    FILE* input = fopen(argv[2], "rb"); assert(input);
    char data[64] = {}; assert(fread(data, 1, 63, input) > 0); fclose(input);
    assert(!strcmp(data, "scoped file access works"));
    fprintf(stdout, "ENGINE_PID %d\nShowing Window\n", getpid()); fflush(stdout);
    if (!strcmp(argv[1], "success")) return 17;
    if (!strcmp(argv[1], "stop")) { char command[32] = {}; assert(fgets(command, sizeof(command), stdin)); assert(!strcmp(command, "quit\n")); return 0; }
    if ((!strcmp(argv[1], "force") || !strcmp(argv[1], "orphan"))) { for (;;) sleep(1); }
    if (!strcmp(argv[1], "icon")) {
        assert(getenv("BOXEDWINE_DOCK_ICON_PNG"));
        SDL_SetMainReady(); assert(!SDL_Init(SDL_INIT_VIDEO));
        macLaunchFinishApplication(true);
        MacPlatformSetRuntimeDockIcon();
        SDL_Window* window = SDL_CreateWindow("Runtime icon test", 300, 300, 600, 240, SDL_WINDOW_SHOWN); assert(window);
        NSDate* end = [NSDate dateWithTimeIntervalSinceNow:1.5];
        while (end.timeIntervalSinceNow > 0) {
            SDL_Event event; while (SDL_PollEvent(&event)) {}
            SDL_Delay(10);
        }
        assert([NSApp.dockTile.contentView isKindOfClass:NSImageView.class]);
        NSImageView* tile = (NSImageView*)NSApp.dockTile.contentView;
        assert(tile.image && tile.frame.size.width > 0 && tile.frame.size.height > 0);
        NSBitmapImageRep* bitmap = [NSBitmapImageRep imageRepWithData:NSApp.applicationIconImage.TIFFRepresentation];
        NSColor* pixel = [[bitmap colorAtX:bitmap.pixelsWide / 2 y:bitmap.pixelsHigh / 2] colorUsingColorSpace:NSColorSpace.deviceRGBColorSpace];
        fprintf(stdout, "Runtime icon center: %.2f %.2f %.2f\n", pixel.redComponent, pixel.greenComponent, pixel.blueComponent);
        // AppKit color-manages the PNG on this display; allow the converted
        // green component while still requiring the solid red test artwork.
        assert(pixel.redComponent > 0.95 && pixel.greenComponent < 0.25 && pixel.blueComponent < 0.05);
        bitmap = [NSBitmapImageRep imageRepWithData:tile.image.TIFFRepresentation];
        pixel = [[bitmap colorAtX:bitmap.pixelsWide / 2 y:bitmap.pixelsHigh / 2] colorUsingColorSpace:NSColorSpace.deviceRGBColorSpace];
        assert(pixel.redComponent > 0.95 && pixel.greenComponent < 0.25 && pixel.blueComponent < 0.05);
        SDL_DestroyWindow(window); SDL_Quit(); return 0;
    }
    if (!strcmp(argv[1], "activation")) {
        // macLaunch created NSApp before SDL, as it does in the packaged app.
        assert(NSApp.activationPolicy == NSApplicationActivationPolicyAccessory);
        SDL_SetMainReady(); assert(!SDL_Init(SDL_INIT_VIDEO));
        macLaunchFinishApplication(true);
        assert(NSApp.activationPolicy == NSApplicationActivationPolicyRegular);
        assert(NSApp.mainMenu.numberOfItems >= 2 && NSApp.windowsMenu.numberOfItems >= 3);
        macLaunchFinishApplication(false);
        assert(NSApp.activationPolicy == NSApplicationActivationPolicyAccessory);
        SDL_Quit(); return 0;
    }
    if (!strcmp(argv[1], "picker")) {
        SDL_SetMainReady(); assert(!SDL_Init(SDL_INIT_VIDEO));
        macLaunchFinishApplication(true);
        SDL_Window* window = SDL_CreateWindow("Independent engine picker test", 300, 300, 600, 240, SDL_WINDOW_SHOWN); assert(window);
        const char* selection = macStatusBarChooseImage(); assert(selection);
        FILE* disc = fopen(selection, "rb"); assert(disc); fclose(disc);
        fprintf(stdout, "Selected and read %s\n", selection); fflush(stdout);
        macStatusBarReleaseImages(); SDL_DestroyWindow(window); SDL_Quit(); return 0;
    }
    return 3;
}
int main(int argc, const char** argv) { return macLaunch(argc, argv, probe); }
