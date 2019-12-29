#pragma once
#include <RWUtils/RwAPI.h>

namespace RH_RWAPI
{

std::string RwV3dToStr( RwV3d vec );

struct rwStreamFrame
{
    RwV3d               right, up, at, pos;
    RwInt32             parentIndex;
    RwUInt32            data;

    std::string to_string() {
        std::string res =
            "rwStreamFrame:\n"
            "at:\t" + RwV3dToStr( at ) + "\n"
            "up:\t" + RwV3dToStr( up ) + "\n"
            "pos:\t" + RwV3dToStr( pos ) + "\n"
            "right:\t" + RwV3dToStr( right ) + "\n"
            "parent:\t" + std::to_string( parentIndex ) + "\n"
            "data:\t" + std::to_string( data ) + "\n";

        return res;
    }
};

rwFrameList* _rwFrameListStreamRead( void* stream, rwFrameList* frameList );

}