#pragma once
class SampHaxx
{
  public:
    static void Patch();
};
template <typename T>
class Wrapper
{
  public:
    T *Get() { return mData; }
    static Wrapper<T> &Inst();
  private:
    Wrapper();
    ~Wrapper();
    T *mData;
};