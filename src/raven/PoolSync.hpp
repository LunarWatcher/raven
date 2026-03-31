#include <mutex>
#include <semaphore>
#include <atomic>
#include <condition_variable>

namespace raven {

/**
 * Synchronisation object used to make sure the SocketServer destroys after all ConnectionPools have closed. Largely
 * useful for running in tests, since a real shutdown wouldn't care as much about the UB.
 */
struct PoolSync {
    std::mutex m;
    std::condition_variable sync;
    std::condition_variable closed;

    std::counting_semaphore<128> pools;
    std::atomic<bool> isRunning{true};

    PoolSync() : pools {0} {}

    ~PoolSync() {
        close();
    }
    void close() {
        if (!isRunning.load()) {
            // Already closed
            return;
        }
        isRunning.store(false);
        if (pools.max() != 0) {
            std::unique_lock l(m);
            sync.wait(
                l,
                [&]() { return pools.max() == 0; }
            );
        }
        closed.notify_all();
    }

    void subscribeToTermination() {
        if (!isRunning) {
            return;
        }
        std::unique_lock l(m);
        closed.wait(l);
    }

    void newConnPool() {
        pools.release();
    }

    void destroyConnPool() {
        pools.acquire();
        sync.notify_all();
    }
};

}
