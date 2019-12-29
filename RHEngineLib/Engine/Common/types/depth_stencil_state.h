#pragma once
#include <cstdint>
#include <memory>
namespace rh::engine {
enum class StencilOp : uint8_t;
enum class ComparisonFunc : uint8_t;
struct StencilOpInfo
{
    StencilOp stencilFailOp;
    StencilOp stencilDepthFailOp;
    StencilOp stencilPassOp;
    ComparisonFunc stencilFunc;
};

struct DepthStencilState
{
    bool enableDepthBuffer;
    bool enableDepthWrite;
    bool enableStencilBuffer;
    uint8_t stencilWriteMask;
    uint8_t stencilReadMask;
    ComparisonFunc depthComparisonFunc;
    StencilOpInfo frontFaceStencilOp;
    StencilOpInfo backFaceStencilOp;
};

inline bool operator==( const DepthStencilState &lhs, const DepthStencilState &rhs ) noexcept
{
    return std::memcmp( &lhs, &rhs, sizeof( DepthStencilState ) ) == 0;
}

} // namespace rh::engine

namespace std {
template<>
struct hash<rh::engine::DepthStencilState>
{
public:
    // TODO: create a better hash func
    size_t operator()( const rh::engine::DepthStencilState &s ) const noexcept
    {
        // depth stencil stuff hash
        const size_t h1 = std::hash<bool>()( s.enableDepthBuffer )
                          ^ std::hash<bool>()( s.enableDepthWrite )
                          ^ ( std::hash<bool>()( s.enableStencilBuffer ) << 1 );
        const size_t h2 = std::hash<uint8_t>()( s.stencilWriteMask ) << 2;
        const size_t h3 = std::hash<uint8_t>()( s.stencilReadMask ) << 2;
        const size_t h4 = std::hash<uint8_t>()( static_cast<uint8_t>( s.depthComparisonFunc ) )
                          << 10;

        // stencil buffer hashes
        const size_t h5
            = std::hash<uint8_t>()( static_cast<uint8_t>( s.frontFaceStencilOp.stencilFunc ) )
              << 14;
        const size_t h6
            = std::hash<uint8_t>()( static_cast<uint8_t>( s.backFaceStencilOp.stencilFunc ) ) << 14;

        const size_t h7
            = std::hash<uint8_t>()( static_cast<uint8_t>( s.frontFaceStencilOp.stencilFailOp ) )
              << 18;
        const size_t h8
            = std::hash<uint8_t>()( static_cast<uint8_t>( s.backFaceStencilOp.stencilFailOp ) )
              << 18;

        const size_t h9
            = std::hash<uint8_t>()( static_cast<uint8_t>( s.frontFaceStencilOp.stencilDepthFailOp ) )
              << 22;
        const size_t h10
            = std::hash<uint8_t>()( static_cast<uint8_t>( s.backFaceStencilOp.stencilDepthFailOp ) )
              << 22;

        const size_t h11
            = std::hash<uint8_t>()( static_cast<uint8_t>( s.frontFaceStencilOp.stencilPassOp ) )
              << 26;
        const size_t h12
            = std::hash<uint8_t>()( static_cast<uint8_t>( s.backFaceStencilOp.stencilPassOp ) )
              << 26;
        return h1 ^ h2 ^ h3 ^ h4 ^ h5 ^ h6 ^ h7 ^ h8 ^ h9 ^ h10 ^ h11 ^ h12;
    }
};
} // namespace std
