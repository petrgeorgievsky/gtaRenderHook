// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "stdafx.h"
#include "D3D1XTextureMemoryManager.h"
#include "D3D1XTexture.h"
std::list<CD3D1XTexture*> CD3D1XTextureMemoryManager::textureList = {};

void CD3D1XTextureMemoryManager::AddNew(CD3D1XTexture* &tex)
{
	textureList.push_back(tex);
}

void CD3D1XTextureMemoryManager::Remove(CD3D1XTexture * &tex)
{
	textureList.remove(tex);
	delete tex;
}

void CD3D1XTextureMemoryManager::Shutdown()
{
	for (auto &tex : textureList)
		delete tex;
	
}
