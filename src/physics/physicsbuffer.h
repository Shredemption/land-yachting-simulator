#ifndef PHYSICSBUFFER_H
#define PHYSICSBUFFER_H

#include <memory>

#include "physics/physics.hpp"

struct PhysicsBuffer
{
    std::unique_ptr<Physics> buffers[2];
    int readIndex = 0;

    Physics *getReadBuffer()
    {
        while (Physics::isSwapping.load(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
        return buffers[readIndex % 2].get();
    }

    Physics *getWriteBuffer() { return buffers[(readIndex + 1) % 2].get(); }

    void swapBuffers() { readIndex = (readIndex + 1) % 2; }
};

#endif