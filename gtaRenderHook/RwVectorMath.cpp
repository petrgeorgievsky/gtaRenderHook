#include "stdafx.h"
#include "RwVectorMath.h"

using namespace RW;

/*
*	3D Vectors
*/

V3d::V3d()
{
	x_ = y_ = z_ = 0;
}

V3d::V3d(const RwV3d & vec)
{
	x_ = vec.x;
	y_ = vec.y;
	z_ = vec.z;
}


V3d::V3d(const float& x, const float& y, const float& z)
{
	x_ = x;
	y_ = y;
	z_ = z;
}


V3d V3d::operator+(const V3d & b)
{
	return V3d(x_+b.x_, y_ + b.y_, z_ + b.z_);
}


V3d V3d::operator-(const V3d & b)
{
	return V3d(x_ - b.x_, y_ - b.y_, z_ - b.z_);
}


V3d V3d::operator-() const
{
	return V3d(-x_, -y_, -z_);
}


V3d V3d::operator*(const float & b)
{
	return V3d(x_ * b, y_ * b, z_ * b);
}


V3d V3d::operator/(const float & b)
{
	return V3d(x_ / b, y_ / b, z_ / b);
}


float V3d::dot(const V3d & b)
{
	return x_ * b.x_ + y_ * b.y_ + z_ * b.z_;
}


V3d V3d::cross(const V3d & b)
{
	return V3d(y_*b.z_ - z_*b.y_, z_*b.x_ - x_*b.z_, x_*b.y_ - y_*b.x_);
}

V3d V3d::operator*(const Matrix & mat)
{
	V3d col_0 = { mat.getRight().getX(), mat.getUp().getX(),mat.getAt().getX() };
	V3d col_1 = { mat.getRight().getY(), mat.getUp().getY(),mat.getAt().getY() };
	V3d col_2 = { mat.getRight().getZ(), mat.getUp().getZ(),mat.getAt().getZ() };
	return V3d(dot(col_0), dot(col_1), dot(col_2));
}


// TODO: Add fast approximation option for those who want it super fast(or less square roots)
float V3d::length()
{
	return sqrt(x_ * x_ + y_ * y_ + z_ * z_);
}


void V3d::normalize()
{
	float len = length();
	x_ /= len;	y_ /= len;	z_ /= len;
}


RwV3d V3d::getRWVector()
{
	return RwV3d{ x_, y_, z_ };
}

/*
*	4D Vectors
*/


V4d::V4d()
{
	x_ = y_ = z_ = w_ = 0;
}


V4d::V4d(const RwV4d & vec)
{
	x_ = vec.x;
	y_ = vec.y;
	z_ = vec.z;
	w_ = vec.w;
}

V4d::V4d(const V3d & vec, const float & w)
{
	x_ = vec.getX();
	y_ = vec.getY();
	z_ = vec.getZ();
	w_ = w;
}


V4d::V4d(const float& x, const float& y, const float& z, const float& w)
{
	x_ = x;
	y_ = y;
	z_ = z;
	w_ = w;
}


V4d V4d::operator+(const V4d & b)
{
	return V4d(x_ + b.x_, y_ + b.y_, z_ + b.z_, w_ + b.w_);
}


V4d V4d::operator-(const V4d & b)
{
	return V4d(x_ - b.x_, y_ - b.y_, z_ - b.z_, w_ - b.w_);
}

V4d V4d::operator-() const
{
	return V4d(-x_, -y_, -z_, -w_);
}

V4d V4d::operator*(const float & b)
{
	return V4d(x_ * b, y_ * b, z_ * b, w_ * b);
}


V4d V4d::operator/(const float & b)
{
	return V4d(x_ / b, y_ / b, z_ / b, w_ / b);
}


float V4d::dot(const V4d & b)
{
	return x_ * b.x_ + y_ * b.y_ + z_ * b.z_ + w_ * b.w_;
}

V4d V4d::operator*(const Matrix & mat)
{
	V4d col_0 = { mat.getRight().getX(), mat.getUp().getX(),mat.getAt().getX(),mat.getPos().getX() };
	V4d col_1 = { mat.getRight().getY(), mat.getUp().getY(),mat.getAt().getY(),mat.getPos().getY() };
	V4d col_2 = { mat.getRight().getZ(), mat.getUp().getZ(),mat.getAt().getZ(),mat.getPos().getZ() };
	V4d col_3 = { mat.getRight().getW(), mat.getUp().getW(),mat.getAt().getW(),mat.getPos().getW() };
	return V4d(dot(col_0), dot(col_1), dot(col_2), dot(col_3));
}


// It is V3d cross product with w = 1.0
V4d V4d::cross(const V4d & b)
{
	return V4d(y_*b.z_ - z_*b.y_, z_*b.x_ - x_*b.z_, x_*b.y_ - y_*b.x_, 0.0);
}


// TODO: Add fast approximation option for those who want it super fast(or less square roots)
float V4d::length()
{
	return sqrt(x_ * x_ + y_ * y_ + z_ * z_ + w_ * w_);
}


void V4d::normalize()
{
	float len = length();
	x_ /= len;	y_ /= len;	z_ /= len; w_ /= len;
}


RwV4d V4d::getRWVector()
{
	return RwV4d{ x_, y_, z_, w_ };
}

RwV3d RW::V4d::getRW3Vector()
{
	return RwV3d{ x_, y_, z_ };
}

RW::Matrix::Matrix()
{
	right_ = { };
	up_ = {  };
	at_ = {  };
	pos_ = {  };
}

Matrix::Matrix(const RwMatrix & mat)
{
	RwUInt32 rightW = mat.flags;
	RwUInt32 upW = mat.pad1;
	RwUInt32 atW = mat.pad2;
	RwUInt32 posW = mat.pad3;
	right_ = { mat.right.x,mat.right.y,mat.right.z, reinterpret_cast<float &>(rightW) };
	up_ = { mat.up.x,mat.up.y,mat.up.z,reinterpret_cast<float &>(upW) };
	at_ = { mat.at.x,mat.at.y,mat.at.z,reinterpret_cast<float &>(atW) };
	pos_ = { mat.pos.x,mat.pos.y,mat.pos.z,reinterpret_cast<float &>(posW) };
}

Matrix::Matrix(const V4d & right, const V4d & up, const V4d & at, const V4d & pos)
{
	right_ = right;
	up_ = up;
	at_ = at;
	pos_ = pos;
}

Matrix Matrix::operator*(const Matrix & mat)
{
	V4d col_0 = { mat.right_.getX(), mat.up_.getX(),mat.at_.getX(),mat.pos_.getX() };
	V4d col_1 = { mat.right_.getY(), mat.up_.getY(),mat.at_.getY(),mat.pos_.getY() };
	V4d col_2 = { mat.right_.getZ(), mat.up_.getZ(),mat.at_.getZ(),mat.pos_.getZ() };
	V4d col_3 = { mat.right_.getW(), mat.up_.getW(),mat.at_.getW(),mat.pos_.getW() };
	return {	V4d{ right_.dot(col_0) ,right_.dot(col_1) ,right_.dot(col_2) ,right_.dot(col_3) } ,
				V4d{ up_.dot(col_0) ,up_.dot(col_1) ,up_.dot(col_2) ,up_.dot(col_3) } ,
				V4d{ at_.dot(col_0) ,at_.dot(col_1) ,at_.dot(col_2) ,at_.dot(col_3) } ,
				V4d{ pos_.dot(col_0) ,pos_.dot(col_1) ,pos_.dot(col_2) ,pos_.dot(col_3) } };
}

Matrix Matrix::inverse()
{
	float det = right_.dot(up_.cross(at_));

	return { 
			V4d(   up_.getY()*at_.getZ() - up_.getZ()*at_.getY(),
				right_.getZ()*at_.getY() - right_.getY()*at_.getZ(),
				right_.getY()*up_.getZ() - right_.getZ()*up_.getY(),0.0 )/det,

			V4d(   up_.getZ()*at_.getX() - up_.getX()*at_.getZ(),
				right_.getX()*at_.getZ() - right_.getZ()*at_.getX(),
				right_.getZ()*up_.getX() - right_.getX()*up_.getZ(),0.0 ) / det,

			V4d(   up_.getX()*at_.getY() - up_.getY()*at_.getX(),
				right_.getY()*at_.getX() - right_.getX()*at_.getY(),
				right_.getX()*up_.getY() - right_.getY()*up_.getX(),0.0) / det, {}
	};
}

RwMatrix RW::Matrix::getRWMatrix()
{
	RwMatrix mat;
	float rightW = right_.getW();
	float upW = up_.getW();
	float atW = at_.getW();
	float posW = pos_.getW();
	mat.right = right_.getRW3Vector();
	mat.flags = reinterpret_cast<RwUInt32&>(rightW);
	mat.up = up_.getRW3Vector();
	mat.pad1 = reinterpret_cast<RwUInt32&>(upW);
	mat.at = at_.getRW3Vector();
	mat.pad2 = reinterpret_cast<RwUInt32&>(atW);
	mat.pos = pos_.getRW3Vector();
	mat.pad3 = reinterpret_cast<RwUInt32&>(posW);
	return mat;
}

BBox::BBox()
{
	min_ = { FLT_MAX,FLT_MAX,FLT_MAX };
	max_ = { -FLT_MAX,-FLT_MAX,-FLT_MAX };
}

BBox::BBox(const RwBBox & bbox)
{
	min_ = { bbox.inf };
	max_ = { bbox.sup };
}

BBox::BBox(const V3d & min, const V3d & max)
{
	min_ = min;
	max_ = max;
}

BBox::BBox(const V3d * points, UINT count)
{
	min_ = { FLT_MAX,FLT_MAX,FLT_MAX };
	max_ = { -FLT_MAX,-FLT_MAX,-FLT_MAX };
	for (UINT i = 0; i < count; i++) {
		min_ = { min(points[i].getX(),min_.getX()),min(points[i].getY(),min_.getY()),min(points[i].getZ(),min_.getZ()) };
		max_ = { max(points[i].getX(),max_.getX()),max(points[i].getY(),max_.getY()),max(points[i].getZ(),max_.getZ()) };
	}
}

std::vector<V3d> RW::BBox::getVerticles()
{
	std::vector<V3d> result;
	result.push_back(max_);
	result.push_back(min_);
	result.push_back({ min_.getX(), max_.getY(), max_.getZ() });
	result.push_back({ max_.getX(), min_.getY(), max_.getZ() });
	result.push_back({ max_.getX(), max_.getY(), min_.getZ() });

	result.push_back({ max_.getX(), min_.getY(), min_.getZ() });
	result.push_back({ min_.getX(), max_.getY(), min_.getZ() });
	result.push_back({ min_.getX(), min_.getY(), max_.getZ() });
	return result;
}

bool BBox::inside(const V3d & pos)
{
	float x = pos.getX();
	float y = pos.getY();
	float z = pos.getZ();
	return	x >= min_.getX() && x <= max_.getX() &&
		y >= min_.getY() && y <= max_.getY() &&
		z >= min_.getZ() && z <= max_.getZ();
}

bool BBox::intersects(const BBox & b)
{
	if (max_.getX() < b.min_.getX()) return false; // a is left of b
	if (min_.getX() > b.max_.getX()) return false; // a is right of b
	if (max_.getY() < b.min_.getY()) return false; // a is above b
	if (min_.getY() > b.max_.getY()) return false; // a is below b
	if (max_.getZ() < b.min_.getZ()) return false; // a is above b
	if (min_.getZ() > b.max_.getZ()) return false; // a is below b
	return true; // boxes overlap
}

bool BBox::inside2D(const V3d & pos)
{
	float x = pos.getX();
	float y = pos.getY();
	return	x >= min_.getX() && x <= max_.getX() &&
		y >= min_.getY() && y <= max_.getY();
}

bool BBox::intersects2D(const BBox & b)
{
	if (max_.getX() < b.min_.getX()) return false; // a is left of b
	if (min_.getX() > b.max_.getX()) return false; // a is right of b
	if (max_.getY() < b.min_.getY()) return false; // a is above b
	if (min_.getY() > b.max_.getY()) return false; // a is below b
	return true; // boxes overlap
}

void RW::BBox::extend(const V3d & pos)
{
	max_ = { max(max_.getX(),pos.getX()) ,max(max_.getY(),pos.getY()) ,max(max_.getZ(),pos.getZ()) };
	min_ = { min(min_.getX(),pos.getX()) ,min(min_.getY(),pos.getY()) ,min(min_.getZ(),pos.getZ()) };
}

void RW::BBox::extendZ(const V3d & pos)
{
	max_ = { max_.getX() ,max_.getY() ,max(max_.getZ(),pos.getZ()) };
	min_ = { min_.getX(),min_.getY() ,min(min_.getZ(),pos.getZ()) };
}

void RW::BBox::operator+=(const V3d & b)
{
	max_ = max_ + b;
	min_ = min_ + b;
}

V3d BBox::getCenter()
{
	return (max_ + min_)*0.5f;
}

V3d BBox::getTopCenter()
{
	V3d center = getCenter();
	return { center.getX(), center.getY(), max_.getZ() };
}

float BBox::getSizeX() const
{
	return max_.getX() - min_.getX();
}

float BBox::getSizeY() const
{
	return max_.getY() - min_.getY();
}

float BBox::getSizeZ() const
{
	return max_.getZ() - min_.getZ();
}
