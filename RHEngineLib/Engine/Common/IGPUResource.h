#pragma once

namespace rh::engine {
class IGPUResource
{
public:
    virtual ~IGPUResource() = default;
    virtual void *GetImplResource() { return nullptr; }
};
class GPUResourcePtr
{
public:
    GPUResourcePtr() { mPtr = nullptr; }
    GPUResourcePtr( IGPUResource *resource ) { mPtr = resource; }
    ~GPUResourcePtr() { delete mPtr; }

    GPUResourcePtr( const GPUResourcePtr & ) = delete;
    GPUResourcePtr &operator=( const GPUResourcePtr & ) = delete;
    GPUResourcePtr( GPUResourcePtr &&other )
    {
        mPtr = other.mPtr;
        other.mPtr = nullptr;
    }
    GPUResourcePtr &operator=( GPUResourcePtr &&other )
    {
        mPtr = other.mPtr;
        other.mPtr = nullptr;
        return *this;
    }

    IGPUResource *mPtr;
};
} // namespace rh::engine
