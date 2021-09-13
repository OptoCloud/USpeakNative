#ifndef USPEAK_SCOPEDSPINLOCK_H
#define USPEAK_SCOPEDSPINLOCK_H

#include <atomic>

namespace USpeakNative::Internal {

struct ScopedSpinLock {
    ScopedSpinLock(std::atomic_bool& lock) : m_lock(lock) {
        // Optimistically assume the lock is free on the first try
        for (;;) {
            if (!m_lock.exchange(true, std::memory_order_acquire)) {
                return;
            }

            // Wait for lock to be released without generating cache misses
            while (m_lock.load(std::memory_order_relaxed)) {
                // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between hyper-threads:
                //__builtin_ia32_pause();
            }
        }
    }
    ~ScopedSpinLock() {
        m_lock.store(false, std::memory_order_release);
    }
private:
    std::atomic_bool& m_lock;
};

}

#endif // USPEAK_SCOPEDSPINLOCK_H
