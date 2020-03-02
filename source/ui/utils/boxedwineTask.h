#ifndef __BOXEDWINE_TASK_H__
#define __BOXEDWINE_TASK_H__

class BoxedwineTask {
public:
    BoxedwineTask(int data, std::function<void()> f);

    void start();

    std::thread thread;    
    bool isDone() {return this->isThreadDone;}

    int data;

private:
    static void runTask(BoxedwineTask* task);

    std::function<void()> function;
    bool isThreadDone;
};

#endif