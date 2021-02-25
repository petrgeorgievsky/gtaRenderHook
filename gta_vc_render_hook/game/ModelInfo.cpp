//
// Created by peter on 23.05.2020.
//

#include "ModelInfo.h"
//#include "../call_redirection_util.h"

BaseModelInfo **ModelInfo::mModelInfoPtrs =
    reinterpret_cast<BaseModelInfo **>( 0x92D4C8 );

RpAtomic *SimpleModelInfo::GetAtomicFromDistance( float d )
{
    d /= 4.0f;
    for ( int i = mIsDamaged ? mFirstDamaged : 0; i < mNumAtomics; i++ )
        if ( d < mLodDistances[i] )
            return mAtomics[i];
    return nullptr;
}
