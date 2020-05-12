//
// Created by jmaerte on 08.12.2019.
//

#ifndef ANUBIS_PROJECT_THREAD_POOL_HPP
#define ANUBIS_PROJECT_THREAD_POOL_HPP

#include <queue>
#include <functional>
#include <vector>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <condition_variable>


// Type checkers
template<typename _Type>
struct IsVoid
{
    static const bool value = false;
};

template<>
struct IsVoid<void>
{
    static const bool value = true;
};

// Callable signature interfce
template<typename _ReturnType, typename..._ArgTypes>
struct Callable
{
    typedef _ReturnType ReturnType;
    typedef _ReturnType (*SignatureType)(_ArgTypes...);

    virtual _ReturnType operator()(_ArgTypes...args) = 0;
};

// Function and lambda closure wrapper
template<typename _ClosureType, typename _ReturnType, typename..._ArgTypes>
struct Closure: public Callable<_ReturnType, _ArgTypes...>
{
    typedef _ClosureType ClosureType;

    const _ClosureType closureHandler;

    Closure(const _ClosureType& handler)
            : closureHandler(handler)
    {
    }

    _ReturnType operator()(_ArgTypes...args) override
    {
        if(IsVoid<_ReturnType>::value)
            closureHandler(args...);
        else
            return closureHandler(args...);
    }
};

// Fuction template selector
template <typename _FunctionType>
class Function
        : public Function<decltype(&_FunctionType::operator())>
{
};

// Function, lambda, functor...
template <typename _ReturnType, typename... _ArgTypes>
class Function<_ReturnType(*)(_ArgTypes...)>
{
public:
    typedef Function<_ReturnType(*)(_ArgTypes...)> SelfType;
    typedef _ReturnType(*SignatureType)(_ArgTypes...);
    Callable<_ReturnType, _ArgTypes...>* callableClosure;

    Function(_ReturnType(*function)(_ArgTypes...))
            : callableClosure(new Closure<SignatureType, _ReturnType, _ArgTypes...>(function))
    {
    }

    // Captured lambda specialization
    template<typename _ClosureType>
    Function(const _ClosureType& function)
            : callableClosure(new Closure<decltype(function), _ReturnType, _ArgTypes...>(function))
    {
    }

    _ReturnType operator()(_ArgTypes... args)
    {
        if(IsVoid<_ReturnType>::value)
            (*callableClosure)(args...);
        else
            return (*callableClosure)(args...);
    }
};

// Member method
template <typename _ClassType, typename _ReturnType, typename... _ArgTypes>
class Function<_ReturnType(_ClassType::*)(_ArgTypes...)>
{
public:
    typedef Function<_ReturnType(_ClassType::*)(_ArgTypes...)> SelfType;
    typedef _ReturnType(_ClassType::*SignatureType)(_ArgTypes...);

    SignatureType methodSignature;

    Function(_ReturnType(_ClassType::*method)(_ArgTypes...))
            : methodSignature(method)
    {
    }

    _ReturnType operator()(_ClassType* object, _ArgTypes... args)
    {
        if(IsVoid<_ReturnType>::value)
            (object->*methodSignature)(args...);
        else
            return (object->*methodSignature)(args...);
    }
};

// Const member method
template <typename _ClassType, typename _ReturnType, typename... _ArgTypes>
class Function<_ReturnType(_ClassType::*)(_ArgTypes...) const>
{
public:
    typedef Function<_ReturnType(_ClassType::*)(_ArgTypes...) const> SelfType;
    typedef _ReturnType(_ClassType::*SignatureType)(_ArgTypes...) const;

    SignatureType methodSignature;

    Function(_ReturnType(_ClassType::*method)(_ArgTypes...) const)
            : methodSignature(method)
    {
    }

    _ReturnType operator()(_ClassType* object, _ArgTypes... args)
    {
        if(IsVoid<_ReturnType>::value)
            (object->*methodSignature)(args...);
        else
            return (object->*methodSignature)(args...);
    }
};





class thread_pool {
public:

    explicit thread_pool(std::size_t max_tasks, std::size_t num_threads = boost::thread::hardware_concurrency() - 1);

    ~thread_pool();

    template <typename Class, typename... Args>
    void add_work(Function<void(Class::*)(Args...)>, Args&&...);

    template <typename... Args>
    void add_work(std::function<void(Args...)>, Args&&...);

    template<typename F, typename... Args>
    auto add_job(F&& fn, Args&&... args) -> boost::unique_future<typename boost::result_of<F(Args...)>::type>;

    std::size_t count_jobs();

private:

    typedef std::function<void()> task;
    std::size_t max_tasks;
    std::queue<task> tasks;
    std::vector<boost::thread> workers;
    bool finalize = false;
    std::condition_variable has_work;
    std::condition_variable is_full;
    std::mutex queue_mutex;
    std::mutex push_mutex;
};

#include "../../src/thread_pool.cpp"
#endif //ANUBIS_PROJECT_THREAD_POOL_HPP
