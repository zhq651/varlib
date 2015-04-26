#include <windows.h>
#include <assert.h>
#include "pump.h"
int NowTicks(){
    return ::GetTickCount();
}
int(* const kTime)()= NowTicks;

MiniPumpDefault::MiniPumpDefault()
: keep_running_(true),
event_(false, false) {
}

void MiniPumpDefault::Run(Delegate* delegate) {

    assert(keep_running_);
    for (;;) {

        bool did_work = delegate->DoWork();
        if (!keep_running_)
            break;

        did_work |= delegate->DoDelayedWork(&delayed_work_time_);
        if (!keep_running_)
            break;

        if (did_work)//干活了说明后面可能还用活没干，继续循环
            continue;

        did_work = delegate->DoIdleWork();//DoIdleWork优先级最低，只有前面的活干完了才轮到她
        if (!keep_running_)
            break;

        if (did_work)//同上
            continue;

        if (!delayed_work_time_) {//通过DoDelayedWork返回下一个延迟任务执行的时间点，为空说明没有延迟任务，进行阻塞
            event_.Wait();
        }
        else {
            int delay = delayed_work_time_ - kTime();//计算下一个延迟任务超时的时间间隔
            if (delay > 0) {
                event_.TimedWait(delay);
            }
            else {
                // It looks like delayed_work_time_ indicates a time in the past, so we
                // need to call DoDelayedWork now.
                delayed_work_time_ = kTime();
            }
        }
        // Since event_ is auto-reset, we don't need to do anything special here
        // other than service each delegate method.
    }

    keep_running_ = true;
}

void MiniPumpDefault::Quit() {
    keep_running_ = false;
}

void MiniPumpDefault::ScheduleWork() {
    event_.Signal();
}

void MiniPumpDefault::ScheduleDelayedWork(const int& delayed_work_time) {
    delayed_work_time_ = delayed_work_time;
}

