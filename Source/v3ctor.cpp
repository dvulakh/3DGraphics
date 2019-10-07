
#include "v3ctor.h"

#include <string>
#include <cmath>

#define PI 3.141592635897931

// Constructors
/// Given components
v3ctor::v3ctor(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}
/// Default
v3ctor::v3ctor() {
	this->x = 0;
	this->y = 0;
	this->z = 0;
}

// Overloaded operators
/// Vector increment
v3ctor v3ctor::operator+=(const v3ctor& v)
{
	v3ctor s = v3ctor(*this);
	this->x += v.x;
	this->y += v.y;
	this->z += v.z;
	return s;
}
/// Vector decrement
v3ctor v3ctor::operator-=(const v3ctor& v)
{
	v3ctor s = v3ctor(*this);
	this->x -= v.x;
	this->y -= v.y;
	this->z -= v.z;
	return s;
}
/// Vector add
v3ctor v3ctor::operator+(const v3ctor& v) const
{
	v3ctor s = v3ctor();
	s.x = this->x + v.x;
	s.y = this->y + v.y;
	s.z = this->z + v.z;
	return s;
}
/// Vector subtract
v3ctor v3ctor::operator-(const v3ctor& v) const
{
	v3ctor s = v3ctor();
	s.x = this->x - v.x;
	s.y = this->y - v.y;
	s.z = this->z - v.z;
	return s;
}
/// Scalar multiply
v3ctor v3ctor::operator*(double d) const
{
	v3ctor s = v3ctor();
	s.x = this->x * d;
	s.y = this->y * d;
	s.z = this->z * d;
	return s;
}
/// Scalar divide
v3ctor v3ctor::operator/(double d) const
{
	v3ctor s = v3ctor();
	s.x = this->x / d;
	s.y = this->y / d;
	s.z = this->z / d;
	return s;
}
/// Scalar multiply and assign
v3ctor v3ctor::operator*=(double d)
{
	v3ctor s = v3ctor(*this);
	this->x *= d;
	this->y *= d;
	this->z *= d;
	return s;
}
/// Scalar divide and assign
v3ctor v3ctor::operator/=(double d)
{
	v3ctor s = v3ctor(*this);
	this->x /= d;
	this->y /= d;
	this->z /= d;
	return s;
}

// Basic math
/// Magnitude of vector
double abs(const v3ctor& v) { return sqrt(dot(v, v)); }
/// Dot product
double dot(const v3ctor& v, const v3ctor& w) { return v.x * w.x + v.y * w.y + v.z * w.z; }
/// Cross product
v3ctor cross(const v3ctor& v, const v3ctor& w) {
	return v3ctor(
		v.y * w.z - v.z * w.y,
		v.z * w.x - v.x * w.z,
		v.x * w.y - v.y * w.x
	);
}
/// Angle between two vectors
double angle(const v3ctor& v, const v3ctor& w) {
	double c = dot(v, w) / abs(v) / abs(w);
	return c >= -1 ? c <= 1 ? acos(c) : acos(1) : acos(-1);
}
/// Projection of v onto w
v3ctor project(const v3ctor& v, const v3ctor& w) { return w * (dot(v, w) / dot(w, w)); }
/// Unit vector in direction of v
v3ctor unit(const v3ctor& v) { return v / abs(v); }
/// Rotate vector v about u by r radians
v3ctor rot(const v3ctor& v, v3ctor u, double r)
{
	u = unit(u);
	double c = cos(r);
	double s = sin(r);
	return v3ctor(
		v.x * (c + u.x * u.x * (1 - c)) + v.y * (u.x * u.y * (1 - c) - u.z * s) + v.z * (u.x * u.z * (1 - c) + u.y * s),
		v.x * (u.y * u.x * (1 - c) + u.z * s) + v.y * (c + u.y * u.y * (1 - c)) + v.z * (u.y * u.z * (1 - c) - u.x * s),
		v.x * (u.z * u.x * (1 - c) - u.y * s) + v.y * (u.z * u.y * (1 - c) + u.x * s) + v.z * (c + u.z * u.z * (1 - c))
	);
}

// To string
string to_string(const v3ctor& v)
{
	string s = "<";
	s += to_string(v.x) + ", ";
	s += to_string(v.y) + ", ";
	s += to_string(v.z);
	return s + ">";
}