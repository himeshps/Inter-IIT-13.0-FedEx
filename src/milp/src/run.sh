#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

echo "Step 1: Compiling the program with make..."
make

echo "Step 2: Running the program to generate full.csv... This step might take a while depending upon the timelimits specified in the main.cpp file in the Solution1 or Solution2 method of Pipeline class."
./program

echo "Step 3: Running wt_chk_and_fix.py with full.csv..."
python3 wt_and_gravity_fix/wt_chk_and_fix.py -f full.csv

echo "Step 4: Running gravity_chk_fix.py with wt_chk_sol.csv..."
python3 wt_and_gravity_fix/gravity_chk_and_fix.py -f wt_chk_sol.csv -i 1

echo "Step 5: Running convert_to_final.py with gravity_fix_sol.csv..."
python3 convert_to_final.py

echo "Workflow completed successfully! The final output is in solution.txt."
cp solution.txt ../../../results/milp/.
