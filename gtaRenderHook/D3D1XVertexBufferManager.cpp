#include "stdafx.h"
#include "D3D1XVertexBufferManager.h"


std::list<ID3D11Buffer*> CD3D1XVertexBufferManager::bufferList = {};

void CD3D1XVertexBufferManager::AddNew(ID3D11Buffer*& tex)
{
	bufferList.push_back(tex);
}

void CD3D1XVertexBufferManager::Remove(ID3D11Buffer*& tex)
{
	bufferList.remove(tex);
	tex->Release();
	tex = nullptr;
}

void CD3D1XVertexBufferManager::Shutdown()
{
	for (auto &tex : bufferList)
	{
		tex->Release();
		tex = nullptr;
	}
}
