//
// Created by jmaerte on 28.11.19.
//

#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <random>

template<class T>
class suspension {
    static T const & force (suspension * s) {
        return s->set_mem();
    }

    static T const & get (suspension * s) {
        return s->get_mem();
    }

    T const & get_mem() {
        return _mem;
    }

    T const & set_mem() {
        _mem = _func();
        _get = &get;
        return get_mem();
    }
public:
    explicit suspension (std::function<T()> func): _func(func), _get(&force), _mem(T()) {}
    T const & get() {
        return _get(this);
    }

private:
    T const & (*_get)(suspension *);
    mutable T _mem;
    std::function<T()> _func;
};

template<typename T>
class stream;

template<typename T>
class next {
public:
    next() {}
    next(T v, stream<T> const & n): _v(v), _next(n) {}
    explicit next(T v): _v(v) {}
    T val() const;
    stream<T> pop_front() const;
private:
    T _v;
    stream<T> _next;
};

template<typename T>
class stream {
private:
    std::shared_ptr<suspension<next<T>>> _lazy;
public:
    stream() {}
    stream(std::function<next<T>()> f): _lazy(std::make_shared<suspension<next<T>>>(f)) {}
    stream(stream<T> && s): _lazy(std::move(s._lazy)) {}
    stream(stream<T> const & s): _lazy(std::move(s._lazy)) {}

    stream & operator=(stream && s);
    bool is_empty() const;
    T get() const;
    stream<T> pop_front() const;
    stream<T> take(int n) const;

    template<class F>
    T flatten(F f) const;

    std::vector<T> vectorize() const;
};

stream<double> drand() {
    return stream<double>([]() {
        const double lower_bound = -1;
        const double upper_bound = 1;
        std::uniform_real_distribution<double> unif (lower_bound, upper_bound);
        std::random_device rand_dev;
        std::mt19937 rand_engine(rand_dev());
        return next<double>(unif(rand_engine), drand());
    });
}

stream<int> ints_from(int n) {
    return stream<int>([n]() {
        return next<int>(n, ints_from(n + 1));
    });
}

template<class T, class F>
void forEach(stream<T> strm, F f) {
    while (!strm.is_empty()) {
        f(strm.get());
        strm = strm.pop_front();
    }
}

template<class T, class F>
auto transform(stream<T> strm, F f) -> stream<decltype(f(strm.get()))> {
    using U = decltype(f(strm.get()));
    static_assert(std::is_convertible<F, std::function<U(T)>>::value, "streams can only be transformed by functions that are parametrized correctly.");

    if (strm.is_empty()) return stream<U>();
    return stream<U>([strm, f]() {
        return next<U>(f(strm.get()), transform(strm.pop_front(), f));
    });
}

template<class T, class S, class F>
auto zip(const stream<T> & s1, const stream<S> & s2, F f) -> stream<decltype(f(s1.get(), s2.get()))> {
    using U = decltype(f(s1.get(), s2.get()));
    static_assert(std::is_convertible<F, std::function<U(T, S)>>::value, "Zip-Operator must take 2 arguments of fitting types.");

    if(s1.is_empty() || s2.is_empty()) return stream<U>();
    return stream<U>([s1, s2, f]() {
        return next<U>(f(s1.get(), s2.get()), zip(s1.pop_front(), s2.pop_front(), f));
    });
}

#include "stream.cpp"