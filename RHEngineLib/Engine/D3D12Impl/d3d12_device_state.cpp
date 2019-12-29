#include "d3d12_device_state.h"
#include "../D3D11Impl/D3D11Common.h"

rh::engine::D3D12DeviceState::D3D12DeviceState()
{
    IDXGIAdapter *adapterPtr = nullptr;

    // dxgi factory initialization
    if ( !CALL_D3D_API( CreateDXGIFactory( __uuidof( IDXGIFactory ),
                                           reinterpret_cast<void **>( &m_pdxgiFactory ) ),
                        TEXT( "Failed to create DXGI Factory." ) ) )
        return;

    // Init adapter list.
    for ( UINT i = 0; m_pdxgiFactory->EnumAdapters( i, &adapterPtr ) != DXGI_ERROR_NOT_FOUND; ++i )
        m_vAdapters.push_back( adapterPtr );

    SetCurrentAdapter( 0 );
    SetCurrentOutput( 0 );
}

rh::engine::D3D12DeviceState::~D3D12DeviceState()
{
    rh::debug::DebugLogger::Log( "D3D12DeviceState destructor..." );

    for ( auto output : m_vOutputs ) {
        output->Release();
        output = nullptr;
    }

    for ( auto adapter : m_vAdapters ) {
        adapter->Release();
        adapter = nullptr;
    }
}

bool rh::engine::D3D12DeviceState::Init()
{
    return false;
}
