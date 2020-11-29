//
// Created by peter on 30.10.2020.
//

#pragma once

#include "../game/Entity.h"
#include "../game/Vector.h"
#include <cmath>
#include <common_headers.h>
#include <cstdint>
#include <span>

class CVector2D
{
  public:
    float x, y;
};

class CReference
{
};

class CBuilding : public Entity
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
    Vector     pos;
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

    Vector &GetPosition( void ) { return pos; }
    void    SetPosition( const Vector &p ) { pos = p; }
    float   GetX( void ) { return pos.x; }
    float   GetY( void ) { return pos.y; }
    float   GetZ( void ) { return pos.z; }

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
    Vector  pos;
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
    void CalcNodeCoors( short x, short y, short z, int id, Vector *out );
    void PreparePathDataForType( uint8_t type, CTempNode *tempnodes,
                                 CPathInfoForObject *objectpathinfo,
                                 float maxdist, void *detachednodes,
                                 int unused );
    static void __thiscall PreparePathDataForType_Jmp(
        CPathFind *_obj, uint8_t type, CTempNode *tempnodes,
        CPathInfoForObject *objectpathinfo, float maxdist, void *detachednodes,
        int unused );

    static void Patch();

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