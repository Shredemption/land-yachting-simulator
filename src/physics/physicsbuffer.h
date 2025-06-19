#pragma once

#include <memory>
#include <thread>
#include <variant>

#include "physics/yacht.hpp"
#include "physics/physics_util.hpp"

using Physics = std::variant<PhysicsYacht>;

struct PhysicsBuffer
{
    std::unique_ptr<Physics> buffers[2];
    int readIndex = 0;

    Physics *getReadBuffer()
    {
        while (PhysicsUtil::isSwapping.load(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
        return buffers[readIndex % 2].get();
    }

    Physics *getWriteBuffer() { return buffers[(readIndex + 1) % 2].get(); }

    void swapBuffers() { readIndex = (readIndex + 1) % 2; }

    template <typename T>
    T *getReadAs()
    {
        Physics *buf = getReadBuffer();
        return std::get_if<T>(buf);
    }

    template <typename T>
    T *getWriteAs()
    {
        Physics *buf = getWriteBuffer();
        return std::get_if<T>(buf);
    }
};