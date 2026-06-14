#pragma once
#include <Engine/Common/hash_data.hpp>

#include <array>
#include <cstdint>

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
    std::array<AttachmentBlendState, MAX_RT_COUNT> renderTargetBlendState{};
    std::array<float, 4>                           blendConstants{};
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
        return rh::engine::fast_hash( &s, sizeof( s ) );
    }
};

} // namespace std
