#pragma once
#include <stdint.h>
class CRenderer
{
public:
    static void **ms_aVisibleEntityPtrs;
    static uint32_t &ms_nNoOfVisibleEntities;

public:
    static void ConstructRenderList();
    static void PreRender();
};
