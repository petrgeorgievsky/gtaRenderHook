//
// Created by peter on 04.08.2020.
//
#pragma once
#include <cassert>

namespace rh::engine
{
template <typename T> class ScopedPointer
{
  public:
    ScopedPointer() = default;
    ScopedPointer<T>( T *ptr ) { mPtr = ptr; }
    ~ScopedPointer()
    {
        delete mPtr;
        mPtr = nullptr;
    }

    ScopedPointer<T> &operator=( T *ptr )
    {
        assert( ptr );
        mPtr = ptr;
        return *this;
    }

    T *operator->() const noexcept { return mPtr; }
       operator T *() { return mPtr; }

  private:
    T *mPtr = nullptr;
};
} // namespace rh::engine