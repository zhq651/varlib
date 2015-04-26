/**
 * @file waitable_event.h
 * @brief 
 * @author liangsijian
 * @version 
 * @date 2015-04-26
 */

#ifndef WAITABLE_EVENT_H_BCPZO4MV
#define WAITABLE_EVENT_H_BCPZO4MV

#include <windows.h>
static const int kNoTimeout = -1;
class WaitableEvent{
public:
    // If manual_reset is true, then to set the event state to non-signaled, a
    // consumer must call the Reset method.  If this parameter is false, then the
    // system automatically resets the event state to non-signaled after a single
    // waiting thread has been released.
    WaitableEvent(bool manual_reset, bool initially_signaled);

    // Create a WaitableEvent from an Event HANDLE which has already been
    // created. This objects takes ownership of the HANDLE and will close it when
    // deleted.
    explicit WaitableEvent(HANDLE event_handle);

    // Releases ownership of the handle from this object.
    HANDLE Release();

    ~WaitableEvent();

    // Put the event in the un-signaled state.
    void Reset();

    // Put the event in the signaled state.  Causing any thread blocked on Wait
    // to be woken up.
    void Signal();

    // Returns true if the event is in the signaled state, else false.  If this
    // is not a manual reset event, then this test will cause a reset.
    bool IsSignaled();

    // Wait indefinitely for the event to be signaled.  Returns true if the event
    // was signaled, else false is returned to indicate that waiting failed.
    bool Wait();

    // Wait up until max_time has passed for the event to be signaled.  Returns
    // true if the event was signaled.  If this method returns false, then it
    // does not necessarily mean that max_time was exceeded.
    bool TimedWait(const int& max_time);

    HANDLE handle() const    {        return handle_;    }

    // Wait, synchronously, on multiple events.
    //   waitables: an array of WaitableEvent pointers
    //   count: the number of elements in @waitables
    //
    // returns: the index of a WaitableEvent which has been signaled.
    //
    // You MUST NOT delete any of the WaitableEvent objects while this wait is
    // happening.
    static size_t WaitMany(WaitableEvent** waitables, size_t count);
private:
    WaitableEvent(const WaitableEvent& other);
    void operator = (const WaitableEvent& other);

    HANDLE handle_;


};


#endif /* end of include guard: WAITABLE_EVENT_H_BCPZO4MV */
