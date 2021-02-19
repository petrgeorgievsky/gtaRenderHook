//
// Created by peter on 19.02.2021.
//
#pragma once
#include <common_headers.h>
#include <span>
namespace rh::rw::engine
{

class RpGeometryInterface
{
  public:
    virtual ~RpGeometryInterface() = default;
    void          Init( void *geometry ) { m_pGeometryImpl = geometry; }
    virtual void *GetThis() { return m_pGeometryImpl; }

    virtual void *            GetResEntry()                    = 0;
    virtual RwResEntry *&     GetResEntryRef()                 = 0;
    virtual int32_t           GetVertexCount()                 = 0;
    virtual int32_t           GetTriangleCount()               = 0;
    virtual RpTriangle *      GetTrianglePtr()                 = 0;
    virtual uint32_t          GetFlags()                       = 0;
    virtual int32_t           GetMorphTargetCount()            = 0;
    virtual RpMeshHeader *    GetMeshHeader() const            = 0;
    virtual RpMorphTarget *   GetMorphTarget( uint32_t id )    = 0;
    virtual RwTexCoords *     GetTexCoordSetPtr( uint32_t id ) = 0;
    virtual RwRGBA *          GetVertexColorPtr()              = 0;
    virtual void              Unlock()                         = 0;
    virtual std::span<RpMesh> GetMeshList() const              = 0;

  protected:
    void *m_pGeometryImpl = nullptr;
};
} // namespace rh::rw::engine