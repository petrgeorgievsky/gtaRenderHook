#pragma once
#include <rwtestsample.h>

/**
 * @brief Simple test sample for RenderHook, write your own code as you wish
 *
 */
class SimpleSample : public rh::rw::engine::RwTestSample
{
public:
    SimpleSample( rh::engine::RenderingAPI api, HINSTANCE inst )
        : rh::rw::engine::RwTestSample( api, inst )
    {}
    bool CustomInitialize() final;
    void CustomShutdown() final;
    void CustomRender() override;
    void CustomUpdate( float dt ) override;

private:
    void LoadTXD( const rh::engine::String &path );
    void GenerateQuad( float w, float h );

private:
    std::vector<RwIm2DVertex> m_vQuad;
    RwTexDictionary *m_currentTexDict = nullptr;
    std::vector<rh::engine::String> m_vTexDictList;
    std::vector<RwTexture *> m_vTextureList;
    uint32_t m_nTexId = 0;
    uint32_t m_nTXDId = 0;
    bool button_toggled[4] = {false, false, false, false};
};
