#pragma once
#include "D3D1XVertexBuffer.h"
class CD3D1XVertexBufferManager
{
public:
	static std::list<CD3D1XVertexBuffer*> bufferList;
	static void AddNew(CD3D1XVertexBuffer* &buffer);
	static void Remove(CD3D1XVertexBuffer* &buffer);
	static void Shutdown();
};

