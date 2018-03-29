#pragma once
class CD3D1XVertexBuffer
{
public:
	CD3D1XVertexBuffer();
	~CD3D1XVertexBuffer();
	CComPtr<ID3D11Buffer>			m_pBuffer = nullptr;
};

