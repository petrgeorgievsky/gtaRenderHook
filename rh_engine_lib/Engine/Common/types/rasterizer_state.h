#pragma once
#include <cstdint>
#include <memory>
namespace rh::engine {
enum class CullMode : uint8_t;
struct RasterizerState
{
    CullMode cullMode;
};
inline bool operator==( const RasterizerState &lhs, const RasterizerState &rhs ) noexcept
{
    return std::memcmp( &lhs, &rhs, sizeof( RasterizerState ) ) == 0;
}
} // namespace rh::engine
namespace std {
template<>
struct hash<rh::engine::RasterizerState>
{
public:
    size_t operator()( const rh::engine::RasterizerState &s ) const noexcept
    {
        const size_t h1 = std::hash<uint8_t>()( static_cast<uint8_t>( s.cullMode ) );
        return h1;
    }
};
} // namespace std
