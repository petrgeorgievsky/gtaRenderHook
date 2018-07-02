#pragma once
#include <tinyxml2.h>
#include <AntTweakBar.h>
#include "SettingsFields.h"
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
	virtual tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	virtual void Load(const tinyxml2::XMLDocument &doc);
	virtual void Reset();
	virtual void InitGUI(TwBar* guiholder);
	virtual void ReloadShaders();

	int GetInt(const std::string& name) {
		IntSField* field = dynamic_cast<IntSField*>(m_aFields[name]);
		return field != nullptr ? field->GetValue() : 0;
	}

	unsigned int GetUInt(const std::string& name) {
		UIntSField* field = dynamic_cast<UIntSField*>(m_aFields[name]);
		return field != nullptr ? field->GetValue() : 0;
	}

	float GetFloat(const std::string& name) {
		FloatSField* field = dynamic_cast<FloatSField*>(m_aFields[name]);
		return field != nullptr ? field->GetValue() : 0.0f;
	}

	bool GetToggleField(const std::string& name) {
		ToggleSField* field = dynamic_cast<ToggleSField*>(m_aFields[name]);
		return field != nullptr ? field->GetValue() : false;
	}

protected:
	std::unordered_map<std::string, SettingsField*> m_aFields;
};
// Global shader settings block class, used to hold shader defenitions
class ShaderDefinesSettingsBlock : public SettingsBlock {
public:
	ShaderDefinesSettingsBlock() {
		m_sName = "ShaderSettings";
		m_aFields["UsePBR"] = new ToggleSField("UsePBR", true, false, true, m_sName, true);
		Reset();
	}
};
// Debug settings block class, used to hold debugging settings
class DebugSettingsBlock: public SettingsBlock {
public:
	DebugSettingsBlock() {
		m_sName = "Debug";
		m_aFields["Windowed"] = new ToggleSField("Windowed", true, false, false, m_sName, true);
		m_aFields["UseDefaultAdapter"] = new ToggleSField("UseDefaultAdapter", true, false, false, m_sName, true);
		m_aFields["UseIdleHook"] = new ToggleSField("UseIdleHook", false, true, false, m_sName, true);
		m_aFields["ShowPreformanceCounters"] = new ToggleSField("ShowPreformanceCounters", false, false, false, m_sName);
		m_aFields["DebugMessaging"] = new ToggleSField("DebugMessaging", false, false, false, m_sName, true);
		m_aFields["DebugRenderTarget"] = new ToggleSField("DebugRenderTarget", false, true, false, m_sName);
		m_aFields["DebugLevel"] = new IntSField("DebugLevel", true, false, false, m_sName, 0, 0, 2);
		Reset();
	}
	virtual void Reset() override;
	virtual void InitGUI(TwBar* guiholder) override;
public:
	unsigned int DebugRenderTargetNumber;
	std::vector<RwRaster*> DebugRenderTargetList;
};

extern DebugSettingsBlock gDebugSettings;
extern ShaderDefinesSettingsBlock gShaderDefineSettings;