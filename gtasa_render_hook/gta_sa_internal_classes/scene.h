#pragma once
#include <common.h>
class CScene
{
public:
    RpWorld *m_pRpWorld;
    RwCamera *m_pRwCamera;
};
extern CScene &Scene;
