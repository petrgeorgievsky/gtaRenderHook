//
// Created by peter on 08.05.2020.
//
#pragma once
#include <common_headers.h>
#include <cstdint>
namespace rh
{
namespace rw::engine
{

struct PluginPtrTable;
struct BackendMaterialExt;

/**
 * RenderHook material plugin, holds additional material info
 */
class BackendMaterialPlugin
{
  public:
    BackendMaterialPlugin( const PluginPtrTable &plugin_cb );
    static uint8_t *           GetAddress( RpMaterial *material );
    static BackendMaterialExt &GetData( RpMaterial *material );

    /**
     * Calls "Always" callback on material, used in case rw version doesn't
     * support "Always" callback
     */
    static int32_t CallAlwaysCb( RpMaterial *material );

  private:
    static int32_t StreamAlwaysCallback(
        void *object, [[maybe_unused]] [[maybe_unused]] int32_t offsetInObject,
        [[maybe_unused]] [[maybe_unused]] int32_t sizeInObject );
    static int32_t Offset;
};

struct BackendMaterialExt
{
    RwTexture *mSpecTex = nullptr;
    float      Emission = 0.0f;
};

struct MaterialData
{
    int32_t mTexture;
    RwRGBA  mColor = RwRGBA{ 255, 0, 0, 255 };
    int32_t mSpecTexture;
    float   specular; /**< specular reflection coefficient */
    // float   diffuse;  /**< reflection coefficient */
};

MaterialData ConvertMaterialData( RpMaterial *material );

} // namespace rw::engine
} // namespace rh