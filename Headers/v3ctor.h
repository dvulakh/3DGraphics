
#include <string>

using namespace std;

#ifndef V3CTOR_H
#define V3CTOR_H

struct v3ctor
{
public:

	double x, y, z;

	v3ctor(double, double, double);
	v3ctor();

	v3ctor operator+(const v3ctor&) const;
	v3ctor operator-(const v3ctor&) const;
	v3ctor operator+=(const v3ctor&);
	v3ctor operator-=(const v3ctor&);
	v3ctor operator*(double) const;
	v3ctor operator/(double) const;
	v3ctor operator*=(double);
	v3ctor operator/=(double);

};

v3ctor rot(const v3ctor&, v3ctor, double);
v3ctor project(const v3ctor&, const v3ctor&);
v3ctor cross(const v3ctor&, const v3ctor&);
double angle(const v3ctor&, const v3ctor&);
double dot(const v3ctor&, const v3ctor&);
v3ctor unit(const v3ctor&);
double abs(const v3ctor&);

string to_string(const v3ctor&);

#endif