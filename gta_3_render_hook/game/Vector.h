//
// Created by peter on 29.11.2020.
//

#pragma once

#include <cmath>
#include <common_headers.h>

class Vector
{
  public:
    float x, y, z;
    Vector( float _x, float _y, float _z ) : x( _x ), y( _y ), z( _z ) {}
    Vector( const RwV3d &from ) : x( from.x ), y( from.y ), z( from.z ) {}
    Vector( RwV3d &&from ) : x( from.x ), y( from.y ), z( from.z ) {}
    float Magnitude( void ) const { return sqrt( x * x + y * y + z * z ); }
};

inline Vector operator-( const Vector &left, const Vector &right )
{
    return Vector{ left.x - right.x, left.y - right.y, left.z - right.z };
}

inline Vector operator+( const Vector &left, const Vector &right )
{
    return Vector{ left.x + right.x, left.y + right.y, left.z + right.z };
}

inline Vector operator*( const Vector &left, float right )
{
    return Vector{ left.x * right, left.y * right, left.z * right };
}
