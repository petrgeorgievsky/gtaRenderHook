#include "stdafx.h"
#include "D3D1XVertexBufferManager.h"


std::list<CD3D1XVertexBuffer*> CD3D1XVertexBufferManager::bufferList = {};

void CD3D1XVertexBufferManager::AddNew(CD3D1XVertexBuffer*& buf)
{
	bufferList.push_back(buf);
}

void CD3D1XVertexBufferManager::Remove(CD3D1XVertexBuffer*& buf)
{
	bufferList.remove(buf);
	delete buf;
	buf = nullptr;
}

void CD3D1XVertexBufferManager::Shutdown()
{
	for (auto &buf : bufferList)
	{
		delete buf;
		buf = nullptr;
	}
}
