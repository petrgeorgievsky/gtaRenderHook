#ifndef D3D1XDefaultPipeline_h__
#define D3D1XDefaultPipeline_h__
#include "D3D1XPipeline.h"
class CD3D1XDefaultPipeline: public CD3D1XPipeline
{
public:
	CD3D1XDefaultPipeline();
	~CD3D1XDefaultPipeline();
	bool Instance	(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance) const;
	void Render		(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
private:
	CD3D1XShader*		m_pHS = nullptr;
	CD3D1XShader*		m_pDS = nullptr;
};
#endif // D3D1XDefaultPipeline_h__

