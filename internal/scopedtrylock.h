#ifndef USPEAK_SCOPEDTRYLOCK_H
#define USPEAK_SCOPEDTRYLOCK_H

#include <atomic>

namespace USpeakNative::Internal {

struct ScopedTryLock {
    ScopedTryLock(std::atomic_bool& lock) : m_lock(&lock) {
        // First do a relaxed load to check if lock is free in order to prevent
        // unnecessary cache misses if someone does while(!try_lock())
        if (lock.load(std::memory_order_relaxed) || lock.exchange(true, std::memory_order_acquire)) {
            m_lock = nullptr;
        }
    }
    bool lockSuccess() {
        return m_lock != nullptr;
    }
    ~ScopedTryLock() {
        if (m_lock != nullptr) {
            m_lock->store(false, std::memory_order_release);
        }
    }
private:
    std::atomic_bool* m_lock;
};

}

#endif // USPEAK_SCOPEDTRYLOCK_H
