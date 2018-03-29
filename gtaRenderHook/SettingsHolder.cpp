#include "SettingsHolder.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <crtdbg.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

SettingsHolder SettingsHolder::Instance{};

DebugSettingsBlock gDebugSettings;

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

tinyxml2::XMLElement * DebugSettingsBlock::Save(tinyxml2::XMLDocument *doc)
{
	// Debug settings node.
	auto debugSettingsNode = doc->NewElement(m_sName.c_str());

	debugSettingsNode->SetAttribute("ShowPreformanceCounters", ShowPreformanceCounters);
	debugSettingsNode->SetAttribute("DebugMessaging", DebugMessaging);
	debugSettingsNode->SetAttribute("DebugLevel", DebugLevel);

	return debugSettingsNode;
}

void DebugSettingsBlock::Load(const tinyxml2::XMLDocument &doc)
{
	auto debugSettingsNode = doc.FirstChildElement("DebugSettings");
	// Debug
	ShowPreformanceCounters = debugSettingsNode->BoolAttribute("ShowPreformanceCounters", true);
	DebugMessaging = debugSettingsNode->BoolAttribute("DebugMessaging", false);
	DebugLevel = debugSettingsNode->BoolAttribute("DebugLevel", 0);
}

void DebugSettingsBlock::Reset()
{
	// Debug
	ShowPreformanceCounters = true;
	DebugMessaging = false;
	DebugLevel = 0;
}

void DebugSettingsBlock::InitGUI(TwBar * guiholder)
{
}
