import matplotlib.pyplot as plt

# Example data for cost-volume sorting with window size 30
datasets = [
    {
        "time": [0, 17, 300, 488, 773, 889, 1731, 2017, 2129, 2268],
        "swaps": [0, 2, 44, 61, 89, 100, 184, 219, 234, 250],
        "cost": [29500, 29393, 29300, 29144, 28915, 28902, 28770, 28701, 28658, 28658],
        "label": "Cost-volume sorting (window size 30)"
    },
]

# Define font sizes
title_fontsize = 20
label_fontsize = 16
tick_fontsize = 14
legend_fontsize = 14
swap_fontsize = 16  # Larger font for swap values

# Create the plot
fig, ax = plt.subplots(figsize=(10, 6))

# Plot cost vs time
for data in datasets:
    ax.plot(data["time"], data["cost"], marker='s', linestyle='--', label=data["label"])

    # Annotate each point with corresponding swap value
    for i, txt in enumerate(data["swaps"]):
        ax.annotate(txt, (data["time"][i], data["cost"][i]), textcoords="offset points", xytext=(0, 5), ha='center', fontsize=swap_fontsize)
# Add a label to indicate that the values near the points represent the number of swaps
ax.text(0.72, 0.885, 'Number of swaps are indicated near the points', 
        ha='center', va='top', fontsize=14, color='black', transform=ax.transAxes)
# Set titles, labels, and grid
ax.set_title('Left-over Cost vs Time', fontsize=title_fontsize)
ax.set_xlabel('Time (seconds)', fontsize=label_fontsize)
ax.set_ylabel('Left-over Cost', fontsize=label_fontsize)
ax.tick_params(axis='both', labelsize=tick_fontsize)
ax.legend(fontsize=legend_fontsize)
ax.grid()

# Save the plot
plt.tight_layout()
plt.savefig('../../results/extreme_points/cost_vs_time_with_larger_swaps.png', dpi = 300)
