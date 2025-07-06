import os
import csv
import argparse
import textwrap
def export_to_csv(lst, output_file="wt_chk_sol.csv"):
    with open(output_file, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Package-ID", "ULD-ID", "x0", "y0", "z0", "x1", "y1", "z1"])
        for uld_id, packages in lst.items():
            for package_entry in packages:
                writer.writerow([
                    package_entry['Package-ID'],
                    uld_id,
                    package_entry['x0'],
                    package_entry['y0'],
                    package_entry['z0'],
                    package_entry['x1'],
                    package_entry['y1'],
                    package_entry['z1']
                ])

def ep_clean(data):
    data['Package-ID'] = data['Package-ID'][2:]
    data['ULD-ID'] = data['ULD-ID'][1:]
    return data

def parse_csv(file_path, clean=False):
    with open(file_path, mode='r') as file:
        reader = csv.DictReader(file)
        if not clean:
            return [row for row in reader]
        else:
            return [ep_clean(row) for row in reader]

def check_solution(directory, sol_file):
    pkg_file = os.path.join(directory, "pkg.csv")
    uld_file = os.path.join(directory, "uld.csv")

    packages = parse_csv(pkg_file)
    ulds = parse_csv(uld_file)
    solution = parse_csv(sol_file, False)
    lst = {}
    for entry in solution:
        uld_id = entry['ULD-ID']
        if uld_id not in lst:
            lst[uld_id] = []
        lst[uld_id].append(entry)
        
    cost = 0
    delays = []
    for pkg in packages:
        if pkg['Delay_cost'] != '-':
            cost += int(pkg['Delay_cost'])
            delays.append(int(pkg['Delay_cost']))

    delays.sort()

    for uld in ulds:
        uld_id = uld['ULD-ID']
        uld_l = float(uld['Length'])
        uld_w = float(uld['Width'])
        uld_h = float(uld['Height'])

        if uld_id in lst:
            main_box = (uld_l, uld_w, uld_h)
            boxes = []
            volume = 0
            weight = 0
            uld_volume = uld_l * uld_w * uld_h
            for package_entry in lst[uld_id]:
                pkg_id = package_entry['Package-ID']
                pkg_det = next((pkg for pkg in packages if pkg['Package-ID'] == pkg_id), None)
                weight += int(pkg_det['Weight'])
                if pkg_det['Delay_cost'] != '-':
                    cost -= int(pkg_det['Delay_cost'])
                if not pkg_det:
                    raise ValueError(f"Package-ID {pkg_id} in solution file not found in pkg.csv.")
            
            eco_pkg = [
                package_entry for package_entry in lst[uld_id]
                if next(pkg for pkg in packages if pkg['Package-ID'] == package_entry['Package-ID'])['Delay_cost'] != "-"
            ]
            if weight > int(uld['Weight-Limit']):
                eco_pkg.sort(
                    key=lambda entry: int(next(pkg for pkg in packages if pkg['Package-ID'] == entry['Package-ID'])['Delay_cost']) /
                                    int(next(pkg for pkg in packages if pkg['Package-ID'] == entry['Package-ID'])['Weight']),
                    reverse=False
                )
                copy=weight-int(uld['Weight-Limit'])
                for x in eco_pkg:
                    pkg_details = next(pkg for pkg in packages if pkg['Package-ID'] == x['Package-ID'])
                    pkg_weight = int(pkg_details['Weight'])
                    pkg_cost = int(pkg_details['Delay_cost'])
                    if copy > 0:
                        copy -= pkg_weight
                        cost += pkg_cost
                        lst[uld_id].remove(x)        
                        weight-=pkg_weight

    export_to_csv(lst)
if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=textwrap.dedent('''\
            check solution and export ULD list.
        '''),
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        '-f', 
        '--file', 
        type=str, 
        default='sol.csv', 
        help=textwrap.dedent('''\
        Path to the solution file.
        '''),
    )
    
    args = parser.parse_args()

    data_directory = "wt_and_gravity_fix"
    check_solution(data_directory, args.file)
