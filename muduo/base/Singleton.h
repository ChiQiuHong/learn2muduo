#pragma once

#include "muduo/base/noncopyable.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h> // atexit
#include <utility>

// 线程安全的 Singleton 实现

namespace muduo
{

    namespace detail
    {
        // 使用 SFINAE 判断某个类内是否有某个成员函数
        template <typename T>
        struct has_no_destroy
        {
            template <typename C>
            static char test(decltype(&C::no_destroy));

            template <typename C>
            static int32_t test(...);

            const static bool value = sizeof(test<T>(0)) == 1;
        };

        // 或者 这种写法
        template <typename T>
        bool checkHasNoDestroy(decltype(std::declval<T>().no_destroy()) *test = nullptr)
        {
            return true;
        }

        template <typename T>
        bool checkHasNoDestroy(...)
        {
            return false;
        }

    } // namespace detail

    template <typename T>
    class Singleton : noncopyable
    {
    public:
        static T &instance()
        {
            pthread_once(&ponce_, &Singleton::init);
            assert(value_ != NULL);
            return *value_;
        }

    private:
        Singleton();
        ~Singleton();

        static void init()
        {
            value_ = new T();
            if (!detail::checkHasNoDestroy<T>(nullptr))
            // if (!detail::has_no_destroy<T>::value)
            {
                ::atexit(destroy);
            }
        }

        static void destroy()
        {
            delete value_;
            value_ = NULL;
        }

        static pthread_once_t ponce_;
        static T *value_;
    };

    template <typename T>
    pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

    template <typename T>
    T *Singleton<T>::value_ = NULL;

} // namespace muduo