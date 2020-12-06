//
// Created by peter on 23.05.2020.
//

#include "ModelInfo.h"
#include "../call_redirection_util.h"
#include "../config/GameRendererConfigBlock.h"

BaseModelInfo **ModelInfo::mModelInfoPtrs = reinterpret_cast<BaseModelInfo **>(
    GetAddressByGame( 0x83D408, 0x83D408, 0x84D548 ) );
;

RpAtomic *SimpleModelInfo::GetAtomicFromDistance( float d )
{
    d /= GameRendererConfigBlock::It.LodMultiplier;
    for ( int i = mIsDamaged ? mFirstDamaged : 0; i < mNumAtomics; i++ )
        if ( d < mLodDistances[i] )
            return mAtomics[i];
    return nullptr;
}
