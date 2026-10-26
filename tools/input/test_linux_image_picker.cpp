/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#include "../../platform/linux/linuxImagePicker.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

int main(int argc, char** argv) {
    const std::string name = fs::path(argv[0]).filename();
    if (name == "zenity" || name == "kdialog") {
        assert(argc == (name == "zenity" ? 5 : 6));
        const bool wayland = std::string(getenv("PICKER_TEST_BACKEND")) == "wayland";
        assert(std::string(getenv("GDK_BACKEND")) == (wayland ? "wayland" : "x11"));
        assert(std::string(getenv("QT_QPA_PLATFORM")) == (wayland ? "wayland" : "xcb"));
        assert(std::string(argv[2]).find("Choose CD-ROM disc image") != std::string::npos);
        std::ofstream("ready.tmp") << getpid();
        fs::rename("ready.tmp", "ready");
        while (!fs::exists("release")) std::this_thread::sleep_for(1ms);
        // Preserve UTF-8, spaces, quotes and shell metacharacters literally.
        puts("/tmp/disc ü '$(no-shell)' name.iso");
        return std::stoi(getenv("PICKER_TEST_EXIT"));
    }
    fs::path executable = fs::canonical("/proc/self/exe");
    char temporary[] = "/tmp/boxedwine-picker-test-XXXXXX";
    assert(mkdtemp(temporary));
    fs::path previous = fs::current_path();
    fs::current_path(temporary);
    fs::create_symlink(executable, "zenity");
    fs::create_symlink(executable, "kdialog");
    setenv("PATH", temporary, 1);
    setenv("GDK_BACKEND", "x11", 1);
    setenv("QT_QPA_PLATFORM", "xcb", 1);

    LinuxImagePicker picker;
    std::string path;
    bool failed = false;
    auto start = [&](bool wayland, int exitCode) {
        fs::remove("ready"); fs::remove("release");
        setenv("PICKER_TEST_BACKEND", wayland ? "wayland" : "x11", 1);
        setenv("PICKER_TEST_EXIT", std::to_string(exitCode).c_str(), 1);
        assert(picker.open(wayland));
        assert(picker.open(wayland)); // A second click keeps the existing picker.
        auto deadline = std::chrono::steady_clock::now() + 5s;
        while (!fs::exists("ready") && std::chrono::steady_clock::now() < deadline) std::this_thread::sleep_for(1ms);
        assert(fs::exists("ready"));
        assert(!picker.poll(path, failed)); // Waiting for the user never blocks.
        assert(std::string(getenv("GDK_BACKEND")) == "x11");
        assert(std::string(getenv("QT_QPA_PLATFORM")) == "xcb");
    };
    auto finish = [&]() {
        std::ofstream("release") << "done";
        auto deadline = std::chrono::steady_clock::now() + 5s;
        while (!picker.poll(path, failed)) {
            assert(std::chrono::steady_clock::now() < deadline);
            std::this_thread::sleep_for(1ms);
        }
        assert(!picker.isOpen());
    };
    for (bool wayland : {false, true}) {
        start(wayland, 0); finish();
        assert(!failed && path == "/tmp/disc ü '$(no-shell)' name.iso");
        start(wayland, 1); finish();
        assert(!failed && path.empty()); // Cancel must not insert helper output.
        start(wayland, 2); finish();
        assert(failed && path.empty());
        start(wayland, 0);
        int child; std::ifstream("ready") >> child;
        picker.cancel();
        assert(!picker.isOpen() && !picker.poll(path, failed));
        assert(kill(child, 0) < 0 && errno == ESRCH);
        assert(waitpid(child, nullptr, WNOHANG) < 0 && errno == ECHILD);
    }
    fs::remove("zenity");
    start(true, 0); finish(); // Fall back to kdialog when Zenity isn't installed.
    assert(!failed && !path.empty());
    fs::remove("kdialog");
    assert(!picker.open(true) && !picker.isOpen());
    fs::current_path(previous);
    fs::remove_all(temporary);
    puts("Linux image picker: asynchronous completion, cancellation, backend isolation, UTF-8 paths, fallback and child cleanup passed");
}
