#pragma once
#include <game_sa\CEntity.h>
#include <game_sa\CLinkList.h>
class CStreamingRH
{
public:
    static void Patch();
    /*
        Fixed function for original GTA SA memory leak when there is more than 1000 entities
        streamed simultaneously.
    */
    static CLink<CEntity*>* AddEntity( CEntity *pEntity );
};

