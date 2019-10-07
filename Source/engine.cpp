
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <queue>
#include <list>

#include "engine.h"
#include "v3ctor.h"

using namespace Gdiplus;
using namespace std;

#pragma comment (lib, "Gdiplus.lib")

#define PI 3.141592635897931
#define ALPHA 0.00001

/*** Z-BUFFER POINT ***/

// Constructors
/// Given coordinate, depth, color
zpoint::zpoint(int x, int y, double d, int col, form* l)
{
	this->col = col;
	this->x = x;
	this->y = y;
	this->d = d;
	this->l = l;
}
/// Given coordinates
zpoint::zpoint(int x, int y)
{
	this->col = 0x00000000;
	this->d = DBL_MAX;
	this->l = NULL;
	this->x = x;
	this->y = y;
}
/// Default
zpoint::zpoint()
{
	this->col = 0x00000000;
	this->d = DBL_MAX;
	this->l = NULL;
	this->x = 0;
	this->y = 0;
}

/*** ENGINE CORE ***/

// Constructors
/// Given all params
engine::engine(double prox, v3ctor& pos, v3ctor& dir, v3ctor& up, HWND& hWnd, int winx, int winy)
{
	this->forms = list<form*>();
	this->hWnd = &hWnd;
	this->prox = prox;
	this->pos = pos;
	this->dir = dir;
	this->up = up;
	setup(winx, winy);
}
/// Assume positive z is up
engine::engine(double prox, v3ctor& pos, v3ctor& dir, HWND& hWnd, int winx, int winy)
{
	this->up = v3ctor(0, 0, 1000);
	this->forms = list<form*>();
	this->hWnd = &hWnd;
	this->prox = prox;
	this->pos = pos;
	this->dir = dir;
	setup(winx, winy);
}
/// Given window dimensions and window
engine::engine(int winx, int winy, HWND& hWnd)
{
	this->dir = v3ctor(1000, 0, 0);
	this->up = v3ctor(0, 0, 1000);
	this->forms = list<form*>();
	this->pos = v3ctor(0, 0, 0);
	this->hWnd = &hWnd;
	this->prox = 1;
	setup(winx, winy);
}
/// Given window dimensions
engine::engine(int winx, int winy)
{
	this->dir = v3ctor(1000, 0, 0);
	this->up = v3ctor(0, 0, 1000);
	this->forms = list<form*>();
	this->pos = v3ctor(0, 0, 0);
	this->prox = 1;
	setup(winx, winy);
}
/// Default constructor: 500x500 pixels
engine::engine()
{
	this->dir = v3ctor(1000, 0, 0);
	this->up = v3ctor(0, 0, 1000);
	this->forms = list<form*>();
	this->pos = v3ctor(0, 0, 0);
	this->prox = 0;
	setup(500, 500);
}

// Setup engine core
void engine::setup(int winx, int winy)
{

	/// Store dimensions
	this->winx = winx;
	this->winy = winy;

	/// Initialize buffer
	buffer = vector<vector<zpoint>>(winx, vector<zpoint>(winy));
	for (int x = 0; x < winx; x++)
		for (int y = 0; y < winy; y++)
			buffer[x][y] = zpoint();

}

// Fill z buffer
void engine::fill_buff()
{

	/// Clear buffer
	delete bmp;
	bmp = new Bitmap(CreateCompatibleBitmap(GetDC(*hWnd), winx, winy), NULL);
	for (int x = 0; x < winx; x++)
		for (int y = 0; y < winy; y++)
			buffer[x][y] = zpoint(x, y);

	/// Refill buffer
	for (form* f : forms)
		f->buff();

}

/*** POINT ***/

// Constructors
/// Given all params
point::point(double x, double y, double z, int col, engine* g)
{
	this->col = col;
	this->x = x;
	this->y = y;
	this->z = z;
	this->g = g;
}
/// Given coordinate, white point
point::point(double x, double y, double z, engine* g)
{
	this->col = 0x00FFFFFF;
	this->x = x;
	this->y = y;
	this->z = z;
	this->g = g;
}
/// Default
point::point()
{
	this->col = 0x00FFFFFF;
	this->g = NULL;
	this->x = 0;
	this->y = 0;
	this->z = 0;
}

// Overloaded Operators
/// Sum of two points a + b is a new point at the sum of their coordinates
point point::operator+(const point& p) { return point(x + p.x, y + p.y, z + p.z, col, g); }
/// Difference of two points a - b is the v3ctor from a to b
v3ctor point::operator-(const point& p) { return to_v3ctor(*this) - to_v3ctor(p); }
/// Scaling a point
point point::operator*(double d) { return point(x * d, x * d, x * d, col, g); }
point point::operator/(double d) { return point(x / d, y / d, z / d, col, g); }
/// Increment and scale
void point::operator+=(const point& p)
{
	x += p.x;
	y += p.y;
	z += p.z;
}
void point::operator*=(double d)
{
	x *= d;
	y *= d;
	z *= d;
}
void point::operator/=(double d)
{
	x /= d;
	y /= d;
	z /= d;
}

// Projecting and caching the point
/// Project point onto screen
zpoint* point::screen()
{

	/// Convert point to vector and find projection onto observation line
	v3ctor p_dis = to_v3ctor(*this) - g->pos; /// Point as a displacement vector from observer
	v3ctor extend = project(p_dis, g->dir); /// Projection onto main centre line
	v3ctor plot = (p_dis - extend) * abs(g->dir) / abs(extend); /// Displacement from screen centre
	if (abs(angle(g->dir, extend) - PI) <= ALPHA)
		return NULL;

	/// Find pixel
	v3ctor dy = project(plot, g->up);
	v3ctor dx = plot - dy;
	return new zpoint((int)(g->winx / 2 + (angle(plot, cross(g->dir, g->up)) < PI / 2 ? 1 : -1) * abs(dx)), (int)(g->winy / 2 - (angle(plot, g->up) < PI / 2 ? 1 : -1) * abs(dy)), abs(p_dis), col, NULL);

}
/// Cache buffer at point
bool point::buff()
{
	zpoint* z_p = screen();
	if (z_p == NULL) {
		delete z_p;
		return FALSE;
	}
	if (z_p->x >= 0 && z_p->x < g->winx && z_p->y >= 0 && z_p->y < g->winy) {
		if (z_p->d < g->buffer[z_p->x][z_p->y].d) {
			g->bmp->SetPixel(z_p->x, z_p->y, z_p->col);
			g->buffer[z_p->x][z_p->y] = zpoint(*z_p);
		}
		delete z_p;
		return TRUE;
	}
	delete z_p;
	return FALSE;
}

/*** LINE ***/

// Constructors
/// Given all params
line::line(const point& p1, const point& p2, int col, engine* g)
{
	this->col = col;
	this->p1 = p1;
	this->p2 = p2;
	this->g = g;
}
/// Given points, make white
line::line(const point& p1, const point& p2, engine* g)
{
	this->col = 0x00FFFFFF;
	this->p1 = p1;
	this->p2 = p2;
	this->g = g;
}
/// Default
line::line()
{
	this->col = 0x00FFFFFF;
	this->p1 = point();
	this->p2 = point();
	this->g = NULL;
}

// Cache buffer along line
bool line::buff()
{
	
	/// Find unit vector along line
	v3ctor v = to_v3ctor(p2) - to_v3ctor(p1);
	v3ctor u = unit(v);

	/// Take steps with unit vector caching each
	point p = point(p1);
	p.col = col;
	for (int i = 0; i <= abs(v); i++) {
		p.buff();
		p = point(p.x + u.x, p.y + u.y, p.z + u.z, p.col, p.g);
	}
	return TRUE;

}

/*** GENERAL FILLING FUNCTION ***/

void fill_form::flood(zpoint* z)
{

	/// Recursion queue
	queue<zpoint*> q;
	q.push(z);

	/// While not empty
	while (!q.empty()) {

		/// Pop
		z = q.front();
		q.pop();

		/// End condition
		if (
			z == NULL ||
			z->x < 0 || z->x >= g->winx || z->y < 0 || z->y >= g->winy ||
			!inside(*z) ||
			g->buffer[z->x][z->y].l == this
			) {
			delete z;
			continue;
		}

		/// Cache
		if (abs(g->pos - to_v3ctor(extend(z))) < g->buffer[z->x][z->y].d && abs(angle(project(to_v3ctor(extend(z)) - g->pos, g->dir), g->dir)) <= ALPHA) {
			g->buffer[z->x][z->y] = zpoint(z->x, z->y, abs(g->pos - to_v3ctor(extend(z))), col, NULL);
			g->bmp->SetPixel(z->x, z->y, col);
		}

		/// Flood
		g->buffer[z->x][z->y].l = this;
		for (int i = -1; i <= 1; i++)
			for (int j = -1; j <= 1; j++)
				q.push(new zpoint(z->x + i, z->y + j));

		/// Cleanup
		delete z;

	}

}

/*** TRI ***/

// Constructors
/// Given all params
tri::tri(const point& p1, const point& p2, const point& p3, int col, engine* g)
{
	this->vertices[0] = p1;
	this->vertices[1] = p2;
	this->vertices[2] = p3;
	this->col = col;
	this->g = g;
}
/// Default
tri::tri()
{
	this->vertices[0] = point();
	this->vertices[1] = point();
	this->vertices[2] = point();
	this->col = 0x00FFFFFF;
	this->g = NULL;
}

// Operations for caching triangle in z-buffer
/// Find centroid
point tri::centroid()
{
	point c = point(0, 0, 0, col, g);
	for (point p : vertices)
		c += p;
	return c / 3;
}
/// Return whether point inside triangle
bool tri::inside(const zpoint& z)
{
	zpoint* c = centroid().screen();
	zpoint* v[] = { vertices[0].screen(), vertices[1].screen(), vertices[2].screen() };
	bool b =
		same_side(*c, z, *v[0], *v[1]) &&
		same_side(*c, z, *v[1], *v[2]) &&
		same_side(*c, z, *v[2], *v[0]);
	for (zpoint* z : v)
		delete z;
	delete c;
	return b;
}
/// Find point on plane of triangle at zpoint
point tri::extend(zpoint* z)
{

	/// Compute equation of plane
	v3ctor n = cross(vertices[0] - vertices[1], vertices[0] - vertices[2]);
	double d = dot(n, to_v3ctor(vertices[1]));

	/// Compute equation of line through z
	v3ctor l = v3ctor(g->dir);
	l += unit(cross(g->dir, g->up)) * (z->x - g->winx / 2);
	l -= unit(g->up) * (z->y - g->winy / 2);

	/// Find intersection
	double t = (d - dot(n, g->pos)) / dot(n, l);
	v3ctor p = g->pos + l * t;
	return point(p.x, p.y, p.z, col, g);

}
/// Cache buffer with triangle
bool tri::buff()
{
	
	/// Cache wireframe
	line(vertices[0], vertices[1], col, g).buff();
	line(vertices[1], vertices[2], col, g).buff();
	line(vertices[2], vertices[0], col, g).buff();

	/// Flood fill internals
	if (centroid().buff()) {
		flood(centroid().screen());
		return TRUE;
	}
	return FALSE;

}

/*** BIG POINTS ***/

// Constructors
/// Given all params
big_point::big_point(const point& p, int r, engine* g)
{
	this->col = p.col;
	this->p = p;
	this->r = r;
	this->g = g;
}
/// Given only point -- radius 5
big_point::big_point(const point& p, engine* g)
{
	this->col = p.col;
	this->r = 5;
	this->p = p;
	this->g = g;
}
/// Default
big_point::big_point()
{
	this->col = 0x00FFFFFF;
	this->p = point();
	this->g = NULL;
	this->r = 5;
}

// Add large point to buffer
/// Return whether zpoint within
bool big_point::inside(const zpoint& z)
{
	zpoint* z2 = p.screen();
	bool b = abs(to_v3ctor(z) - to_v3ctor(*z2)) <= r;
	delete z2;
	return b;
}
/// Use centre to approximate distance
point big_point::extend(const zpoint& z) { return p; }
/// Add to buffer
bool big_point::buff()
{
	if (p.buff()) {
		flood(p.screen());
		return TRUE;
	}
	return FALSE;
}

/*** SPHERE POINT ***/

// Constructors
/// Given all params
sphere_point::sphere_point(const point& p, int r, engine* g)
{
	this->col = p.col;
	this->p = p;
	this->r = r;
	this->g = g;
}
/// Given only point -- radius 5
sphere_point::sphere_point(const point& p, engine* g)
{
	this->col = p.col;
	this->r = 5;
	this->p = p;
	this->g = g;
}
/// Default
sphere_point::sphere_point()
{
	this->col = 0x00FFFFFF;
	this->p = point();
	this->g = NULL;
	this->r = 5;
}

// Add sphere point to buffer
/// Return whether zpoint within
bool sphere_point::inside(const zpoint& z)
{
	zpoint* z2 = p.screen();
	bool b = abs(to_v3ctor(z) - to_v3ctor(*z2)) * abs(g->pos - to_v3ctor(p)) / abs(g->dir) <= r;
	delete z2;
	return b;
}
/// Use centre to approximate distance
point sphere_point::extend(const zpoint& z) { return p; }
/// Add to buffer
bool sphere_point::buff()
{
	if (p.buff()) {
		flood(p.screen());
		return TRUE;
	}
	return FALSE;
}

/*** VECTOR MATH ***/
/// Return whether two points are on same side of line
bool same_side(const zpoint& p1, const zpoint& p2, const zpoint& a, const zpoint& b) {
	return
		cross(to_v3ctor(b) - to_v3ctor(a), to_v3ctor(p1) - to_v3ctor(a)).z *
		cross(to_v3ctor(b) - to_v3ctor(a), to_v3ctor(p2) - to_v3ctor(a)).z >=
		0;
}
/// Convert point to v3ctor
v3ctor to_v3ctor(const const point& p) { return v3ctor(p.x, p.y, p.z); }
v3ctor to_v3ctor(const const zpoint& z) { return v3ctor(z.x, z.y, 0); }