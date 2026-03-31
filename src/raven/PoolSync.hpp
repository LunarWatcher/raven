#include <mutex>
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

    std::atomic<size_t> pools;
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
        if (pools != 0) {
            std::unique_lock l(m);
            sync.wait(
                l,
                [&]() {
                    return pools == 0;
                }
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
        pools += 1;
    }

    void destroyConnPool() {
        if (pools == 0) {
            throw std::runtime_error("There are no pools to destroy");
        }
        pools -= 1;
        sync.notify_all();
    }
};

}
