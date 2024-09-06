#pragma once
#include <mutex>
#include <memory>
namespace zeus
{
template<typename T>
class Singleton
{
public:
    static T& Instance()
    {
        static T s_instance;
        return s_instance;
    }
private:
    Singleton() = default;
    ~Singleton() = default;
};

template<typename T>
class ComplexSingleton
{
public:
    static T& Instance()
    {
        std::call_once(_oc, []()
                       {
                           _instance = std::make_unique<T>();
                       });
        return *_instance;
    }

    static void Destory()
    {
        _instance.reset();
    }
private:
    ComplexSingleton() = default;
    ~ComplexSingleton() = default;
private:
    static std::unique_ptr<T> _instance;
    static std::once_flag _oc;
};
template <typename T>
std::unique_ptr<T> ComplexSingleton<T> ::_instance = nullptr;
template <typename T>
std::once_flag ComplexSingleton<T> ::_oc;

template<typename T>
class RefCountSingleton
{
private:
    struct Context
    {
        int count;
        std::unique_ptr<T> data;
        std::mutex mutex;
    };

    static Context* GetContext()
    {
        static Context contextInstance;
        return &contextInstance;
    }
    RefCountSingleton() = default;
    ~RefCountSingleton() = default;
public:
    static T& Instance()
    {
        auto context = GetContext();
        return *context->data;
    }

    static void IncRef()
    {
        auto context = GetContext();
        std::lock_guard<std::mutex> lock(context->mutex);
        if (0 == context->count)
        {
            context->data = std::make_unique<T>();
        }
        context->count++;
    }

    static void DecRef()
    {
        auto context = GetContext();
        std::lock_guard<std::mutex> lock(context->mutex);
        context->count--;
        if (0 == context->count)
        {
            context->data.reset();
        }
    }
};
}

#include "zeus/foundation/core/zeus_compatible.h"
