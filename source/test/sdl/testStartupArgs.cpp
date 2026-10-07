/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST
#include "../cpu/testCPU.h"
#include "../../sdl/startupArgs.h"

void testStartupArgsMouseSensitivity() {
    for (const char* value : {"0", "1", "50", "100", "200", "1000"}) {
        StartUpArgs startup;
        const char* argv[] = {"boxedwine", "-rel_mouse_sensitivity", value};
        if (!startup.parseStartupArgs(3, argv) || startup.rel_mouse_sensitivity != (U32)atoi(value)) {
            testFail("Mouse sensitivity percentage was not parsed: %s", value);
        }
        auto child = startup.buildArgs();
        auto option = std::find(child.begin(), child.end(), B("-rel_mouse_sensitivity"));
        if (atoi(value) && (option == child.end() || ++option == child.end() || *option != value)) {
            testFail("Mouse sensitivity was not preserved in child arguments: %s", value);
        }
    }
    for (const char* value : {"", "-1", "1001", "50.5", "abc", "50percent", "4294967296"}) {
        StartUpArgs startup;
        const char* argv[] = {"boxedwine", "-rel_mouse_sensitivity", value};
        if (startup.parseStartupArgs(3, argv)) testFail("Invalid mouse sensitivity accepted: %s", value);
    }
    StartUpArgs startup;
    const char* missing[] = {"boxedwine", "-rel_mouse_sensitivity"};
    if (startup.parseStartupArgs(2, missing)) testFail("Missing mouse sensitivity accepted");
}

static bool hasExactEnvValue(const std::vector<BString>& envValues, const char* value) {
    for (auto& envValue : envValues) {
        if (envValue == value) {
            return true;
        }
    }
    return false;
}

void testStartupArgsDefaultUtf8LocaleEnvironment() {
    std::vector<BString> unsupportedEnvValues;
    StartUpArgs::addDefaultUtf8LocaleEnv(unsupportedEnvValues, false);

    if (unsupportedEnvValues.size() != 0) {
        testFail("default UTF-8 locale env values were added without guest locale support");
    }

    std::vector<BString> envValues;
    StartUpArgs::addDefaultUtf8LocaleEnv(envValues, true);

    if (!hasExactEnvValue(envValues, "LANG=en_US.UTF-8")) {
        testFail("default LANG was not added");
    }
    if (!hasExactEnvValue(envValues, "LC_ALL=en_US.UTF-8")) {
        testFail("default LC_ALL was not added");
    }

    std::vector<BString> explicitEnvValues;
    explicitEnvValues.push_back(B("LANG=C"));
    explicitEnvValues.push_back(B("LC_ALL=C"));
    StartUpArgs::addDefaultUtf8LocaleEnv(explicitEnvValues, true);

    if (explicitEnvValues.size() != 2) {
        testFail("explicit locale env values were not preserved");
    }
    if (!hasExactEnvValue(explicitEnvValues, "LANG=C")) {
        testFail("explicit LANG was overwritten");
    }
    if (!hasExactEnvValue(explicitEnvValues, "LC_ALL=C")) {
        testFail("explicit LC_ALL was overwritten");
    }
}

void testStartupArgsLinearMemoryOption() {
    StartUpArgs startupArgs;
    const char* argv[] = {"boxedwine", "-disableLinearMemory"};
    if (!startupArgs.parseStartupArgs(2, argv) || !startupArgs.disableLinearMemory) {
        testFail("-disableLinearMemory was not parsed");
        return;
    }

    std::vector<BString> childArgs = startupArgs.buildArgs();
    bool found = false;
    for (const BString& arg : childArgs) {
        if (arg == "-disableLinearMemory") {
            found = true;
            break;
        }
    }
    if (!found) {
        testFail("-disableLinearMemory was not propagated to a child process");
    }
}

#endif
