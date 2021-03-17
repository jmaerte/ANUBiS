//
// Created by Ina on 08.12.2019.
//

#include "../include/data_types/thread_pool.hpp"

template<typename F, typename... Args>
auto thread_pool::add_job(F &&fn, Args &&... args) -> boost::unique_future<typename boost::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<boost::packaged_task<return_type()>>(std::bind(std::forward<F>(fn), std::forward<Args>(args)...));
    boost::unique_future<return_type> result = task->get_future();
    while (tasks.size() >= max_tasks) {}
    tasks.push([task]() {*task();});
    return result;
}

template <typename Class, typename... Args>
void thread_pool::add_work(Function<void(Class::*)(Args...)> fn, Args&&... args) {
    while (tasks.size() >= max_tasks) {}
    tasks.push([=]() {fn(args...);});
}

thread_pool::~thread_pool() {
    finalize = true;
    while (!tasks.empty()) {
        has_work.notify_one();
    }
    has_work.notify_all();
    for (auto &thread : workers) {
        thread.join();
    }
}

thread_pool::thread_pool(std::size_t max_tasks, std::size_t num_threads): max_tasks(max_tasks) {
    num_threads = std::max<int>(num_threads, 1);
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
//                    this->is_full.notify_one();
                }
                task();
            }
        });
    }
}

template<typename... Args>
void thread_pool::add_work(std::function<void(Args...)> fn, Args &&... args) {
    while (tasks.size() == max_tasks) {}
//    std::unique_lock<std::mutex> lock(this->push_mutex);
//    this->is_full.wait(lock, [this] {
//        return this->tasks.size() < max_tasks;
//    });
    tasks.push([=]() {
        fn(args...);
    });
    has_work.notify_all();
}

std::size_t thread_pool::count_jobs() {
    return tasks.size();
}

void thread_pool::join() {
    for (int i = 0; i < workers.size(); i++) {
        workers[i].join();
    }
}