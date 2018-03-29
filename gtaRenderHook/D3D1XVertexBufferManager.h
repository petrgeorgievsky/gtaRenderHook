#pragma once
class CD3D1XVertexBufferManager
{
public:
	static std::list<ID3D11Buffer*> bufferList;
	static void AddNew(ID3D11Buffer* &buffer);
	static void Remove(ID3D11Buffer* &buffer);
	static void Shutdown();
};

