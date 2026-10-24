/* Copyright (C) 2026 The BoxedWine Team. SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

// Autorun is an explicit installation action, never a side effect of a swap.
// Keep parsing independent of the host shell and of the ISO/CUE sector layout.
namespace CdromAutorun {
inline std::string trim(std::string value) {
    auto first = value.find_first_not_of(" \t\r\n");
    return first == std::string::npos ? "" : value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
}
inline std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return value;
}
inline void utf8(std::string& out, unsigned cp) {
    if (cp < 0x80) out += (char)cp;
    else if (cp < 0x800) { out += (char)(0xc0 | (cp >> 6)); out += (char)(0x80 | (cp & 63)); }
    else if (cp < 0x10000) { out += (char)(0xe0 | (cp >> 12)); out += (char)(0x80 | ((cp >> 6) & 63)); out += (char)(0x80 | (cp & 63)); }
    else { out += (char)(0xf0 | (cp >> 18)); out += (char)(0x80 | ((cp >> 12) & 63)); out += (char)(0x80 | ((cp >> 6) & 63)); out += (char)(0x80 | (cp & 63)); }
}
inline std::string decode(const std::string& bytes) {
    if (bytes.size() > 65536) return "";
    if (bytes.compare(0, 3, "\xef\xbb\xbf") == 0) return bytes.substr(3);
    if (bytes.size() < 2 || ((unsigned char)bytes[0] != 0xff || (unsigned char)bytes[1] != 0xfe)) return bytes;
    if (bytes.size() % 2) return "";
    std::string result;
    auto unit = [&](size_t i) { return (unsigned char)bytes[i] | ((unsigned char)bytes[i + 1] << 8); };
    for (size_t i = 2; i < bytes.size(); i += 2) {
        unsigned cp = unit(i);
        if (cp >= 0xd800 && cp <= 0xdbff) {
            if (i + 3 >= bytes.size()) return "";
            unsigned tail = unit(i + 2);
            if (tail < 0xdc00 || tail > 0xdfff) return "";
            cp = 0x10000 + ((cp - 0xd800) << 10) + tail - 0xdc00; i += 2;
        } else if (cp >= 0xdc00 && cp <= 0xdfff) return "";
        utf8(result, cp);
    }
    return result;
}
// Windows command-line quoting, including escaped quotes and trailing slashes.
inline std::vector<std::string> split(const std::string& command) {
    std::vector<std::string> result;
    size_t i = 0;
    while (i < command.size()) {
        while (i < command.size() && (command[i] == ' ' || command[i] == '\t')) ++i;
        if (i == command.size()) break;
        std::string word; bool quoted = false;
        while (i < command.size() && (quoted || (command[i] != ' ' && command[i] != '\t'))) {
            size_t slashes = 0;
            while (i < command.size() && command[i] == '\\') { ++slashes; ++i; }
            if (i < command.size() && command[i] == '"') {
                word.append(slashes / 2, '\\');
                if (slashes % 2) word += '"';
                else if (quoted && i + 1 < command.size() && command[i + 1] == '"') { word += '"'; ++i; }
                else quoted = !quoted;
                ++i;
            } else {
                word.append(slashes, '\\');
                if (i < command.size() && (quoted || (command[i] != ' ' && command[i] != '\t'))) word += command[i++];
            }
        }
        if (quoted) return {}; // Incomplete entries fall back to browsing.
        result.push_back(word);
        if (result.size() > 256) return {};
    }
    return result;
}
inline std::vector<std::string> command(const std::string& bytes) {
    std::string ini = decode(bytes), open, shell;
    if (ini.find('\0') != std::string::npos) return {};
    bool section = false;
    for (size_t i = 0; i < ini.size();) {
        size_t end = ini.find('\n', i);
        std::string line = trim(ini.substr(i, end == std::string::npos ? end : end - i));
        i = end == std::string::npos ? ini.size() : end + 1;
        if (line.empty() || line[0] == ';') continue;
        if (line[0] == '[') { section = lower(line) == "[autorun]"; continue; }
        if (!section) continue;
        size_t equals = line.find('=');
        if (equals == std::string::npos) continue;
        std::string key = lower(trim(line.substr(0, equals))), value = trim(line.substr(equals + 1));
        if (key == "open" && open.empty()) open = value;
        else if (key == "shellexecute" && shell.empty()) shell = value;
    }
    return split(open.empty() ? shell : open);
}
// Only resolve a target on this disc. Never reinterpret it as a host path or
// shell expression. The caller resolves case-insensitively through FsIso.
inline std::vector<std::string> path(std::string target, char drive) {
    std::replace(target.begin(), target.end(), '\\', '/');
    if (target.size() >= 2 && target[1] == ':') {
        if (std::tolower((unsigned char)target[0]) != std::tolower((unsigned char)drive)) return {};
        target.erase(0, 2);
    }
    if (target.find(':') != std::string::npos || target.starts_with("//")) return {};
    std::vector<std::string> parts;
    for (size_t i = 0; i < target.size();) {
        size_t end = target.find('/', i);
        std::string part = target.substr(i, end == std::string::npos ? end : end - i);
        i = end == std::string::npos ? target.size() : end + 1;
        if (part == "..") return {};
        if (!part.empty() && part != ".") parts.push_back(part);
    }
    return parts;
}
}
