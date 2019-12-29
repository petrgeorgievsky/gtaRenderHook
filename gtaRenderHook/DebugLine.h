#pragma once
#include "DebugRenderObject.h"
class DebugLine :
    public DebugRenderObject
{
public:
    DebugLine();
    ~DebugLine();
    void Render();
};

