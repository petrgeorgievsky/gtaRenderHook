//
// Created by peter on 23.05.2020.
//

#pragma once
#include <iterator>

template <typename T> class PtrNode
{
  public:
    T *         item;
    PtrNode<T> *prev;
    PtrNode<T> *next;
};

template <typename T> class PtrList
{
  public:
    PtrNode<T> *first;

    class iterator
    {
        const PtrNode<T> *mIterPtr;

      public:
        // The std::iterator class template (used as a base class to provide
        // typedefs) is deprecated in C++17. (The <iterator> header is NOT
        // deprecated.) The C++ Standard has never required user-defined
        // iterators to derive from std::iterator.
        // To fix this warning, stop deriving from std::iterator and start
        // providing publicly accessible typedefs named iterator_category,
        // value_type, difference_type, pointer, and reference.
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;

        explicit iterator( const PtrNode<T> *ptr = nullptr ) : mIterPtr( ptr )
        {
        }
        const T * operator*() const { return mIterPtr->item; }
        T *       operator*() { return mIterPtr->item; }
        iterator &operator++()
        {
            mIterPtr = mIterPtr->next;
            return *this;
        }
        iterator operator++( int )
        {
            iterator retval = *this;
            ++( *this );
            return retval;
        }
        iterator &operator--()
        {
            mIterPtr = mIterPtr->prev;
            return *this;
        }
        iterator operator--( int )
        {
            iterator retval = *this;
            --( *this );
            return retval;
        }
        bool operator==( const iterator &other ) const
        {
            return mIterPtr == other.mIterPtr;
        }
        bool operator!=( const iterator &other ) const
        {
            return !( *this == other );
        }
    };

    iterator begin() const { return iterator( first ); }
    iterator end() const { return iterator(); }
    iterator begin() { return iterator( first ); }
    iterator end() { return iterator(); }
};