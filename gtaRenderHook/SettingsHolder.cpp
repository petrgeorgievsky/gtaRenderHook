#include "SettingsHolder.h"
#include "D3D1XShader.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <crtdbg.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

SettingsHolder SettingsHolder::Instance{};

DebugSettingsBlock gDebugSettings; 
ShaderDefinesSettingsBlock gShaderDefineSettings;

SettingsHolder::SettingsHolder()
{
	// Try to find settings file, if not found create default.
	/*if (mSettingsFile.LoadFile("settings.xml") == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND) 
	{
		ResetSettings();
		SaveSettings();
	}
	else {
		LoadSettings(mSettingsFile);
		m_bInitialized = true;
	}*/
	
}


SettingsHolder::~SettingsHolder()
{
	
}

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

	for (auto block : m_aSettingsBlocks)
		newSettings.InsertEndChild(block->Save(&newSettings));

	newSettings.SaveFile("settings.xml");
}

void SettingsHolder::ReloadFile()
{
	// If initialized, then try to reload file
	if (m_bInitialized) {
		LoadSettings(mSettingsFile);
		return;
	}
	// Try to find settings file, if not found create default.
	if (mSettingsFile.LoadFile("settings.xml") == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND)
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
	SettingsHolder::Instance.SaveSettings();
}

void TW_CALL ReloadDataCallBack(void *value)
{
	SettingsHolder::Instance.ReloadFile();
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

tinyxml2::XMLElement * DebugSettingsBlock::Save(tinyxml2::XMLDocument *doc)
{
	// Debug settings node.
	auto debugSettingsNode = doc->NewElement(m_sName.c_str());

	debugSettingsNode->SetAttribute("ShowPreformanceCounters", ShowPreformanceCounters);
	debugSettingsNode->SetAttribute("DebugMessaging", DebugMessaging);
	debugSettingsNode->SetAttribute("DebugLevel", DebugLevel);
	debugSettingsNode->SetAttribute("UseIdleHook", UseIdleHook);
	debugSettingsNode->SetAttribute("Windowed", Windowed);
	debugSettingsNode->SetAttribute("UseDefaultAdapter", UseDefaultAdapter);

	return debugSettingsNode;
}

void DebugSettingsBlock::Load(const tinyxml2::XMLDocument &doc)
{
	auto debugSettingsNode = doc.FirstChildElement("DebugSettings");
	// Debug
	ShowPreformanceCounters = debugSettingsNode->BoolAttribute("ShowPreformanceCounters", true);
	DebugMessaging = debugSettingsNode->BoolAttribute("DebugMessaging", false);
	DebugLevel = debugSettingsNode->IntAttribute("DebugLevel", 0);
	UseIdleHook = debugSettingsNode->BoolAttribute("UseIdleHook", true);
	Windowed = debugSettingsNode->BoolAttribute("Windowed", true);
	UseDefaultAdapter = debugSettingsNode->BoolAttribute("UseDefaultAdapter", true);
	DebugRenderTarget = false;
	DebugRenderTargetNumber = 0;
}

void DebugSettingsBlock::Reset()
{
	// Debug
	ShowPreformanceCounters = true;
	DebugMessaging = false;
	DebugLevel = 0;
	UseIdleHook = true;
	DebugRenderTarget = false;
	DebugRenderTargetNumber=0;
	Windowed = true;
	UseDefaultAdapter = true;
}

void DebugSettingsBlock::InitGUI(TwBar * guiholder)
{
	TwAddVarRW(guiholder, "Enable RenderHook Idle", TwType::TW_TYPE_BOOL8, &UseIdleHook, "group=Global");
	TwAddVarRW(guiholder, "Show RenderTarget", TwType::TW_TYPE_BOOL8, &DebugRenderTarget, "group=Debug");
	std::string rtIdSettings = "min=0 max=";
	TwAddVarRW(guiholder, "RenderTarget ID", TwType::TW_TYPE_UINT32, &DebugRenderTargetNumber, 
		(rtIdSettings +std::to_string(DebugRenderTargetList.size())+" group=Debug").c_str());
}

tinyxml2::XMLElement * ShaderDefinesSettingsBlock::Save(tinyxml2::XMLDocument * doc)
{
	auto shaderSettingsNode = doc->NewElement(m_sName.c_str());

	shaderSettingsNode->SetAttribute("UsePhysicallyBasedRendering", UsePBR);

	return shaderSettingsNode;
}

void ShaderDefinesSettingsBlock::Load(const tinyxml2::XMLDocument & doc)
{
	auto shaderSettingsNode = doc.FirstChildElement(m_sName.c_str());
	UsePBR = shaderSettingsNode->BoolAttribute("UsePhysicallyBasedRendering", true);
}

void ShaderDefinesSettingsBlock::Reset()
{
	UsePBR = true;
}

void ShaderDefinesSettingsBlock::InitGUI(TwBar * guiholder)
{
}

void SettingsBlock::ReloadShaders()
{
	for (auto shader : m_aShaderPointers)
		shader->Reload(m_pShaderDefineList);
	m_bShaderReloadRequired = false;
}
