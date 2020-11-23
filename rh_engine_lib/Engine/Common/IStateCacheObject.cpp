#include "IStateCacheObject.h"
using namespace rh::engine;
bool IStateCacheObject::IsDirty()
{
    return m_bDirty;
}

void IStateCacheObject::MakeDirty()
{
    m_bDirty = true;
}

void IStateCacheObject::Flush( void *deviceObject )
{
    if ( m_bDirty ) {
        OnFlush( deviceObject );
        m_bDirty = false;
    }
}

void IStateCacheObject::Invalidate()
{
    MakeDirty();
}
