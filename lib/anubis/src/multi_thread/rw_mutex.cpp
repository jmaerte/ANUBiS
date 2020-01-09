//
// Created by Ina on 08.12.2019.
//

#include "rw_mutex.hpp"

void rw_mutex::lock_read() {
    boost::unique_lock<boost::mutex> lk(shared);
    while( waiting_writers != 0 )
        reader.wait(lk);
    ++active_readers;
    lk.unlock();
}

void rw_mutex::unlock_read() {
    boost::unique_lock<boost::mutex> lk(shared);
    --active_readers;
    lk.unlock();
    writer.notify_one();
}

void rw_mutex::lock_write() {
    boost::unique_lock<boost::mutex> lk(shared);
    ++waiting_writers;
    while( active_readers != 0 || active_writers != 0 )
        writer.wait(lk);
    ++active_writers;
    lk.unlock();
}

void rw_mutex::unlock_write() {
    boost::unique_lock<boost::mutex> lk(shared);
    --waiting_writers;
    --active_writers;
    if(waiting_writers > 0)
        writer.notify_one();
    else
        reader.notify_all();
    lk.unlock();
}

rw_mutex::rw_mutex(): shared(), reader(), writer(), active_readers(0), waiting_writers(0), active_writers(0) {}
