/**
 * @file mini_loop.h
 * @brief a message loop for processing job or delay job
 * @author liangsijian
 * @version 
 * @date 2015-04-26
 */

#ifndef MINI_LOOP_H_5F47RPXT
#define MINI_LOOP_H_5F47RPXT
#include <vector>
#include <queue>
#include "pump.h"
#include "lighweight_mutex.h"
class Job
{
public:
    Job(){};
    virtual ~Job(){};
    virtual void Run() = 0;
private:
};


class MiniLoop : public MiniPump::Delegate
{

public:
    enum LOOP_TYPE
    {
        DEFAULT_LOOP = 0,
    };
    MiniLoop(LOOP_TYPE type = DEFAULT_LOOP);
    void PostJob(Job* job){
        PostJob_Helper(job, 0);
    }
    void PostDelayJob(Job* job, int dely_ms){
        if (dely_ms > 0) {
            PostJob_Helper(job, dely_ms);
        }
        else
            PostDelayJob(job, 0);
    }
    void Run();
    void Quit();
protected:
    virtual bool DoWork();
    virtual bool DoIdleWork();
    virtual bool DoDelayedWork(int* next_delayed_work_time);
    void ReloadWorkQueue();
    class PendingJob
    {
    public:
        PendingJob(Job* job)
            : job(job), delay_run_ticks(0), seq_num(0){

        }
        Job* job;
        int delay_run_ticks;
        int seq_num;
        bool operator < (const PendingJob& other) const {
            if (delay_run_ticks < other.delay_run_ticks) {
                return false;
            }
            else if (delay_run_ticks > other.delay_run_ticks) {
                return true;
            }
            return (seq_num - other.seq_num) > 0;
        }
    };
    void AddToDelayedWorkQueue(PendingJob job);

    void PostJob_Helper(Job* job, int delay_ms);
    bool DeferOrRunPendingJob(PendingJob pending_job);
    void RunJob(Job* job);
    struct RunState{
        int run_depth;
        bool quit_received;
    };
    class AutoRunState : RunState {
    public:
        explicit AutoRunState(MiniLoop* loop);
        ~AutoRunState();
    private:
        MiniLoop* loop_;
        RunState* previous_state_;
    };

private:
    typedef std::queue<PendingJob> JobQueue;
    typedef std::priority_queue<PendingJob> DelayedJobQueue;
    int next_seq_num;
    JobQueue incoming_job_;
    JobQueue work_job_;
    DelayedJobQueue delayed_job_;
    lightweight_mutex incoming_job_lock_;
    MiniPump* pump_;
    RunState *state_;
};


#endif /* end of include guard: MINI_LOOP_H_5F47RPXT */
