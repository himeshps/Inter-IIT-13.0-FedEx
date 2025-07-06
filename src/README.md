# Extreme Points Heuristic with Neighbourhood Swapping

This solution uses heuristics for deciding the ordering for objects, the extreme points heuristic for deciding which objects go where, and then uses neighbourhood swapping to explore and find better solutions.

`/../../results/extreme_points/FINAL_SOLUTION_28658.txt` contains the **FINAL SOLUTION** for the Heuristics Solutoin as well as overall.

To run this, simly execute
```
make
./solution
```
The default run is configured to run it with one iteration, with a relatively small neighbourhood
This should generate a few files under the path `../../results/extreme_points`, `log.txt` - for the log files, which contain timestamps for each solution found, some technical information about the ordering of the objects as well as the information about which swaps happen when; `heuristic_best_solution.txt` and - this stores the best solution the solver has been able to achieve till now, and `COST_heuristic_TIMESTAMP.txt` to look at the solutions generated at each time.

The script `plot_cost_v_time.py` was used to generate the plots in the reports. You can populate it with your time, swaps and costs according to your logs to generate a graph.  

## Approach:

We use a modified, novel approach based on Extreme Points, a commonly used approach in 3D Bin Packing Problems, to decide where an object should be inserted into a given partially packed ULD. We use another noval approach for finding an ordering that leads to an optimal solution, we first come up with a good base solution by tuning heuristics to get a minima solution. After that we introduce a function, Neighbourhood Swapping, which swaps elements in the neighbourhood to try and achieve a lower cost, and accordingly updates the insertion order if it finds a better solution.
