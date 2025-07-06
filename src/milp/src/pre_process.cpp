/*
 * Pre_process.cpp
 * Contains code to pre-process ULDs, packages
 */

#include "pre_process.h"

bool pkg_comparator(package &a, package &b)
{
	return (a.l * a.b * a.h) < (b.l * b.b * b.h);
}

ULDHeuristic::ULDHeuristic(vector<uld> _ulds)
	: ulds{_ulds}
{
}

/* ULD Heuristic 1: Does nothing, simply returns the ULDs */
vector<uld> ULDHeuristic::ULDHeuristic1()
{
	return ulds;
}

/* ULD Heuristic 2: Swaps L, B, H to H, B, L to ensure constant B, H in ULDs 1-2, 4-6 */
vector<uld> ULDHeuristic::ULDHeuristic2()
{
	vector<uld> ret(ulds);
	for (auto &e : ret)
	{
		swap(e.l, e.h);
	}
	return ret;
}

PkgHeuristic::PkgHeuristic(vector<package> _pkgs)
	: pkgs{_pkgs}
{
	sort(pkgs.begin(), pkgs.end(), pkg_comparator);

	for (auto &pkg : pkgs)
	{
		if (!pkg.priority)
			pkgs_eco.push_back(pkg);
		else
			pkgs_pri.push_back(pkg);
	}
}

/* Heuristic which returns 'cnt' top economy packages */
/* Brief description of the algo:
 *  - All packages are unmarked
 *  - Iterate i from 1 to n
 *  - Mark pkg i if there exists an unmarked pkg j < i st.
 *     - all dimensions of pkg j are smaller than pkg i in some orientation
 *     - delay cost of j is less than i
 *  - Delete n - cnt marked packages, first deleting those with larger indices
 *  - Return the remaining packages
 */
vector<package> PkgHeuristic::EcoHeuristic1(int cnt)
{
	int n = pkgs_eco.size();
	vector<int> mark(n, 0);

	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < i; ++j)
		{
			if (pkgs_eco[j].delay > pkgs_eco[i].delay &&
				is_pkg_contained(pkgs_eco[j], pkgs_eco[i]))
			{
				mark[i] = 1;
				break;
			}
		}
	}

	int rem = n - cnt;
	vector<int> del(n, 0);
	for (int i = n - 1; i >= 0 && rem > 0; --i)
	{
		if (mark[i])
		{
			del[i] = 1;
			rem--;
		}
	}

	assert(rem == 0); // If it fails, reduce cnt
	vector<package> ret;
	for (int i = 0; i < n; ++i)
	{
		if (!del[i])
			ret.push_back(pkgs_eco[i]);
	}
	return ret;
}

bool comparator(pair<int, package> &a, pair<int, package> &b)
{
	if (a.first != b.first)
		return a.first > b.first;
	package &p1 = a.second, &p2 = b.second;
	int m1 = min(p1.l, min(p1.b, p1.h));
	int m2 = min(p2.l, min(p2.b, p2.h));

	int v1 = p1.l * p1.b * p1.h;
	int v2 = p2.l * p2.b * p2.h;

	if (m1 != m2)
		return m1 < m2;
	return v1 < v2;
}

vector<package> PkgHeuristic::EcoHeuristic3(int cnt)
{
	int n = pkgs_eco.size();
	vector<int> order(n, 0);

	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			if (pkgs_eco[j].delay < pkgs_eco[i].delay &&
				is_pkg_contained(pkgs_eco[i], pkgs_eco[j]))
				order[i]++;
		}
	}

	vector<pair<int, package>> vals;
	for (int i = 0; i < n; ++i)
		vals.push_back({order[i], pkgs_eco[i]});
	sort(vals.begin(), vals.end(), comparator);

	for (auto &e : vals)
		printf("%d %d\n", e.first, e.second.id);

	vector<package> ret;
	for (int i = 0; i < cnt; ++i)
		ret.push_back(vals[i].second);
	return ret;
}

/* Heuristic 2 for economy pkgs: does nothing, simply returns all economy packages */
vector<package> PkgHeuristic::EcoHeuristic2()
{
	return pkgs_eco;
}

/* Heuristic 1 for priority pkgs: does nothing, simply returns all priority packages */
vector<package> PkgHeuristic::PriHeuristic1()
{
	return pkgs_pri;
}
