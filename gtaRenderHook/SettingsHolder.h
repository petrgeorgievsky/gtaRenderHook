#pragma once
#include "tinyxml2.h"
#include "AntTweakBar.h"
class SettingsBlock;

// Render Hook settings holder singleton
class SettingsHolder
{
	std::list<SettingsBlock*>	m_aSettingsBlocks;
	tinyxml2::XMLDocument			mSettingsFile;
	bool m_bInitialized=false;
public:
	static SettingsHolder Instance;
	SettingsHolder();
	~SettingsHolder();

	void AddSettingBlock(SettingsBlock* block);
	void ResetSettings();
	void LoadSettings(const tinyxml2::XMLDocument &file);
	void SaveSettings();
	void ReloadFile();
};

// Settings block base class, used to hold various settings
class SettingsBlock {
public:
	std::string m_sName;
	SettingsBlock() { }
	virtual tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc) = 0;
	virtual void Load(const tinyxml2::XMLDocument &doc) = 0;
	virtual void Reset() = 0;
	virtual void InitGUI(TwBar* guiholder) = 0;
};
// Global shader settings block class, used to hold shader defenitions
class ShaderDefinesSettingsBlock : public SettingsBlock {
public:
	ShaderDefinesSettingsBlock() {
		m_sName = "ShaderSettings";
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar* guiholder);
public:
	bool UsePBR;
	int SSRSampleCount;
};
// Debug settings block class, used to hold debugging settings
class DebugSettingsBlock: public SettingsBlock {
public:
	DebugSettingsBlock() {
		m_sName = "DebugSettings";
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar* guiholder);
public:
	bool ShowPreformanceCounters;
	bool DebugMessaging;
	int  DebugLevel;
};

extern DebugSettingsBlock gDebugSettings;
extern ShaderDefinesSettingsBlock gShaderDefineSettings;