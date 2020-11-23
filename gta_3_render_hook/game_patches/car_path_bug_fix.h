//
// Created by peter on 30.10.2020.
//

#pragma once

#include <cmath>
#include <common_headers.h>
#include <cstdint>
#include <span>

// TODO: Move out
class CVector
{
  public:
    float x, y, z;
    float Magnitude( void ) const { return sqrt( x * x + y * y + z * z ); }
};
inline CVector operator-( const CVector &left, const CVector &right )
{
    return CVector{ left.x - right.x, left.y - right.y, left.z - right.z };
}

class CVector2D
{
  public:
    float x, y;
};

class CReference
{
};
class CMatrix
{
  public:
    void *    vtable;
    RwMatrix  m_matrix;
    RwMatrix *m_attachment;
    bool      m_hasRwMatrix; // are we the owner?
};
class CPlaceable
{
  public:
    // disable allocation
    static void *operator new( size_t ) = delete;

    CMatrix m_matrix;
};
static_assert( sizeof( CPlaceable ) == 0x4C );
class CEntity : public CPlaceable
{
  public:
    RwObject *m_rwObject;

  protected:
    uint32_t m_type : 3;

  private:
    uint32_t m_status : 5;

  public:
    // flagsA
    uint32_t bUsesCollision : 1;      // does entity use collision
    uint32_t bCollisionProcessed : 1; // has object been processed by a
    // ProcessEntityCollision function
    uint32_t bIsStatic : 1;     // is entity static
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
    uint32_t bHasHitWall : 1; // has collided with a building (changes
    // subsequent collisions)
    uint32_t bImBeingRendered : 1; // don't delete me because I'm being rendered
    uint32_t bTouchingWater : 1;   // used by cBuoyancy::ProcessBuoyancy
    uint32_t bIsSubway : 1; // set when subway, but maybe different meaning?
    uint32_t bDrawLast : 1; // draw object last
    uint32_t bNoBrightHeadLights : 1;
    uint32_t bDoNotRender : 1;

    // flagsE
    uint32_t bDistanceFade : 1; // Fade entity because it is far away
    uint32_t m_flagE2 : 1;

    uint16_t    m_scanCode;
    uint16_t    m_randomSeed;
    int16_t     m_modelIndex;
    uint16_t    m_level; // int16
    CReference *m_pFirstReference;
};

static_assert( sizeof( CEntity ) == 0x64 );

class CBuilding : public CEntity
{
};
static_assert( sizeof( CBuilding ) == 0x64 );

class CTreadable : public CBuilding
{
  public:
    int16_t m_nodeIndices[2][12]; // first car, then ped
};

static_assert( sizeof( CTreadable ) == 0x94 );

/// Path constants
constexpr auto NUM_PATHNODES = 4930, NUM_CARPATHLINKS = 2076,
               NUM_MAPOBJECTS = 1250, NUM_PATHCONNECTIONS = 10260;

struct CPathNode
{
    CVector    pos;
    CPathNode *prev;
    CPathNode *next;
    int16_t    distance; // in path search
    int16_t    objectIndex;
    int16_t    firstLink;
    uint8_t    numLinks;

    uint8_t unkBits : 2;
    uint8_t bDeadEnd : 1;
    uint8_t bDisabled : 1;
    uint8_t bBetweenLevels : 1;

    int8_t group;

    CVector &GetPosition( void ) { return pos; }
    void     SetPosition( const CVector &p ) { pos = p; }
    float    GetX( void ) { return pos.x; }
    float    GetY( void ) { return pos.y; }
    float    GetZ( void ) { return pos.z; }

    CPathNode *GetPrev( void ) { return prev; }
    CPathNode *GetNext( void ) { return next; }
    void       SetPrev( CPathNode *node ) { prev = node; }
    void       SetNext( CPathNode *node ) { next = node; }
};

struct CCarPathLink
{
    CVector2D pos;
    CVector2D dir;
    int16_t   pathNodeIndex;
    int8_t    numLeftLanes;
    int8_t    numRightLanes;
    uint8_t   trafficLightType;

    uint8_t bBridgeLights : 1;
    // more?

    CVector2D &GetPosition( void ) { return pos; }
    CVector2D &GetDirection( void ) { return dir; }
    float      GetX( void ) { return pos.x; }
    float      GetY( void ) { return pos.y; }
    float      GetDirX( void ) { return dir.x; }
    float      GetDirY( void ) { return dir.y; }

    float OneWayLaneOffset()
    {
        if ( numLeftLanes == 0 )
            return 0.5f - 0.5f * numRightLanes;
        if ( numRightLanes == 0 )
            return 0.5f - 0.5f * numLeftLanes;
        return 0.5f;
    }
};

union CConnectionFlags
{
    uint8_t flags;
    struct
    {
        uint8_t bCrossesRoad : 1;
        uint8_t bTrafficLight : 1;
    };
};

struct CTempNode
{
    CVector pos;
    float   dirX;
    float   dirY;
    int16_t link1;
    int16_t link2;
    int8_t  numLeftLanes;
    int8_t  numRightLanes;
    int8_t  linkState;
};

struct CPathInfoForObject
{
    int16_t x;
    int16_t y;
    int16_t z;
    int8_t  type;
    int8_t  next;
    int8_t  numLeftLanes;
    int8_t  numRightLanes;
    uint8_t crossing : 1;
};

/// Path find class
class CPathFind
{
  public:
    /// Calculate node coordinates, todo: describe
    void CalcNodeCoors( short x, short y, short z, int id, CVector *out );
    void PreparePathDataForType( uint8_t type, CTempNode *tempnodes,
                                 CPathInfoForObject *objectpathinfo,
                                 float maxdist, void *detachednodes,
                                 int unused );
    static void __thiscall PreparePathDataForType_Jmp(
        CPathFind *_obj, uint8_t type, CTempNode *tempnodes,
        CPathInfoForObject *objectpathinfo, float maxdist, void *detachednodes,
        int unused );

    CPathNode        m_pathNodes[NUM_PATHNODES];
    CCarPathLink     m_carPathLinks[NUM_CARPATHLINKS];
    CTreadable *     m_mapObjects[NUM_MAPOBJECTS];
    uint8_t          m_objectFlags[NUM_MAPOBJECTS];
    int16_t          m_connections[NUM_PATHCONNECTIONS];
    int16_t          m_distances[NUM_PATHCONNECTIONS];
    CConnectionFlags m_connectionFlags[NUM_PATHCONNECTIONS];
    int16_t          m_carPathConnections[NUM_PATHCONNECTIONS];

    int32_t   m_numPathNodes;
    int32_t   m_numCarPathNodes;
    int32_t   m_numPedPathNodes;
    int16_t   m_numMapObjects;
    int16_t   m_numConnections;
    int32_t   m_numCarPathLinks;
    int32_t   unk;
    uint8_t   m_numGroups[2];
    CPathNode m_searchNodes[512];

  private:
    const CPathNode &GetLinkedPathNode( CPathNode &node, int16_t idx );
};
static_assert( sizeof( CPathFind ) == 0x49bf4 );