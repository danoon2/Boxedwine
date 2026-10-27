/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
int macLaunch(int argc, const char** argv, int (*run)(int, const char**));
bool macLaunchHasControl();
void macLaunchReportExit(int status);
bool macLaunchIsCommandLine();
const char* macLaunchWorkingDirectory();
// CLI-only access checks. Document launches receive grants from their launcher.
bool macLaunchAccess(const char* path, bool directory, bool write, bool allowMissing = false);
// Call after SDL_Init, before installing the per-app Dock icon.
void macLaunchFinishApplication(bool visible);
