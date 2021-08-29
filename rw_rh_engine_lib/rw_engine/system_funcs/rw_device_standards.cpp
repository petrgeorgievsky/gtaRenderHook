//
// Created by peter on 16.02.2021.
//

#include "rw_device_standards.h"

#include <DebugUtils/DebugLogger.h>

#include <rw_engine/rw_standard_render_commands/camerabeginupdatecmd.h>
#include <rw_engine/rw_standard_render_commands/cameraclearcmd.h>
#include <rw_engine/rw_standard_render_commands/cameraendupdatecmd.h>
#include <rw_engine/rw_standard_render_commands/imagefindrasterformat.h>
#include <rw_engine/rw_standard_render_commands/native_texture_get_size_cmd.h>
#include <rw_engine/rw_standard_render_commands/native_texture_write_cmd.h>
#include <rw_engine/rw_standard_render_commands/nativetexturereadcmd.h>
#include <rw_engine/rw_standard_render_commands/rastercreatecmd.h>
#include <rw_engine/rw_standard_render_commands/rasterdestroycmd.h>
#include <rw_engine/rw_standard_render_commands/rasterlockcmd.h>
#include <rw_engine/rw_standard_render_commands/rastersetimagecmd.h>
#include <rw_engine/rw_standard_render_commands/rastershowrastercmd.h>
#include <rw_engine/rw_standard_render_commands/rasterunlockcmd.h>

namespace rh::rw::engine
{
std::map<RwDeviceStandardFn, RwStandardFunc> GetStandardMap()
{
    static std::map<RwDeviceStandardFn, RwStandardFunc> standards{
        { rwSTANDARDNASTANDARD,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log(
                  std::string( "RWGAMEHOOKS_LOG: rwSTANDARDNASTANDARD" ) );
              return 1;
          } },
        { rwSTANDARDCAMERABEGINUPDATE,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwCameraBeginUpdateCmd cmd( static_cast<RwCamera *>( pInOut ) );
              return cmd.Execute();
          } },
        { rwSTANDARDRGBTOPIXEL,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log(
                  std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRGBTOPIXEL" ) );
              return 1;
          } },
        { rwSTANDARDPIXELTORGB,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log(
                  std::string( "RWGAMEHOOKS_LOG: rwSTANDARDPIXELTORGB" ) );
              return 1;
          } },
        { rwSTANDARDRASTERCREATE,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwRasterCreateCmd cmd( static_cast<RwRaster *>( pInOut ), nI );
              return cmd.Execute();
          } },
        { rwSTANDARDRASTERDESTROY,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwRasterDestroyCmd cmd( static_cast<RwRaster *>( pInOut ) );
              return cmd.Execute();
          } },
        { rwSTANDARDIMAGEGETRASTER,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log(
                  std::string( "RWGAMEHOOKS_LOG: rwSTANDARDIMAGEGETRASTER" ) );
              return 1;
          } },
        { rwSTANDARDRASTERSETIMAGE,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwRasterSetImageCmd set_img_cmd(
                  static_cast<RwRaster *>( pOut ),
                  static_cast<RwImage *>( pInOut ) );
              return set_img_cmd.Execute();
          } },
        { rwSTANDARDTEXTURESETRASTER,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDTEXTURESETRASTER" ) );
              return 1;
          } },
        { rwSTANDARDIMAGEFINDRASTERFORMAT,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwImageFindRasterFormatCmd find_raster_cmd(
                  static_cast<RwRaster *>( pOut ),
                  static_cast<RwImage *>( pInOut ),
                  static_cast<uint32_t>( nI ) );
              return find_raster_cmd.Execute();
          } },
        { rwSTANDARDCAMERAENDUPDATE,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwCameraEndUpdateCmd cmd( static_cast<RwCamera *>( pInOut ) );
              return cmd.Execute();
          } },
        { rwSTANDARDSETRASTERCONTEXT,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDSETRASTERCONTEXT" ) );
              return 1;
          } },
        { rwSTANDARDRASTERSUBRASTER,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log(
                  std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERSUBRASTER" ) );
              return 1;
          } },
        { rwSTANDARDRASTERCLEARRECT,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log(
                  std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERCLEARRECT" ) );
              return 1;
          } },
        { rwSTANDARDRASTERCLEAR,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log(
                  std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERCLEAR" ) );
              return 1;
          } },
        { rwSTANDARDRASTERLOCK,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwRasterLockCmd lock_cmd( static_cast<RwRaster *>( pInOut ), nI );
              void          *&pOutData = *static_cast<void **>( pOut );
              return lock_cmd.Execute( pOutData );
          } },
        { rwSTANDARDRASTERUNLOCK,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwRasterUnlockCmd unlock_cmd( static_cast<RwRaster *>( pInOut ) );
              return unlock_cmd.Execute();
          } },
        { rwSTANDARDRASTERRENDER,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log(
                  std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERRENDER" ) );
              return 1;
          } },
        { rwSTANDARDRASTERRENDERSCALED,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDRASTERRENDERSCALED" ) );
              return 1;
          } },
        { rwSTANDARDRASTERRENDERFAST,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDRASTERRENDERFAST" ) );
              return 1;
          } },
        { rwSTANDARDRASTERSHOWRASTER,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwRasterShowRasterCmd cmd( static_cast<RwRaster *>( pOut ), nI );
              return cmd.Execute();
          } },
        { rwSTANDARDCAMERACLEAR,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              RwCameraClearCmd cmd( static_cast<RwCamera *>( pOut ),
                                    static_cast<RwRGBA *>( pInOut ), nI );
              return cmd.Execute();
          } },
        { rwSTANDARDHINTRENDERF2B,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              // debug::DebugLogger::Log(
              //     std::string( "RWGAMEHOOKS_LOG: rwSTANDARDHINTRENDERF2B" )
              //     );
              return 1;
          } },
        { rwSTANDARDRASTERLOCKPALETTE,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDRASTERLOCKPALETTE" ) );
              return 1;
          } },
        { rwSTANDARDRASTERUNLOCKPALETTE,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDRASTERUNLOCKPALETTE" ) );
              return 1;
          } },
        { rwSTANDARDNATIVETEXTUREGETSIZE,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDNATIVETEXTUREGETSIZE" ) );
              RwNativeTextureGetSizeCmd cmd(
                  static_cast<RwTexture *>( pInOut ) );
              return cmd.Execute( *static_cast<uint32_t *>( pOut ) );
          } },
        { rwSTANDARDNATIVETEXTUREREAD,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDNATIVETEXTUREREAD" ) );

              RwNativeTextureReadCmd cmd( static_cast<RwStream *>( pOut ),
                                          static_cast<RwTexture **>( pInOut ) );
              return cmd.Execute();
          } },
        { rwSTANDARDNATIVETEXTUREWRITE,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDNATIVETEXTUREWRITE" ) );
              RwNativeTextureWriteCmd cmd( static_cast<RwStream *>( pOut ),
                                           static_cast<RwTexture *>( pInOut ) );
              return cmd.Execute();
          } },
        { rwSTANDARDRASTERGETMIPLEVELS,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t
          {
              debug::DebugLogger::Log( std::string(
                  "RWGAMEHOOKS_LOG: rwSTANDARDRASTERGETMIPLEVELS" ) );
              return 1;
          } },
        { rwSTANDARDNUMOFSTANDARD,
          []( void *pOut, void *pInOut, int32_t nI ) -> int32_t { return 1; } },
    };
    return standards;
}
} // namespace rh::rw::engine