/*
 * pre_process.h - heuristic team 30
 * created on dec 1, 2024
 *
 * Header file for pre_process.c
 */

#ifndef _PRE_PROCESS_H
#define _PRE_PROCESS_H

#include <bits/stdc++.h>

#include "base.h"

using namespace std;

class ULDHeuristic
{
private:
	vector<uld> ulds;

public:
	ULDHeuristic(vector<uld> _ulds);
	ULDHeuristic() = default;

	/* ULD Heuristic 1: Does nothing, simply returns the ULDs */
	vector<uld> ULDHeuristic1();

	/* ULD Heuristic 2: Swaps L, B, H to B, H, L to ensure constant B, H in ULDs 1-2, 4-6 */
	vector<uld> ULDHeuristic2();
};

class PkgHeuristic
{
private:
	/* All packages are stored sorted by their volumes */
	vector<package> pkgs;
	vector<package> pkgs_eco;
	vector<package> pkgs_pri;

	/* Utility function for EcoHeuristic1 */
	/* Returns true is pkg a can fit inside pkg b in some orientation */
	bool is_pkg_contained(package &a, package &b)
	{
		vector<int> perm = {a.l, a.b, a.h};
		sort(perm.begin(), perm.end());

		do
		{
			if (perm[0] <= b.l && perm[1] <= b.h && perm[2] <= b.b)
				return true;
		} while (next_permutation(perm.begin(), perm.end()));
		return false;
	}

public:
	PkgHeuristic(vector<package> _pkgs);
	PkgHeuristic() = default;

	/* Heuristic which returns 'cnt' top economy packages */
	vector<package> EcoHeuristic1(int cnt);

	/* Heuristic 2 for economy pkgs: does nothing, simply returns all economy packages */
	vector<package> EcoHeuristic2();

	vector<package> EcoHeuristic3(int cnt);

	/* Heuristic 1 for priority pkgs: does nothing, simply returns all priority packages */
	vector<package> PriHeuristic1();
};

#endif
