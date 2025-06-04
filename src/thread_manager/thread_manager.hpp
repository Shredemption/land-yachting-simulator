#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace ThreadManager
{
    // Define threads
    inline std::thread physicsThread;
    inline std::thread animationThread;
    inline std::thread renderBufferThread;

    // Synchronisations
    inline std::mutex physicsMutex;
    inline std::condition_variable physicsCV;
    inline std::atomic<bool> physicsTrigger(false);
    inline std::atomic<int> physicsSteps(0);
    inline std::atomic<bool> physicsBusy(false);
    inline std::atomic<bool> physicsShouldExit(false);

    inline std::mutex animationMutex;
    inline std::atomic<float> animationAlpha(0.0f);
    inline std::atomic<bool> animationShouldExit(false);
    inline std::condition_variable animationCanWriteCV;
    inline std::condition_variable renderCanReadCV;
    inline bool animationDoneWriting = false;
    inline bool renderDoneReading = true;

    inline std::mutex renderBufferMutex;
    inline std::condition_variable renderBufferCV;
    inline std::atomic<bool> renderBufferShouldExit(false);
    inline std::atomic<bool> sceneReadyForRender(false);

    // Thread Functions
    void physicsThreadFunction();
    void animationThreadFunction();
    void renderBufferThreadFunction();

    void startup();
    void shutdown();

    void startRenderThread();
    void stopRenderThread();
};

#endif