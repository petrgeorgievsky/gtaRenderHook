//
// Created by peter on 30.10.2020.
//

#include "car_path_bug_fix.h"
#include "../call_redirection_util.h"
#include <MemoryInjectionUtils/InjectorHelpers.h>

void CPathFind::CalcNodeCoors( short x, short y, short z, int id, Vector *out )
{
    InMemoryThisCall<void>( GetAddressByGame( 0x429560, 0x429560, 0x429560 ),
                            this, x, y, z, id, out );
}

enum
{
    NodeTypeExtern = 1,
    NodeTypeIntern = 2,

    UseInRoadBlock = 1,
    ObjectEastWest = 2,
};

enum
{
    PATH_CAR = 0,
    PATH_PED = 1,
};

int32_t TempListLength;

void CPathFind::PreparePathDataForType( uint8_t type, CTempNode *tempnodes,
                                        CPathInfoForObject *objectpathinfo,
                                        float maxdist, void *detachednodes,
                                        int unused )
{
    static Vector CoorsXFormed{};
    int           oldNumPathNodes, oldNumLinks;
    oldNumPathNodes  = m_numPathNodes;
    oldNumLinks      = m_numConnections;
    auto OBJECTINDEX = [this]( auto n ) -> auto &
    {
        return ( m_pathNodes[( n )].objectIndex );
    };

    for ( auto &obj : std::span( m_mapObjects, m_numMapObjects ) )
        for ( auto &node_idx : obj->m_nodeIndices[type] )
            node_idx = -1;

    // Calculate internal nodes, store them and connect them to defining object
    for ( auto i = 0; i < m_numMapObjects; i++ )
    {
        auto obj   = m_mapObjects[i];
        auto start = 12 * obj->mModelIndex;
        for ( auto j = 0; j < 12; j++ )
        {
            if ( objectpathinfo[start + j].type != NodeTypeIntern )
                continue;
            auto &node = m_pathNodes[m_numPathNodes];
            CalcNodeCoors( objectpathinfo[start + j].x,
                           objectpathinfo[start + j].y,
                           objectpathinfo[start + j].z, i, &CoorsXFormed );
            node.SetPosition( CoorsXFormed );
            node.objectIndex            = i;
            node.unkBits                = 1;
            obj->m_nodeIndices[type][j] = m_numPathNodes;
            m_numPathNodes++;
        }
    }

    // Insert external nodes into TempList
    TempListLength = 0;
    for ( auto i = 0; i < m_numMapObjects; i++ )
    {
        auto obj   = m_mapObjects[i];
        auto start = 12 * obj->mModelIndex;
        for ( auto j = 0; j < 12; j++ )
        {
            const auto &path_info = objectpathinfo[start + j];
            if ( path_info.type != NodeTypeExtern )
                continue;
            CalcNodeCoors( path_info.x, path_info.y, path_info.z, i,
                           &CoorsXFormed );

            // find closest unconnected node
            auto nearestId   = -1;
            auto nearestDist = maxdist;
            for ( auto k = 0; k < TempListLength; k++ )
            {
                const auto &node = tempnodes[k];
                if ( node.linkState != 1 )
                    continue;
                auto dx = node.pos.x - CoorsXFormed.x;
                auto dy = node.pos.y - CoorsXFormed.y;
                if ( abs( dx ) < nearestDist && abs( dy ) < nearestDist )
                {
                    nearestDist = max( abs( dx ), abs( dy ) );
                    nearestId   = k;
                }
            }

            if ( nearestId < 0 )
            {
                // None found, add this one to temp list
                auto &new_node = tempnodes[TempListLength];
                new_node.pos   = CoorsXFormed;

                auto next = path_info.next;
                if ( next < 0 )
                {
                    // no link from this node, find link to this node
                    next = 0;
                    for ( auto k = start; j != objectpathinfo[k].next; k++ )
                        next++;
                }
                // link to connecting internal node
                new_node.link1 = obj->m_nodeIndices[type][next];
                if ( type == PATH_CAR )
                {
                    new_node.numLeftLanes  = path_info.numLeftLanes;
                    new_node.numRightLanes = path_info.numRightLanes;
                }
                new_node.linkState = 1;
                TempListLength++;
            }
            else
            {
                // Found nearest, connect it to our neighbour
                auto &nearest_node = tempnodes[nearestId];
                auto  next         = path_info.next;
                if ( next < 0 )
                {
                    // no link from this node, find link to this node
                    next = 0;
                    for ( auto k = start; j != objectpathinfo[k].next; k++ )
                        next++;
                }
                nearest_node.link2     = obj->m_nodeIndices[type][next];
                nearest_node.linkState = 2;

                // collapse this node with nearest we found
                auto dx = m_pathNodes[nearest_node.link1].GetX() -
                          m_pathNodes[nearest_node.link2].GetX();
                auto dy = m_pathNodes[nearest_node.link1].GetY() -
                          m_pathNodes[nearest_node.link2].GetY();
                nearest_node.pos =
                    Vector{ ( nearest_node.pos.x + CoorsXFormed.x ) * 0.5f,
                            ( nearest_node.pos.y + CoorsXFormed.y ) * 0.5f,
                            ( nearest_node.pos.z + CoorsXFormed.z ) * 0.5f };
                auto mag          = sqrt( dx * dx + dy * dy );
                nearest_node.dirX = dx / mag;
                nearest_node.dirY = dy / mag;
                // do something when number of lanes doesn't agree
                if ( type == PATH_CAR )
                    if ( nearest_node.numLeftLanes != 0 &&
                         nearest_node.numRightLanes != 0 &&
                         ( path_info.numLeftLanes == 0 ||
                           path_info.numRightLanes == 0 ) )
                    {
                        // why switch left and right here?
                        nearest_node.numLeftLanes  = path_info.numRightLanes;
                        nearest_node.numRightLanes = path_info.numLeftLanes;
                    }
            }
        }
    }

    // Loop through previously added internal nodes and link them
    for ( auto i = oldNumPathNodes; i < m_numPathNodes; i++ )
    {
        // Init link
        auto &node     = m_pathNodes[i];
        node.numLinks  = 0;
        node.firstLink = m_numConnections;

        // See if node connects to external nodes
        for ( auto j = 0; j < TempListLength; j++ )
        {
            auto &temp_node = tempnodes[j];
            if ( temp_node.linkState != 2 )
                continue;
            auto &new_connection   = m_connections[m_numConnections];
            auto &new_distance     = m_distances[m_numConnections];
            auto &new_conect_flags = m_connectionFlags[m_numConnections];

            // Add link to other side of the external
            // NB this clears the flags in MIAMI
            if ( temp_node.link1 == i )
                new_connection = temp_node.link2;
            else if ( temp_node.link2 == i )
                new_connection = temp_node.link1;
            else
                continue;

            auto dist = ( node.GetPosition() -
                          m_pathNodes[new_connection].GetPosition() )
                            .Magnitude();
            new_distance           = dist;
            new_conect_flags.flags = 0;

            if ( type == PATH_CAR )
            {
                auto &new_car_connection =
                    m_carPathConnections[m_numConnections];

                // IMPROVE: use a goto here
                // Find existing car path link
                auto k = 0;
                for ( ; k < m_numCarPathLinks; k++ )
                {
                    const auto &path_link = m_carPathLinks[k];
                    if ( path_link.dir.x == temp_node.dirX &&
                         path_link.dir.y == temp_node.dirY &&
                         path_link.pos.x == temp_node.pos.x &&
                         path_link.pos.y == temp_node.pos.y )
                    {
                        new_car_connection = k;
                        k                  = m_numCarPathLinks;
                    }
                }
                // k is m_numCarPathLinks+1 if we found one
                if ( k == m_numCarPathLinks )
                {
                    auto &new_car_link = m_carPathLinks[m_numCarPathLinks];
                    new_car_link.dir.x = temp_node.dirX;
                    new_car_link.dir.y = temp_node.dirY;
                    new_car_link.pos.x = temp_node.pos.x;
                    new_car_link.pos.y = temp_node.pos.y;
                    new_car_link.pathNodeIndex    = i;
                    new_car_link.numLeftLanes     = temp_node.numLeftLanes;
                    new_car_link.numRightLanes    = temp_node.numRightLanes;
                    new_car_link.trafficLightType = 0;
                    assert( m_numCarPathLinks <= NUM_CARPATHLINKS );
                    new_car_connection = m_numCarPathLinks++;
                }
            }

            node.numLinks++;
            m_numConnections++;
        }

        // Find i inside path segment
        auto iseg = 0;
        for ( auto j = max( oldNumPathNodes, i - 12 ); j < i; j++ )
            if ( OBJECTINDEX( j ) == OBJECTINDEX( i ) )
                iseg++;

        auto istart = 12 * m_mapObjects[node.objectIndex]->mModelIndex;
        // Add links to other internal nodes
        for ( auto j = max( oldNumPathNodes, i - 12 );
              j < min( m_numPathNodes, i + 12 ); j++ )
        {
            if ( OBJECTINDEX( i ) != OBJECTINDEX( j ) || i == j )
                continue;
            // N.B.: in every path segment, the externals have to be at the end
            auto jseg = j - i + iseg;

            auto jstart =
                12 * m_mapObjects[m_pathNodes[j].objectIndex]->mModelIndex;
            if ( objectpathinfo[istart + iseg].next == jseg ||
                 objectpathinfo[jstart + jseg].next == iseg )
            {
                // Found a link between i and jConnectionSetCrossesRoad
                // NB this clears the flags in MIAMI
                m_connections[m_numConnections] = j;
                auto dist =
                    ( node.GetPosition() - m_pathNodes[j].GetPosition() )
                        .Magnitude();
                m_distances[m_numConnections] = dist;

                if ( type == PATH_CAR )
                {
                    auto posx = ( node.GetX() + m_pathNodes[j].GetX() ) * 0.5f;
                    auto posy = ( node.GetY() + m_pathNodes[j].GetY() ) * 0.5f;
                    auto dx   = m_pathNodes[j].GetX() - node.GetX();
                    auto dy   = m_pathNodes[j].GetY() - node.GetY();
                    auto mag  = sqrt( dx * dx + dy * dy );
                    dx /= mag;
                    dy /= mag;
                    if ( i < j )
                    {
                        dx = -dx;
                        dy = -dy;
                    }
                    // IMPROVE: use a goto here
                    // Find existing car path link
                    auto k = 0;
                    for ( ; k < m_numCarPathLinks; k++ )
                    {
                        if ( m_carPathLinks[k].dir.x == dx &&
                             m_carPathLinks[k].dir.y == dy &&
                             m_carPathLinks[k].pos.x == posx &&
                             m_carPathLinks[k].pos.y == posy )
                        {
                            m_carPathConnections[m_numConnections] = k;
                            k = m_numCarPathLinks;
                        }
                    }
                    // k is m_numCarPathLinks+1 if we found one
                    if ( k == m_numCarPathLinks )
                    {
                        m_carPathLinks[m_numCarPathLinks].dir.x         = dx;
                        m_carPathLinks[m_numCarPathLinks].dir.y         = dy;
                        m_carPathLinks[m_numCarPathLinks].pos.x         = posx;
                        m_carPathLinks[m_numCarPathLinks].pos.y         = posy;
                        m_carPathLinks[m_numCarPathLinks].pathNodeIndex = i;
                        m_carPathLinks[m_numCarPathLinks].numLeftLanes  = -1;
                        m_carPathLinks[m_numCarPathLinks].numRightLanes = -1;
                        m_carPathLinks[m_numCarPathLinks].trafficLightType = 0;
                        assert( m_numCarPathLinks <= NUM_CARPATHLINKS );
                        m_carPathConnections[m_numConnections] =
                            m_numCarPathLinks++;
                    }
                }
                else
                {
                    // Crosses road
                    if ( objectpathinfo[istart + iseg].next == jseg &&
                             objectpathinfo[istart + iseg].crossing ||
                         objectpathinfo[jstart + jseg].next == iseg &&
                             objectpathinfo[jstart + jseg].crossing )
                        m_connectionFlags[m_numConnections].bCrossesRoad = true;
                    else
                        m_connectionFlags[m_numConnections].bCrossesRoad =
                            false;
                }

                node.numLinks++;
                m_numConnections++;
            }
        }
    }

    if ( type == PATH_CAR )
    {
        auto done = false;
        // Set number of lanes for all nodes somehow
        // very strange code
        for ( auto k = 0; !done && k < 10; k++ )
        {
            done = true;
            for ( auto i = 0; i < m_numPathNodes; i++ )
            {
                const auto &node = m_pathNodes[i];
                if ( node.numLinks != 2 )
                    continue;
                auto  l1          = m_carPathConnections[node.firstLink];
                auto  l2          = m_carPathConnections[node.firstLink + 1];
                auto &first_link  = m_carPathLinks[l1];
                auto &second_link = m_carPathLinks[l2];

                auto swap_lanes = []( CCarPathLink &a, CCarPathLink &b,
                                      int16_t i ) {
                    if ( b.pathNodeIndex == i )
                    {
                        // why switch left and right here?
                        a.numLeftLanes  = b.numRightLanes;
                        a.numRightLanes = b.numLeftLanes;
                    }
                    else
                    {
                        a.numLeftLanes  = b.numLeftLanes;
                        a.numRightLanes = b.numRightLanes;
                    }
                    a.pathNodeIndex = i;
                };

                if ( first_link.numLeftLanes != -1 &&
                     second_link.numLeftLanes != -1 )
                    continue;

                done = false;
                if ( first_link.numLeftLanes == -1 &&
                     second_link.numLeftLanes != -1 )
                    swap_lanes( first_link, second_link, i );
                else if ( first_link.numLeftLanes != -1 &&
                          second_link.numLeftLanes == -1 )
                    swap_lanes( second_link, first_link, i );
            }
        }

        // Fall back to default values for number of lanes
        for ( auto &path_node : std::span( m_pathNodes, m_numPathNodes ) )
        {
            for ( auto j = 0; j < path_node.numLinks; j++ )
            {
                auto  k = m_carPathConnections[path_node.firstLink + j];
                auto &car_path_link = m_carPathLinks[k];
                if ( car_path_link.numLeftLanes < 0 )
                    car_path_link.numLeftLanes = 1;
                if ( car_path_link.numRightLanes < 0 )
                    car_path_link.numRightLanes = 1;
            }
        }
    }

    // Set flags for car nodes
    if ( type == PATH_CAR )
    {
        // iterate over all nodes and mark all dead ends(not the most optimal
        // algorithm)
        auto can_cont = true;
        while ( can_cont )
        {
            can_cont = false;
            for ( auto &path_node : std::span( m_pathNodes, m_numPathNodes ) )
            {
                path_node.bDisabled      = false;
                path_node.bBetweenLevels = false;
                if ( path_node.bDeadEnd )
                    continue;
                auto non_dead_end_count = 0;
                // TODO: add linked node iter?
                for ( auto j = 0; j < path_node.numLinks; j++ )
                    if ( !GetLinkedPathNode( path_node, j ).bDeadEnd )
                        non_dead_end_count++;
                if ( non_dead_end_count >= 2 )
                    continue;
                path_node.bDeadEnd = true;
                can_cont           = true;
            }
        }
    }

    // Remove isolated ped nodes
    if ( type == PATH_PED )
        for ( auto i = oldNumPathNodes; i < m_numPathNodes; i++ )
        {
            if ( m_pathNodes[i].numLinks != 0 )
                continue;

            // Remove node
            for ( auto j = i; j < m_numPathNodes - 1; j++ )
                m_pathNodes[j] = m_pathNodes[j + 1];

            // Fix links
            for ( auto j = oldNumLinks; j < m_numConnections; j++ )
            {
                int node = m_connections[j];
                if ( node >= i )
                    m_connections[j] = node - 1;
            }

            // Also in treadables
            for ( auto j = 0; j < m_numMapObjects; j++ )
                for ( auto k = 0; k < 12; k++ )
                {
                    if ( m_mapObjects[j]->m_nodeIndices[PATH_PED][k] == i )
                    {
                        // remove this one
                        for ( auto l = k; l < 12 - 1; l++ )
                            m_mapObjects[j]->m_nodeIndices[PATH_PED][l] =
                                m_mapObjects[j]->m_nodeIndices[PATH_PED][l + 1];
                        m_mapObjects[j]->m_nodeIndices[PATH_PED][11] = -1;
                    }
                    else if ( m_mapObjects[j]->m_nodeIndices[PATH_PED][k] > i )
                        m_mapObjects[j]->m_nodeIndices[PATH_PED][k]--;
                }

            i--;
            m_numPathNodes--;
        }
}

const CPathNode &CPathFind::GetLinkedPathNode( CPathNode &node, int16_t idx )
{
    return m_pathNodes[m_connections[node.firstLink + idx]];
}

void __thiscall CPathFind::PreparePathDataForType_Jmp(
    CPathFind *_obj, uint8_t type, CTempNode *tempnodes,
    CPathInfoForObject *objectpathinfo, float maxdist, void *detachednodes,
    int unused )
{
    _obj->PreparePathDataForType( type, tempnodes, objectpathinfo, maxdist,
                                  detachednodes, unused );
}

void CPathFind::Patch()
{
    // "Fix" car paths, ugly bug appears for no reason that trashes all
    // memory for some reason, for now replace this function with slightly
    // modified version from re3
    RedirectCall(
        GetAddressByGame( 0x4298F7, 0x4298F7, 0x4298F7 ),
        reinterpret_cast<void *>( CPathFind::PreparePathDataForType_Jmp ) );
}
