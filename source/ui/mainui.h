#ifndef __MAIN_UI_H__
#define __MAIN_UI_H__

bool uiShow(BString basePath); // returns true if should launch app
bool uiContinue();

bool uiLoop();
void uiShutdown();
bool uiIsRunning();

void runOnMainUI(std::function<bool()> f, U64 delayInMillies=0);
void runAfterFrame(std::function<bool()> f, U64 delayInMillies=0);

#endif