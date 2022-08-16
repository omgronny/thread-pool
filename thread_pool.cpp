
#include "thread_pool.h"

thread_pool::thread_pool(uint64_t num_threads) {

    threads.reserve(num_threads);

    for (uint64_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(&thread_pool::run, this);
    }

}

// это метод потока
void thread_pool::run() {

    while (!this->is_quite) {

        // берем задачу
        std::unique_lock<std::mutex> lock(q_mutex);
        q_cv.wait(lock, [&]()->bool {
           return this->is_quite || !this->q.empty();
        });

        if (!this->q.empty()) {
            auto [task, idx] = std::move(q.front());
            q.pop();
            lock.unlock();

            // выполняем
            task.get();

            // кладем в выполненные
            std::lock_guard<std::mutex> lockGuard(completed_task_ids_mutex);
            completed_task_ids.insert(idx);
            completed_task_ids_cv.notify_all();
        }

    }

}

bool thread_pool::unsafe_calculated(uint64_t task_id) {
    return completed_task_ids.contains(task_id);
}

bool thread_pool::calculated(uint64_t task_id) {
    std::lock_guard<std::mutex> lockGuard(completed_task_ids_mutex);
    return unsafe_calculated(task_id);
}

void thread_pool::wait(uint64_t task_id) {

    std::unique_lock<std::mutex> lock(completed_task_ids_mutex);

    completed_task_ids_cv.wait(lock, [&]() -> bool {
        return unsafe_calculated(task_id);
    });

}

void thread_pool::wait_all() {

    std::unique_lock<std::mutex> lock(this->completed_task_ids_mutex);

    completed_task_ids_cv.wait(lock, [this]()->bool {
        std::lock_guard<std::mutex> task_lock(this->q_mutex);
        return q.empty() && this->next_task_id == this->completed_task_ids.size();
    });

}

thread_pool::~thread_pool() {

    wait_all();
    this->is_quite = true;

    for (auto& x : threads) {
        q_cv.notify_all(); // зачем?
        x.join();
    }

}




