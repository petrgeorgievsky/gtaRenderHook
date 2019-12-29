#pragma once
class CPostProcessEffect
{
public:
    CPostProcessEffect( std::string name );
    ~CPostProcessEffect();
    virtual void Render( RwRaster* inputRaster );
private:
    std::string m_effectName = "PostEffect";
};

