/**
 * @file pump.h
 * @brief core of message loop, schedule the work
 * @author liangsijian
 * @version 
 * @date 2015-04-26
 */

#ifndef PUMP_H_7ITFKA4L
#define PUMP_H_7ITFKA4L
#include "waitable_event.h"
extern int(* const kTime)();
class MiniPump 
{
public:
    class Delegate {
    public:
        virtual ~Delegate() {}
        virtual bool DoWork() = 0;
        virtual bool DoDelayedWork(int* next_delayed_work_time) = 0;
        virtual bool DoIdleWork() = 0;
    };
    virtual ~MiniPump() {}
    virtual void Run(Delegate* delegate) = 0;
    virtual void Quit() = 0;
    virtual void ScheduleWork() = 0;
    virtual void ScheduleDelayedWork(const int& delayed_work_time) = 0;
};
class MiniPumpDefault : public MiniPump {
public:
    MiniPumpDefault();
    ~MiniPumpDefault() {}

    // MessagePump methods:
    virtual void Run(Delegate* delegate);
    virtual void Quit();
    virtual void ScheduleWork();
    virtual void ScheduleDelayedWork(const int& delayed_work_time);

private:
    bool keep_running_;

    WaitableEvent event_;

    int delayed_work_time_;
};

#endif /* end of include guard: PUMP_H_7ITFKA4L */
