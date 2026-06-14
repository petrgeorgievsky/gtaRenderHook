#pragma once
#include <Engine/Common/hash_data.hpp>

#include <cstdint>
#include <memory>

namespace rh::engine
{

enum class StencilOp : uint8_t;
enum class ComparisonFunc : uint8_t;

struct StencilOpInfo
{
    StencilOp      stencilFailOp;
    StencilOp      stencilDepthFailOp;
    StencilOp      stencilPassOp;
    ComparisonFunc stencilFunc;
    auto           operator<=>( const StencilOpInfo & ) const = default;
};

#pragma pack( push, 1 )
struct DepthStencilState
{
    bool           enableDepthBuffer;
    bool           enableDepthWrite;
    bool           enableStencilBuffer;
    uint8_t        stencilWriteMask;
    uint8_t        stencilReadMask;
    ComparisonFunc depthComparisonFunc;
    StencilOpInfo  frontFaceStencilOp;
    StencilOpInfo  backFaceStencilOp;
    auto           operator<=>( const DepthStencilState & ) const = default;
};
#pragma pack( pop )

} // namespace rh::engine

namespace std
{

template <> struct hash<rh::engine::DepthStencilState>
{
  public:
    size_t operator()( const rh::engine::DepthStencilState &s ) const noexcept
    {
        return rh::engine::fast_hash( &s, sizeof( s ) );
    }
};

} // namespace std
