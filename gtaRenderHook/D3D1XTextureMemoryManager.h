#pragma once
class CD3D1XTexture;
class CD3D1XTextureMemoryManager
{
public:
	// todo: replace with faster structure
	static std::list<CD3D1XTexture*> textureList;
	static void AddNew(CD3D1XTexture* &texture);
	static void Remove(CD3D1XTexture* &texture);
	static void Shutdown();
};

