//
// Created by maertej on 29.09.19.
//

#ifndef ANUBIS_RW_MUTEX_HPP
#define ANUBIS_RW_MUTEX_HPP


class rw_mutex {
public:
    rw_mutex(): shared(), reader(), writer(), active_readers(0), waiting_writers(0), active_writers(0) {}

    void lock_read() {
        std::unique_lock<std::mutex> lk(shared);
        while( waiting_writers != 0 )
            reader.wait(lk);
        ++active_readers;
        lk.unlock();
    }

    void unlock_read() {
        std::unique_lock<std::mutex> lk(shared);
        --active_readers;
        lk.unlock();
        writer.notify_one();
    }

    void lock_write() {
        std::unique_lock<std::mutex> lk(shared);
        ++waiting_writers;
        while( active_readers != 0 || active_writers != 0 )
            writer.wait(lk);
        ++active_writers;
        lk.unlock();
    }

    void unlock_write() {
        std::unique_lock<std::mutex> lk(shared);
        --waiting_writers;
        --active_writers;
        if(waiting_writers > 0)
            writer.notify_one();
        else
            reader.notify_all();
        lk.unlock();
    }

private:
    std::mutex shared;
    std::condition_variable reader;
    std::condition_variable writer;
    int active_readers;
    int waiting_writers;
    int active_writers;
};


#endif //ANUBIS_RW_MUTEX_HPP
