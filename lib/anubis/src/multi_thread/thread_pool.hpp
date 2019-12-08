//
// Created by jmaerte on 08.12.2019.
//

#ifndef ANUBIS_PROJECT_THREAD_POOL_HPP
#define ANUBIS_PROJECT_THREAD_POOL_HPP

#include <queue>
#include <functional>
#include <thread>
#include <future>
#include <vector>
#include <condition_variable>

class thread_pool {
public:

    explicit thread_pool(std::size_t max_tasks);

    ~thread_pool();

    template<typename F, typename... Args>
    void add_work(F&& fn, Args&&... args);

    template<typename F, typename... Args>
    auto add_job(F&& fn, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

private:

    typedef std::function<void()> task;
    std::size_t max_tasks;
    std::queue<task> tasks;
    std::vector<std::thread> workers;
    bool finalize = false;
    std::condition_variable has_work;
    std::mutex queue_mutex;
};

#include "thread_pool.cpp"
#endif //ANUBIS_PROJECT_THREAD_POOL_HPP
