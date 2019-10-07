
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <queue>
#include <list>

#include "v3ctor.h"

using namespace Gdiplus;
using namespace std;

#pragma comment (lib,"Gdiplus.lib")

#ifndef ENGINE_H
#define ENGINE_H

class form;

struct zpoint
{
public:

	int x, y;
	double d;
	form* l;
	int col;

	zpoint(int, int, double, int, form*);
	zpoint(int, int);
	zpoint();

};

struct engine
{
public:

	vector<vector<zpoint>> buffer;
	list<form*> forms;
	Bitmap *bmp;
	HWND *hWnd;

	double prox;
	v3ctor pos;
	v3ctor dir;
	v3ctor up;
	int winx;
	int winy;

	engine(double, v3ctor&, v3ctor&, v3ctor&, HWND&, int, int);
	engine(double, v3ctor&, v3ctor&, HWND&, int, int);
	engine(int, int, HWND&);
	engine(int, int);
	engine();

	void setup(int, int);
	void fill_buff();

};

class form
{
public:

	engine *g;
	int col;

	virtual bool buff() { return FALSE; };

};

class point : public form
{
public:

	double x, y, z;

	point(double, double, double, int, engine*);
	point(double, double, double, engine*);
	point();

	v3ctor operator-(const point&);
	point operator+(const point&);
	point operator*(double);
	point operator/(double);
	void operator+=(const point&);
	void operator*=(double);
	void operator/=(double);

	zpoint* screen();
	bool buff();

};

class line : public form
{
public:

	point p1, p2;

	line(const point&, const point&, int, engine*);
	line(const point&, const point&, engine*);
	line();

	bool buff();

};

class fill_form : public form
{
public:

	virtual point extend(zpoint*) { return point(); };
	virtual bool inside(const zpoint&) { return FALSE; };

	void flood(zpoint*);

};


class tri : public fill_form
{
public:

	point vertices[3];

	tri(const point&, const point&, const point&, int, engine*);
	tri();

	point extend(zpoint*);
	bool inside(const zpoint&);
	point centroid();
	bool buff();

};

class big_point : public fill_form
{
public:

	point p;
	int r;

	big_point(const point&, int, engine*);
	big_point(const point&, engine*);
	big_point();

	point extend(const zpoint&);
	bool inside(const zpoint&);
	bool buff();

};

class sphere_point : public fill_form
{
public:

	point p;
	int r;

	sphere_point(const point&, int, engine*);
	sphere_point(const point&, engine*);
	sphere_point();

	point extend(const zpoint&);
	bool inside(const zpoint&);
	bool buff();

};

bool same_side(const zpoint&, const zpoint&, const zpoint&, const zpoint&);
v3ctor to_v3ctor(const const zpoint&);
v3ctor to_v3ctor(const const point&);

#endif