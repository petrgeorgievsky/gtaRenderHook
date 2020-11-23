#include "font.h"

void CFont::InitPerFrame()
{
    static auto f_ptr = 0x719800;
    return reinterpret_cast<decltype( &CFont::InitPerFrame )>( f_ptr )();
}

void CFont::RenderFontBuffer()
{
    static auto f_ptr = 0x71A210;
    return reinterpret_cast<decltype( &CFont::RenderFontBuffer )>( f_ptr )();
}
