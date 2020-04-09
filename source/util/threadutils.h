#ifndef __THREAD_UTILS_H__
#define __THREAD_UTILS_H__

void runInBackgroundThread(std::function<void(void)> f);

#endif