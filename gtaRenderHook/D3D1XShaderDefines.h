#pragma once
/*!
    \class CD3D1XShaderDefine
    \brief Shader definition.
*/
class CD3D1XShaderDefine
{
public:
    CD3D1XShaderDefine( const std::string &name, const std::string &def );
    std::string m_sName;
    std::string m_sDefinition;
};
/*!
    \class CD3D1XGlobalShaderDefines
    \brief Global shader define holder.

    This class holds global shader definitions(e.g. feature level).
*/
class CD3D1XShaderDefineList
{
public:
    /*!
        Initializes global shader define list
    */
    CD3D1XShaderDefineList();
    /*!
        Adds a global shader defenition
    */
    void AddDefine( const std::string &defineName, const std::string &defineValue );
    /*!
        Resets global define list
    */
    void Reset();
    /*!
        Returns global shader define list.
    */
    std::vector<CD3D1XShaderDefine> GetDefineList() { return m_aDefines; }
private:
    std::vector<CD3D1XShaderDefine> m_aDefines;
};

extern CD3D1XShaderDefineList* g_pGlobalShaderDefines;