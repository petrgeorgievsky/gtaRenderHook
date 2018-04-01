#pragma once
class CD3D1XBuffer
{
public:
	CD3D1XBuffer(unsigned int size, D3D11_USAGE usage, D3D11_BIND_FLAG bindingFlags, unsigned int cpuAccessFlags,
		unsigned int miscFlags=0, unsigned int elementSize=0, const D3D11_SUBRESOURCE_DATA* initialData=nullptr);
	~CD3D1XBuffer();
	void Update(void* data, int size=-1);
	ID3D11Buffer* getBuffer() { return m_pBuffer; }
protected:
	ID3D11Buffer*			m_pBuffer = nullptr;
	unsigned int m_uiSize;
};

