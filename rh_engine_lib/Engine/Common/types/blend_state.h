#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace rh::engine
{
enum class BlendOp : uint8_t;
enum class BlendCombineOp : uint8_t;
struct AttachmentBlendState
{
    BlendOp        srcBlend;
    BlendOp        destBlend;
    BlendCombineOp blendCombineOp;
    BlendOp        srcBlendAlpha;
    BlendOp        destBlendAlpha;
    BlendCombineOp blendAlphaCombineOp;
    bool           enableBlending;
};
static constexpr size_t MAX_RT_COUNT = 8;
struct BlendState
{
    std::vector<AttachmentBlendState> renderTargetBlendState;
    std::array<float, 4>              blendConstants;
};

inline bool operator==( const BlendState &lhs, const BlendState &rhs ) noexcept
{
    return std::memcmp( &lhs, &rhs, sizeof( BlendState ) ) == 0;
}
} // namespace rh::engine
namespace std
{

template <> struct hash<rh::engine::BlendState>
{
  public:
    size_t operator()( const rh::engine::BlendState &s ) const noexcept
    {
        const size_t h1 =
            std::hash<bool>()( s.renderTargetBlendState[0].enableBlending );
        const size_t h2 = std::hash<int>()( static_cast<int>(
                              s.renderTargetBlendState[0].srcBlend ) )
                          << 1;
        const size_t h3 = std::hash<int>()( static_cast<int>(
                              s.renderTargetBlendState[0].destBlend ) )
                          << 1;
        return h1 ^ h2 ^ h3;
    }
};
} // namespace std
