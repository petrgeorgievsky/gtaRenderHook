#pragma once
#include <common.h>
#include <common_headers.h>
class CScene
{
  public:
    RpWorld * m_pRpWorld;
    RwCamera *m_pRwCamera;
};
extern CScene &Scene;
