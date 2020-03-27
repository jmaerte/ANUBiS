//
// Created by jmaerte on 28.11.19.
//

namespace jmaerte {
    namespace typings {

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

    }
}