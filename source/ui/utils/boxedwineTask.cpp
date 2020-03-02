#include "boxedwine.h"
#include "../boxedwineui.h"

void BoxedwineTask::runTask(BoxedwineTask* task) {
    task->function();
    task->isThreadDone=true;
}

BoxedwineTask::BoxedwineTask(int data, std::function<void()> f) : data(data), function(f), isThreadDone(false) {
}

void BoxedwineTask::start() {
    this->thread = std::thread(runTask, this);
}

