//
// Created by peter on 16.02.2021.
//

#pragma once
#include <cstdint>
#include <map>

enum RwDeviceStandardFn
{
    rwSTANDARDNASTANDARD,
    rwSTANDARDCAMERABEGINUPDATE,     /* Start 3d camera update */
    rwSTANDARDRGBTOPIXEL,            /* For an RGB value return a pixel value */
    rwSTANDARDPIXELTORGB,            /* For a pixel value returns a RGB value */
    rwSTANDARDRASTERCREATE,          /* Create an raster */
    rwSTANDARDRASTERDESTROY,         /* Raster destroy */
    rwSTANDARDIMAGEGETRASTER,        /* Get image from a raster */
    rwSTANDARDRASTERSETIMAGE,        /* Set raster from an image */
    rwSTANDARDTEXTURESETRASTER,      /* Set texture's raster */
    rwSTANDARDIMAGEFINDRASTERFORMAT, /* Find a suitable raster format for an
                                        image */
    rwSTANDARDCAMERAENDUPDATE,       /* End 3d camera update */
    rwSTANDARDSETRASTERCONTEXT,      /* Start destination of 2d operations */
    rwSTANDARDRASTERSUBRASTER,       /* Make a raster inside another raster */
    rwSTANDARDRASTERCLEARRECT, /* Clear a rectangle of the current dest raster
                                */
    rwSTANDARDRASTERCLEAR,     /* Clear the current dest raster */
    rwSTANDARDRASTERLOCK,      /* Lock a raster to get it's pixels */
    rwSTANDARDRASTERUNLOCK,    /* Unlock a raster to return it's pixels */
    rwSTANDARDRASTERRENDER,    /* Render a raster (not scaled, but masked) */
    rwSTANDARDRASTERRENDERSCALED, /* Render a raster (scaled and masked) */
    rwSTANDARDRASTERRENDERFAST,   /* Render a raster (not scaled or masked) */
    rwSTANDARDRASTERSHOWRASTER,   /* Show a camera raster */
    rwSTANDARDCAMERACLEAR,        /* Clear a camera's raster and/or Z raster */
    rwSTANDARDHINTRENDERF2B, /* Set hint for rendering direction in the world */
    rwSTANDARDRASTERLOCKPALETTE,    /* Lock a raster to get it's palette */
    rwSTANDARDRASTERUNLOCKPALETTE,  /* Unlock a raster to return it's palette */
    rwSTANDARDNATIVETEXTUREGETSIZE, /* Get size of native texture when written
                                       to a stream */
    rwSTANDARDNATIVETEXTUREREAD,    /* Read native texture from the stream */
    rwSTANDARDNATIVETEXTUREWRITE,   /* Write native texture to the stream */
    rwSTANDARDRASTERGETMIPLEVELS, /* Get the number of mip levels in a raster */
    rwSTANDARDNUMOFSTANDARD,

};
namespace rh::rw::engine
{
using RwStandardFunc = int32_t ( * )( void *pOut, void *pInOut, int32_t nI );
std::map<RwDeviceStandardFn, RwStandardFunc> GetStandardMap();
} // namespace rh::rw::engine