// Functions _gtaRenderHook.asi exports for other plugins. Look them up with
// GetProcAddress( GetModuleHandleA( "_gtaRenderHook.asi" ), "<name>" ); a missing export means an older build.
#pragma once
#include <d3d11.h>

/*!
    One frame of temporal anti-aliasing, as RenderHook would do it (after the deferred and forward passes, before
    tonemapping).
*/
struct RHTemporalAAFrame
{
    unsigned int              size;     // sizeof( RHTemporalAAFrame ), for future additions
    ID3D11DeviceContext *     context;  // RenderHook's immediate context
    ID3D11Texture2D *         input;    // HDR colour of this frame, rendered with the jitter below
    ID3D11Texture2D *         output;   // write the anti-aliased colour here (same size and format as input)
    ID3D11ShaderResourceView *gbuffer1; // G-buffer 1: normals (xy) and view-space depth (z/w), as TAA reads it
    float                     jitterX;  // offset added to this frame's projection _31 (x) and _32 (y), in NDC
    float                     jitterY;
};

/*!
    Called once per frame in place of RenderHook's temporal AA. Return true when output was written; false lets
    RenderHook run its own TAA for this frame.
*/
typedef bool( __cdecl *RHTemporalAAHandler )( const RHTemporalAAFrame *frame );

/*!
    Registers a handler that replaces RenderHook's temporal AA (for example NVIDIA DLAA), or removes it with nullptr.
    The TAA block's EnableTAA must stay on: the handler runs where TAA runs, and the projection is jittered as for
    TAA.
*/
extern "C" __declspec( dllexport ) void __cdecl RHSetTemporalAAHandler( RHTemporalAAHandler handler );
