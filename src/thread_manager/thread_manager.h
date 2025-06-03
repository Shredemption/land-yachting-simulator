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
    static std::atomic<float> animationAlpha;
    static std::atomic<bool> animationShouldExit;
    static std::condition_variable animationCanWriteCV;
    static std::condition_variable renderCanReadCV;
    static bool animationDoneWriting;
    static bool renderDoneReading;

    static std::mutex renderBufferMutex;
    static std::condition_variable renderBufferCV;
    static std::atomic<bool> renderBufferShouldExit;
    static std::atomic<bool> sceneReadyForRender;

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