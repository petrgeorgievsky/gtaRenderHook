
bool TraverseBLAS(Ray r, TLASLeaf tlasLeaf, out PayLoad payLoad, float maxDist, bool any_hit)
{
    r.origin = mul(tlasLeaf.world_transform,r.origin);
    r.origin.a = 1;
    r.direction = mul(tlasLeaf.world_transform, r.direction);
    r.direction.a = 0;
    
    CalculateRayIntrinsics(r);

    INIT_PAYLOAD_MISS(payLoad, r, maxDist);

    PayLoad payLoadCurr;
    payLoadCurr.hitDist = payLoad.hitDist;
    
    uint todoOffset = 0, nodeNum = 0;
    int todo[64];
    bool hit = false;
    
    bool dirIsNeg[3] = { r.invdirection.x < 0, r.invdirection.y < 0, r.invdirection.z < 0 };
    while (true)
    {
        BLASLeaf currentLeaf = bottomLevelAS[nodeNum + tlasLeaf.blasBVHOffset];
        if (RayAABBIntersection(r, currentLeaf.aabb, payLoadCurr))
        {
            if (currentLeaf.primitiveCount > 0)
            {
                for (uint i = currentLeaf.primitivesOffset;
                         i < currentLeaf.primitivesOffset + currentLeaf.primitiveCount; 
                         i++)
                {
                    if (RayTriangleIntersection( r, triangleBuffer[i + tlasLeaf.primitiveOffset], 
                                                 payLoadCurr, tlasLeaf.vertexOffset ) )
                    {
                        if ( payLoadCurr.hitDist < payLoad.hitDist && 
                             payLoadCurr.hitDist > 0.0f )
                        {
                            payLoad = payLoadCurr;
                        }
                        hit = true;
                        if(any_hit)
                            break;
                    }
                }
                if (todoOffset == 0) break;
                nodeNum = todo[--todoOffset];
            }
            else
            {
                if(dirIsNeg[currentLeaf.axis])
                {
                    todo[todoOffset++] = nodeNum + 1;
                    nodeNum = currentLeaf.secondChildOffset;
                }
                else{
                    todo[todoOffset++] = currentLeaf.secondChildOffset;
                    nodeNum = nodeNum + 1;
                }
            }
        }
        else
        {
            if (todoOffset == 0) break;
            nodeNum = todo[--todoOffset];
        }
    }

    if(hit)
        payLoad.normal = mul(float4(payLoad.normal.xyz,0),tlasLeaf.world_transform);
    
    return hit;
}

bool TraverseSceneTLAS(Ray r, out PayLoad payLoad, float maxDist, bool any_hit)
{
    INIT_PAYLOAD_MISS(payLoad, r, maxDist);

    PayLoad payLoadCurr;
    payLoadCurr.hitDist = payLoad.hitDist;
    

    int currentDepth = 0;
    int maxDepth = 1000;

    uint todoOffset = 0, nodeNum = 0;
    int todo[64];
    bool hit = false;
    
    bool dirIsNeg[3] = { r.invdirection.x < 0, r.invdirection.y < 0, r.invdirection.z < 0 };
    while (true)
    {
        TLASLeaf currentLeaf = topLevelAS[nodeNum];
        if (RayAABBIntersection(r, currentLeaf.aabb, payLoadCurr) && payLoadCurr.hitDist<maxDist)
        {
            if (currentLeaf.lowLevel.x > 0)
            {
                if (TraverseBLAS(r, currentLeaf, payLoadCurr, maxDist,any_hit))
                {
                    if (payLoadCurr.hitDist < payLoad.hitDist && payLoadCurr.hitDist > 0.0f)
                    {
                        payLoad = payLoadCurr;
                    }
                    hit = true;
                    if(any_hit)
                        break;
                }
                if (todoOffset == 0) break;
                nodeNum = todo[--todoOffset];
            }
            else
            {
                if(dirIsNeg[currentLeaf.axis])
                {
                    todo[todoOffset++] = nodeNum + 1;
                    nodeNum = currentLeaf.secondChildOffset;
                }
                else{
                    todo[todoOffset++] = currentLeaf.secondChildOffset;
                    nodeNum = nodeNum + 1;
                }
            }
        }
        else
        {
            if (todoOffset == 0) break;
            nodeNum = todo[--todoOffset];
        }
    }
    return hit;
}