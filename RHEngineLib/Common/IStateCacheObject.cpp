#include "stdafx.h"
#include "IStateCacheObject.h"

bool RHEngine::IStateCacheObject::IsDirty()
{
	return m_bDirty;
}

void RHEngine::IStateCacheObject::MakeDirty()
{
	m_bDirty = true;
}

void RHEngine::IStateCacheObject::Flush(void* deviceObject)
{
	if (m_bDirty) 
	{
		OnFlush(deviceObject);
		m_bDirty = false;
	}
}
