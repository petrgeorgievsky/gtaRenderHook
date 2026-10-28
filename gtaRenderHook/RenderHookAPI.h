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
    float                     view[16];       // this frame's view matrix (row vectors, as RwD3D9D3D9ViewTransform)
    float                     projection[16]; // this frame's projection without the jitter (as RwD3D9D3D9ProjTransform)
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
#ifdef GTARENDERHOOK_EXPORTS
#define RENDERHOOK_API extern "C" __declspec( dllexport )
#else
#define RENDERHOOK_API extern "C" __declspec( dllimport )
#endif

RENDERHOOK_API void __cdecl RHSetTemporalAAHandler( RHTemporalAAHandler handler );

/*!
    Pass of a building draw (RHBuildingDraw::pass).
*/
enum RHDrawPass
{
    RH_PASS_FORWARD = 0, // forward shading (also reflections)
    RH_PASS_GBUFFER = 1, // deferred G-buffer pass
    RH_PASS_SHADOW  = 2, // sun shadow map
    RH_PASS_BLENDED = 3, // alpha-blended pass after the deferred passes (queued alpha meshes)
    RH_PASS_OTHER   = 4  // anything else (voxelization)
};

/*!
    One mesh of the building pipelines (CustomBuilding and CustomBuildingDN), about to be drawn or queued for the
    blended pass.
*/
struct RHBuildingDraw
{
    unsigned int         size;          // sizeof( RHBuildingDraw ), for future additions
    ID3D11DeviceContext *context;       // RenderHook's immediate context, e.g. to set extra constant buffers
    void *               atomic;        // RpAtomic * the mesh belongs to
    const float *        worldMatrix;   // RwMatrix it is drawn with (16 floats, position at [12], [13], [14])
    int                  meshIndex;     // mesh of the atomic's geometry
    int                  pass;          // RHDrawPass
    int                  dualPassEdges; // 1: blended pass of a dual-pass mesh, which draws only its soft edges
};

/*!
    Flags an RHBuildingDrawCallback can return.
    RH_DRAW_VERTEX_ALPHA_IS_NOT_OPACITY: the mesh's prelight alpha holds other data (e.g. a wind weight), so it does
    not make the mesh transparent; it can then use dual pass (UseDualPassForAlphaObjects) like any cut-out mesh.
*/
const unsigned int RH_DRAW_VERTEX_ALPHA_IS_NOT_OPACITY = 1;

typedef unsigned int( __cdecl *RHBuildingDrawCallback )( const RHBuildingDraw *draw );

/*!
    Registers a callback called before each building mesh is drawn or queued, in every pass, or removes it with
    nullptr. Constant buffers it binds stay bound for that draw (RenderHook itself uses slots b0-b11).
*/
RENDERHOOK_API void __cdecl RHSetBuildingDrawCallback( RHBuildingDrawCallback callback );
