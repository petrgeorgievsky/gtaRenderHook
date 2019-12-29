#pragma once
#include "../ImageBuffers/ImageBuffer.h"
#include "Engine/Common/types/string_typedefs.h"
#include <d3d11_3.h>
// TODO: Improve documentation for this file

namespace rh::engine
{

struct D3D11BufferInfo
{
    unsigned int    size;
    D3D11_USAGE     usage;
    D3D11_BIND_FLAG bindingFlags;
    unsigned int    cpuAccessFlags;
};

/**
 * @class D3D11Buffer
 * @brief Base D3D hardware buffer class.
 *
 * This class represents GPU memory buffer.
 */
class D3D11Buffer : public D3D11BindableResource
{
  public:
    /**
     * @brief Initializes D3D hardware buffer.
     *
     *
     * @param device
     * @param info
     * @param miscFlags
     * @param elementSize
     * @param initialData
     *
     * Allocates hardware buffer of some non-negative size(in bytes) with custom
     * usage, binding, cpu access, flags, element structure size(in bytes) and
     * initial data if provided.
     */
    D3D11Buffer( ID3D11Device *device, const D3D11BufferInfo &info,
                 unsigned int miscFlags = 0, unsigned int elementSize = 0,
                 const D3D11_SUBRESOURCE_DATA *initialData = nullptr );

    /**
     * @brief Releases memory held by hardware buffer.
     *
     */
    ~D3D11Buffer() override;

    /**
     * @brief Updates data inside buffer.
     *
     * @param context
     * @param data
     * @param size
     *
     * @note If size is negative, than all buffer data will be updated,
     * otherwise only part of it.
     */
    virtual void Update( ID3D11DeviceContext *context, const void *data,
                         int size = -1 );

    /**
     * @brief Sets debugging info for this buffer
     *
     * @param name
     */
    void SetDebugName( const String &name );

    ID3D11Buffer *GetBuffer() noexcept { return m_pBuffer; }

  protected:
    ID3D11Buffer *m_pBuffer = nullptr;
    unsigned int  m_uiSize;
    String        m_sDebugName;
};
}; // namespace rh::engine
