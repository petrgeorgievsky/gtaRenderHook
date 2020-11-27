//
// Created by peter on 23.05.2020.
//

#include "ModelInfo.h"
#include "Renderer.h"
BaseModelInfo **ModelInfo::mModelInfoPtrs =
    reinterpret_cast<BaseModelInfo **>( 0x83D408 );
;

RpAtomic *SimpleModelInfo::GetAtomicFromDistance( float d )
{
    d /= GameRendererConfigBlock::It.LodMultiplier;
    for ( int i = mIsDamaged ? mFirstDamaged : 0; i < mNumAtomics; i++ )
        if ( d < mLodDistances[i] )
            return mAtomics[i];
    return nullptr;
}
