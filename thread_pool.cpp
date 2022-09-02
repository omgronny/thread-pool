
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
        std::unique_lock<hpx::mutex> q_lock(this->q_mutex);
        q_cv.wait(q_lock, [&]() -> bool {
           return this->stop_waiting_q || this->is_quite || this->q.empty();
        });
        this->stop_waiting_q = false;

        if (this->is_quite) {
            return;
        }

        if (!this->q.empty()) {

            auto [task, idx] = std::move(q.front());
            q.pop();

            q_lock.unlock();

            // выполняем
            task.get();

            // кладем в выполненные
            std::unique_lock<hpx::mutex> lockGuard(ct_mutex);
            completed_task_ids.insert(idx);

            ct_cv.notify_all();
            this->stop_waiting_ct = true;
        }

    }

}

bool thread_pool::unsafe_calculated(uint64_t task_id) {
    return completed_task_ids.contains(task_id);
}

bool thread_pool::calculated(uint64_t task_id) {
    std::lock_guard<hpx::mutex> lockGuard(ct_mutex);
    return unsafe_calculated(task_id);
}

void thread_pool::wait(uint64_t task_id) {

    while (true) {

        std::unique_lock<hpx::mutex> lock(ct_mutex);

        ct_cv.wait(lock, [&]() -> bool {
            return this->stop_waiting_ct || this->is_quite || this->q.empty();
        });
        this->stop_waiting_ct = false;

        if (unsafe_calculated(task_id)) {
            return;
        } else {
            lock.unlock();
        }

    }
    
}

void thread_pool::wait_all() {

    std::unique_lock<hpx::mutex> lock(this->ct_mutex);

    ct_cv.wait(lock, [this]()->bool {
        return this->stop_waiting_ct && this->next_task_id != this->completed_task_ids.size();
    });
    this->stop_waiting_ct = false;
    
}

thread_pool::~thread_pool() {

    this->is_quite = true;

    for (auto& x : threads) {
        q_cv.notify_all(); // зачем?
        this->stop_waiting_q = true;
        x.join();
    }

}




