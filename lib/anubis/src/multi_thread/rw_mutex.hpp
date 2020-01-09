//
// Created by jmaerte on 08.12.2019.
//

#ifndef ANUBIS_PROJECT_RW_MUTEX_HPP
#define ANUBIS_PROJECT_RW_MUTEX_HPP

#include <mutex>
#include <condition_variable>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

class rw_mutex {
public:
    rw_mutex();

    void lock_read();

    void unlock_read();

    void lock_write();

    void unlock_write();

private:
    boost::mutex shared;
    boost::condition_variable reader;
    boost::condition_variable writer;
    int active_readers;
    int waiting_writers;
    int active_writers;
};

#endif //ANUBIS_PROJECT_RW_MUTEX_HPP
