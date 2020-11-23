#pragma once
#include <cstdint>
#include <filesystem>
struct RpClump;
struct RpAtomic;
struct rwFrameList;
struct rpGeometryList;

namespace rh::rw::engine
{
RpClump * RpClumpCreate() noexcept;
int32_t   RpClumpDestroy( RpClump *clump );
RpClump * RpClumpStreamRead( void *stream );
RpClump * RpClumpAddAtomic( RpClump *clump, RpAtomic *atomic ) noexcept;
RpAtomic *ClumpAtomicStreamRead( void *stream, rwFrameList *fl,
                                 rpGeometryList *gl );
bool      LoadClump( RpClump *&clump, const std::filesystem::path &dff_path );
} // namespace rh::rw::engine
