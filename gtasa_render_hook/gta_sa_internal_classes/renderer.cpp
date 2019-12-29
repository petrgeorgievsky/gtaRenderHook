#include "renderer.h"
void **CRenderer::ms_aVisibleEntityPtrs = reinterpret_cast<void **>( 0xB75898 );
uint32_t &CRenderer::ms_nNoOfVisibleEntities = *reinterpret_cast<uint32_t *>( 0xB76844 );

void CRenderer::ConstructRenderList()
{
    static auto FPTR = 0x5556E0;
    reinterpret_cast<void( __cdecl * )()>( FPTR )();
}

void CRenderer::PreRender()
{
    static auto FPTR = 0x553910;
    reinterpret_cast<void( __cdecl * )()>( FPTR )();
}
