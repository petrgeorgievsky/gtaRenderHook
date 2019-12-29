#pragma once
class CD3D1XVertexDeclaration;
class CD3DRenderer;
class CD3D1XShader;
class CD3D1XVertexDeclarationManager
{
    static std::list<CD3D1XVertexDeclaration*> vdeclList;
public:
    static CD3D1XVertexDeclaration* AddNew( CD3D1XShader* pVS, UINT flags );
    static void Shutdown();
};

