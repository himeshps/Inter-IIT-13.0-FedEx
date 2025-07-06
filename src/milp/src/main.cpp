/*
 * main.cpp
 * Solves the problem using gurobi (uses the academic license), setup instructions in README
 */

#include <bits/stdc++.h>

#include "base.h"
#include "pre_process.h"
#include "solver.h"

#include "gurobi_c++.h"

using namespace std;

class Pipeline
{
private:
	/* Pre-processed ULDs, pkgs */
	ULDHeuristic pre_ulds;
	PkgHeuristic pre_pkgs;

	vector<vector<int>> read_csv(string filename, bool header = false)
	{
		std::ifstream file(filename);

		if (!file.is_open())
		{
			std::cerr << "Error: Could not open the file " << filename << std::endl;
			exit(1);
		}

		std::string line;
		std::vector<std::vector<int>> data;

		// Read the file line by line
		while (std::getline(file, line))
		{
			if (header)
			{
				header = false;
				continue;
			}
			std::stringstream ss(line);
			std::string cell;
			std::vector<int> row;

			// Split the line by commas
			while (std::getline(ss, cell, ','))
			{
				row.push_back(std::stoi(cell)); // Convert each cell to an integer and add to the row
			}

			data.push_back(row); // Add the row to the data vector
		}

		file.close();
		return data;
	}

	int coin_toss(mt19937 &gen, uniform_int_distribution<> &distrib, int lim, double prob)
	{
		int rnum = distrib(gen);
		return rnum > static_cast<int>(prob * static_cast<double>(lim));
	}

public:
	Pipeline(string uld_path, string pkg_path)
	{
		vector<vector<int>> praw, uraw;
		praw = read_csv(pkg_path);
		uraw = read_csv(uld_path);

		vector<package> pkgs;
		vector<uld> ulds;
		for (auto &e : praw)
		{
			pkgs.push_back(package(e[0], e[1], e[2], e[3], e[4], e[5], e[6]));
		}

		for (auto &e : uraw)
		{
			ulds.push_back(uld(e[0], e[1], e[2], e[3], e[4]));
		}

		pre_ulds = ULDHeuristic(ulds);
		pre_pkgs = PkgHeuristic(pkgs);
	}

	void save(vector<pkg_soln> &soln, string filepath, bool header=false) {
		auto flag = fstream::app|fstream::out;
		if (header) flag = fstream::out;

		ofstream file(filepath,flag);
		
		if (header) {
			file << "Package-ID,ULD-ID,x0,y0,z0,x1,y1,z1" << endl;
		}

		for (auto &e: soln) {
			file << e.id << ","
				<< e.uld_id << ","
			       	<< (int)e.sx << "," << (int)e.sy << "," << (int)e.sz << ","
				<< (int)e.ex << "," << (int)e.ey << "," << (int)e.ez << "\n";	
		}
	}

	vector<pkg_soln> load(string filepath)
	{
		vector<vector<int>> data = read_csv(filepath, true);
		vector<pkg_soln> ret;
		for (auto &e : data)
		{
			pkg_soln nw(e[0], false, e[1], e[2], e[3], e[4], e[5], e[6], e[7]);
			if (nw.uld_id != -1)
				nw.assigned = true;
			ret.push_back(nw);
		}
		return ret;
	}

	/* Brief description of Solution 1:
	 *  - Use eco heuristic 1 to get top 200 economy pkgs.
	 *  - ULD 1: Let cqm pick some subset of the top 75 pkgs and pack the ULD.
	 *  - ULD 2: Same as ULD 1 -  ignores already packed pkgs and replaces them with other top packages to get 75.
	 *  - ULD 3: Let cqm pick some subset of all remaining economy packages.
	 *  - ULD 4: CQM packs as many priority pkgs. as possible.
	 *  - ULD 5-6: CQM packs priority + some subset of remaining economy.
	 * CQM lower bound almost never goes above 0 but incumbent solution usually plateaus, hence we put a TL on each cqm step.
	 * Only returns pkgs which are assigned to some ULD.
	 */
	vector<pkg_soln> Solution1(string filepath)
	{
		vector<double> TL = {300, 300, 300, 300, 300, 300};
		const int PKG_TRIM = 200;
		const int STAGE_1_CNT = 100;
		const int STAGE_2_CNT = 100;
		const int STAGE_3_CNT = 10000;
		const int STAGE_6_MAX = 75;

		vector<package> pkgs = pre_pkgs.EcoHeuristic1(PKG_TRIM);
		vector<package> pri_pkg = pre_pkgs.PriHeuristic1();
		vector<uld> ulds = pre_ulds.ULDHeuristic2();

		vector<pkg_soln> ret;
		set<int> assigned_pkgs; // Contains ids of pkgs which have been assigned

		// Stage 1
		vector<package> pkg1;
		for (int i = 0; i < STAGE_1_CNT; ++i)
			pkg1.push_back(pkgs[i]);

		GRBEnv env1 = GRBEnv();
		env1.set("LogFile", "log/st1.log");

		Solver st1(env1, pkg1.size(), 1, pkg1, {ulds[0]});
		st1.solve(TL[0]);
		vector<pkg_soln> soln1 = st1.ExtractSolution(true);

		ret.insert(ret.end(), soln1.begin(), soln1.end());
		for (auto &e : soln1)
			assigned_pkgs.insert(e.id);

		printf("Stage 1: Assigned %lu pkgs.\n", soln1.size());
		save(soln1, filepath, true);

		// Stage 2
		vector<package> pkg2;
		for (int i = 0; i < pkgs.size() && pkg2.size() < STAGE_2_CNT; ++i)
		{
			if (assigned_pkgs.find(pkgs[i].id) == assigned_pkgs.end())
				pkg2.push_back(pkgs[i]);
		}

		GRBEnv env2 = GRBEnv();
		env2.set("LogFile", "log/st2.log");

		Solver st2(env2, pkg2.size(), 1, pkg2, {ulds[1]});
		st2.solve(TL[1]);
		vector<pkg_soln> soln2 = st2.ExtractSolution(true);

		ret.insert(ret.end(), soln2.begin(), soln2.end());
		for (auto &e : soln2)
			assigned_pkgs.insert(e.id);

		printf("Stage 2: Assigned %lu pkgs.\n", soln2.size());

		save(soln2, filepath, false);

		// Stage 3
		vector<package> pkg3;
		for (int i = 0; i < pkgs.size() && pkg3.size() < STAGE_3_CNT; ++i)
		{
			if (assigned_pkgs.find(pkgs[i].id) == assigned_pkgs.end())
				pkg3.push_back(pkgs[i]);
		}

		GRBEnv env3 = GRBEnv();
		env3.set("LogFile", "log/st3.log");

		Solver st3(env3, pkg3.size(), 1, pkg3, {ulds[2]});
		st3.solve(TL[2]);
		vector<pkg_soln> soln3 = st3.ExtractSolution(true);

		ret.insert(ret.end(), soln3.begin(), soln3.end());
		for (auto &e : soln3)
			assigned_pkgs.insert(e.id);

		printf("Stage 3: Assigned %lu pkgs.\n", soln3.size());

		save(soln3, filepath, false);

		// Stage 4
		vector<package> pkg4;
		pkg4.insert(pkg4.end(), pri_pkg.begin(), pri_pkg.end());

		GRBEnv env4 = GRBEnv();
		env4.set("LogFile", "log/st4.log");

		Solver st4(env4, pkg4.size(), 1, pkg4, {ulds[3]}, false);
		st4.solve(TL[3]);
		vector<pkg_soln> soln4 = st4.ExtractSolution(true);

		ret.insert(ret.end(), soln4.begin(), soln4.end());
		for (auto &e : soln4)
			assigned_pkgs.insert(e.id);

		printf("Stage 4: Assigned %lu pkgs.\n", soln4.size());

		save(soln4, filepath, false);

		// Stage 5
		vector<package> pkg5;
		for (auto &e : pri_pkg)
			if (assigned_pkgs.find(e.id) == assigned_pkgs.end())
				pkg5.push_back(e);

		for (auto &e : pkgs)
		{
			if (assigned_pkgs.find(e.id) == assigned_pkgs.end())
				pkg5.push_back(e);
		}

		GRBEnv env5 = GRBEnv();
		env5.set("LogFile", "log/st5.log");

		Solver st5(env5, pkg5.size(), 1, pkg5, {ulds[4]}, false);
		st5.solve(TL[4]);
		vector<pkg_soln> soln5 = st5.ExtractSolution(true);

		ret.insert(ret.end(), soln5.begin(), soln5.end());
		for (auto &e : soln5)
			assigned_pkgs.insert(e.id);

		printf("Stage 5: Assigned %lu pkgs.\n", soln5.size());

		// Stage 6
		vector<package> pkg6;
		for (auto &e : pri_pkg)
			if (assigned_pkgs.find(e.id) == assigned_pkgs.end())
				pkg6.push_back(e);

		printf("Pri left: %lu\n", pkg6.size());

		for (auto &e : pkgs)
		{
			if (pkg6.size() > STAGE_6_MAX)
				break;
			if (assigned_pkgs.find(e.id) == assigned_pkgs.end())
				pkg6.push_back(e);
		}

		GRBEnv env6 = GRBEnv();
		env6.set("LogFile", "log/st6.log");
		env6.set(GRB_IntParam_MIPFocus, 1);

		Solver st6(env6, pkg6.size(), 1, pkg6, {ulds[5]});
		st6.solve(TL[5]);
		vector<pkg_soln> soln6 = st6.ExtractSolution(true);

		ret.insert(ret.end(), soln6.begin(), soln6.end());

		printf("Stage 6: Assigned %lu pkgs.\n", soln6.size());

		/* Because we used ULD Heuristic 2 */
		for (auto &e : ret)
		{
			swap(e.sx, e.sz);

			swap(e.ex, e.ez);
		}
		return ret;
	}

	/* Brief description of Solution 2:
	 *  - Use eco heuristic 3 to get top 250 economy pkgs.
	 *  - Use priority heuristic 1 to get all priority pkgs.
	 *  - ULD 4: Let cqm pick some subset of the top 90 pkgs and pack the ULD.
	 *  - ULD 5: Let cqm pick some subset of the top 40 pkgs and pack the ULD.
	 *  - ULD 6: Let cqm pick some subset of the top 35 pkgs and pack the ULD.
	 *  - ULD 1: Let cqm pick some subset of the top 75 pkgs and pack the ULD.
	 *  - ULD 2: Let cqm pick some subset of the top 75 pkgs and pack the ULD.
	 *  - ULD 3: Let cqm pick some subset of the top 75 pkgs and pack the ULD.
	 * CQM lower bound almost never goes above 0 but incumbent solution usually plateaus, hence we put a TL on each cqm step.
	 * Only returns pkgs which are assigned to some ULD.
	 */
	vector<pkg_soln> Solution2(string filepath)
	{
		random_device rd;
		mt19937 gen(rd());
		const int RAND_LIM = 1000;
		uniform_int_distribution<> distrib(1, RAND_LIM);

		vector<double> TL = {300, 300, 300, 300, 300, 300};
		const int PKG_TRIM = 250;
		const int STAGE_1_CNT = 90;
		const int STAGE_2_MIX = 40;
		const int STAGE_3_MIX = 35;
		const int STAGE_4_CNT = 75;
		const int STAGE_5_CNT = 75;
		const int STAGE_6_CNT = 75;

		const double STAGE_2_SKP = 0.8;
		const double STAGE_4_SKP = 0.4;
		const double STAGE_5_SKP = 0.3;

		vector<package> pkgs = pre_pkgs.EcoHeuristic3(PKG_TRIM);
		vector<package> pri_pkgs = pre_pkgs.PriHeuristic1();
		vector<uld> ulds = pre_ulds.ULDHeuristic2();

		vector<pkg_soln> ret;
		set<int> assigned_pkgs; // Contains ids of pkgs which have been assigned

		// Stage 1
		vector<package> pkg1;
		for (int i = 0; i < STAGE_1_CNT; ++i)
			pkg1.push_back(pri_pkgs[i]);

		GRBEnv env1 = GRBEnv();
		env1.set("LogFile", "log/st1.log");
		env1.set(GRB_IntParam_MIPFocus, 1);

		Solver st1(env1, pkg1.size(), 1, pkg1, {ulds[3]}, false);
		st1.solve(TL[0]);
		vector<pkg_soln> soln1 = st1.ExtractSolution(true);

		ret.insert(ret.end(), soln1.begin(), soln1.end());
		for (auto &e : soln1)
			assigned_pkgs.insert(e.id);

		printf("Stage 1: Assigned %lu pkgs.\n", soln1.size());
		save(soln1, filepath, true);

		// Stage 2
		vector<package> pkg2;
		for (int i = 0; i < pkgs.size() && pkg2.size() < STAGE_2_MIX; i++)
		{
			if (assigned_pkgs.find(pkgs[i].id) == assigned_pkgs.end())
				pkg2.push_back(pkgs[i]);
			i += coin_toss(gen, distrib, RAND_LIM, STAGE_2_SKP);
		}
		for (int i = 0; i < pri_pkgs.size(); ++i)
		{
			if (assigned_pkgs.find(pri_pkgs[i].id) == assigned_pkgs.end())
				pkg2.push_back(pri_pkgs[i]);
		}

		GRBEnv env2 = GRBEnv();
		env2.set("LogFile", "log/st2.log");
		env2.set(GRB_IntParam_MIPFocus, 1);

		Solver st2(env2, pkg2.size(), 1, pkg2, {ulds[4]}, false);
		st2.solve(TL[1]);
		vector<pkg_soln> soln2 = st2.ExtractSolution(true);

		ret.insert(ret.end(), soln2.begin(), soln2.end());
		for (auto &e : soln2)
			assigned_pkgs.insert(e.id);

		printf("Stage 2: Assigned %lu pkgs.\n", soln2.size());

		save(soln2, filepath, false);

		// Stage 3
		vector<package> pkg3;
		for (int i = 0; i < pkgs.size() && pkg3.size() < STAGE_3_MIX; ++i)
		{
			if (assigned_pkgs.find(pkgs[i].id) == assigned_pkgs.end())
				pkg3.push_back(pkgs[i]);
		}
		for (int i = 0; i < pri_pkgs.size(); ++i)
		{
			if (assigned_pkgs.find(pri_pkgs[i].id) == assigned_pkgs.end())
				pkg3.push_back(pri_pkgs[i]);
		}

		GRBEnv env3 = GRBEnv();
		env3.set("LogFile", "log/st3.log");
		env3.set(GRB_IntParam_MIPFocus, 1);

		Solver st3(env3, pkg3.size(), 1, pkg3, {ulds[5]}, true);
		st3.solve(TL[2]);
		vector<pkg_soln> soln3 = st3.ExtractSolution(true);

		ret.insert(ret.end(), soln3.begin(), soln3.end());
		for (auto &e : soln3)
			assigned_pkgs.insert(e.id);

		printf("Stage 3: Assigned %lu pkgs.\n", soln3.size());

		save(soln3, filepath, false);

		// Stage 4
		vector<package> pkg4;
		for (int i = 0; i < pkgs.size() && pkg4.size() < STAGE_4_CNT; i++)
		{
			if (assigned_pkgs.find(pkgs[i].id) == assigned_pkgs.end())
				pkg4.push_back(pkgs[i]);
			i += coin_toss(gen, distrib, RAND_LIM, STAGE_4_SKP);
		}

		GRBEnv env4 = GRBEnv();
		env4.set("LogFile", "log/st4.log");
		env4.set(GRB_IntParam_MIPFocus, 1);

		Solver st4(env4, pkg4.size(), 1, pkg4, {ulds[0]});
		st4.solve(TL[3]);
		vector<pkg_soln> soln4 = st4.ExtractSolution(true);

		ret.insert(ret.end(), soln4.begin(), soln4.end());
		for (auto &e : soln4)
			assigned_pkgs.insert(e.id);

		printf("Stage 4: Assigned %lu pkgs.\n", soln4.size());

		save(soln4, filepath, false);

		// Stage 5
		vector<package> pkg5;
		for (int i = 0; i < pkgs.size() && pkg5.size() < STAGE_5_CNT; i++)
		{
			if (assigned_pkgs.find(pkgs[i].id) == assigned_pkgs.end())
				pkg5.push_back(pkgs[i]);
			i += coin_toss(gen, distrib, RAND_LIM, STAGE_5_SKP);
		}

		GRBEnv env5 = GRBEnv();
		env5.set("LogFile", "log/st5.log");
		env5.set(GRB_IntParam_MIPFocus, 1);

		Solver st5(env5, pkg5.size(), 1, pkg5, {ulds[1]});
		st5.solve(TL[4]);
		vector<pkg_soln> soln5 = st5.ExtractSolution(true);

		ret.insert(ret.end(), soln5.begin(), soln5.end());
		for (auto &e : soln5)
			assigned_pkgs.insert(e.id);

		printf("Stage 5: Assigned %lu pkgs.\n", soln5.size());

		// Stage 6
		vector<package> pkg6;
		for (int i = 0; i < pkgs.size() && pkg6.size() < STAGE_6_CNT; ++i)
		{
			if (assigned_pkgs.find(pkgs[i].id) == assigned_pkgs.end())
				pkg6.push_back(pkgs[i]);
		}

		GRBEnv env6 = GRBEnv();
		env6.set("LogFile", "log/st6.log");
		env6.set(GRB_IntParam_MIPFocus, 1);

		Solver st6(env6, pkg6.size(), 1, pkg6, {ulds[2]});
		st6.solve(TL[5]);
		vector<pkg_soln> soln6 = st6.ExtractSolution(true);

		ret.insert(ret.end(), soln6.begin(), soln6.end());

		printf("Stage 6: Assigned %lu pkgs.\n", soln6.size());

		/* Because we used ULD Heuristic 2 */
		for (auto &e : ret)
		{
			swap(e.sx, e.sz);

			swap(e.ex, e.ez);
		}
		return ret;
	}
};

int main(void)
{
	try
	{
		Pipeline Execute("data/uld.csv", "data/pkg.csv");

		vector<pkg_soln> soln = Execute.Solution2("sol.csv");
		Execute.save(soln, "full.csv", true);
	}
	catch (GRBException e)
	{
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	catch (...)
	{
		cout << "Exception during optimization" << endl;
	}
	return 0;
}
