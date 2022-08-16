//
// Created by omgronny on 16.08.2022.
//

#ifndef THREAD_POOL_THREAD_POOL_H
#define THREAD_POOL_THREAD_POOL_H

#include <iostream>
#include <queue>
#include <future>
#include <unordered_set>

class thread_pool {

    std::queue<std::pair<std::future<void>, uint64_t>> q;
    std::mutex q_mutex;
    std::condition_variable q_cv;

    std::unordered_set<uint64_t> completed_task_ids;
    std::condition_variable completed_task_ids_cv;
    std::mutex completed_task_ids_mutex;

    std::vector<std::thread> threads;

    std::atomic<bool> is_quite{false};

    std::atomic<uint64_t> next_task_id{0};

    void run();
    bool unsafe_calculated(uint64_t task_id);

public:

    explicit thread_pool(uint64_t num_threads);

    template <typename Func, typename ...Args>
    uint64_t add_task(const Func& func, Args&&... args) {
        uint64_t idx = this->next_task_id++;

        std::lock_guard<std::mutex> q_lock(this->q_mutex);
        q.emplace(std::async(std::launch::deferred, func, args...), idx);

        q_cv.notify_one();
        return idx;
    }

    bool calculated(uint64_t task_id);

    void wait(uint64_t task_id);
    void wait_all();

    ~thread_pool();

};


#endif //THREAD_POOL_THREAD_POOL_H
