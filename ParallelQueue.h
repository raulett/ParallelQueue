//
// Created by raulett on 07.08.2023.
//

#ifndef PARALLELQUEUE_PARALLELQUEUE_H
#define PARALLELQUEUE_PARALLELQUEUE_H
#include <memory>
#include <condition_variable>
#include <mutex>
#include <atomic>

template<typename T>
struct WithMutex {
    template<class TT>
    explicit WithMutex(TT&& ptr): ptr{std::forward<TT>(ptr)} {}
    T ptr;
    std::mutex mutex;
};

template<typename T>
class ParallelQueue {
public:
    ParallelQueue():
            head{new Node}, tail{head.ptr.get()}, queue_size{0}, m_stopped{false}{}

    bool pop(T &entry) {
        std::unique_lock<std::mutex> lock{head.mutex};
        m_conditional.wait(lock, [this](){return m_stopped || head.ptr.get() != getTailSafe();});
        if (m_stopped) return false;
        entry = std::move(head.ptr->value);
        takeHeadUnsafe();
        queue_size--;
        return true;
    }

    bool pop_pair(T &entry1, T&entry2, std::atomic<unsigned int>& is_working_count){
        std::unique_lock<std::mutex> lock{head.mutex};
        is_working_count--;
        m_conditional.wait(lock, [this](){return m_stopped ||
                get_size() >= 2;});
        is_working_count++;
        if (m_stopped) return false;
        entry1 = std::move(head.ptr->value);
        takeHeadUnsafe();
        queue_size--;
        entry2 = std::move(head.ptr->value);
        takeHeadUnsafe();
        queue_size--;
        return true;
    }

    template<typename TT>
    void push(TT &&value) {
        std::lock_guard<std::mutex> lock{tail.mutex};
        if(!m_stopped){
            tail.ptr->value = std::forward<TT>(value);
            tail.ptr->next.reset(new Node);
            tail.ptr = tail.ptr->next.get();
            queue_size++;
            m_conditional.notify_one();
        }
    }

    void stop() {
        std::scoped_lock lock(head.mutex, tail.mutex);
        m_stopped = true;
        m_conditional.notify_all();
    }

    bool is_stopped(){
        return m_stopped;
    }

    size_t get_size(){
        return queue_size;
    }


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
    std::atomic<bool> m_stopped;
    std::atomic<size_t> queue_size;
};




#endif //PARALLELQUEUE_PARALLELQUEUE_H
