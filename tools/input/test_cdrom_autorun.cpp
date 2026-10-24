/* Copyright (C) 2026 The BoxedWine Team. SPDX-License-Identifier: GPL-2.0-or-later
 * g++ -std=c++20 tools/input/test_cdrom_autorun.cpp -o /tmp/test-cdrom-autorun && /tmp/test-cdrom-autorun
 */
#include "../../source/io/cdromautorun.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace CdromAutorun;
    assert(command("[other]\nopen=bad.exe\n [AutoRun] \r\nOpen = \"Setup Files\\setup.exe\" /language \"English UK\"\n") ==
        (std::vector<std::string>{"Setup Files\\setup.exe", "/language", "English UK"}));
    assert(command("[autorun]\nshellexecute=setup.msi\n") == std::vector<std::string>{"setup.msi"});
    assert(command("[autorun]\nshellexecute=other.exe\nopen=setup.exe\n") == std::vector<std::string>{"setup.exe"});
    assert(command("[autorun]\n;open=bad.exe\n[other]\nopen=bad.exe\n").empty());
    assert(command("[autorun]\nopen=\"unfinished.exe\n").empty());
    assert(command(std::string(65537, 'x')).empty());
    assert(command(std::string("[autorun]\nopen=setup.exe\0bad", 27)).empty());
    assert(command("\xef\xbb\xbf[autorun]\nopen=setup.exe") == std::vector<std::string>{"setup.exe"});
    std::string utf16 = "\xff\xfe";
    for (char c : std::string("[autorun]\nopen=setup.exe")) { utf16 += c; utf16 += '\0'; }
    assert(command(utf16) == std::vector<std::string>{"setup.exe"});
    assert(split(R"(setup.exe "" "C:\a b\\" "say \"hello\"" A&B %value%)") ==
        (std::vector<std::string>{"setup.exe", "", "C:\\a b\\", "say \"hello\"", "A&B", "%value%"}));
    assert(path(".\\Setup Files\\setup.exe", 'e') == (std::vector<std::string>{"Setup Files", "setup.exe"}));
    assert(path("E:\\setup.exe", 'e') == std::vector<std::string>{"setup.exe"});
    for (auto value : {"..\\setup.exe", "C:\\setup.exe", "\\\\server\\setup.exe", "https://example.com/setup.exe", ""}) assert(path(value, 'e').empty());
    std::cout << "CD-ROM autorun parsing, quoting, encoding and disc path checks passed\n";
}
