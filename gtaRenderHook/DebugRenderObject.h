#pragma once
/*!
    \class DebugRenderObject

    Base class for rendering debug objects.
*/
class DebugRenderObject
{
public:
    DebugRenderObject();
    ~DebugRenderObject();
    virtual void Render() = 0;
};

