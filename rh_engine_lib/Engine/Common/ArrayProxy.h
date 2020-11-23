#pragma once
#include <array>
#include <cstdint>
#include <iterator>
#include <vector>

namespace rh::engine
{
/*
    Non-owning immutable proxy to an array on stack or heap, used to transfer
   data from higher levels of abstraction.
*/
template <typename T> class ArrayProxy
{
  private:
    const T *   mData;
    std::size_t mSize;

  public:
    ArrayProxy()
    {
        mData = nullptr;
        mSize = 0;
    }
    ArrayProxy( const T *data, std::size_t size )
    {
        mData = data;
        mSize = size;
    }
    ArrayProxy( const std::vector<T> &vec )
    {
        mData = vec.data();
        mSize = vec.size();
    }
    template <size_t Size> ArrayProxy( const std::array<T, Size> &arr )
    {
        mData = arr.data();
        mSize = arr.size();
    }
    ArrayProxy( std::initializer_list<T> initializerList )
    {
        mData = initializerList.begin();
        mSize = initializerList.size();
    }

    const T *   Data() const { return mData; }
    std::size_t Size() const { return mSize; }

    const T &operator[]( size_t id ) const { return mData[id]; }

    class iterator
    {
        const T *   mArrPtr;
        std::size_t mOffset;

      public:
        // The std::iterator class template (used as a base class to provide
        // typedefs) is deprecated in C++17. (The <iterator> header is NOT
        // deprecated.) The C++ Standard has never required user-defined
        // iterators to derive from std::iterator.
        // To fix this warning, stop deriving from std::iterator and start
        // providing publicly accessible typedefs named iterator_category,
        // value_type, difference_type, pointer, and reference.
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;

        explicit iterator( const T *ptr = 0, size_t offset = 0 )
            : mArrPtr( ptr ), mOffset( offset )
        {
        }
        const T & operator*() const { return mArrPtr[mOffset]; }
        iterator &operator++()
        {
            mOffset++;
            return *this;
        }
        iterator operator++( int )
        {
            iterator retval = *this;
            ++( *this );
            return retval;
        }
        difference_type operator-( const iterator &other ) const
        {
            return other.mOffset - mOffset;
        }
        bool operator==( const iterator &other ) const
        {
            return mOffset == other.mOffset;
        }
        bool operator!=( const iterator &other ) const
        {
            return !( *this == other );
        }
    };

    iterator begin() const { return iterator( mData ); }
    iterator end() const { return iterator( mData, mSize ); }
};
} // namespace rh::engine
