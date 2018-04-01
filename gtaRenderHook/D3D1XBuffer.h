#pragma once
class CD3D1XBuffer
{
public:
	CD3D1XBuffer(unsigned int size, D3D11_USAGE usage, D3D11_BIND_FLAG bindingFlags,D3D11_CPU_ACCESS_FLAG cpuAccessFlags,
		unsigned int miscFlags=0, unsigned int elementSize=0);
	~CD3D1XBuffer();
	void Update(void* data, int size=-1);
	ID3D11Buffer* getBuffer() { return m_pBuffer; }
protected:
	ID3D11Buffer*			m_pBuffer = nullptr;
	unsigned int m_uiSize;
};

