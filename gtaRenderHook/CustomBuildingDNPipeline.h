#pragma once
#include "DeferredPipeline.h"
class CCustomBuildingDNPipeline :
	public CDeferredPipeline
{
public:
	CCustomBuildingDNPipeline();
	~CCustomBuildingDNPipeline();
	static void Patch();
	void SetPipeline();
	void Render(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
private:
};
extern CCustomBuildingDNPipeline* g_pCustomBuildingDNPipe;