#include <assert.h>
#include <windows.h>
#include "waitable_event.h"


WaitableEvent::WaitableEvent(bool manual_reset, bool signaled)
        : handle_(CreateEvent(NULL, manual_reset, signaled, NULL)){
    // We're probably going to crash anyways if this is ever NULL, so we might as
    // well make our stack reports more informative by crashing here.
    assert(handle_);
}

WaitableEvent::WaitableEvent(HANDLE handle)
        : handle_(handle){
    assert(handle);
}

WaitableEvent::~WaitableEvent(){
    CloseHandle(handle_);
}

HANDLE WaitableEvent::Release(){
    HANDLE rv = handle_;
    handle_ = INVALID_HANDLE_VALUE;
    return rv;
}

void WaitableEvent::Reset(){
    ResetEvent(handle_);
}

void WaitableEvent::Signal(){
    SetEvent(handle_);
}

bool WaitableEvent::IsSignaled(){
    return TimedWait(0);
}

bool WaitableEvent::Wait(){
    DWORD result = WaitForSingleObject(handle_, INFINITE);
    // It is most unexpected that this should ever fail.  Help consumers learn
    // about it if it should ever fail.
    assert(result == WAIT_OBJECT_0);
    return result == WAIT_OBJECT_0;
}

bool WaitableEvent::TimedWait(const int& milliSeconds){
    assert(milliSeconds >= 0);
    // Be careful here.  TimeDelta has a precision of microseconds, but this API
    // is in milliseconds.  If there are 5.5ms left, should the delay be 5 or 6?
    // It should be 6 to avoid returning too early.
    DWORD result = WaitForSingleObject(handle_, static_cast<DWORD>(milliSeconds));
    switch (result)    {
    case WAIT_OBJECT_0:
        return true;
    case WAIT_TIMEOUT:
        return false;
    }
    // It is most unexpected that this should ever fail.  Help consumers learn
    // about it if it should ever fail.
    assert(false);
    return false;
}

// static
size_t WaitableEvent::WaitMany(WaitableEvent** events, size_t count){
    HANDLE handles[MAXIMUM_WAIT_OBJECTS];
    assert(count <= MAXIMUM_WAIT_OBJECTS);

    for (size_t i = 0; i < count; ++i)
        handles[i] = events[i]->handle();

    DWORD result =
        WaitForMultipleObjects(count, handles,
                               FALSE,      // don't wait for all the objects
                               INFINITE);  // no timeout
    if (result < WAIT_OBJECT_0 || result >= WAIT_OBJECT_0 + count)    {
        assert(false);
        return 0;
    }

    return result - WAIT_OBJECT_0;
}
#ifdef TEST
#include "platform_thread.h"
#define EXPECT_TRUE(x) assert((x))
#define EXPECT_FALSE(x) assert(!(x))
#define EXPECT_EQ(x,y) assert((x)==(y))
class WaitableEventSignaler : public PlatformThread::Delegate {
public:
    WaitableEventSignaler(double seconds, WaitableEvent* ev)
        : seconds_(seconds),
        ev_(ev) {
    }

    void ThreadMain() {
        PlatformThread::Sleep(static_cast<int>(seconds_ * 1000));
        ev_->Signal();
    }

private:
    const double seconds_;
    WaitableEvent *const ev_;
};
int main(int argc, const char * argv[])
{
    {
        WaitableEvent event(true, false);

        EXPECT_FALSE(event.IsSignaled());

        event.Signal();
        EXPECT_TRUE(event.IsSignaled());
        EXPECT_TRUE(event.IsSignaled());

        event.Reset();
        EXPECT_FALSE(event.IsSignaled());
        EXPECT_FALSE(event.TimedWait(0));

        event.Signal();
        EXPECT_TRUE(event.Wait());
        EXPECT_TRUE(event.TimedWait(10));

    }
    {
        WaitableEvent* ev[5];
        for (unsigned i = 0; i < 5; ++i)
            ev[i] = new WaitableEvent(false, false);

        ev[3]->Signal();
        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 3u);

        ev[3]->Signal();
        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 3u);

        ev[4]->Signal();
        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 4u);

        ev[0]->Signal();
        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 0u);

        for (unsigned i = 0; i < 5; ++i)
            delete ev[i];
    }
    {
        WaitableEvent* ev[5];
        for (unsigned i = 0; i < 5; ++i)
            ev[i] = new WaitableEvent(false, false);

        WaitableEventSignaler signaler(0.1, ev[2]);
        PlatformThreadHandle thread;
        PlatformThread::Create(0, &signaler, &thread);

        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 2u);
        PlatformThread::Join(thread);

        for (unsigned i = 0; i < 5; ++i)
            delete ev[i];
    }
   return 0;
}

#endif

