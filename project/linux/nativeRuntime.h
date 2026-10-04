/* Copyright (C) 2026 The Boxedwine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>

// A blocking fgets holds glibc's stdin lock. Process exit then waits forever
// while flushing/closing stdio if the launcher still owns the write end.
// Read the descriptor directly so a naturally exiting Linux child can finish.
inline void startLinuxNativeControl() {
    std::thread([]() {
        char command[32];
        size_t length = 0;
        bool overflow = false;
        char value;
        for (;;) {
            const ssize_t count = ::read(STDIN_FILENO, &value, 1);
            if (count < 0 && errno == EINTR) {
                continue;
            }
            if (count <= 0) {
                std::_Exit(0); // The owning launcher closed or crashed.
            }
            if (value == '\n') {
                if (!overflow && length == 4 && !memcmp(command, "quit", 4)) {
                    KNativeSystem::postQuit();
                }
                length = 0;
                overflow = false;
            } else if (length < sizeof(command)) {
                command[length++] = value;
            } else {
                overflow = true;
            }
        }
    }).detach();
}
