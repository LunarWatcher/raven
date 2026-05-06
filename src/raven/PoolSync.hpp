#include "raven/Logging.hpp"
#include <iostream>
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

    std::mutex readyLock;
    std::condition_variable ready;

    std::atomic<size_t> pools;
    std::atomic<bool> isRunning{true};

    std::atomic<bool> isReady{false};

    PoolSync() : pools {0} {}

    ~PoolSync() {
        close();
    }

    /**
     * Sets running to false and optionally waits for the threads to join
     *
     * \param wait      Whether or not to wait for termination. Set to false to pre-signal termination of the PoolSync
     *                  instance. 
     */
    void close(bool wait = true) {
        isRunning.store(false);
        if (wait) {
            if (pools != 0) {
                std::unique_lock l(m);
                sync.wait(
                    l,
                    [&]() {
                        std::cout << "pools = " << pools << std::endl;
                        return pools == 0;
                    }
                );
            }
            closed.notify_all();
        }
    }

    void subscribeToTermination() {
        if (!isRunning) {
            return;
        }
        std::unique_lock l(m);
        closed.wait(l);
    }

    void newConnPool() {
        // Note to self: inlining ++this->pools in RavenLog results in ++pools never being evaluated if
        // RAVEN_DEBUG_LOGGING = false
        [[maybe_unused]]
        auto pools = ++this->pools;
        RavenLog("Registered %li pools\n", pools);
    }

    void destroyConnPool() {
        if (pools == 0) {
            throw std::runtime_error("There are no pools to destroy");
        }
        [[maybe_unused]]
        auto pools = --this->pools;
        RavenLog("Deregistered: %li pools remain\n", pools);
        sync.notify_all();
    }

    void signalReady() {
        std::unique_lock l(readyLock);
        isReady = true;
        ready.notify_all();
    }

    void waitForReady() {
        std::unique_lock l(readyLock);
        if (isReady) {
            return;
        }
        ready.wait(l);
    }
};

}
