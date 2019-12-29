#pragma once

class BilateralBlurPass
{
public:
    BilateralBlurPass();
    void *Blur( void *image, void *gbNormals, void *gbPos );

private:
    void *mBlurCS;
    void *mBlurV;
    void *mResult;
};
