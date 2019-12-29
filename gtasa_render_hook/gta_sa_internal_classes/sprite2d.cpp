#include "sprite2d.h"

void CSprite2d::InitPerFrame()
{
    static auto f_ptr = 0x727350;
    return reinterpret_cast<decltype( &CSprite2d::InitPerFrame )>( f_ptr )();
}
