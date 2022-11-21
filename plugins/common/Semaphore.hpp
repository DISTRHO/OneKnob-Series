/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2022 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
# include <winsock2.h>
# include <windows.h>
#elif defined(__MOD_DEVICES__)
# include <linux/futex.h>
# include <sys/time.h>
# include <errno.h>
# include <syscall.h>
# include <unistd.h>
#else
# include <semaphore.h>
# include <sys/time.h>
#endif

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class Semaphore
{
public:
    Semaphore(const int initialValue = 0)
    {
       #if defined(DISTRHO_OS_MAC)
        DISTRHO_SAFE_ASSERT_RETURN(semaphore_create(mach_task_self(),
                                                    &sem,
                                                    SYNC_POLICY_FIFO,
                                                    initialValue) == KERN_SUCCESS,);
       #elif defined(DISTRHO_OS_WINDOWS)
        handle = ::CreateSemaphoreA(nullptr, initialValue, std::max(initialValue, 1), nullptr);
        DISTRHO_SAFE_ASSERT_RETURN(handle != INVALID_HANDLE_VALUE,);
       #elif defined(__MOD_DEVICES__)
        value = initialValue;
       #else
        ::sem_init(&sem, 0, initialValue);
       #endif
    }

    ~Semaphore()
    {
       #if defined(DISTRHO_OS_MAC)
        ::semaphore_destroy(mach_task_self(), sem);
       #elif defined(DISTRHO_OS_WINDOWS)
        ::CloseHandle(handle);
       #elif defined(__MOD_DEVICES__)
        // nothing here
       #else
        ::sem_destroy(&sem);
       #endif
    }

    void post()
    {
       #if defined(DISTRHO_OS_MAC)
        ::semaphore_signal(sem);
       #elif defined(DISTRHO_OS_WINDOWS)
        ::ReleaseSemaphore(handle, 1, nullptr);
       #elif defined(__MOD_DEVICES__)
        // if already unlocked, do not wake futex
        if (::__sync_bool_compare_and_swap(&value, 0, 1))
            ::syscall(__NR_futex, &value, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
       #else
        ::sem_post(&sem);
       #endif
    }

    bool wait()
    {
       #if defined(DISTRHO_OS_MAC)
        return ::semaphore_wait(sem) == KERN_SUCCESS;
       #elif defined(DISTRHO_OS_WINDOWS)
        return ::WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0;
       #elif defined(__MOD_DEVICES__)
        for (;;)
        {
            if (::__sync_bool_compare_and_swap(&value, 1, 0))
                return true;

            if (::syscall(__NR_futex, &value, FUTEX_WAIT_PRIVATE, 0, nullptr, nullptr, 0) != 0)
            {
                if (errno != EAGAIN && errno != EINTR)
                    return false;
            }
        }
       #else
        return ::sem_wait(&sem) == 0;
       #endif
    }

    bool timedWait(const uint numSecs)
    {
       #if defined(DISTRHO_OS_MAC)
        const struct mach_timespec time = { numSecs, 0 };
        return ::semaphore_timedwait(sem, time) == KERN_SUCCESS;
       #elif defined(DISTRHO_OS_WINDOWS)
        return ::WaitForSingleObject(handle, numSecs * 1000) == WAIT_OBJECT_0;
       #elif defined(__MOD_DEVICES__)
        const struct timespec timeout = { numSecs, 0 };
        for (;;)
        {
            if (::__sync_bool_compare_and_swap(&value, 1, 0))
                return true;

            if (::syscall(__NR_futex, &value, FUTEX_WAIT_PRIVATE, 0, &timeout, nullptr, 0) != 0)
            {
                if (errno != EAGAIN && errno != EINTR)
                    return false;
            }
        }
       #else
        struct timespec timeout;
        ::clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += numSecs;
        return ::sem_timedwait(&sem, &timeout) == 0;
       #endif
    }
private:
   #if defined(DISTRHO_OS_MAC)
    ::semaphore_t sem;
   #elif defined(DISTRHO_OS_WINDOWS)
    ::HANDLE handle;
   #elif defined(__MOD_DEVICES__)
    int value;
   #else
    ::sem_t sem;
   #endif
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif // DISTRHO_SEMAPHORE_HPP_INCLUDED
