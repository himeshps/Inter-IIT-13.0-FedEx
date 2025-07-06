import pandas as pd
import argparse
import csv
import typing
from dataclasses import dataclass
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import matplotlib.animation as animation
from itertools import product


"""
Coordinates correspond to min x, min y, min z
"""
total_vol_occupied = [[] for _ in range(6)]
total_vol_available = [[] for _ in range(6)]
total_stable_pkgs = [[] for _ in range(6)]
total_pkg = [[] for _ in range(6)]
class Package:
    def __init__(self, id, weight, coordinates, dimensions, priority):
        self.id = id
        self.weight = weight
        self.priority = priority
        self.coordinates = coordinates
        self.dimensions = dimensions


class Container:
    def __init__(self, id, dimensions, weight_limit):
        self.id = id
        self.weight_limit = weight_limit
        self.dimensions = dimensions
        self.packages = []
        self.pe = 0
        self.packages_weight = 0

    def add_package(self, package):
        self.packages.append(package)

    def validate_container(self):
        self.packing_efficiency()
        self.stability_ratio()
        for package in self.packages:
            self.packages_weight += package.weight
            for j in range(3):
                if (
                    package.coordinates[j] < 0
                    or package.coordinates[j] + package.dimensions[j]
                    > self.dimensions[j]
                ):
                    return (False, "B", package.id)

        if self.packages_weight > self.weight_limit:
            return (False, "W")
        for i in range(len(self.packages)):
            for j in range(i + 1, len(self.packages)):
                if intersect(self.packages[i], self.packages[j]):
                    return (False, "I", self.packages[i].id, self.packages[j].id)

        self.packages = sorted(self.packages, key=lambda p: p.coordinates[2])
        supported_packages = set()
        for i, package in enumerate(self.packages):
            on_base = False
            for j in range(i):
                if i != j:
                    if provides_support(self.packages[j], package):
                        supported_packages.add(package.id)
            if package.coordinates[2] == 0:
                supported_packages.add(package.id)
                on_base = True
            if not on_base and package.id not in supported_packages:
                return (False, "G", package.id)
        return (True, None)

    def packing_efficiency(self):
        uld_volume = self.dimensions[0] * self.dimensions[1] * self.dimensions[2]
        volume_occ = sum(
            pkg.dimensions[0] * pkg.dimensions[1] * pkg.dimensions[2]
            for pkg in self.packages
        )
        total_vol_occupied[int(self.id[1:])-1] = volume_occ
        total_vol_available[int(self.id[1:])-1] = uld_volume
        self.pe = volume_occ / uld_volume

    def stability_ratio(self):
        INF = 100000
        packages_in_uld = len(self.packages)
        ct = 0

        for pkg in self.packages:
            top_key = pkg.id
            top = (
                pkg.coordinates[0],
                pkg.coordinates[1],
                pkg.coordinates[2],
                pkg.coordinates[0] + pkg.dimensions[0],
                pkg.coordinates[1] + pkg.dimensions[1],
                pkg.coordinates[2] + pkg.dimensions[2],
            )
            xmin = INF
            xmax = -INF
            ymin = INF
            ymax = -INF

            for pkg2 in self.packages:
                bottom_key = pkg2.id
                bottom = (
                    pkg2.coordinates[0],
                    pkg2.coordinates[1],
                    pkg2.coordinates[2],
                    pkg2.coordinates[0] + pkg2.dimensions[0],
                    pkg2.coordinates[1] + pkg2.dimensions[1],
                    pkg2.coordinates[2] + pkg2.dimensions[2],
                )
                if top_key == bottom_key:
                    continue
                if (
                    bottom[5] == top[2]
                    and (top[0] - bottom[0]) * (top[0] - bottom[3]) <= 0
                    and (top[1] - bottom[1]) * (top[1] - bottom[4]) <= 0
                ):
                    xmin = min(xmin, bottom[0])
                    xmax = max(xmax, bottom[3])
                    ymin = min(ymin, bottom[1])
                    ymax = max(ymax, bottom[4])

            if xmin == INF and xmax == -INF and ymin == INF and ymax == -INF:
                ct += 1
            elif (
                (top[0] + top[3]) / 2 >= xmin
                and (top[0] + top[3]) / 2 <= xmax
                and (top[1] + top[4]) / 2 >= ymin
                and (top[1] + top[4]) / 2 <= ymax
            ):
                ct += 1

        temp = ct / packages_in_uld if packages_in_uld > 0 else 0
        total_stable_pkgs[int(self.id[1:])-1] = ct
        total_pkg[int(self.id[1:])-1] = packages_in_uld
        print(
            f"Number of packages stable in ULD {self.id} are {ct} out of {packages_in_uld} with a ratio {temp:.2f}"
        )

    def visualize(self):
        print(
            f"Visualizing {self.id} with {len(self.packages)} packages, packing efficiency: {self.pe}, weight: {self.packages_weight}"
        )
        visualizer = PackingVisualizer(self.dimensions)
        """animated_visualization"""
        # visualizer.visualize_animated(self.packages)
        """static visualization"""
        visualizer.visualize_static(self.packages)


class PackingVisualizer:
    _DEFAULT_COLORS = [
        "orange",
        "green",
        "red",
        "purple",
        "blue",
        "yellow",
        "pink",
        "cyan",
        "brown",
        "lime",
        "teal",
        "magenta",
        "gold",
        "silver",
        "navy",
        "indigo",
        "maroon",
        "olive",
        "turquoise",
        "lavender",
    ]

    def __init__(self, main_pkg: tuple[float, float, float]):
        self.main_pkg = main_pkg
        self.fig = plt.figure(figsize=(10, 8))
        self.ax = self.fig.add_subplot(111, projection="3d")
        self.main_pkg_added = False

    def _generate_item_vertices(self, pkg: Package) -> list[list[tuple]]:
        x, y, z = pkg.coordinates
        sx, sy, sz = pkg.dimensions
        x_min, x_max = x, x + sx
        y_min, y_max = y, y + sy
        z_min, z_max = z, z + sz

        corners = list(product([x_min, x_max], [y_min, y_max], [z_min, z_max]))

        return [
            [corners[0], corners[1], corners[3], corners[2]],
            [corners[4], corners[5], corners[7], corners[6]],
            [corners[0], corners[1], corners[5], corners[4]],
            [corners[2], corners[3], corners[7], corners[6]],
            [corners[1], corners[3], corners[7], corners[5]],
            [corners[0], corners[2], corners[6], corners[4]],
        ]

    def _create_uld_vertices(self) -> list[list[tuple]]:
        dx, dy, dz = self.main_pkg
        main_corners = list(product([0, dx], [0, dy], [0, dz]))

        return [
            [main_corners[0], main_corners[1], main_corners[3], main_corners[2]],
            [main_corners[4], main_corners[5], main_corners[7], main_corners[6]],
            [main_corners[0], main_corners[1], main_corners[5], main_corners[4]],
            [main_corners[2], main_corners[3], main_corners[7], main_corners[6]],
            [main_corners[1], main_corners[3], main_corners[7], main_corners[5]],
            [main_corners[0], main_corners[2], main_corners[6], main_corners[4]],
        ]

    def _setup_axes(self, padding: float = 0.5):
        dx, dy, dz = self.main_pkg

        self.ax.set_xlim3d(0 - padding, dx + padding)
        self.ax.set_ylim3d(0 - padding, dy + padding)
        self.ax.set_zlim3d(0 - padding, dz + padding)

        self.ax.set_xlabel("X")
        self.ax.set_ylabel("Y")
        self.ax.set_zlabel("Z")

    def visualize_static(self, packages: list[Package]):
        if not self.main_pkg_added:
            main_verts = self._create_uld_vertices()
            self.ax.add_collection3d(
                Poly3DCollection(
                    main_verts,
                    facecolors="lightblue",
                    linewidths=0.5,
                    edgecolors="black",
                    alpha=0.2,
                )
            )
            self.main_pkg_added = True

        for idx, pkg in enumerate(packages):
            color = self._DEFAULT_COLORS[idx % len(self._DEFAULT_COLORS)]
            pkg_verts = self._generate_item_vertices(pkg)
            self.ax.add_collection3d(
                Poly3DCollection(
                    pkg_verts,
                    facecolors=color,
                    linewidths=0.3,
                    edgecolors="black",
                    alpha=0.6,
                )
            )

        self._setup_axes()
        plt.tight_layout()
        plt.show()

    def add_pkg(self, pkg: Package):
        self.visualize_static([pkg])

    def visualize_animated(self, packages: list[Package]):
        def init():
            self._setup_axes()
            main_verts = self._create_uld_vertices()
            self.ax.add_collection3d(
                Poly3DCollection(
                    main_verts,
                    facecolors="lightblue",
                    linewidths=1,
                    edgecolors="black",
                    alpha=0.2,
                )
            )
            return []

        added_packages = set()

        def animate(frame):
            if frame not in added_packages:
                added_packages.add(frame)
                pkg = packages[frame]
                color = self._DEFAULT_COLORS[frame % len(self._DEFAULT_COLORS)]
                pkg_verts = self._generate_item_vertices(pkg)

                self.ax.add_collection3d(
                    Poly3DCollection(
                        pkg_verts,
                        facecolors=color,
                        linewidths=0.5,
                        edgecolors="black",
                        alpha=0.5,
                    )
                )

            return self.ax.collections

        anim = animation.FuncAnimation(
            self.fig,
            animate,
            init_func=init,
            frames=len(packages),
            interval=500,
            repeat=False,
        )

        plt.tight_layout()
        plt.show()


ID = 0
L = 1
W = 2
H = 3
WEIGHT = 4
P = 5
COST = 6
VOL = 7
DENSITY = 8


def parse_pd_row(row):
    if row[COST] == "-":
        val = -1
    else:
        val = int(row[COST])
    return [
        row[ID],
        int(row[L]),
        int(row[W]),
        int(row[H]),
        int(row[WEIGHT]),
        int(row[P] == "Priority"),
        val,
        int(row[L]) * int(row[W]) * int(row[H]),
        int(row[WEIGHT]) / (int(row[L]) * int(row[W]) * int(row[H])),
    ]


def populate_packages(filepath):
    with open(filepath) as f:
        pd_reader = csv.reader(f)
        next(pd_reader)
        pd_data = [parse_pd_row(row) for row in pd_reader]
        pd_data.sort(key=lambda x: x[VOL])
        # pd_data_priority = [x for x in pd_data if x[P] == 1]
        # pd_data_non_priority = [x for x in pd_data if x[P] == 0]

    pd_df = pd.DataFrame(
        pd_data, columns=["ID", "L", "W", "H", "WEIGHT", "P", "COST", "VOL", "DENSITY"]
    )
    return pd_df


def populate_containers(filepath):
    with open(filepath) as f:
        uld_reader = csv.reader(f)
        next(uld_reader)
        uld_data = [row for row in uld_reader]
        containers = {
            row[0]: Container(
                row[0], (int(row[1]), int(row[2]), int(row[3])), int(row[4])
            )
            for row in uld_data
        }
    return containers


def intersect(p1: Package, p2: Package):
    for i in range(3):
        if (
            p1.coordinates[i] + p1.dimensions[i] <= p2.coordinates[i]
            or p2.coordinates[i] + p2.dimensions[i] <= p1.coordinates[i]
        ):
            return False
    return True


def provides_support(p1: Package, p2: Package):
    b1 = (
        p1.coordinates[0],
        p1.coordinates[1],
        p1.coordinates[2],
        p1.coordinates[0] + p1.dimensions[0],
        p1.coordinates[1] + p1.dimensions[1],
        p1.coordinates[2] + p1.dimensions[2],
    )
    b2 = (
        p2.coordinates[0],
        p2.coordinates[1],
        p2.coordinates[2],
        p2.coordinates[0] + p2.dimensions[0],
        p2.coordinates[1] + p2.dimensions[1],
        p2.coordinates[2] + p2.dimensions[2],
    )
    if b2[2] != b1[5]:
        return False
    x_overlap = not (b1[3] <= b2[0] or b1[0] >= b2[3])
    y_overlap = not (b1[4] <= b2[1] or b1[1] >= b2[4])
    return x_overlap and y_overlap


def validate_and_visualize(datafile, containers, package_data, viz_on):
    reader = csv.reader(datafile)
    data = []
    row_counter = 0
    priority_containers = []
    cost = 0
    packages_used = []
    packages_unused = []
    k = 5000
    for row in reader:
        data.append(row)
        if row_counter == 0:
            solution_cost = int(row[0])
            num_packed = int(row[1])
            ulds_used = int(row[2])
        else:
            data = []
            for i in range(len(row)):
                if i < 2:
                    data.append(row[i])
                else:
                    data.append(int(row[i]))
            pid, uid, x0, y0, z0, x1, y1, z1 = tuple(data)
            if uid.lower() != "none":
                if pid[0] != "P":
                    pid = f"P-{pid}"
                if uid[0] != "U":
                    uid = f"U{uid}"
                if (
                    uid not in priority_containers
                    and package_data.loc[package_data["ID"] == pid].iloc[0]["P"] == 1
                ):
                    priority_containers.append(uid)
                package = Package(
                    pid,
                    package_data.loc[package_data["ID"] == pid].iloc[0]["WEIGHT"],
                    (x0, y0, z0),
                    (x1 - x0, y1 - y0, z1 - z0),
                    package_data.loc[package_data["ID"] == pid].iloc[0]["P"],
                )
                packages_used.append(pid)

                containers[uid].add_package(package)
            else:
                packages_unused.append(pid)
                if package_data.loc[package_data["ID"] == pid].iloc[0]["P"] == 1:
                    pass
                else:
                    cost += package_data.loc[package_data["ID"] == pid].iloc[0]["COST"]
        row_counter += 1

    for _, row in package_data[package_data["P"] == 1].iterrows():
        if row["ID"] not in packages_used:
            solution_is_valid = False
            print(f"Priority package {row['ID']} is not in a container")

    print("Packages used: ", len(packages_used))
    top_row_valid = True
    if len(packages_used) != num_packed:
        print("\033[91mNumber of packages packed is wrong.\033[0m")
        top_row_valid = False
        
    print("Packages unused: ", len(packages_unused))
    print(
        "Packages unlisted: ",
        package_data.shape[0] - len(packages_used) - len(packages_unused),
    )
    
    solution_is_valid = True
    for uid, container in containers.items():
        ret = container.validate_container()
        if ret[0] == False:
            solution_is_valid = False
            if ret[1] == "B":
                print(
                    f"\033[91mPackage {ret[2]} in container {uid} is out of bounds.\033[0m"
                )
            elif ret[1] == "W":
                print(f"\033[91mContainer {uid} is overweight.\033[0m")
            elif ret[1] == "I":
                print(
                    f"\033[91mPackage {ret[2]} and {ret[3]} in container {uid} intersect.\033[0m"
                )
            elif ret[1] == "G":
                print(
                    f"\033[91mPackage {ret[2]} in container {uid} is not supported.\033[0m"
                )
        if viz_on:
            container.visualize()
            
    cost += len(priority_containers) * k
    
    if cost != solution_cost:
        print("\033[91mSolution cost is wrong.\033[0m")
        top_row_valid = False
        
    if len(priority_containers) != ulds_used:
        print("\033[91mNumber of ULDs used for Priority packages is wrong.\033[0m")
        top_row_valid = False
        
    if solution_is_valid:
        print("\033[93mPacking Validated\033[0m")
        print("Cost: ", cost)
        print("Priority Containers Used: ", len(priority_containers))
        print("Packages packed: ", len(packages_used))
        if not top_row_valid:
            print("\033[91mTop row of the solution file is wrong.\033[0m")
        else:
            print("\033[92mSolution file is correct.\033[0m")
            average_packing_efficiency = sum(total_vol_occupied)/sum(total_vol_available)
            average_stability_ratio = sum(total_stable_pkgs)/sum(total_pkg)
            print(f"Average Packing Efficiency is {average_packing_efficiency}.")
            print(f"Average Stability Ratio is {average_stability_ratio}.")
            
    return containers


parser = argparse.ArgumentParser()
parser.add_argument("--path", type=str, required=False, default="solution.txt")
parser.add_argument("--pack", type=str, required=False, default="PackageData.txt")
parser.add_argument("--uld", type=str, required=False, default="UldData.txt")
parser.add_argument("--viz", type=int, required=False, default=1)
args = parser.parse_args()
path = args.path
pack = args.pack
uld = args.uld
viz_on = args.viz
with open(path, "r") as f:
    containers = populate_containers(uld)
    packages = populate_packages(pack)
    validate_and_visualize(f, containers, packages, viz_on)
