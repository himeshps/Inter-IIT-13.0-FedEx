/*
 * base.h
 * Contains basic data structure definitions
 */

#ifndef _BASE_H
#define _BASE_H

#include <bits/stdc++.h>

using namespace std;

typedef struct package
{
	int id;
	int l, b, h; // length, breadth, height
	int weight;
	bool priority;
	int delay;

	package(int _id, int _l, int _b, int _h, int _weight, bool _priority, int _delay)
		: id{_id}, l{_l}, b{_b}, h{_h}, weight{_weight},
		  priority{_priority}, delay{_delay}
	{
	}
} package;

typedef struct uld
{
	int id;
	int l, b, h;
	int max_weight;

	uld(int _id, int _l, int _b, int _h, int _max_weight)
		: id{_id}, l{_l}, b{_b}, h{_h}, max_weight{_max_weight}
	{
	}
} uld;

typedef struct pkg_soln
{
	int id;
	bool assigned;
	int uld_id;
	double sx, sy, sz; // Starting x, y, z
	double ex, ey, ez; // Coordinates of corner opposite to (sx, sy, sz); ex >= sx, ey >= sy, ez >= sz

	pkg_soln() = default;
	pkg_soln(int _id, bool _assigned, int _uld_id, double _sx, double _sy, double _sz,
			 double _ex, double _ey, double _ez)
		: id{_id}, assigned{_assigned}, uld_id{_uld_id},
		  sx{_sx}, sy{_sy}, sz{_sz},
		  ex{_ex}, ey{_ey}, ez{_ez}
	{
	}
} pkg_soln;

#endif
