#include "menumanager.h"
CMenuManager *g_pFrontEndMenuManager = reinterpret_cast<CMenuManager *>( 0xBA6748 );
void CMenuManager::DrawFrontEnd()
{
    static auto f_ptr = 0x57C290;
    reinterpret_cast<void( __thiscall * )( CMenuManager * )>( f_ptr )( this );
}
