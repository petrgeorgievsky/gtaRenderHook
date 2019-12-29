#pragma once
#include "D3D11Common.h"
#include "Engine/Common/IGPUResource.h"
#include "Shaders\D3D11VertexShader.h"
#include <d3d11_3.h>
#include <vector>
namespace rh::engine {

class D3D11InputLayout : public IGPUResource
{
public:
    D3D11InputLayout( ID3D11Device *device,
                      const std::vector<D3D11_INPUT_ELEMENT_DESC> &elements,
                      D3D11VertexShader *shader );
    ~D3D11InputLayout();
    void Set( ID3D11DeviceContext *context );

private:
    ID3D11InputLayout *m_inputLayout = nullptr;
};
}; // namespace rh::engine
