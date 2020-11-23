#pragma once
namespace rh::engine
{
enum class SyncPrimitiveType : unsigned char
{
    GPU,
    CPU,
    CPU_GPU
};

/**
 * @brief Wrapper over synchronization primitive, marks
 *
 * @tparam T 
 */
template <class T> class Awaiter
{
    T *mData;

  public:
    Awaiter( T *data ) { mData = data; }
    ~Awaiter() { mData->Free(); }
    operator T *() { return mData; }
};

class ISyncPrimitive
{
  public:
    virtual ~ISyncPrimitive() = default;
    /*virtual Awaiter<ISyncPrimitive> Await()   = 0;
    virtual void                    Capture() = 0;
    virtual void                    Free()    = 0;
    virtual bool                    IsFree()  = 0;*/
};
} // namespace rh::engine
