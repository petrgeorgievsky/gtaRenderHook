#pragma once
#include "game_sa/CFileLoader.h"
#include <unordered_map>
/*!
    Physically based shader material
*/
class CPBSMaterial
{
public:
    CPBSMaterial( const std::string&fname );
  std::string m_sName{};
    RwTexture * m_tSpecRoughness{};
  RwTexture * m_tNormals{};
};

class CPBSMaterialMgr
{
public:
    static void LoadMaterials();
    static void SetMaterial( const char* textureName );
    static RwTexDictionary* materialsTXD;
    static std::unordered_map<std::string, CPBSMaterial*> materials;
};
