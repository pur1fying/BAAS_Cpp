#ifndef BAAS_SAFEQUEUE_H_
#define BAAS_SAFEQUEUE_H_

#include <mutex>
#include <queue>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

// Thread safe implementation of a Queue using an std::queue
template <typename T>
class SafeQueue {
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;

public:
    SafeQueue() {

    }

    SafeQueue(SafeQueue& other) {
        //TODO:
    }

    ~SafeQueue() {

    }

    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void push(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(t);
    }

    bool pop(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        t = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    T front() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return T();
        }
        return m_queue.front();
    }

    bool pop() {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_queue.empty()) {
            return false;
        }
        m_queue.pop();  // Remove the front element
        return true;
    }
};

BAAS_NAMESPACE_END

#endif // BAAS_SAFEQUEUE_H_
