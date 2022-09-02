//
// Created by omgronny on 16.08.2022.
//

#ifndef THREAD_POOL_THREAD_POOL_H
#define THREAD_POOL_THREAD_POOL_H

#include <iostream>
#include <queue>
#include <future>
#include <unordered_set>

#include <hpx/condition_variable.hpp>
#include <hpx/mutex.hpp>
#include <hpx/thread.hpp>

class thread_pool {

    std::queue<std::pair<std::future<void>, uint64_t>> q;
    hpx::mutex q_mutex;
    hpx::condition_variable q_cv;

    std::unordered_set<uint64_t> completed_task_ids;
    hpx::condition_variable ct_cv;
    hpx::mutex ct_mutex;

    std::vector<hpx::thread> threads;

    std::atomic<bool> is_quite{false};

    std::atomic<uint64_t> next_task_id{0};

    std::atomic<bool> stop_waiting_q{false};
    std::atomic<bool> stop_waiting_ct{false};

    void run();
    bool unsafe_calculated(uint64_t task_id);

public:

    explicit thread_pool(uint64_t num_threads);

    template <typename Func, typename ...Args>
    uint64_t add_task(const Func& func, Args&&... args) {

        uint64_t idx = this->next_task_id++;

        std::cout << "3 - " << std::this_thread::get_id() << std::endl;

        std::unique_lock<hpx::mutex> q_lock(this->q_mutex);
        q.emplace(std::async(std::launch::deferred, func, args...), idx);

        std::cout << "4 - " << std::this_thread::get_id() << std::endl;

        q_cv.notify_one();
        this->stop_waiting_q = true;

        return idx;
    }

    bool calculated(uint64_t task_id);

    void wait(uint64_t task_id);
    void wait_all();

    ~thread_pool();

};


#endif //THREAD_POOL_THREAD_POOL_H
