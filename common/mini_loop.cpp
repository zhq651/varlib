#include <windows.h>
#include <assert.h>
#include "pump.h"
#include "mini_loop.h"
MiniLoop::AutoRunState::AutoRunState(MiniLoop* loop)
{
    loop_ = loop;
    previous_state_ = loop->state_;
    if (previous_state_) {
       run_depth = previous_state_->run_depth + 1;
    }
    else
       run_depth = 1;
    loop_->state_ = this;
    quit_received = false;
}

MiniLoop::AutoRunState::~AutoRunState()
{
    loop_->state_ = previous_state_;
}

MiniLoop::MiniLoop(LOOP_TYPE type /*= DEFAULT_LOOP*/)
: next_seq_num(0), state_(NULL)
{
    if (type == DEFAULT_LOOP) {
        pump_ = new MiniPumpDefault();
    }
    else {
        assert(false);
    }
}
void MiniLoop::PostJob_Helper(Job* job, int delay_ms)
{
    PendingJob pending_job(job);
    if (delay_ms > 0) {
        pending_job.delay_run_ticks = kTime() + delay_ms;
    }
    MiniPump* pump = pump_;
    {
        lightweight_mutex::scoped_lock lock(incoming_job_lock_);
        bool is_empty = incoming_job_.empty();
        incoming_job_.push(pending_job);
        if (!is_empty) {
            return;
        }
    }
    pump->ScheduleWork();
}


void MiniLoop::Run()
{
    AutoRunState save_state(this);
    pump_->Run(this);
}

bool MiniLoop::DoWork()
{
    for (;;) {
        ReloadWorkQueue();
        if (work_job_.empty()) {
            break;
        }
        do 
        {
            PendingJob pending_job = work_job_.front();
            work_job_.pop();
            if (pending_job.delay_run_ticks > 0) {
                AddToDelayedWorkQueue(pending_job);
                if (delayed_job_.top().job == pending_job.job) {
                    pump_->ScheduleDelayedWork(pending_job.delay_run_ticks);
                }
            }
            else{
                if (DeferOrRunPendingJob(pending_job))
                    return true;
            }
        } while (!work_job_.empty());
    }
    return false;
}

bool MiniLoop::DoDelayedWork(int* next_delayed_work_time)
{
    if (delayed_job_.empty()) {
        *next_delayed_work_time = 0;
        return false;
    }
    if (delayed_job_.top().delay_run_ticks > kTime()) {
        *next_delayed_work_time = delayed_job_.top().delay_run_ticks;
        return false;
    }
    PendingJob pending_job = delayed_job_.top();
    delayed_job_.pop();
    if (!delayed_job_.empty()) {
        *next_delayed_work_time = delayed_job_.top().delay_run_ticks;
    }
    return DeferOrRunPendingJob(pending_job);
}

bool MiniLoop::DoIdleWork()
{
    if (state_->quit_received)
        pump_->Quit();
    return false;
}

void MiniLoop::RunJob(Job* job)
{
    job->Run();
    delete job;
}

void MiniLoop::ReloadWorkQueue()
{
    if (!work_job_.empty()) {
        return;
    }
    {
        lightweight_mutex::scoped_lock lock(incoming_job_lock_);
        if (incoming_job_.empty()) {
            return;
        }
        std::swap(work_job_, incoming_job_);
        assert(incoming_job_.empty());
    }
}

void MiniLoop::AddToDelayedWorkQueue(PendingJob job)
{
    assert(job.delay_run_ticks > 0);
    PendingJob new_job(job);
    job.seq_num = next_seq_num ++;
    delayed_job_.push(job);
}

bool MiniLoop::DeferOrRunPendingJob(PendingJob pending_job)
{
    Job* job = pending_job.job;
    RunJob(job);
    return true;
}

void MiniLoop::Quit()
{
    if (state_) {
        state_->quit_received = true;
    }
    else {
        assert(false);
    }
}

