#pragma once
/*
*	Render ware vector math wraper for C++.
*	Allows to perform vector operations using C++ classes instead of standard C structure way.
*/
namespace RW
{
    class Matrix;
    // 2D Vector class
    class V2d
    {
    public:
        V2d( const RwV2d& vec );
        V2d( const float& x, const float& y );
        V2d operator+( const V2d& b );
        V2d operator-( const V2d& b );
        V2d operator*( const float& b );
        V2d operator/( const float& b );
        float dot( const V2d& b );
        V2d cross( const V2d& b );
        float length();
        void normalize();
    private:
        float x_, y_;
    };

    // 3D Vector class
    class V3d
    {
    public:
        V3d();
        V3d( const RwV3d& vec );
        V3d( const float& x, const float& y, const float& z );
        V3d operator+( const V3d& b );
        V3d operator-( const V3d& b );
        V3d operator-() const;
        V3d operator*( const float& b );
        V3d operator/( const float& b );
        V3d cross( const V3d& b );
        float dot( const V3d& b );
        V3d operator*( const Matrix& mat );
        float length();
        void normalize();
        RwV3d getRWVector();
        float getX() const { return x_; }
        float getY() const { return y_; }
        float getZ() const { return z_; }
    private:
        float x_, y_, z_;
    };

    // 4D Vector class
    class V4d
    {
    public:
        V4d();
        V4d( const RwV4d& vec );
        V4d( const V3d& vec, const float& w = 0 );
        V4d( const float& x, const float& y, const float& z, const float& w );
        V4d operator+( const V4d& b );
        V4d operator-( const V4d& b );
        V4d operator-() const;
        V4d operator*( const float& b );
        V4d operator/( const float& b );
        V4d cross( const V4d& b );
        float dot( const V4d& b );
        V4d operator*( const Matrix& mat );
        float length();
        void normalize();
        RwV4d getRWVector();
        RwV3d getRW3Vector();
        float getX() const { return x_; }
        float getY() const { return y_; }
        float getZ() const { return z_; }
        float getW() const { return w_; }
    private:
        float x_, y_, z_, w_;
    };

    // 4x4 Matrix
    class Matrix
    {
    public:
        Matrix();
        Matrix( const RwMatrix& mat );
        Matrix( const V4d& right, const V4d& up, const V4d& at, const V4d& pos );

        Matrix operator*( const Matrix& mat );
        Matrix inverse();
        V4d getRight() const { return right_; }
        V3d getRightv3() { return V3d{ right_.getRW3Vector() }; }
        V4d getUp() const { return up_; }
        V3d getUpv3() { return V3d{ up_.getRW3Vector() }; }
        V4d getAt() const { return at_; }
        V3d getAtv3() { return V3d{ at_.getRW3Vector() }; }
        V4d& getPos() { return pos_; }
        V4d getPos() const { return pos_; }
        RwMatrix getRWMatrix();
    private:
        V4d right_;
        V4d up_;
        V4d at_;
        V4d pos_;
    };

    // Bounding Box
    class BBox
    {
    public:
        BBox();
        BBox( const RwBBox& mat );
        BBox( const V3d& min, const V3d& max );
        BBox( const V3d* points, UINT count );
        std::vector<V3d> getVerticles();
        bool inside( const V3d& pos );
        bool inside2D( const V3d& pos );
        bool intersects( const BBox& b );
        bool intersects2D( const BBox& b );
        void extend( const V3d& pos );
        void extendZ( const V3d& pos );
        void operator+=( const V3d& b );
        V3d getMin() const { return min_; }
        V3d getMax() const { return max_; }
        V3d getCenter();
        V3d getTopCenter();
        float getSizeX() const;
        float getSizeY() const;
        float getSizeZ() const;
    private:
        V3d min_;
        V3d max_;
    };
}

