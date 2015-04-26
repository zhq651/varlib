/**
 * @file lighweight_mutex.h
 * @brief simple wrap for win32 cs, the code is from boost_1_56_0/boost/smart_ptr/detail/lwm_win32_cs.hpp
 * @author liangsijian
 * @version 
 * @date 2015-04-26
 */

#ifndef LIGHWEIGHT_MUTEX_H_36CAD5NB
#define LIGHWEIGHT_MUTEX_H_36CAD5NB

#include <windows.h>
typedef ::CRITICAL_SECTION critical_section;
class lightweight_mutex
{
private:

    critical_section cs_;

    lightweight_mutex(lightweight_mutex const &);
    lightweight_mutex & operator=(lightweight_mutex const &);

public:

    lightweight_mutex()
    {
#if BOOST_PLAT_WINDOWS_RUNTIME
        InitializeCriticalSectionEx(&cs_, 4000, 0);
#else
        InitializeCriticalSection(&cs_);
#endif
    }

    ~lightweight_mutex()
    {
        DeleteCriticalSection(&cs_);
    }

    class scoped_lock;
    friend class scoped_lock;

    class scoped_lock
    {
    private:

        lightweight_mutex & m_;

        scoped_lock(scoped_lock const &);
        scoped_lock & operator=(scoped_lock const &);

    public:

        explicit scoped_lock(lightweight_mutex & m): m_(m)
        {
            EnterCriticalSection(&m_.cs_);
        }

        ~scoped_lock()
        {
            LeaveCriticalSection(&m_.cs_);
        }
    };
};


#endif /* end of include guard: LIGHWEIGHT_MUTEX_H_36CAD5NB */
