/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __SYNCHRONIZATION_H__
#define __SYNCHRONIZATION_H__

class BoxedWineCondition;

class BoxedWineConditionChild {
public:
    BoxedWineConditionChild(BoxedWineCondition* cond, const std::function<void(void)>& doneWaitingCallback) : cond(cond), doneWaitingCallback(doneWaitingCallback) {}
    BoxedWineCondition* cond;
    std::function<void(void)> doneWaitingCallback;
};

#ifdef BOXEDWINE_MULTI_THREADED
class BoxedWineCondition : public std::enable_shared_from_this<BoxedWineCondition> {
public:
    BoxedWineCondition(BString name);
    BoxedWineCondition() = default;
    ~BoxedWineCondition();

    bool tryLock();
    void lock();
    void signal();
    void signalAll();
    void wait(std::unique_lock<std::mutex>& lock);
    void waitWithTimeout(std::unique_lock<std::mutex>& lock, U32 ms);
    void unlock();
    void addParentCondition(const std::shared_ptr<BoxedWineCondition>& parent);
    void removeParentCondition(const std::shared_ptr<BoxedWineCondition>& parent);
    U32 parentsCount();
    U32 waitCount() {return parentsCount();}
    const BString name;

    std::mutex m;
    std::condition_variable c;
    U32 lockOwner = 0;

private:
#define MAX_PARENTS 2
    U32 parentCount = 0;
    std::weak_ptr<BoxedWineCondition> parents[MAX_PARENTS];
    std::mutex parentsMutex;
};

class BoxedWineCriticalSectionCond {
public:
    BoxedWineCriticalSectionCond(const std::shared_ptr<BoxedWineCondition>& cond);
    ~BoxedWineCriticalSectionCond();

private:
    std::shared_ptr<BoxedWineCondition> cond;
};

#define BOXEDWINE_CRITICAL_SECTION static std::recursive_mutex csMutex; const std::lock_guard<std::recursive_mutex> lock(csMutex);
#define BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(csMutex) const std::lock_guard<std::recursive_mutex> lock(csMutex);
#define BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION_MUTEX(csMutex) std::unique_lock<std::mutex> boxedWineCriticalSection(csMutex);
#define BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(csCond) std::unique_lock<std::mutex> boxedWineCriticalSection((csCond)->m);

#define BOXEDWINE_MUTEX std::recursive_mutex
#define BOXEDWINE_MUTEX_NR std::mutex
#define BOXEDWINE_MUTEX_LOCK(mutex) mutex.lock()
#define BOXEDWINE_MUTEX_TRY_LOCK(mutex) mutex.try_lock()
#define BOXEDWINE_MUTEX_UNLOCK(mutex) mutex.unlock()

#define BOXEDWINE_CONDITION std::shared_ptr<BoxedWineCondition>
#define BOXEDWINE_CONDITION_SIGNAL(cond) (cond)->signal()
#define BOXEDWINE_CONDITION_SIGNAL_ALL(cond) (cond)->signalAll()
#define BOXEDWINE_CONDITION_WAIT(cond) (cond)->wait(boxedWineCriticalSection)
#define BOXEDWINE_CONDITION_WAIT_TIMEOUT(cond, t) (cond)->waitWithTimeout(boxedWineCriticalSection, t)
#define BOXEDWINE_CONDITION_ADD_PARENT(cond, parent) (cond)->addParentCondition(parent)
#define BOXEDWINE_CONDITION_REMOVE_PARENT(cond, parent) (cond)->removeParentCondition(parent)

#define BoxedWineConditionTimer BoxedWineCondition
#else
#define BOXEDWINE_CRITICAL_SECTION
#define BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(csMutex)
#define BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION_MUTEX(csMutex)
#define BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(csCond)

typedef void* BOXEDWINE_MUTEX;
typedef void* BOXEDWINE_MUTEX_NR;
#define BOXEDWINE_MUTEX_LOCK(mutex)
#define BOXEDWINE_MUTEX_TRY_LOCK(mutex) true
#define BOXEDWINE_MUTEX_UNLOCK(mutex)

class KThread;
class BoxedWineCondition;

class BoxedWineConditionTimer : public KTimerCallback {
public:
    bool run() override;

    BoxedWineCondition* cond = nullptr;
};

class BoxedWineCondition : public std::enable_shared_from_this<BoxedWineCondition> {
public:
    BoxedWineCondition(BString name);
    ~BoxedWineCondition();

    void signal();
    void signalAll();
    U32 wait();
    U32 waitWithTimeout(U32 ms);
    U32 waitCount();

    void addParentCondition(const std::shared_ptr<BoxedWineCondition>& parent);
    void removeParentCondition(const std::shared_ptr<BoxedWineCondition>& parent);
    U32 parentsCount();    
    const BString name;
private:
    KList<KThread*> waitingThreads;    

#define MAX_PARENTS 2
    U32 parentCount = 0;
    std::weak_ptr<BoxedWineCondition> parents[MAX_PARENTS];

    friend BoxedWineConditionTimer;
    void signalThread(bool all);
};

#define BOXEDWINE_CONDITION std::shared_ptr<BoxedWineCondition>
#define BOXEDWINE_CONDITION_LOCK(cond)
#define BOXEDWINE_CONDITION_UNLOCK(cond)
#define BOXEDWINE_CONDITION_SIGNAL(cond) (cond)->signal()
#define BOXEDWINE_CONDITION_SIGNAL_ALL(cond) (cond)->signalAll()
#define BOXEDWINE_CONDITION_WAIT(cond) return (cond)->wait()
#define BOXEDWINE_CONDITION_WAIT_TIMEOUT(cond, ms) return (cond)->waitWithTimeout(ms)
#define BOXEDWINE_CONDITION_ADD_PARENT(cond, parent) (cond)->addParentCondition(parent)
#define BOXEDWINE_CONDITION_REMOVE_PARENT(cond, parent) (cond)->removeParentCondition(parent)

#endif

#endif
