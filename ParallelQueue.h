//
// Created by raulett on 07.08.2023.
//

#ifndef PARALLELQUEUE_PARALLELQUEUE_H
#define PARALLELQUEUE_PARALLELQUEUE_H
#include <memory>
template<typename T>
struct WithMutex {
    template<class TT>
    WithMutex(TT&& ptr): ptr{std::forward<TT>(ptr)} {}

    T ptr;
    std::mutex mutex;
};

template<typename T>
class ParallelQueue {
    ParallelQueue(){}
private:
    struct Node {
        T value;
        std::unique_ptr<Node> next;
    };

    Node* getTailSafe() {
        std::lock_guard<std::mutex> lck{tail.mutex};
        return tail.ptr;
    }

    std::unique_ptr<Node> takeHeadUnsafe() {
        std::unique_ptr<Node> prevHead = std::move(head.ptr);
        head.ptr = std::move(prevHead->next);
        return prevHead;
    }

    // head element of the WaitingQueue
    WithMutex<std::unique_ptr<Node>> head;
    // pointer to the tail element
    WithMutex<Node*> tail;
    // Condition for waiting data
    std::condition_variable m_conditional;
    // Stop flag
    bool m_stopped;
};
#endif //PARALLELQUEUE_PARALLELQUEUE_H
