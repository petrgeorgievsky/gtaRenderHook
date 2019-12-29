#pragma once
struct RwFrame;
struct rwFrameList;
struct RwMatrixTag;

namespace rh::rw::engine {
RwFrame *RwFrameCreate();
void RwFrameAddChild( RwFrame *parent, RwFrame *child );
void RwFrameRemoveChild( RwFrame *child );
void RwFrameUpdateObjects( RwFrame *frame ) noexcept;
RwMatrixTag *RwFrameGetLTM( RwFrame *frame );
void _rwFrameSyncHierarchyLTM( RwFrame *frame );
rwFrameList *_rwFrameListStreamRead( void *stream, rwFrameList *frameList );
} // namespace rw_rh_engine
