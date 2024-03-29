#pragma once
#include <common_headers.h>
#include <cstdint>
#include <iterator>

namespace rh::rw::engine
{

namespace rwTexture
{

constexpr auto rwTEXTUREFILTERMODEMASK  = 0x000000FF;
constexpr auto rwTEXTUREADDRESSINGUMASK = 0x00000F00;
constexpr auto rwTEXTUREADDRESSINGVMASK = 0x0000F000;
constexpr auto rwTEXTUREADDRESSINGMASK =
    ( rwTEXTUREADDRESSINGUMASK | rwTEXTUREADDRESSINGVMASK );

constexpr auto GetFilterMode( RwTexture *_tex )
{
    return _tex->filterAddressing & rwTEXTUREFILTERMODEMASK;
}
constexpr auto GetAddressingU( RwTexture *_tex )
{
    return static_cast<RwTextureAddressMode>(
        ( _tex->filterAddressing & rwTEXTUREADDRESSINGUMASK ) >> 8 );
}
constexpr auto GetAddressingV( RwTexture *_tex )
{
    return static_cast<RwTextureAddressMode>(
        ( _tex->filterAddressing & rwTEXTUREADDRESSINGVMASK ) >> 12 );
}
constexpr void SetFilterMode( RwTexture *_tex, uint32_t _filtering )
{
    _tex->filterAddressing =
        ( _tex->filterAddressing &
          static_cast<uint32_t>( ~rwTEXTUREFILTERMODEMASK ) ) |
        ( _filtering & rwTEXTUREFILTERMODEMASK );
}

constexpr void SetAddressingU( RwTexture *_tex, uint32_t _addressing )
{
    _tex->filterAddressing =
        ( _tex->filterAddressing &
          static_cast<uint32_t>( ~rwTEXTUREADDRESSINGUMASK ) ) |
        ( ( _addressing << 8 ) & rwTEXTUREADDRESSINGUMASK );
}

constexpr void SetAddressingV( RwTexture *_tex, uint32_t _addressing )
{
    _tex->filterAddressing =
        ( _tex->filterAddressing &
          static_cast<uint32_t>( ~rwTEXTUREADDRESSINGVMASK ) ) |
        ( ( _addressing << 12 ) & rwTEXTUREADDRESSINGVMASK );
}

constexpr void SetAddressing( RwTexture *_tex, uint32_t _addressing )
{
    _tex->filterAddressing =
        ( _tex->filterAddressing &
          static_cast<uint32_t>( ~rwTEXTUREADDRESSINGMASK ) ) |
        ( ( _addressing << 8 ) & rwTEXTUREADDRESSINGUMASK ) |
        ( ( _addressing << 12 ) & rwTEXTUREADDRESSINGVMASK );
}

} // namespace rwTexture
namespace rwRGBA
{

constexpr uint32_t Long( RwRGBA color )
{
    return static_cast<uint32_t>( ( ( color.alpha ) << 24 ) |
                                  ( ( color.red ) << 16 ) |
                                  ( ( color.green ) << 8 ) | ( color.blue ) );
}

constexpr uint32_t Long_RGB( RwRGBA color )
{
    return static_cast<uint32_t>( ( ( 0xFF ) << 24 ) | ( ( color.red ) << 16 ) |
                                  ( ( color.green ) << 8 ) | ( color.blue ) );
}
constexpr uint32_t Long( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
    return static_cast<uint32_t>( ( ( a ) << 24 ) | ( ( r ) << 16 ) |
                                  ( ( g ) << 8 ) | ( b ) );
}
constexpr void Assign( RwRGBA *_target, RwRGBA *_source )
{
    *_target = *_source;
}
} // namespace rwRGBA
namespace rpMaterial
{

constexpr void AddRef( RpMaterial *_material ) { _material->refCount++; }

constexpr void SetSurfaceProperties( RpMaterial                *_material,
                                     const RwSurfaceProperties &_surfProps )
{
    _material->surfaceProps = _surfProps;
}
constexpr void SetColor( RpMaterial *_material, const RwRGBA &_color )
{
    _material->color = _color;
}
} // namespace rpMaterial
namespace rwObject
{

constexpr void *GetParent( void *object )
{
    return static_cast<const RwObject *>( object )->parent;
}
constexpr void SetParent( void *c, void *p )
{
    static_cast<RwObject *>( c )->parent = p;
}
constexpr uint8_t GetPrivateFlags( void *c )
{
    return static_cast<const RwObject *>( c )->privateFlags;
}
constexpr void SetPrivateFlags( void *c, uint8_t f )
{
    static_cast<RwObject *>( c )->privateFlags = f;
}

constexpr uint8_t TestPrivateFlags( void *c, uint8_t flag )
{
    return static_cast<RwObject *>( c )->privateFlags & flag;
}
constexpr void Initialize( void *o, uint8_t t, uint8_t s )
{
    static_cast<RwObject *>( o )->type         = t;
    static_cast<RwObject *>( o )->subType      = s;
    static_cast<RwObject *>( o )->flags        = 0;
    static_cast<RwObject *>( o )->privateFlags = 0;
    static_cast<RwObject *>( o )->parent       = nullptr;
}
constexpr void HasFrameInitialize( void *o, uint8_t type, uint8_t subtype,
                                   RwObjectHasFrameSyncFunction syncFunc )
{
    Initialize( o, type, subtype );
    static_cast<RwObjectHasFrame *>( o )->sync = syncFunc;
}
constexpr uint8_t GetFlags( void *o )
{
    return static_cast<const RwObject *>( o )->flags;
}
constexpr void SetFlags( void *o, uint8_t f )
{
    static_cast<RwObject *>( o )->flags = f;
}
} // namespace rwObject
namespace rwLLLink
{
constexpr auto GetNext( RwLLLink *linkvar ) { return linkvar->next; }
template <typename type>
constexpr type *GetData( void *linkvar, uint32_t offset )
{
    return static_cast<type *>(
        static_cast<void *>( static_cast<uint8_t *>( linkvar ) - offset ) );
}
constexpr void Initialize( RwLLLink *linkvar )
{
    linkvar->prev = nullptr;
    linkvar->next = nullptr;
}
} // namespace rwLLLink
namespace rwLinkList
{

constexpr void AddLLLink( RwLinkList *list, RwLLLink *linkvar )
{
    linkvar->next         = list->link.next;
    linkvar->prev         = &list->link;
    list->link.next->prev = linkvar;
    list->link.next       = linkvar;
}

constexpr void RemoveLLLink( void *list )
{
    static_cast<RwLLLink *>( list )->prev->next =
        static_cast<RwLLLink *>( list )->next;
    static_cast<RwLLLink *>( list )->next->prev =
        static_cast<RwLLLink *>( list )->prev;
}

constexpr auto GetTerminator( void *list )
{
    return &static_cast<RwLinkList *>( list )->link;
}
constexpr auto GetTerminator( const void *list )
{
    return &static_cast<const RwLinkList *>( list )->link;
}
constexpr auto GetLastLLLink( const void *list )
{
    return static_cast<const RwLinkList *>( list )->link.prev;
}
constexpr auto GetFirstLLLink( const void *list )
{
    return static_cast<const RwLinkList *>( list )->link.next;
}
constexpr void Initialize( void *list )
{
    static_cast<RwLinkList *>( list )->link.next =
        static_cast<RwLLLink *>( list );
    static_cast<RwLinkList *>( list )->link.prev =
        static_cast<RwLLLink *>( list );
}

template <typename T> class Iterator
{
  private:
    RwLinkList &mList;

  public:
    explicit Iterator( RwLinkList &list ) : mList( list ) {}
    class iterator
    {
        RwLLLink *mIterPtr;

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

        explicit iterator( RwLLLink *ptr = nullptr ) : mIterPtr( ptr ) {}
        const T *operator*() const
        {
            return rwLLLink::GetData<T>( mIterPtr, offsetof( T, inClumpLink ) );
        }
        T *operator*()
        {
            return rwLLLink::GetData<T>( mIterPtr, offsetof( T, inClumpLink ) );
        }
        iterator &operator++()
        {
            mIterPtr = rw::engine::rwLLLink::GetNext( mIterPtr );
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

    iterator begin() const { return iterator( GetFirstLLLink( &mList ) ); }
    iterator end() const { return iterator( GetTerminator( &mList ) ); }
    iterator begin() { return iterator( GetFirstLLLink( &mList ) ); }
    iterator end() { return iterator( GetTerminator( &mList ) ); }
};
} // namespace rwLinkList
namespace rwFrame
{
constexpr RwFrame *GetParent( void *_f )
{
    return static_cast<RwFrame *>( rwObject::GetParent( _f ) );
}
} // namespace rwFrame
} // namespace rh::rw::engine
