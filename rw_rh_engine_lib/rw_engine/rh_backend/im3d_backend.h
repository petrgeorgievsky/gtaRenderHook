//
// Created by peter on 19.11.2020.
//

#pragma once
#include <vector>

#include <common_headers.h>
#include <ipc/MemoryWriter.h>

namespace rh::rw::engine
{

void *  Im3DTransform( void *pVerts, uint32_t numVerts, RwMatrix *ltm,
                       uint32_t flags );
int32_t Im3DRenderIndexedPrimitive( RwPrimitiveType primType, uint16_t *indices,
                                    int32_t numIndices );
int32_t Im3DRenderPrimitive( RwPrimitiveType primType );
int32_t Im3DRenderLine( int32_t vert1, int32_t vert2 );
int32_t Im3DRenderTriangle( int32_t vert1, int32_t vert2, int32_t vert3 );
int32_t Im3DEnd();

} // namespace rh::rw::engine