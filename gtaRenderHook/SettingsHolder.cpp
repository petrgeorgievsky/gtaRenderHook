// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "SettingsHolder.h"
#include "D3D1XShader.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <crtdbg.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

DebugSettingsBlock gDebugSettings; 
ShaderDefinesSettingsBlock gShaderDefineSettings;
std::string SettingsHolder::m_sSettingsFileName = "settings.xml";

void SettingsHolder::AddSettingBlock(SettingsBlock * block)
{
	m_aSettingsBlocks.push_back(block);
}

// Resets settings
void SettingsHolder::ResetSettings()
{
	for (auto block : m_aSettingsBlocks)
		block->Reset();
}

void SettingsHolder::LoadSettings(const tinyxml2::XMLDocument &file)
{
	// Load settings
	for (auto block : m_aSettingsBlocks)
		block->Load(file);
}

void SettingsHolder::SaveSettings()
{
	tinyxml2::XMLDocument newSettings;
	// Save each settings block
	for (auto block : m_aSettingsBlocks)
		newSettings.InsertEndChild(block->Save(&newSettings));

	newSettings.SaveFile(m_sSettingsFileName.c_str());
}

void SettingsHolder::ReloadFile()
{
	// Try to find settings file, if not found create default.
	if (mSettingsFile.LoadFile(m_sSettingsFileName.c_str()) == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND)
	{
		ResetSettings();
		SaveSettings();
	}
	else {
		LoadSettings(mSettingsFile);
		m_bInitialized = true;
	}
}

void SettingsHolder::ReloadShadersIfRequired()
{
	for (auto block : m_aSettingsBlocks) {
		if (block->m_bShaderReloadRequired)
			block->ReloadShaders();
	}
}
void TW_CALL SaveDataCallBack(void *value)
{
	SettingsHolder::Instance()->SaveSettings();
}

void TW_CALL ReloadDataCallBack(void *value)
{
	SettingsHolder::Instance()->ReloadFile();
}

void SettingsHolder::InitGUI()
{
	if (m_pGuiholder != nullptr)
		return;
	// Init new settings bar
	m_pGuiholder = TwNewBar("Settings");
	// Init each settings block
	for (auto block : m_aSettingsBlocks)
		block->InitGUI(m_pGuiholder);
	// Init save and reload buttons
	TwAddButton(m_pGuiholder, "Save", SaveDataCallBack, nullptr, "");
	TwAddButton(m_pGuiholder, "Reload", ReloadDataCallBack, nullptr, "");
}

void SettingsHolder::DrawGUI()
{
	if (!m_bDrawTweakBar) return;

	TwDraw();
}

bool SettingsHolder::IsGUIEnabled()
{
	return m_bDrawTweakBar;
}

void SettingsHolder::EnableGUI()
{
	m_bDrawTweakBar = true;
}

void SettingsHolder::DisableGUI()
{
	m_bDrawTweakBar = false;
}

void DebugSettingsBlock::Reset()
{
	SettingsBlock::Reset();
	DebugRenderTargetNumber=0;
}

void DebugSettingsBlock::InitGUI(TwBar * guiholder)
{
	SettingsBlock::InitGUI(guiholder);
	std::string rtIdSettings = "min=0 max=";
	TwAddVarRW(guiholder, "RenderTarget ID", TwType::TW_TYPE_UINT32, &DebugRenderTargetNumber, 
		(rtIdSettings +std::to_string(DebugRenderTargetList.size())+" group=Debug").c_str());
}

tinyxml2::XMLElement * SettingsBlock::Save(tinyxml2::XMLDocument * doc)
{
	auto settingsNode = doc->NewElement(m_sName.c_str());
	for (auto field : m_aFields)
		if (field.second->ToXML(settingsNode)<0)
			break;

	return settingsNode;
}

void SettingsBlock::Load(const tinyxml2::XMLDocument & doc)
{
	auto debugSettingsNode = doc.FirstChildElement(m_sName.c_str());
	// Debug
	for (auto field : m_aFields)
		if (field.second->FromXML(debugSettingsNode)<0)
			break;
}

void SettingsBlock::Reset()
{
	for (auto field : m_aFields)
		field.second->Reset();
}

void SettingsBlock::InitGUI(TwBar * guiholder)
{
	for (auto field : m_aFields)
		field.second->Draw(guiholder);
}

void SettingsBlock::ReloadShaders()
{
	for (auto shader : m_aShaderPointers)
		shader->Reload(&m_ShaderDefineList);
	m_bShaderReloadRequired = false;
}
