/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <string>

// The external picker must never block SDL's event/Wayland dispatch thread.
class LinuxImagePicker {
public:
    LinuxImagePicker() = default;
    LinuxImagePicker(const LinuxImagePicker&) = delete;
    LinuxImagePicker& operator=(const LinuxImagePicker&) = delete;
    ~LinuxImagePicker();
    bool open(bool wayland);
    bool poll(std::string& path, bool& failed);
    bool isOpen() const { return pid > 0; }
    void cancel();
private:
    int pid = -1;
    int output = -1;
    std::string result;
    bool readFailed = false;
};
