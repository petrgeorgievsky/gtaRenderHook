#pragma once
#include <game_sa\CVehicle.h>
#include <game_sa\CVector.h>
class CGTASAHooks
{
public:
    static void Patch();

    /*
        For some reason R* used some sort of cache system for d3d rasters, currently we just ignore it
        and replace it with empty function.
        TODO: Implement it the way original game, or decide to throw it away and replace by some generic patch function
    */
    static void InitD3DCacheSystem();

    /*
        Shutdown function for R* cache system
    */
    static void ShutdownD3DCacheSystem();

    /*
        Should return best refresh rate for selected window characteristics.
        TODO: Remove it and make better video adapter selection window with refresh rate selection.
    */
    static int  GetBestRefreshRate( int width, int height, int depth );

    /*
        Original rw raster destructor.
        TODO: Probably should be moved to CRwGameHooks
    */
    static void* RwD3D11RasterDestructor( void* raster, RwInt32 offset, RwInt32 size );

    /*
        Replaces original radar alphatest mask with stencil mask
    */
    static void RenderRadar( RwPrimitiveType prim, RwIm2DVertex * vert, int count );

    /*
        Replaces original draw radar map function, to reset stencil render states.
    */
    static void RenderRadarMap();

    /*
        Initializes renderware stuff, currently replaces CHud::Initialize function.
    */
    static void InitWithRW();

    /*
        Initializes renderware stuff.
    */
    static void ShutdownWithRW();

    /*

    */
    static LRESULT CALLBACK AntTweakBarHookMsgProc( int code, WPARAM wParam, LPARAM lParam );

    static bool AddLight( char type, CVector pos, CVector dir, float radius, float red, float green, float blue, char fogType, char generateExtraShadows, CEntity *entityAffected );

    static bool AddLightNoSpot( char type, CVector pos, CVector dir, float radius, float red, float green, float blue, char fogType, char generateExtraShadows, CEntity *entityAffected );

    static void __fastcall  AddSpotLight( CVehicle* vehicle, void *Unknown, int a, CMatrix* matrix, bool isRight );
private:
    static HHOOK m_hAntTweakBarHook;
};

