#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include "thread_pool.h"

void sum(int& ans, std::vector<int>& arr) {
    for (int i : arr) {
        ans += i;
    }
}

int main() {

    thread_pool tp(3);
    std::vector<int> s1 = { 1, 2, 3 };
    int ans1 = 0;

    std::vector<int> s2 = { 4, 5 };
    int ans2 = 0;

    std::vector<int> s3 = { 8, 9, 10 };
    int ans3 = 0;

    // добавляем в thread_pool выполняться 3 задачи
    /*
     * благодаря std::ref будет передана ссылка, а не копия объекта
     * (это особенность передачи аргумента в std::future).
     */
    uint64_t id1 = tp.add_task(sum, std::ref(ans1), std::ref(s1));
    uint64_t id2 = tp.add_task(sum, std::ref(ans2), std::ref(s2));
    uint64_t id3 = tp.add_task(sum, std::ref(ans3), std::ref(s3));

    if (tp.calculated(id1)) {
        std::cout << ans1 << std::endl;
    } else {
        tp.wait(id1);
        std::cout << ans1 << std::endl;
    }

    tp.wait_all();

    std::cout << ans2 << std::endl;
    std::cout << ans3 << std::endl;
    return 0;
}
