//
// Created by peter on 28.11.2020.
//
#pragma once
#include <utility>

namespace rh::engine
{

/**
 * Fast pimpl implementation, based on Yandex talk "Taxi C++ tricks", main
 * purpose is to improve build speeds when using dependencies, while getting
 * same performance
 * @tparam Type Implementation type name
 * @tparam Align Implementation type expected alignment
 * @tparam Size Implementation type expected size
 */
template <typename Type, size_t Align, size_t Size> class FastPimpl
{
  public:
    template <class... Args> explicit FastPimpl( Args &&... args )
    {
        new ( Ptr() ) Type( std::forward<Args>( args )... );
    }
    ~FastPimpl()
    {
        Validate<sizeof( Type ), alignof( Type )>();
        Ptr()->~Type();
    }

    Type *      operator->() noexcept { return Ptr(); }
    const Type *operator->() const noexcept { return Ptr(); }
    Type &      operator*() noexcept { return *Ptr(); }
    const Type &operator*() const noexcept { return *Ptr(); }

    const Type *Ptr() const noexcept
    {
        return reinterpret_cast<Type *>( &mData );
    }

    Type *Ptr() noexcept { return reinterpret_cast<Type *>( &mData ); }

  private:
    template <std::size_t ActualSize, std::size_t ActualAlignment>
    static void Validate() noexcept
    {
        static_assert( Size == ActualSize, "Size and sizeof(T) mismatch" );
        static_assert( Align == ActualAlignment,
                       "Align and alignof(T) mismatch" );
    }
    std::aligned_storage<Size, Align> mData;
};
} // namespace rh::engine
