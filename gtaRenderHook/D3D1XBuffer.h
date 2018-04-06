#pragma once
/*!
	\class CD3D1XBuffer
	\brief Base D3D hardware buffer class.

	This class represents GPU memory buffer.
*/
class CD3D1XBuffer
{
public:
	/*!
		\brief Initializes D3D hardware buffer.

		Allocates hardware buffer of some non-negative size(in bytes) with custom usage, 
		binding, cpu access, flags, element structure size(in bytes) and initial data if provided.
	*/
	CD3D1XBuffer(unsigned int size, D3D11_USAGE usage, D3D11_BIND_FLAG bindingFlags, unsigned int cpuAccessFlags,
		unsigned int miscFlags=0, unsigned int elementSize=0, const D3D11_SUBRESOURCE_DATA* initialData=nullptr);

	/*!
		Releases memory held by hardware buffer.
	*/
	~CD3D1XBuffer();

	/*!
		\brief Updates data inside buffer.

		If size is negative, than all buffer data will be updated, otherwise only part of it.
	*/
	void Update(void* data, int size=-1);

	/*!
		Sets debugging info for this buffer
	*/
	void SetDebugName(std::string name);

	/*!
		Returns d3d buffer pointer
	*/
	ID3D11Buffer* getBuffer() const { return m_pBuffer; }
protected:
	ID3D11Buffer*	m_pBuffer = nullptr;
	unsigned int	m_uiSize;
	std::string		m_sDebugName;
};

