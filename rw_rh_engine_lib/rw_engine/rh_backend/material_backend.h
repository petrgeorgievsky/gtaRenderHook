//
// Created by peter on 08.05.2020.
//
#pragma once

#include <Engine/ResourcePool.h>
#include <common_headers.h>
#include <cstdint>
namespace rh
{

namespace engine
{
} // namespace engine

namespace rw::engine
{

struct BackendMaterialExt
{
    uint64_t   mMaterialId;
    RwTexture *mSpecTex;
};

extern int32_t gBackendMaterialExtOffset;

/* Plugin Attach */
int32_t BackendMaterialPluginAttach();
void *  BackendMaterialCtor( void *                   object,
                             [[maybe_unused]] int32_t offsetInObject,
                             [[maybe_unused]] int32_t sizeInObject );
void *  BackendMaterialDtor( void *                   object,
                             [[maybe_unused]] int32_t offsetInObject,
                             [[maybe_unused]] int32_t sizeInObject );

int32_t
BackendMaterialStreamAlwaysCallback( void *                   object,
                                     [[maybe_unused]] int32_t offsetInObject,
                                     [[maybe_unused]] int32_t sizeInObject );

/* Open/Close */
// void BackendRasterOpen();
// void BackendRasterClose();

BackendMaterialExt *GetBackendMaterialExt( RpMaterial *material );

struct MaterialInitData
{
    uint64_t mTexture;
    uint64_t mSpecTexture;
    RwRGBA   mColor;
    float    ambient;  /**< ambient reflection coefficient */
    float    specular; /**< specular reflection coefficient */
    float    diffuse;  /**< reflection coefficient */
};

struct MaterialData
{
    int32_t mTexture;
    RwRGBA  mColor = RwRGBA{ 255, 0, 0, 255 };
    int32_t mSpecTexture;
    float   specular; /**< specular reflection coefficient */
    // float   diffuse;  /**< reflection coefficient */
};

uint64_t     CreateMaterialData( RpMaterial *material );
MaterialData ConvertMaterialData( RpMaterial *material );

} // namespace rw::engine
} // namespace rh