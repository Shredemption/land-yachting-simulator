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
    static std::thread renderBufferThread;

    // Synchronisations
    static std::mutex physicsMutex;
    static std::condition_variable physicsCV;
    static std::atomic<bool> physicsTrigger;
    static std::atomic<int> physicsSteps;
    static std::atomic<bool> physicsBusy;
    static std::atomic<bool> physicsShouldExit;

    static std::mutex animationMutex;
    static std::condition_variable animationCV;
    static std::atomic<bool> animationTrigger;
    static std::atomic<float> animationAlpha;
    static std::atomic<bool> animationShouldExit;

    static std::mutex renderBufferMutex;
    static std::condition_variable renderBufferCV;
    static std::atomic<bool> renderPrepReady;
    static std::atomic<bool> renderExecuteReady;
    static std::atomic<bool> renderBufferShouldExit;

    // Thread Functions
    static void physicsThreadFunction();
    static void animationThreadFunction();
    static void renderBufferThreadFunction();

    static void startup();
    static void shutdown();

    static void startRenderThread();
    static void stopRenderThread();
};

#endif