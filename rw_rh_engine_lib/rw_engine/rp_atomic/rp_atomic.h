#pragma once
#include <cstdint>
struct RpAtomic;
struct RwFrame;
struct RpGeometry;
namespace rh::rw::engine {
RpAtomic *RpAtomicCreate();

RpAtomic *RpAtomicSetFrame( RpAtomic *atomic, RwFrame *frame );

RpAtomic *RpAtomicSetGeometry( RpAtomic *atomic, RpGeometry *geometry, uint32_t flags );

void RpAtomicDestroy( RpAtomic *atomic );
} // namespace rw_rh_engine
