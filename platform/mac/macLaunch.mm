/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#import <Cocoa/Cocoa.h>
#include <SDL.h>
#include "macLaunch.h"
#include <atomic>
#include <fcntl.h>
#include <chrono>
#include <dirent.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

@interface BWEngineLaunch : NSObject <NSApplicationDelegate>
@property NSURL* request;
@end
@implementation BWEngineLaunch
- (void)applicationWillFinishLaunching:(NSNotification*)notification {
    [NSAppleEventManager.sharedAppleEventManager setEventHandler:self andSelector:@selector(openApplication:withReplyEvent:)
        forEventClass:kCoreEventClass andEventID:kAEOpenApplication];
}
- (void)openApplication:(NSAppleEventDescriptor*)event withReplyEvent:(NSAppleEventDescriptor*)reply {
    if (self.request) return;
    NSAppleEventDescriptor* descriptor = [event paramDescriptorForKeyword:0x42574c51]; // 'BWLQ'
    NSData* bookmark = descriptor.data;
    if (descriptor.descriptorType != typeData || !bookmark.length || bookmark.length > 64 * 1024) return;
    // An implicit-scope bookmark carries the launcher's access to just this
    // request. No document-handler registration or Automation permission is needed.
    self.request = [NSURL URLByResolvingBookmarkData:bookmark options:NSURLBookmarkResolutionWithoutUI
        relativeToURL:nil bookmarkDataIsStale:nil error:nil];
}
- (void)application:(NSApplication*)application openURLs:(NSArray<NSURL*>*)urls {
    if (!self.request && urls.count == 1) self.request = urls.firstObject;
}
@end

@interface BWEngineApplicationCommands : NSObject
- (void)quit:(id)sender;
@end
@implementation BWEngineApplicationCommands
- (void)quit:(id)sender {
    SDL_Event event = {};
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}
@end

void macLaunchFinishApplication(bool visible) {
    if (!NSApp) return;
    // Cocoa_RegisterApp skips activation policy and menus when our launch
    // document or permission panel has already created NSApplication.
    if (!NSApp.mainMenu) {
        static BWEngineApplicationCommands* commands = [BWEngineApplicationCommands new];
        NSString* name = [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleDisplayName"] ?:
            [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleName"] ?: @"Boxedwine";
        NSMenu* main = [NSMenu new];
        NSMenu* application = [[NSMenu alloc] initWithTitle:name];
        NSMenuItem* item = [main addItemWithTitle:name action:nil keyEquivalent:@""];
        item.submenu = application;
        [application addItemWithTitle:[@"About " stringByAppendingString:name] action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
        [application addItem:NSMenuItem.separatorItem];
        [application addItemWithTitle:[@"Hide " stringByAppendingString:name] action:@selector(hide:) keyEquivalent:@"h"];
        item = [application addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
        item.keyEquivalentModifierMask = NSEventModifierFlagCommand | NSEventModifierFlagOption;
        [application addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
        [application addItem:NSMenuItem.separatorItem];
        item = [application addItemWithTitle:[@"Quit " stringByAppendingString:name] action:@selector(quit:) keyEquivalent:@"q"];
        item.target = commands;
        NSMenu* windows = [[NSMenu alloc] initWithTitle:@"Window"];
        [windows addItemWithTitle:@"Close" action:@selector(performClose:) keyEquivalent:@"w"];
        [windows addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
        [windows addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];
        item = [main addItemWithTitle:@"Window" action:nil keyEquivalent:@""];
        item.submenu = windows;
        NSApp.mainMenu = main;
        NSApp.windowsMenu = windows;
    }
    [NSApp setActivationPolicy:visible ? NSApplicationActivationPolicyRegular : NSApplicationActivationPolicyAccessory];
    if (visible) [NSApp activateIgnoringOtherApps:YES];
}

static NSMutableArray<NSURL*>* launchAccess;
static std::atomic<int> exitFile{-1};
static bool launchedWithRequest;
static bool stdinControl;
static NSMutableArray<NSURL*>* commandLineAccess;
static NSString* commandLineDirectory;
static NSString* const commandLineBookmarksKey = @"BoxedwineCommandLineFolderBookmarks";

bool macLaunchIsCommandLine() { return !launchedWithRequest; }
const char* macLaunchWorkingDirectory() { return commandLineDirectory.fileSystemRepresentation; }

static void restoreCommandLineAccess() {
    // App Sandbox changes cwd to the container before main. Bash preserves the
    // original directory in PWD; use it for CLI host paths without requesting
    // access to that entire directory unless the command actually needs it.
    commandLineDirectory = NSFileManager.defaultManager.currentDirectoryPath;
    const char* pwd = getenv("PWD");
    NSString* shellDirectory = pwd ? [NSString stringWithUTF8String:pwd] : nil;
    if ([shellDirectory isAbsolutePath] &&
        [commandLineDirectory.stringByResolvingSymlinksInPath isEqualToString:NSHomeDirectory().stringByResolvingSymlinksInPath]) {
        commandLineDirectory = shellDirectory;
    }
    commandLineAccess = [NSMutableArray array];
    id saved = [NSUserDefaults.standardUserDefaults objectForKey:commandLineBookmarksKey];
    if (![saved isKindOfClass:NSArray.class] || [saved count] > 128) return;
    NSMutableArray* refreshed = [NSMutableArray array];
    for (id data in saved) {
        if (![data isKindOfClass:NSData.class] || [data length] > 64 * 1024) continue;
        BOOL stale = NO;
        NSURL* url = [NSURL URLByResolvingBookmarkData:data options:NSURLBookmarkResolutionWithSecurityScope | NSURLBookmarkResolutionWithoutUI
            relativeToURL:nil bookmarkDataIsStale:&stale error:nil];
        if (!url.isFileURL || ![url startAccessingSecurityScopedResource]) continue;
        [commandLineAccess addObject:url];
        NSData* bookmark = stale ? [url bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope
            includingResourceValuesForKeys:nil relativeToURL:nil error:nil] : data;
        if (bookmark && bookmark.length <= 64 * 1024) [refreshed addObject:bookmark];
    }
    [NSUserDefaults.standardUserDefaults setObject:refreshed forKey:commandLineBookmarksKey];
}

// Probe the actual operation: access(W_OK) alone does not establish that the
// sandbox permits creating files. The directory probe never replaces a file.
static bool canAccessPath(NSURL* url, bool directory, bool write, bool allowMissing) {
    struct stat info;
    if (stat(url.fileSystemRepresentation, &info)) {
        if (errno != ENOENT || !allowMissing) return false;
        return canAccessPath(url.URLByDeletingLastPathComponent, true, write, false);
    }
    if (directory != (bool)S_ISDIR(info.st_mode)) { errno = directory ? ENOTDIR : EISDIR; return false; }
    if (!directory) {
        int fd = open(url.fileSystemRepresentation, (write ? O_WRONLY : O_RDONLY) | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) return false;
        close(fd);
        return true;
    }
    DIR* listing = opendir(url.fileSystemRepresentation);
    if (!listing) return false;
    errno = 0;
    readdir(listing);
    int error = errno;
    if (!error && write) {
        NSString* name = [@".boxedwine-access-" stringByAppendingString:NSUUID.UUID.UUIDString];
        int fd = openat(dirfd(listing), name.UTF8String, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
        if (fd < 0) error = errno;
        else { close(fd); if (unlinkat(dirfd(listing), name.UTF8String, 0)) error = errno; }
    }
    closedir(listing);
    errno = error;
    return error == 0;
}

bool macLaunchAccess(const char* path, bool directory, bool write, bool allowMissing) {
    if (launchedWithRequest) return true;
    NSString* text = path ? [NSString stringWithUTF8String:path] : nil;
    if (!text.length) { fprintf(stderr, "Boxedwine: an empty or invalid host path was supplied.\n"); return false; }
    if (!text.isAbsolutePath && commandLineDirectory) text = [commandLineDirectory stringByAppendingPathComponent:text];
    NSURL* target = [NSURL fileURLWithPath:text].URLByStandardizingPath;
    if (canAccessPath(target, directory, write, allowMissing)) return true;
    int error = errno;
    const char* noPrompt = getenv("BOXEDWINE_NO_FILE_ACCESS_PROMPT");
    if ((error != EPERM && error != EACCES) || (noPrompt && !strcmp(noPrompt, "1"))) {
        fprintf(stderr, "Boxedwine: cannot %s %s: %s\n", write ? "write to" : "read", target.fileSystemRepresentation, strerror(error));
        return false;
    }

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
    [NSApp finishLaunching];
    NSOpenPanel* panel = NSOpenPanel.openPanel;
    if (!panel) { fprintf(stderr, "Boxedwine: file access could not be requested for %s.\n", target.fileSystemRepresentation); return false; }
    NSURL* folder = directory ? target : target.URLByDeletingLastPathComponent;
    struct stat folderInfo;
    if (allowMissing && stat(folder.fileSystemRepresentation, &folderInfo) && errno == ENOENT) folder = folder.URLByDeletingLastPathComponent;
    panel.title = @"Allow Boxedwine Folder Access";
    panel.message = [NSString stringWithFormat:@"The command line needs %@ access to:\n%@\nChoose this folder or a parent folder. Boxedwine will remember your choice.",
        write ? @"read and write" : @"read", target.path];
    panel.prompt = @"Allow Access";
    panel.canChooseDirectories = YES;
    panel.canChooseFiles = NO;
    panel.allowsMultipleSelection = NO;
    panel.directoryURL = folder;
    fprintf(stderr, "Boxedwine: choose a folder in the file-access dialog for %s.\n", target.fileSystemRepresentation);
    [NSApp activateIgnoringOtherApps:YES];
    if ([panel runModal] != NSModalResponseOK) {
        fprintf(stderr, "Boxedwine: folder access cancelled for %s.\n", target.fileSystemRepresentation);
        return false;
    }
    NSURL* selected = panel.URL.URLByResolvingSymlinksInPath.URLByStandardizingPath;
    NSString* resolvedTarget = target.URLByResolvingSymlinksInPath.URLByStandardizingPath.path;
    NSString* prefix = [selected.path isEqualToString:@"/"] ? @"/" : [selected.path stringByAppendingString:@"/"];
    if (![resolvedTarget isEqualToString:selected.path] && ![resolvedTarget hasPrefix:prefix]) {
        fprintf(stderr, "Boxedwine: the selected folder does not contain %s.\n", target.fileSystemRepresentation);
        return false;
    }
    // The Open panel grant is active now. Save an app-scoped bookmark so Bash
    // launches can regain this access without another process or another panel.
    NSURLBookmarkCreationOptions options = NSURLBookmarkCreationWithSecurityScope;
    if (!write) options |= NSURLBookmarkCreationSecurityScopeAllowOnlyReadAccess;
    NSData* bookmark = [selected bookmarkDataWithOptions:options
        includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
    if (bookmark && bookmark.length <= 64 * 1024) {
        NSMutableArray* saved = [[NSUserDefaults.standardUserDefaults arrayForKey:commandLineBookmarksKey] mutableCopy] ?: [NSMutableArray array];
        if (![saved containsObject:bookmark]) {
            while (saved.count >= 128) [saved removeObjectAtIndex:0];
            [saved addObject:bookmark];
            [NSUserDefaults.standardUserDefaults setObject:saved forKey:commandLineBookmarksKey];
        }
    } else {
        fprintf(stderr, "Boxedwine: this folder's access could not be remembered for future launches.\n");
    }
    // Keep the Powerbox URL alive alongside restored security-scoped URLs.
    [launchAccess addObject:selected];
    if (canAccessPath(target, directory, write, allowMissing)) return true;
    fprintf(stderr, "Boxedwine: cannot %s %s after folder selection: %s\n", write ? "write to" : "read", target.fileSystemRepresentation, strerror(errno));
    return false;
}

bool macLaunchHasControl() {
    // A shell pipeline is ordinary CLI input, not a launcher lifetime channel.
    return launchedWithRequest || stdinControl;
}

void macLaunchReportExit(int status) {
    int fd = exitFile.exchange(-1);
    if (fd < 0) return;
    char text[32];
    int length = snprintf(text, sizeof(text), "%d\n", status);
    write(fd, text, length);
    close(fd);
}

static bool validString(id value, NSUInteger limit) {
    return [value isKindOfClass:NSString.class] && [value lengthOfBytesUsingEncoding:NSUTF8StringEncoding] <= limit &&
        [value rangeOfString:[NSString stringWithFormat:@"%C", (unichar)0]].location == NSNotFound;
}

static NSDictionary* loadRequest(NSURL* url) {
    NSNumber* size = nil;
    if (!url.isFileURL || ![url.pathExtension isEqualToString:@"bwlaunch"] ||
        ![url getResourceValue:&size forKey:NSURLFileSizeKey error:nil] || size.unsignedLongLongValue > 20 * 1024 * 1024) return nil;
    NSData* data = [NSData dataWithContentsOfURL:url];
    id request = data ? [NSJSONSerialization JSONObjectWithData:data options:0 error:nil] : nil;
    if (![request isKindOfClass:NSDictionary.class] || ![request[@"version"] isEqual:@1] ||
        ![request[@"arguments"] isKindOfClass:NSArray.class] || [request[@"arguments"] count] > 4096 ||
        ![request[@"bookmarks"] isKindOfClass:NSArray.class] || [request[@"bookmarks"] count] > 512 ||
        ![request[@"environment"] isKindOfClass:NSDictionary.class] ||
        !validString(request[@"directory"], 4096) || ![request[@"directory"] isAbsolutePath]) return nil;
    for (id argument in request[@"arguments"]) if (!validString(argument, 1024 * 1024)) return nil;
    launchAccess = [NSMutableArray array];
    for (id encoded in request[@"bookmarks"]) {
        if (!validString(encoded, 64 * 1024)) return nil;
        NSData* bookmark = [[NSData alloc] initWithBase64EncodedString:encoded options:0];
        NSURL* grant = bookmark ? [NSURL URLByResolvingBookmarkData:bookmark options:NSURLBookmarkResolutionWithoutUI
            relativeToURL:nil bookmarkDataIsStale:nil error:nil] : nil;
        // Transferred bookmarks may be stale because the container changed.
        // Keep their resolved URLs alive for the entire engine session.
        if (!grant.isFileURL) return nil;
        [launchAccess addObject:grant];
    }
    for (id key in request[@"environment"]) {
        if (!validString(key, 256) || !validString(request[@"environment"][key], 1024 * 1024)) return nil;
        if (![key hasPrefix:@"BOXEDWINE_"] && ![key hasPrefix:@"SDL_"] && ![key hasPrefix:@"MVK_"] &&
            ![key hasPrefix:@"MESA_"] && ![key hasPrefix:@"VK_"]) return nil;
    }
    return request;
}

static bool connectSession(NSURL* document, NSDictionary* request) {
    NSURL* directory = document.URLByDeletingLastPathComponent;
    int input = open([directory URLByAppendingPathComponent:@"stdin"].fileSystemRepresentation, O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC);
    int output = open([directory URLByAppendingPathComponent:@"stdout"].fileSystemRepresentation, O_WRONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC);
    struct stat inInfo, outInfo;
    bool valid = input >= 0 && output >= 0 && !fstat(input, &inInfo) && !fstat(output, &outInfo) &&
        S_ISFIFO(inInfo.st_mode) && S_ISFIFO(outInfo.st_mode) && inInfo.st_nlink == 1 && outInfo.st_nlink == 1;
    if (valid) valid = fcntl(input, F_SETFL, 0) == 0 && fcntl(output, F_SETFL, 0) == 0 &&
        dup2(input, STDIN_FILENO) >= 0 && dup2(output, STDOUT_FILENO) >= 0 && dup2(output, STDERR_FILENO) >= 0;
    if (input >= 0) close(input);
    if (output >= 0) close(output);
    if (!valid) return false;
    setvbuf(stdout, nullptr, _IOLBF, 0);
    int status = open([directory URLByAppendingPathComponent:@"exit"].fileSystemRepresentation,
        O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (status < 0) return false;
    exitFile = status;
    if (chdir([request[@"directory"] fileSystemRepresentation])) return false;
    for (NSString* key in request[@"environment"]) setenv(key.UTF8String, [request[@"environment"][key] UTF8String], 1);
    launchedWithRequest = true;
    // Launch Services owns this process, so a sandboxed launcher cannot send
    // it SIGKILL. Keep force-stop and launcher-death handling independent of
    // SDL, guest threads, and even a debugger wait during startup.
    int control = open(directory.fileSystemRepresentation, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (control < 0) return false;
    int lifetime = openat(control, "lifetime", O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC);
    struct stat lifetimeInfo;
    if (lifetime < 0 || fstat(lifetime, &lifetimeInfo) || !S_ISFIFO(lifetimeInfo.st_mode) || lifetimeInfo.st_nlink != 1) {
        if (lifetime >= 0) close(lifetime);
        close(control);
        return false;
    }
    std::thread([control, lifetime]() {
        for (;;) {
            // Darwin does not reliably report POLLHUP for named FIFOs. A
            // dedicated nonblocking stream detects EOF without stealing quit
            // commands from the runtime's stdin reader.
            char ignored;
            bool ended = read(lifetime, &ignored, 1) == 0;
            bool force = faccessat(control, "force", F_OK, 0) == 0;
            if (force || ended) {
                macLaunchReportExit(force ? 137 : 0);
                _Exit(force ? 137 : 0);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }).detach();
    return [[NSString stringWithFormat:@"%d\n", getpid()] writeToURL:[directory URLByAppendingPathComponent:@"ready"]
        atomically:YES encoding:NSUTF8StringEncoding error:nil];
}

#ifdef BOXEDWINE_DEVELOPMENT
extern "C" int MacPlatformWaitForDebugger(void);
#endif

int macLaunch(int argc, const char** argv, int (*run)(int, const char**)) {
    @autoreleasepool {
        NSDictionary* request = nil;
        std::vector<const char*> arguments;
        if (argc == 1 || (argc == 2 && !strncmp(argv[1], "-psn_", 5))) {
            [NSApplication sharedApplication];
            BWEngineLaunch* delegate = [BWEngineLaunch new];
            NSApp.delegate = delegate;
            NSAppleEventManager* events = NSAppleEventManager.sharedAppleEventManager;
            [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
            [NSApp finishLaunching];
            NSDate* deadline = [NSDate dateWithTimeIntervalSinceNow:15];
            while (!delegate.request && deadline.timeIntervalSinceNow > 0) {
                NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]
                    inMode:NSDefaultRunLoopMode dequeue:YES];
                if (event) [NSApp sendEvent:event];
            }
            NSApp.delegate = nil; // SDL installs its usual application delegate.
            [events removeEventHandlerForEventClass:kCoreEventClass andEventID:kAEOpenApplication];
            request = loadRequest(delegate.request);
            if (!request || !connectSession(delegate.request, request)) {
                NSLog(@"Boxedwine could not read its engine launch request.");
                macLaunchReportExit(2);
                return 2;
            }
            arguments.push_back(argv[0]);
            for (NSString* argument in request[@"arguments"]) arguments.push_back(argument.UTF8String);
            argc = (int)arguments.size();
            arguments.push_back(nullptr);
            argv = arguments.data();
        } else {
            launchAccess = [NSMutableArray array];
            restoreCommandLineAccess();
        }
        const char* control = getenv("BOXEDWINE_STDIN_CONTROL");
        stdinControl = control && !strcmp(control, "1");
        unsetenv("BOXEDWINE_STDIN_CONTROL"); // Applies only to this host engine.
#ifdef BOXEDWINE_DEVELOPMENT
        if (!MacPlatformWaitForDebugger()) { macLaunchReportExit(0); return 0; }
#endif
        int status = run(argc, argv);
        fflush(stdout); fflush(stderr);
        macLaunchReportExit(status);
        for (NSURL* url in launchAccess) [url stopAccessingSecurityScopedResource];
        launchAccess = nil;
        for (NSURL* url in commandLineAccess) [url stopAccessingSecurityScopedResource];
        commandLineAccess = nil;
        return status;
    }
}
