//
// Created by peter on 29.11.2020.
//

#pragma once

#include <cmath>

class Vector
{
  public:
    float x, y, z;
    float Magnitude( void ) const { return sqrt( x * x + y * y + z * z ); }
};

inline Vector operator-( const Vector &left, const Vector &right )
{
    return Vector{ left.x - right.x, left.y - right.y, left.z - right.z };
}
