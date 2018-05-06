#pragma once
#include <tinyxml2.h>
#include <AntTweakBar.h>
class SettingsBlock;
class CD3D1XShader;
#include "D3D1XShaderDefines.h"
/*! 
	Render Hook settings holder singleton
*/
class SettingsHolder
{
	std::list<SettingsBlock*>	m_aSettingsBlocks;
	tinyxml2::XMLDocument			mSettingsFile;
	bool m_bInitialized = false;
	bool m_bDrawTweakBar = false;
	TwBar* m_pGuiholder = nullptr;
public:
	static SettingsHolder Instance;
	SettingsHolder();
	~SettingsHolder();

	void AddSettingBlock(SettingsBlock* block);
	void ResetSettings();
	void LoadSettings(const tinyxml2::XMLDocument &file);
	void SaveSettings();
	void ReloadFile();
	void ReloadShadersIfRequired();
	void InitGUI();
	void DrawGUI();
	bool IsGUIEnabled();
	void EnableGUI();
	void DisableGUI();
};

/*!
	Settings block base class, used to hold various settings
*/
class SettingsBlock {
public:
	std::string m_sName;
	std::vector<CD3D1XShader*> m_aShaderPointers;
	CD3D1XShaderDefineList* m_pShaderDefineList;

	bool m_bShaderReloadRequired=false;
	SettingsBlock() {
		m_pShaderDefineList = new CD3D1XShaderDefineList();
		Reset();
	}
	virtual tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc) = 0;
	virtual void Load(const tinyxml2::XMLDocument &doc) = 0;
	virtual void Reset();
	virtual void InitGUI(TwBar* guiholder) = 0;
	virtual void ReloadShaders();
};
// Global shader settings block class, used to hold shader defenitions
class ShaderDefinesSettingsBlock : public SettingsBlock {
public:
	ShaderDefinesSettingsBlock() {
		m_sName = "ShaderSettings";
		Reset();
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar* guiholder);
public:
	bool UsePBR;
};
// Debug settings block class, used to hold debugging settings
class DebugSettingsBlock: public SettingsBlock {
public:
	DebugSettingsBlock() {
		m_sName = "DebugSettings";
		Reset();
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar* guiholder);
public:
	bool Windowed;
	bool UseDefaultAdapter;
	bool UseIdleHook;
	bool ShowPreformanceCounters;
	bool DebugMessaging;
	int  DebugLevel;

	bool DebugRenderTarget;
	int DebugRenderTargetNumber;
	std::vector<RwRaster*> DebugRenderTargetList;
};

extern DebugSettingsBlock gDebugSettings;
extern ShaderDefinesSettingsBlock gShaderDefineSettings;