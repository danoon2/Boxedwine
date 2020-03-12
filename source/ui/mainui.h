#ifndef __MAIN_UI_H__
#define __MAIN_UI_H__

bool uiShow(const std::string& basePath); // returns true if should launch app

bool uiLoop();
void uiShutdown();
bool uiIsRunning();

void runOnMainUI(std::function<bool()> f, U64 delayInMillies=0);

#endif