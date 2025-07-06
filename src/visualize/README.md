# Visualization Tool

## Overview
This tool provides a visualization of the package placement solution stored in `sol.csv`. The visualization can be rendered in two modes:
- **Static Output**: A snapshot of the placement.
- **Animated Output**: A dynamic representation of the placement process.

## Prerequisites
Ensure that you have **Python 3.12** installed along with the required libraries:
- **numpy**
- **matplotlib**

You can install them using the following command:
```bash
pip install numpy matplotlib
```
## Files in the Repository
- **`PackageData.txt`**: Contains package details.
- **`UldData.txt`**: Contains ULD (Unit Load Device) details.
- **`solution.txt`**: Solution file in the official submission format.  

## Instructions
1. Update your solution in `solution.txt` following the specified format.
2. Run `visualizer.py` to visualize and validate the solution.

## Arguments
- `--f` : Path to the `solution` file (default is `solution.txt`)
- `--viz` : `0` or `1`, where `0` is for only validation of solution file and `1` is for validation with visualization.

## Visualization Modes
The script supports two visualization modes:
1. **Static Visualization**
2. **Animated Visualization**

To select the desired mode, modify the following block in `visualizer_f.py`:
```python
# Uncomment the line corresponding to the desired visualization mode
"""animated_visualization""" 
# visualizer.visualize_animated(boxes)

"""static visualization"""
visualizer.visualize_static(boxes)
```
## Running the Script

Execute the visualization script using:
```bash
python visualizer.py
```

## Validation
- **Constraints Checking**:
    - Ensures packages fit within container dimensions.
    - Validates that packages total weight do not exceed the container's weight limit.
    - Confirms packages do not overlap with one another.
    - Checks that no package is unsupported (i.e., floating).
    - Verifies that all Priority packages are packed.
    - Ensures that the top line of the solution.txt file matches the actual data in the solution.

- **Metrics Calculation**:
    - **Packing efficiency**: Volume usage percentage of each container.
    - **Stability ratio**: Proportion of stable packages in the solution.
     - **Average Packing Efficiency** : $$\dfrac{\text{Total volume occupied by packages}}{ \text{Total volume of ULD}}$$
    - **Average Stability ratio** : $$\dfrac{\text{Total stable packages}}{ \text{Total packages}}$$


   
