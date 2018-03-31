#pragma once
#include "game_sa/CFileLoader.h"
#include <unordered_map>
class CPBSMaterial
{
public:
	CPBSMaterial(const std::string&fname);
	std::string m_sName;
	RwTexture* m_tSpecRoughness;
};

class CPBSMaterialMgr
{
public:
	static void LoadMaterials();
	static RwTexDictionary* materialsTXD;
	static std::unordered_map<std::string, CPBSMaterial*> materials;
};
