/*
 * solver.cpp
 * Contains code for solver which gives optimal packing for given pkgs in given ULDs
 */

#include "solver.h"

vector<pkg_soln> Solver::ExtractSolution(bool only_assigned)
{
	vector<pkg_soln> ret;
	for (int i = 0; i < n; ++i)
	{
		int uld = -1;
		pkg_soln to_add;
		to_add.id = packages[i].id;
		for (int j = 0; j < m; ++j)
		{
			if (coords[i][0].get(GRB_DoubleAttr_X) < length_prefix[j])
			{
				uld = j;
				break;
			}
		}

		if (Y[i].get(GRB_DoubleAttr_X) > 0.5)
		{
			to_add.assigned = false;
			to_add.uld_id = -1;
			if (!only_assigned)
				ret.push_back(to_add);
			continue;
		}
		else
		{
			to_add.assigned = true;
			to_add.uld_id = ulds[uld].id;
			assert(uld != -1);
		}

		to_add.sx = coords[i][0].get(GRB_DoubleAttr_X);
		to_add.sy = coords[i][1].get(GRB_DoubleAttr_X);
		to_add.sz = coords[i][2].get(GRB_DoubleAttr_X);

		to_add.ex = to_add.sx + el[i][0].getValue();
		to_add.ey = to_add.sy + el[i][1].getValue();
		to_add.ez = to_add.sz + el[i][2].getValue();

		if (uld)
		{
			to_add.sx -= length_prefix[uld - 1];
			to_add.ex -= length_prefix[uld - 1];
		}

		ret.push_back(to_add);
	}
	return ret;
}

Solver::Solver(GRBEnv _model, int _n, int _m, vector<package> _packages, vector<uld> _ulds, bool _priority_pack)
	: model{_model},
	  n{_n},
	  m{_m},
	  packages{_packages},
	  ulds{_ulds},
	  priority_pack{_priority_pack}
{
	assert((int)packages.size() == n);
	assert((int)ulds.size() == m);

	B = ulds[0].b, H = ulds[0].h;
	for (int i = 1; i < m; ++i)
	{
		assert(ulds[i].b == B);
		assert(ulds[i].h == H);
	}

	D.resize(n, vector<vector<int>>(6, vector<int>(3)));

	length_prefix.resize(m);
	length_prefix[0] = ulds[0].l;
	for (int i = 1; i < m; ++i)
		length_prefix[i] = length_prefix[i - 1] + ulds[i].l;

	Y.resize(n);
	O.resize(n);
	coords.resize(n);
	el.resize(n);

	for (int i = 0; i < n; ++i)
	{
		if (!packages[i].priority || !priority_pack)
		{
			Y[i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
		}
		else
		{
			Y[i] = model.addVar(0.0, 0.0, 0.0, GRB_BINARY);
		}
	}

	for (int i = 0; i < n; ++i)
		for (int k = 0; k < 6; ++k)
			O[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);

	for (int i = 0; i < n; ++i)
	{
		coords[i][0] = model.addVar(0.0, length_prefix[m - 1], 0.0, GRB_CONTINUOUS);
		coords[i][1] = model.addVar(0.0, B, 0.0, GRB_CONTINUOUS);
		coords[i][2] = model.addVar(0.0, H, 0.0, GRB_CONTINUOUS);
	}

	for (int i = 0; i < n; ++i)
	{
		int l = packages[i].l, b = packages[i].b, h = packages[i].h;
		D[i][0][0] = l;
		D[i][1][0] = l;
		D[i][2][0] = b;
		D[i][3][0] = b;
		D[i][4][0] = h;
		D[i][5][0] = h;

		D[i][0][1] = b;
		D[i][1][1] = h;
		D[i][2][1] = l;
		D[i][3][1] = h;
		D[i][4][1] = l;
		D[i][5][1] = b;

		D[i][0][2] = h;
		D[i][1][2] = b;
		D[i][2][2] = h;
		D[i][3][2] = l;
		D[i][4][2] = b;
		D[i][5][2] = l;
	}

	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			el[i][j] = 0;
			for (int k = 0; k < 6; ++k)
			{
				el[i][j] += D[i][k][j] * O[i][k];
			}
		}
	}

	// Geometric constraints

	// ULD Bounds check
	for (int i = 0; i < n; ++i)
	{
		model.addConstr(coords[i][0] + el[i][0] <= length_prefix[m - 1]);
		model.addConstr(coords[i][1] + el[i][1] <= B);
		model.addConstr(coords[i][2] + el[i][2] <= H);

		for (int j = 0; j < m - 1; ++j)
		{
			int sub = length_prefix[j];

			GRBVar tmp = model.addVar(-INF, INF, 0.0, GRB_CONTINUOUS);
			GRBVar tmp2 = model.addVar(-INF, INF, 0.0, GRB_CONTINUOUS);
			GRBVar tmp3 = model.addVar(-INF, INF, 0.0, GRB_CONTINUOUS);

			model.addConstr(tmp2 == sub - coords[i][0]);
			model.addConstr(tmp3 == coords[i][0] + el[i][0] - sub);
			GRBVar hi[] = {tmp2, tmp3};
			model.addGenConstrMax(tmp, hi, 2);
			model.addConstr(tmp >= el[i][0]);
		}
	}

	// Clashing packages check
	for (int i = 0; i < n; ++i)
	{
		for (int j = i + 1; j < n; ++j)
		{
			GRBTempConstr tmp1 = (coords[i][0] + el[i][0] - coords[j][0]) <= 0;
			GRBTempConstr tmp2 = (coords[i][1] + el[i][1] - coords[j][1]) <= 0;
			GRBTempConstr tmp3 = (coords[i][2] + el[i][2] - coords[j][2]) <= 0;
			GRBTempConstr tmp4 = (coords[j][0] + el[j][0] - coords[i][0]) <= 0;
			GRBTempConstr tmp5 = (coords[j][1] + el[j][1] - coords[i][1]) <= 0;
			GRBTempConstr tmp6 = (coords[j][2] + el[j][2] - coords[i][2]) <= 0;

			GRBVar indicators[8];
			for (int u = 0; u < 6; ++u)
				indicators[u] = (model.addVar(0.0, 1.0, 0.0, GRB_BINARY));
			indicators[6] = Y[i];
			indicators[7] = Y[j];

			model.addGenConstrIndicator(indicators[0], 1, tmp1);
			model.addGenConstrIndicator(indicators[1], 1, tmp2);
			model.addGenConstrIndicator(indicators[2], 1, tmp3);
			model.addGenConstrIndicator(indicators[3], 1, tmp4);
			model.addGenConstrIndicator(indicators[4], 1, tmp5);
			model.addGenConstrIndicator(indicators[5], 1, tmp6);

			GRBVar tmp_or = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
			model.addGenConstrOr(tmp_or, indicators, 8);

			model.addConstr(tmp_or == 1);
		}
	}

	// single orientation check
	for (int i = 0; i < n; ++i)
	{
		GRBLinExpr tmp = 0;
		for (int k = 0; k < 6; ++k)
			tmp += O[i][k];
		model.addConstr(tmp == 1);
	}

	GRBLinExpr cost = 0.0;
	for (int i = 0; i < n; ++i)
	{
		if (!packages[i].priority)
		{
			cost += Y[i] * packages[i].delay;
		}
		else if (!priority_pack)
		{
			cost += Y[i] * AVG_DELAY;
		}
	}

	model.setObjective(cost, GRB_MINIMIZE);
}

void Solver::solve(double time_stop, double obj_stop)
{
	if (obj_stop > -0.5)
	{
		model.set(GRB_DoubleParam_BestObjStop, obj_stop);
	}
	if (time_stop > -0.5)
	{
		model.set(GRB_DoubleParam_TimeLimit, time_stop);
	}

	model.optimize();
}
