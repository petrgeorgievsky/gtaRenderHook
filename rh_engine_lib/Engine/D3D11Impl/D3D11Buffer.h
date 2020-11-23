#pragma once
#include "Engine/Common/IBuffer.h"
//
// TODO: Improve documentation for this file

// d3d11 struct forwards:
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

namespace rh::engine
{

struct D3D11BufferCreateInfo : BufferCreateInfo
{
    // Dependencies...
    ID3D11Device *       mDevice;
    ID3D11DeviceContext *mImmediateContext;
};

/**
 * @class D3D11Buffer
 * @brief Base D3D hardware buffer class.
 *
 * This class represents GPU memory buffer.
 */
class D3D11Buffer : public IBuffer
{
  public:
    /**
     * @brief Initializes D3D hardware buffer.
     */
    D3D11Buffer( const D3D11BufferCreateInfo & );

    /**
     * @brief Releases memory held by hardware buffer.
     *
     */
    ~D3D11Buffer() override;

    /**
     * @brief Updates data inside buffer.
     *
     */
    void Update( const void *data, uint32_t size ) override;
    void Update( const void *data, uint32_t size, uint32_t offset ) override;

    ID3D11Buffer *GetImpl() { return m_pBuffer; }
    void *        Lock() override;
    void          Unlock() override;

  protected:
    ID3D11Buffer *       m_pBuffer = nullptr;
    ID3D11DeviceContext *mImmediateContext;
};
}; // namespace rh::engine
