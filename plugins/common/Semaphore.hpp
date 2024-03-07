/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2024 Filipe Coelho <falktx@falktx.com>
 * SPDX-License-Identifier: ISC
 */

#ifndef DISTRHO_SEMAPHORE_HPP_INCLUDED
#define DISTRHO_SEMAPHORE_HPP_INCLUDED

#include "extra/String.hpp"

#if defined(DISTRHO_OS_MAC)
# include <mach/mach.h>
# include <mach/semaphore.h>
#elif defined(DISTRHO_OS_WINDOWS)
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# ifndef NOKERNEL
#  define NOKERNEL
# endif
# ifndef NOSERVICE
#  define NOSERVICE
# endif
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <winsock2.h>
# include <windows.h>
#else
# include <semaphore.h>
# include <sys/time.h>
#endif

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

class Semaphore
{
public:
    Semaphore(const uint16_t initialValue = 0)
     #if defined(DISTRHO_OS_MAC)
      : task(mach_task_self()),
        sem(nullptr)
     #elif defined(DISTRHO_OS_WINDOWS)
      : handle(INVALID_HANDLE_VALUE)
     #else
      : sem()
     #endif
    {
       #if defined(DISTRHO_OS_MAC)
        DISTRHO_SAFE_ASSERT_RETURN(semaphore_create(task,
                                                    &sem,
                                                    SYNC_POLICY_FIFO,
                                                    static_cast<int>(initialValue)) == KERN_SUCCESS,);
       #elif defined(DISTRHO_OS_WINDOWS)
        handle = CreateSemaphoreA(nullptr, initialValue, LONG_MAX, nullptr);
        DISTRHO_SAFE_ASSERT_RETURN(handle != INVALID_HANDLE_VALUE,);
       #else
        DISTRHO_SAFE_ASSERT_RETURN(sem_init(&sem, 0, initialValue) == 0,);
       #endif
    }

    ~Semaphore()
    {
       #if defined(DISTRHO_OS_MAC)
        semaphore_destroy(task, sem);
       #elif defined(DISTRHO_OS_WINDOWS)
        CloseHandle(handle);
       #else
        sem_destroy(&sem);
       #endif
    }

    void post()
    {
       #if defined(DISTRHO_OS_MAC)
        semaphore_signal(sem);
       #elif defined(DISTRHO_OS_WINDOWS)
        ReleaseSemaphore(handle, 1, nullptr);
       #else
        sem_post(&sem);
       #endif
    }

    bool wait()
    {
       #if defined(DISTRHO_OS_MAC)
        return semaphore_wait(sem) == KERN_SUCCESS;
       #elif defined(DISTRHO_OS_WINDOWS)
        return WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0;
       #else
        return sem_wait(&sem) == 0;
       #endif
    }

    bool tryWait()
    {
       #if defined(DISTRHO_OS_MAC)
        const mach_timespec_t zero = { 0, 0 };
        return semaphore_timedwait(sem, zero) == KERN_SUCCESS;
       #elif defined(DISTRHO_OS_WINDOWS)
        return WaitForSingleObject(handle, 0) == WAIT_OBJECT_0;
       #else
        return sem_trywait(&sem) == 0;
       #endif
    }

    bool timedWait(const uint seconds, const uint nanoseconds)
    {
       #if defined(DISTRHO_OS_MAC)
        const mach_timespec_t time = { seconds, static_cast<clock_res_t>(nanoseconds) };
        return semaphore_timedwait(sem, time) == KERN_SUCCESS;
       #elif defined(DISTRHO_OS_WINDOWS)
        const uint milliseconds = seconds * 1000U + nanoseconds / 1000000U;
        return WaitForSingleObject(handle, milliseconds) == WAIT_OBJECT_0;
       #else
        struct timespec timeout;
        DISTRHO_SAFE_ASSERT_RETURN(clock_gettime(CLOCK_REALTIME, &timeout) == 0, false);

        timeout.tv_sec += seconds;
        timeout.tv_nsec += nanoseconds;
        if (timeout.tv_nsec >= 1000000000LL)
        {
           ++timeout.tv_sec;
           timeout.tv_nsec -= 1000000000LL;
        }

        for (int r;;)
        {
            r = sem_timedwait(&sem, &timeout);

            if (r < 0)
                r = errno;

            if (r == EINTR)
                continue;

            return r == 0;
        }
       #endif
    }

private:
   #if defined(DISTRHO_OS_MAC)
    mach_port_t task;
    semaphore_t sem;
   #elif defined(DISTRHO_OS_WINDOWS)
    HANDLE handle;
   #else
    sem_t sem;
   #endif
};

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif // DISTRHO_SEMAPHORE_HPP_INCLUDED
