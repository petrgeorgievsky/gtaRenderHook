//
// Created by peter on 23.05.2020.
//

#include "ModelInfo.h"
BaseModelInfo **ModelInfo::mModelInfoPtrs =
    reinterpret_cast<BaseModelInfo **>( 0x83D408 );
;

RpAtomic *SimpleModelInfo::GetAtomicFromDistance( float d )
{
    for ( int i = mIsDamaged ? mFirstDamaged : 0; i < mNumAtomics; i++ )
        if ( d < mLodDistances[i] * 3.2f /* TheCamera.LODDistMultiplier*/ )
            return mAtomics[i];
    return nullptr;
}
