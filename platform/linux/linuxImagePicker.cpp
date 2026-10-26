/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#if defined(__linux__)
#include "linuxImagePicker.h"
#include <spawn.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <vector>

extern char** environ;

LinuxImagePicker::~LinuxImagePicker() { cancel(); }

bool LinuxImagePicker::open(bool wayland) {
    if (isOpen()) return true;
    int descriptors[2];
    if (pipe2(descriptors, O_CLOEXEC) < 0) return false;
    for (int& fd : descriptors) {
        if (fd <= STDERR_FILENO) {
            int original = fd;
            fd = fcntl(original, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
            close(original);
            if (fd < 0) {
                for (int descriptor : descriptors) if (descriptor >= 0) close(descriptor);
                return false;
            }
        }
    }
    if (fcntl(descriptors[0], F_SETFL, O_NONBLOCK) < 0) {
        close(descriptors[0]); close(descriptors[1]);
        return false;
    }

    // Set the backend only in the child. Launchers may have forced their own
    // GTK/Qt UI to X11, which must not send a Wayland guest's picker to X11.
    std::vector<std::string> environment;
    for (char** entry = environ; *entry; ++entry) {
        std::string value(*entry);
        if (wayland && (value.rfind("GDK_BACKEND=", 0) == 0 || value.rfind("QT_QPA_PLATFORM=", 0) == 0)) continue;
        environment.push_back(std::move(value));
    }
    if (wayland) {
        environment.emplace_back("GDK_BACKEND=wayland");
        environment.emplace_back("QT_QPA_PLATFORM=wayland");
    }
    std::vector<char*> env;
    for (auto& value : environment) env.push_back(value.data());
    env.push_back(nullptr);

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    posix_spawn_file_actions_adddup2(&actions, descriptors[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&actions, descriptors[0]);
    posix_spawn_file_actions_addclose(&actions, descriptors[1]);
    // Arguments are passed directly, without a shell or filename interpolation.
    const char* zenity[] = {"zenity", "--file-selection", "--title=Choose CD-ROM disc image",
        "--file-filter=Disc images | *.iso *.cue *.ISO *.CUE", "--file-filter=All files | *", nullptr};
    const char* kdialog[] = {"kdialog", "--title", "Choose CD-ROM disc image", "--getopenfilename",
        ".", "*.iso *.cue *.ISO *.CUE|Disc images\n*|All files", nullptr};
    pid_t child;
    int error = posix_spawnp(&child, zenity[0], &actions, nullptr, const_cast<char**>(zenity), env.data());
    if (error == ENOENT) error = posix_spawnp(&child, kdialog[0], &actions, nullptr, const_cast<char**>(kdialog), env.data());
    posix_spawn_file_actions_destroy(&actions);
    close(descriptors[1]);
    if (error) { close(descriptors[0]); return false; }
    pid = child;
    output = descriptors[0];
    result.clear();
    readFailed = false;
    return true;
}

bool LinuxImagePicker::poll(std::string& path, bool& failed) {
    if (!isOpen()) return false;
    auto drain = [&]() {
        char buffer[4096];
        for (int reads = 0; reads < 16; ++reads) {
            ssize_t count = read(output, buffer, sizeof(buffer));
            if (count > 0) {
                // Bound malformed helper output without blocking on a full pipe.
                if (result.size() + count <= 65536) result.append(buffer, count);
                else readFailed = true;
            } else if (count < 0 && errno == EINTR) continue;
            else {
                if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) readFailed = true;
                break;
            }
        }
    };
    drain();
    int status = 0;
    pid_t finished = waitpid(pid, &status, WNOHANG);
    if (!finished || (finished < 0 && errno == EINTR)) return false;
    drain(); // Include output written between the first read and child exit.
    close(output);
    output = pid = -1;
    failed = readFailed || finished < 0 || !WIFEXITED(status) || WEXITSTATUS(status) > 1;
    path.clear();
    if (!failed && WEXITSTATUS(status) == 0) {
        if (!result.empty() && result.back() == '\n') result.pop_back();
        path = std::move(result);
    }
    result.clear();
    return true;
}

void LinuxImagePicker::cancel() {
    if (isOpen()) {
        // This is our direct child, never a shell or an unrelated desktop app.
        // Reap it now so a destroyed guest cannot later receive its selection.
        kill(pid, SIGKILL);
        while (waitpid(pid, nullptr, 0) < 0 && errno == EINTR) {}
    }
    if (output >= 0) close(output);
    output = pid = -1;
    result.clear();
}
#endif
