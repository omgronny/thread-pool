#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include "thread_pool.h"
#include <ctime>

#include <hpx/local/init.hpp>
#include <hpx/thread.hpp>


void sum(int64_t& ans, std::vector<int>& arr) {
    for (int i : arr) {
        ans += i;
    }
     std::cout << ans << std::endl;
}

void sub(int64_t& ans, std::vector<int>& arr) {
    for (int i : arr) {
        ans -= i;
    }
     std::cout << ans << std::endl;
}

int hpx_main(int argc, char* argv[]) {

    hpx::mutex m;

    std::unique_lock<hpx::mutex> q_lock(m);
    
    int64_t ans1 = 0;
    int64_t ans2 = 0;

    const size_t VCT_SIZE = 10;

    thread_pool tp(3);

    auto beg = clock();

    std::vector<int32_t> v1(VCT_SIZE);
    auto t1 = hpx::thread([&]() -> void {

        for (int32_t i = 0; i < 5; ++i) {

            for (int32_t j = 0; j < VCT_SIZE; ++j) {
                v1[j] = std::rand();
            }

            tp.add_task(sum, std::ref(ans1), std::ref(v1));

        }

    });

    std::vector<int32_t> v2(VCT_SIZE);
    auto t2 = hpx::thread([&]() -> void {

        for (int32_t i = 0; i < 5; ++i) {

            for (int32_t j = 0; j < VCT_SIZE; ++j) {
                v2[j] = std::rand();
            }

            tp.add_task(sub, std::ref(ans2), std::ref(v2));

        }
    });

    t1.join();
    t2.join();

    tp.wait_all();

    auto end = clock();

    std::cout << (end - beg) / 1000.0  << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {   
    return hpx::local::init(hpx_main, argc, argv);
}