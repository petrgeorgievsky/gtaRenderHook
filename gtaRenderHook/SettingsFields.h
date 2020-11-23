#pragma once
#pragma warning( push, 0 )
#include <tinyxml2.h>
#include <AntTweakBar.h>
#pragma warning( pop )
/*!
    \class SettingsField
    \brief Abstract settings field class.

    This class is used to hold settings fields.
*/
class SettingsField
{
public:
    SettingsField( const std::string& name, bool unchangeable, bool inGameOnly, bool shaderDependant, const std::string &groupName = "" );
    virtual ~SettingsField();
    /*
        Converts value of field to XML node, returns 0 if succeded, -1 if failed,
        1 if XML should be skipped.
    */
    virtual int ToXML( tinyxml2::XMLElement* node );
    /*
        Converts value of field from XML node, returns 0 if succeded, -1 if failed,
        1 if XML should be skipped.
    */
    virtual int FromXML( const tinyxml2::XMLElement* node );
    /*
        Adds GUI representation of a field to AntTweakBar GUI window.
    */
    virtual void Draw( TwBar * guiholder ) = 0;
    /*
        Resets field value to default one.
    */
    virtual void Reset() = 0;
protected:
    /// Field name
    std::string m_sName;
    /// Group name
    std::string m_sGroupName;
    /// Unchangeable in game
    bool m_bUnchangeable;
    /// Not present in xml file
    bool m_bInGameOnly;
    /// Requires shader reloading
    bool m_bShaderDependant;
};

/*!
    \class ToggleSField
    \brief Toggle setting field.

    This class is used hold boolean fields that could be toggled in game/settings file.
*/
class ToggleSField : public SettingsField
{
public:
    ToggleSField( const std::string& name, bool unchangeable, bool inGameOnly,
                  bool shaderDependant, const std::string &groupName = "", bool defaultValue = false );
    virtual int ToXML( tinyxml2::XMLElement* node ) override;
    virtual int FromXML( const tinyxml2::XMLElement* node ) override;
    virtual void Draw( TwBar * guiholder ) override;
    virtual void Reset() override;
    /*
        Returns value of field.
    */
    bool GetValue();
private:
    /// Current value
    bool m_bValue=false;
    /// Default value
    bool m_bDefaultValue = false;
};

/*!
    \class IntSField
    \brief Integer setting field.

    This class is used hold integer fields that could be changed in game/settings file.
*/
class IntSField : public SettingsField
{
public:
    IntSField( const std::string& name, bool unchangeable, bool inGameOnly,
               bool shaderDependant, const std::string &groupName = "", int defaultValue = 0,
               int minValue = INT_MIN, int maxValue = INT_MAX, int step = 1 );
    virtual int ToXML( tinyxml2::XMLElement* node ) override;
    virtual int FromXML( const tinyxml2::XMLElement* node ) override;
    virtual void Draw( TwBar * guiholder ) override;
    virtual void Reset() override;
    /*
        Returns value of field.
    */
    int GetValue();
private:
    /// Current value
  int m_nValue{};
    /// Lower bound of integer
    int m_nMinValue;
    /// Upper bound of integer
    int m_nMaxValue;
    /// Step for UI sliders
    int m_nStep;
    /// Default value
    int m_nDefaultValue = 0;
};

/*!
    \class UIntSField
    \brief Unsigned integer setting field.

    This class is used hold unsigned integer fields that could be changed in game/settings file.
*/
class UIntSField : public SettingsField
{
public:
    UIntSField( const std::string& name, bool unchangeable, bool inGameOnly,
                bool shaderDependant, const std::string &groupName = "", unsigned int defaultValue = 0,
                unsigned int minValue = 0, unsigned int maxValue = UINT_MAX, unsigned int step = 1 );
    virtual int ToXML( tinyxml2::XMLElement* node ) override;
    virtual int FromXML( const tinyxml2::XMLElement* node ) override;
    virtual void Draw( TwBar * guiholder ) override;
    virtual void Reset() override;
    /*
        Returns value of field.
    */
    unsigned int GetValue();
private:
    /// Current value
  unsigned int m_nValue{};
    /// Lower bound of integer
    unsigned int m_nMinValue;
    /// Upper bound of integer
    unsigned int m_nMaxValue;
    /// Step for UI sliders
    unsigned int m_nStep;
    /// Default value
    unsigned int m_nDefaultValue = 0;
};

/*!
    \class FloatSField
    \brief Floating-point number setting field.

    This class is used hold float fields that could be changed in game/settings file.
*/
class FloatSField : public SettingsField
{
public:
    FloatSField( const std::string& name, bool unchangeable, bool inGameOnly,
                 bool shaderDependant, const std::string &groupName = "", float defaultValue = 0.0f,
                 float minValue = -1.0, float maxValue = 1.0, float step = 0.1f );
    virtual int ToXML( tinyxml2::XMLElement* node ) override;
    virtual int FromXML( const tinyxml2::XMLElement* node ) override;
    virtual void Draw( TwBar * guiholder ) override;
    virtual void Reset() override;
    /*
        Returns value of field.
    */
    float GetValue();
private:
    /// Current value
  float m_fValue{};
    /// Lower bound of float
    float m_fMinValue;
    /// Upper bound of integer
    float m_fMaxValue;
    /// Step for UI sliders
    float m_fStep;
    /// Default value
    float m_fDefaultValue = 0.0f;
};