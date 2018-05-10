// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "StreamingRH.h"
#include <game_sa\CStreaming.h>

void CStreamingRH::Patch()
{
	// increase ms_rwObjectInstances size to 200*10^2 to allow for more entities streamed at once
	SetInt(0x5B8E55, 12 * 20000);
	SetInt(0x5B8EB0, 12 * 20000);
	RedirectJump(0x409650, AddEntity);
}

// Hope pluginsdk will include this method soon, starting to hate these definitions after having such convinient sdk
#define rw_objlink_Add(a,b) ((CLink<CEntity*> * (__thiscall*)(CLinkList<CEntity*> *, CEntity **))0x408230)(a,b)

CLink<CEntity*>* CStreamingRH::AddEntity(CEntity * pEntity)
{
	CLink<CEntity*> *result = nullptr;
	CLink<CEntity*> *currLink;

	eEntityType entityType = (eEntityType)pEntity->m_nType;
	// Peds and vehicles are streamed differently, ignore them
	if (entityType != eEntityType::ENTITY_TYPE_PED && 
		entityType != eEntityType::ENTITY_TYPE_VEHICLE)
	{
		result = rw_objlink_Add(&CStreaming::ms_rwObjectInstances, &pEntity);

		if (result == nullptr) // if we fail to add link than something has happend
		{
			currLink = CStreaming::ms_rwObjectInstances.usedListTail.prev;

			// traverse the list from last link to head 
			while (currLink != &CStreaming::ms_rwObjectInstances.usedListHead)
			{
				// if we found some entity that can be deleted - remove it's rwobject
				if (!currLink->data->m_bStreamingDontDelete && !currLink->data->m_bImBeingRendered)
				{
					if(currLink->data->m_pRwObject)
						currLink->data->DeleteRwObject();
					return rw_objlink_Add(&CStreaming::ms_rwObjectInstances, &pEntity);
				}
				currLink = currLink->prev;
			}
			// if list is full, than we remove rwobject of entity we try to add to it
			if(pEntity->m_pRwObject)
				pEntity->DeleteRwObject();
			return nullptr;
		}
	}
	return result;
}
