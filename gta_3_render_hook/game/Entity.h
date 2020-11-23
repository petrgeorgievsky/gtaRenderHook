//
// Created by peter on 23.05.2020.
//

#pragma once
#include <common_headers.h>
#include <cstdint>
class Matrix
{
  public:
    RwMatrix  m_matrix;
    RwMatrix *m_attachment;
    bool      m_hasRwMatrix; // are we the owner?
};

class Placeable
{
  public:
    void * mVtable;
    Matrix mMatrix;
};
static_assert( sizeof( Placeable ) == 0x4C, "CPlaceable: error" );

class Entity : public Placeable
{
  public:
    RwObject *mRwObject;
    uint32_t  mType : 3;
    uint32_t  mStatus : 5;

    // flagsA
    uint32_t bUsesCollision : 1;      // does entity use collision
    uint32_t bCollisionProcessed : 1; // has object been processed by a
                                      // ProcessEntityCollision function
    uint32_t bIsStatic : 1;           // is entity static
    uint32_t bHasContacted : 1; // has entity processed some contact forces
    uint32_t bPedPhysics : 1;
    uint32_t bIsStuck : 1; // is entity stuck
    uint32_t
             bIsInSafePosition : 1; // is entity in a collision free safe position
    uint32_t bUseCollisionRecords : 1;

    // flagsB
    uint32_t bWasPostponed : 1; // was entity control processing postponed
    uint32_t bExplosionProof : 1;
    uint32_t bIsVisible : 1; // is the entity visible
    uint32_t bHasCollided : 1;
    uint32_t bRenderScorched : 1;
    uint32_t bHasBlip : 1;
    uint32_t bIsBIGBuilding : 1; // Set if this entity is a big building
    // VC inserts one more flag here: if drawdist <= 2000
    uint32_t bRenderDamaged : 1; // use damaged LOD models for objects with
                                 // applicable damage

    // flagsC
    uint32_t bBulletProof : 1;
    uint32_t bFireProof : 1;
    uint32_t bCollisionProof : 1;
    uint32_t bMeleeProof : 1;
    uint32_t bOnlyDamagedByPlayer : 1;
    uint32_t bStreamingDontDelete : 1; // Dont let the streaming remove this
    uint32_t bZoneCulled : 1;
    uint32_t bZoneCulled2 : 1; // only treadables+10m

    // flagsD
    uint32_t bRemoveFromWorld : 1; // remove this entity next time it should be
                                   // processed
    uint32_t bHasHitWall : 1;      // has collided with a building (changes
                                   // subsequent collisions)
    uint32_t bImBeingRendered : 1; // don't delete me because I'm being rendered
    uint32_t bTouchingWater : 1;   // used by cBuoyancy::ProcessBuoyancy
    uint32_t bIsSubway : 1; // set when subway, but maybe different meaning?
    uint32_t bDrawLast : 1; // draw object last
    uint32_t bNoBrightHeadLights : 1;
    uint32_t bDoNotRender : 1;

    // flagsE
    uint32_t bDistanceFade : 1; // Fade entity because it is far away
    uint32_t mFlagE2 : 1;

    uint16_t mScanCode;
    uint16_t mRandomSeed;
    int16_t  mModelIndex;
    uint16_t mLevel; // int16
    void *   mFirstReference;
    void     CreateRwObject();
    void     PreRender();
    void     Render();
};
static_assert( sizeof( Entity ) == 0x64, "CEntity: error" );