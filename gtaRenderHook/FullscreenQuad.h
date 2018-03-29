#pragma once
class CD3D1XShader;
struct QuadVertex
{
	RwV4d Position;
	RwV2d TexCoord;
};
class CFullscreenQuad
{
public:
	static void Init();
	static void Shutdown();
	static void Draw();
private:
	static ID3D11Buffer* m_quadVB;
	static ID3D11Buffer* m_quadIB;
	static CD3D1XShader* m_quadVS;
	static ID3D11InputLayout* m_quadIL;
};

