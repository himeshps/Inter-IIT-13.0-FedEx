/*
 * solver.h 
 * Header file for solver.cpp
*/

#ifndef _SOLVER_H
#define _SOLVER_H

#include "base.h"
#include "gurobi_c++.h"

class Solver
{
private:
	const double INF = 10000.0;
	const double AVG_DELAY = 170.0;

	GRBModel model;

	int n; // Number of packages
	int m; // Number of ulds

	bool priority_pack; // If true, then priority packages must be packed

	int B, H;				   // Fixed B and H for all ULDs
	vector<int> length_prefix; // Prefix sums for ULD lengths, arranged on the x axis

	vector<package> packages;
	vector<uld> ulds;
	vector<vector<vector<int>>> D;

	vector<GRBVar> Y;				 // Size n is 1 if unassigned
	vector<array<GRBVar, 6>> O;		 // Size n
	vector<array<GRBVar, 3>> coords; // Size nx3
	vector<array<GRBLinExpr, 3>> el; // Effective length

public:
	/* Solution extractor */
	/* If only assigned is true, it only returns pkgs which have been assigned */
	vector<pkg_soln> ExtractSolution(bool only_assigned = false);

	Solver(GRBEnv _model, int _n, int _m, vector<package> _packages, vector<uld> _ulds, bool _priority_pack = true);

	/* Stops solver at given objective or time if provided */
	void solve(double time_stop = -1.0, double obj_stop = -1.0);
};

#endif
