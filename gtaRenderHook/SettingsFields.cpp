#include "stdafx.h"
#include "SettingsFields.h"




SettingsField::SettingsField( const std::string & name, bool unchangeable, bool inGameOnly, bool shaderDependant, const std::string &groupName ) :
    m_sName( name ), m_bUnchangeable( unchangeable ), m_bInGameOnly( inGameOnly ), m_bShaderDependant( shaderDependant ), m_sGroupName( groupName )
{
}

SettingsField::~SettingsField()
{
}

int SettingsField::ToXML( tinyxml2::XMLElement * node )
{
    if ( m_bInGameOnly )
        return 1;
    if ( node == nullptr )
        return -1;
    return 0;
}

int SettingsField::FromXML( const tinyxml2::XMLElement * node )
{
    if ( m_bInGameOnly )
        return 1;
    if ( node == nullptr )
        return -1;
    return 0;
}

ToggleSField::ToggleSField( const std::string & name, bool unchangeable,
                            bool inGameOnly, bool shaderDependant, const std::string & groupName, bool defaultValue ) :
    SettingsField( name, unchangeable, inGameOnly, shaderDependant, groupName ), m_bDefaultValue( defaultValue )
{
}

int ToggleSField::ToXML( tinyxml2::XMLElement* node )
{
    int result = SettingsField::ToXML( node );
    if ( result != 0 )
        return result;
    node->SetAttribute( m_sName.c_str(), m_bValue );
    return 0;
}

int ToggleSField::FromXML( const tinyxml2::XMLElement * node )
{
    int result = SettingsField::FromXML( node );
    if ( result != 0 )
        return result;
    m_bValue = node->BoolAttribute( m_sName.c_str(), m_bDefaultValue );
    return result;
}

void ToggleSField::Draw( TwBar * guiholder )
{
    if ( m_bUnchangeable ) return;
    std::string settings = "";
    if ( !m_sGroupName.empty() )
    {
        settings += "group=";
        settings += m_sGroupName;
    }
    TwAddVarRW( guiholder, m_sName.c_str(), TwType::TW_TYPE_BOOL8, &m_bValue, settings.c_str() );
}

bool ToggleSField::GetValue()
{
    return m_bValue;
}

void ToggleSField::Reset()
{
    m_bValue = m_bDefaultValue;
}

IntSField::IntSField( const std::string & name, bool unchangeable, bool inGameOnly,
                      bool shaderDependant, const std::string & groupName, int defaultValue, int minValue, int maxValue, int step ) :
    SettingsField( name, unchangeable, inGameOnly, shaderDependant, groupName ),
    m_nDefaultValue( defaultValue ), m_nMinValue( minValue ), m_nMaxValue( maxValue ), m_nStep( step )
{
}

int IntSField::ToXML( tinyxml2::XMLElement * node )
{
    int result = SettingsField::ToXML( node );
    if ( result != 0 )
        return result;
    node->SetAttribute( m_sName.c_str(), m_nValue );
    return result;
}

int IntSField::FromXML( const tinyxml2::XMLElement * node )
{
    int result = SettingsField::FromXML( node );
    if ( result != 0 )
        return result;
    m_nValue = max( min( node->IntAttribute( m_sName.c_str(), m_nDefaultValue ), m_nMaxValue ), m_nMinValue );
    return result;
}

void IntSField::Draw( TwBar * guiholder )
{
    if ( m_bUnchangeable ) return;
    std::string settings = "";
    settings += " min=";
    settings += std::to_string( m_nMinValue );
    settings += " max=";
    settings += std::to_string( m_nMaxValue );
    settings += " step=";
    settings += std::to_string( m_nStep );
    if ( !m_sGroupName.empty() )
    {
        settings += " group=";
        settings += m_sGroupName;
    }
    TwAddVarRW( guiholder, m_sName.c_str(), TwType::TW_TYPE_INT32, &m_nValue, settings.c_str() );
}

void IntSField::Reset()
{
    m_nValue = m_nDefaultValue;
}

int IntSField::GetValue()
{
    return m_nValue;
}


FloatSField::FloatSField( const std::string & name, bool unchangeable, bool inGameOnly,
                          bool shaderDependant, const std::string & groupName, float defaultValue, float minValue, float maxValue, float step ) :
    SettingsField( name, unchangeable, inGameOnly, shaderDependant, groupName ),
    m_fDefaultValue( defaultValue ), m_fMinValue( minValue ), m_fMaxValue( maxValue ), m_fStep( step )
{
}

int FloatSField::ToXML( tinyxml2::XMLElement * node )
{
    int result = SettingsField::ToXML( node );
    if ( result != 0 )
        return result;
    node->SetAttribute( m_sName.c_str(), m_fValue );
    return result;
}

int FloatSField::FromXML( const tinyxml2::XMLElement * node )
{
    int result = SettingsField::FromXML( node );
    if ( result != 0 )
        return result;
    m_fValue = max( min( node->FloatAttribute( m_sName.c_str(), m_fDefaultValue ), m_fMaxValue ), m_fMinValue );
    return result;
}

void FloatSField::Draw( TwBar * guiholder )
{
    if ( m_bUnchangeable ) return;
    std::string settings = "";
    settings += " min=";
    settings += std::to_string( m_fMinValue );
    settings += " max=";
    settings += std::to_string( m_fMaxValue );
    settings += " step=";
    settings += std::to_string( m_fStep );
    if ( !m_sGroupName.empty() )
    {
        settings += " group=";
        settings += m_sGroupName;
    }
    TwAddVarRW( guiholder, m_sName.c_str(), TwType::TW_TYPE_FLOAT, &m_fValue, settings.c_str() );
}

void FloatSField::Reset()
{
    m_fValue = m_fDefaultValue;
}

float FloatSField::GetValue()
{
    return m_fValue;
}

UIntSField::UIntSField( const std::string & name, bool unchangeable, bool inGameOnly, bool shaderDependant,
                        const std::string & groupName, unsigned int defaultValue, unsigned int minValue, unsigned int maxValue, unsigned int step )
    : SettingsField( name, unchangeable, inGameOnly, shaderDependant, groupName ),
    m_nDefaultValue( defaultValue ), m_nMinValue( minValue ), m_nMaxValue( maxValue ), m_nStep( step )
{
}

int UIntSField::ToXML( tinyxml2::XMLElement * node )
{
    int result = SettingsField::ToXML( node );
    if ( result != 0 )
        return result;
    node->SetAttribute( m_sName.c_str(), m_nValue );
    return result;
}

int UIntSField::FromXML( const tinyxml2::XMLElement * node )
{
    int result = SettingsField::FromXML( node );
    if ( result != 0 )
        return result;
    m_nValue = max( min( node->UnsignedAttribute( m_sName.c_str(), m_nDefaultValue ), m_nMaxValue ), m_nMinValue );
    return result;
}

void UIntSField::Draw( TwBar * guiholder )
{
    if ( m_bUnchangeable ) return;
    std::string settings = "";
    settings += " min=";
    settings += std::to_string( m_nMinValue );
    settings += " max=";
    settings += std::to_string( m_nMaxValue );
    settings += " step=";
    settings += std::to_string( m_nStep );
    if ( !m_sGroupName.empty() )
    {
        settings += " group=";
        settings += m_sGroupName;
    }
    TwAddVarRW( guiholder, m_sName.c_str(), TwType::TW_TYPE_UINT32, &m_nValue, settings.c_str() );
}

void UIntSField::Reset()
{
    m_nValue = m_nDefaultValue;
}

unsigned int UIntSField::GetValue()
{
    return m_nValue;
}
