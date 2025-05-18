#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadManager
{
public:
    // Define threads
    static std::thread physicsThread;
    static std::thread animationThread;

    // Synchronisations
    static std::mutex physicsMutex;
    static std::condition_variable physicsCV;
    static std::atomic<bool> physicsTrigger;
    static std::atomic<bool> physicsShouldExit;

    static std::mutex animationMutex;
    static std::condition_variable animationCV;
    static std::atomic<bool> animationTrigger;
    static std::atomic<bool> animationShouldExit;

    // Thread Functions
    static void physicsThreadFunction();
    static void animationThreadFunction();

    static void startup();
    static void shutdown();
};

#endif