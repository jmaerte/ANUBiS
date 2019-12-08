//
// Created by Ina on 08.12.2019.
//

#include "thread_pool.hpp"

template<typename F, typename... Args>
auto thread_pool::add_job(F &&fn, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(fn), std::forward<Args>(args)...));
    std::future<return_type> result = task->get_future();
    while (tasks.size() >= max_tasks) {}
    tasks.push([task]() {*task();});
    return result;
}

template<typename F, typename... Args>
void thread_pool::add_work(F &&fn, Args &&... args) {
    while (tasks.size() >= max_tasks) {}
    tasks.push([=]() {fn(args...);});
}

thread_pool::~thread_pool() {
    finalize = true;
    for (auto &thread : workers) {
        thread.join();
    }
}

thread_pool::thread_pool(std::size_t max_tasks): max_tasks(max_tasks) {
    std::size_t num_threads = std::max<int>(std::thread::hardware_concurrency() - 1, 1);
    for (int i = 0; i < num_threads; i++) {
        workers.emplace_back([this]() {
            while (true) {
                task task;
                {
                    // Make the other workers wait to access the queue.
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    // Wait for the user to push work into the queue.
                    this->has_work.wait(lock, [this] {return this->finalize || !this->tasks.empty();});
                    // Stop the worker if there is no work to do anymore
                    if (this->finalize && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}