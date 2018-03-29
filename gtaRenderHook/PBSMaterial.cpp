
#include "PBSMaterial.h"

#include <experimental\filesystem>

RwTexDictionary* CPBSMaterialMgr::materialsTXD;
std::list<CPBSMaterial*> CPBSMaterialMgr::materials;
bool has_suffix(const std::string& s, const std::string& suffix)
{
	return (s.size() >= suffix.size()) && std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}
CPBSMaterial::CPBSMaterial(const std::string & fname)
{
	auto file= fopen((std::string("materials\\")+ fname+".mat").c_str(), "rt");
	char specFileName[80];
	fscanf(file, "%s", specFileName);
	fclose(file);
	m_sName = fname;
	m_tSpecRoughness = _RwTexDictionaryFindNamedTexture(CPBSMaterialMgr::materialsTXD, specFileName);
	//m_tSpecRoughness
}

void CPBSMaterialMgr::LoadMaterials()
{
	materialsTXD = CFileLoader::LoadTexDictionary("materials\\materials.txd");
	std::string path("materials/");
	std::string ext(".mat");
	for (auto& p : std::experimental::filesystem::recursive_directory_iterator(path))
	{
		if (p.path().extension() == ext)
			materials.push_back(new CPBSMaterial(p.path().stem().string()));
	}
}
