//
// Created by jmaerte on 28.11.19.
//

#include "stream.hpp"

template<typename T>
T next<T>::val() const {
    return _v;
}

template<typename T>
stream<T> next<T>::pop_front() const {
    return _next;
}

template<typename T>
stream<T> &stream<T>::operator=(stream &&s) {
    _lazy = std::move(s._lazy);
    return *this;
}

template<typename T>
bool stream<T>::is_empty() const {
    return !_lazy;
}

template<typename T>
T stream<T>::get() const {
    return _lazy->get().val();
}

template<typename T>
stream<T> stream<T>::pop_front() const {
    return _lazy->get().pop_front();
}

template<typename T>
stream<T> stream<T>::take(int n) const {
    if (n == 0 || is_empty()) return stream<T>();
    auto lazy = _lazy;
    return stream<T>([lazy, n]() {
        auto val = lazy->get().val();
        auto tail = lazy->get().pop_front();
        return next<T>(val, tail.take(n - 1));
    });
}

template<typename T>
template<typename F>
T stream<T>::flatten(F f) const {
    if (pop_front().is_empty()) return get();
    return f(get(), pop_front().flatten(f));
}

template<typename T>
std::vector<T> stream<T>::vectorize() const {
    std::vector<T> v;
    forEach(*this, [&v](T t) {
        v.push_back(t);
    });
    return v;
}

//template<typename T>
//stream<double> drand() {
//    return stream<double>([]() {
//        return next<double>(drand48(), drand());
//    });
//}
//
//template<typename T, typename F>
//void forEach(stream<T> strm, F f){
//    while (!strm.is_empty()) {
//        f(strm.get());
//        strm = strm.pop_front();
//    }
//}
//
//template<class T, class F>
//auto transform(stream<T> strm, F f) -> stream<decltype(f(strm.get()))> {
//    using U = decltype(f(strm.get()));
//    static_assert(std::is_convertible<F, std::function<U(T)>>::value, "streams can only be transformed by functions that are parametrized correctly.");
//
//    if (strm.is_empty()) return stream<U>();
//    return stream<U>([strm, f]() {
//        return next<U>(f(strm.get()), transform(strm.pop_front(), f));
//    });
//}
//
//template<class T, class S, class F>
//auto zip(const stream<T> & s1, const stream<S> & s2, F f) -> stream<decltype(f(s1.get(), s2.get()))> {
//    using U = decltype(f(s1.get(), s2.get()));
//    static_assert(std::is_convertible<F, std::function<U(T, S)>>::value, "Zip-Operator must take 2 arguments of fitting types.");
//
//    if(s1.is_empty() || s2.is_empty()) return stream<U>();
//    return stream<U>([s1, s2, f]() {
//        return next<U>(f(s1.get(), s2.get()), zip(s1.pop_front(), s2.pop_front(), f));
//    });
//}