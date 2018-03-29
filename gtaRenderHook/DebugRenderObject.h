#pragma once
// Base debug visualization class.
class DebugRenderObject
{
public:
	DebugRenderObject();
	~DebugRenderObject();
	virtual void Render()=0;
};

