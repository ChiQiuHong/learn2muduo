#pragma once

#include "muduo/base/noncopyable.h"

#include <stdint.h>

namespace muduo
{
    namespace detail
    {
        template <typename T>
        class AtomicIntegerT : noncopyable
        {
        public:
            AtomicIntegerT()
                : value_(0)
            {
            }

            T get()
            {
                // return __sync_val_compare_and_swap(&value_, 0, 0);
                return __atomic_load_n(&value_, __ATOMIC_SEQ_CST);
            }

            T getAndAdd(T x)
            {
                // return __sync_fetch_and_add(&value_, x);
                return __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST);
            }

            T addAndGet(T x)
            {
                return getAndAdd(x) + x;
            }

            T incrementAndGet()
            {
                return addAndGet(1);
            }

            T decrementAndGet()
            {
                return addAndGet(-1);
            }

            void add(T x)
            {
                getAndAdd(x);
            }

            void increment()
            {
                incrementAndGet();
            }

            void decrement()
            {
                decrementAndGet();
            }

            T getAndSet(T newValue)
            {
                return __atomic_exchange_n(&value_, newValue, __ATOMIC_SEQ_CST);
            }
        
        private:
            volatile T value_;
        };
    } // namespace detail

    typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
    typedef detail::AtomicIntegerT<int64_t> AtomicInt64;

} // namespace muduo
