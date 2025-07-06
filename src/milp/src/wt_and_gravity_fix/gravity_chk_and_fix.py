import pandas as pd
import argparse
import textwrap

def do_boxes_intersect(box1, box2):
    x1_min, y1_min, z1_min = min(box1[0], box1[3]), min(box1[1], box1[4]), min(box1[2], box1[5])
    x1_max, y1_max, z1_max = max(box1[0], box1[3]), max(box1[1], box1[4]), max(box1[2], box1[5])
    x2_min, y2_min, z2_min = min(box2[0], box2[3]), min(box2[1], box2[4]), min(box2[2], box2[5])
    x2_max, y2_max, z2_max = max(box2[0], box2[3]), max(box2[1], box2[4]), max(box2[2], box2[5])
    flag_x = x1_min < x2_max and x2_min < x1_max
    flag_y = y1_min < y2_max and y2_min < y1_max
    flag_z = z1_min < z2_max and z2_min < z1_max
    
    return flag_x and flag_y and flag_z

def check_intersections(file):
    df = pd.read_csv(file)
    grouped = df.groupby('ULD-ID')
    flag = False
    for uld_id, group in grouped:
        if len(group) > 1:
            for i in range(len(group)):
                for j in range(i+1, len(group)):
                    box1 = (
                        group.iloc[i]['x0'], group.iloc[i]['y0'], group.iloc[i]['z0'],
                        group.iloc[i]['x1'], group.iloc[i]['y1'], group.iloc[i]['z1']
                    )
                    box2 = (
                        group.iloc[j]['x0'], group.iloc[j]['y0'], group.iloc[j]['z0'],
                        group.iloc[j]['x1'], group.iloc[j]['y1'], group.iloc[j]['z1']
                    )
                    if do_boxes_intersect(box1, box2):
                        print(f"ULD-ID {uld_id}: Package {group.iloc[i]['Package-ID']} intersects with Package {group.iloc[j]['Package-ID']}")
                        flag = True
    
    if not flag:
        print("No intersections found.")
    return flag

def provides_support(b1, b2):
    if b2[2] != b1[5]:
        return False
    x_overlap = not (b1[3] <= b2[0] or b1[0] >= b2[3])
    y_overlap = not (b1[4] <= b2[1] or b1[1] >= b2[4])
    return x_overlap and y_overlap

def gravity(file,param):
    df = pd.read_csv(file)
    grouped = df.groupby('ULD-ID')
    mp = {}
    s = set()
    mp_uld = {}
    flag = True
    for uld_id, group in grouped:
        if(param==1):
            print(f"Processing ULD-{uld_id}")
        group = group.sort_values(by='z0').reset_index(drop=True)
        for i in range(len(group)):
            flag_base = False
            s.add(group.iloc[i]['Package-ID'])
            mp_uld[group.iloc[i]['Package-ID']] = group.iloc[i]['ULD-ID']
            for j in range(i):
                if i != j:
                    b1 = tuple(group.iloc[i][['x0', 'y0', 'z0', 'x1', 'y1', 'z1']])
                    b2 = tuple(group.iloc[j][['x0', 'y0', 'z0', 'x1', 'y1', 'z1']])
                    if provides_support(b2, b1) and not do_boxes_intersect(b1, b2):
                        if group.iloc[i]['Package-ID'] not in mp:
                            mp[group.iloc[i]['Package-ID']] = True
            if min(group.iloc[i]['z0'], group.iloc[i]['z1']) == 0:
                mp[group.iloc[i]['Package-ID']] = True
                flag_base = True
            if(param==1):
                if(flag_base):
                    continue
                elif(group.iloc[i]['Package-ID'] in mp):
                    continue
                else:
                    flag = False
                    max_z = 0
                    for j in range(i):
                        if i != j:
                            z_top_below = group.iloc[j]['z1']
                            diff = group.iloc[i]['z0'] - z_top_below
                            b1 = (group.iloc[i]['x0'], group.iloc[i]['y0'], z_top_below, 
                            group.iloc[i]['x1'], group.iloc[i]['y1'], group.iloc[i]['z1']-diff)
                            b2 = tuple(group.iloc[j][['x0', 'y0', 'z0', 'x1', 'y1', 'z1']])
                            if provides_support(b2, b1) and not do_boxes_intersect(b1, b2):
                                max_z = max(max_z,z_top_below)               
                    diff = group.iloc[i]['z0'] - max_z
                    package_id = group.iloc[i]['Package-ID']
                    df.loc[df['Package-ID'] == package_id, 'z0'] = max_z
                    df.loc[df['Package-ID'] == package_id, 'z1'] -= diff
                    group.loc[group['Package-ID'] == package_id, 'z0'] = max_z
                    group.loc[group['Package-ID'] == package_id, 'z1'] -= diff
    if(param==0):
        for val in s:
            if val not in mp or not mp[val]:
                flag = False
                print(f"Found no support on package: {val} in uld: {mp_uld[val]}")
        if(flag==True):
            print("No boxes found with zero support.")
        return 
    elif(param==1):                   
        if(not flag):
            df.to_csv('gravity_fix.csv',index=False)
            print("gravity_fix.csv file generated successfully")
        else:
            print("No boxes found with zero support.")
def main():
    parser = argparse.ArgumentParser(
        description=textwrap.dedent('''\
        Gravity Fix Utility
        -------------------
        This script processes package and ULD (Unit Load Device) data 
        to analyze and adjust package positioning based on support 
        and intersection rules.

        Key Features:
        - Checks overlaps between packages
        - Identifies packages without any support
        - Can Adjust package positions
        
        The input parameter controls the script's behavior:
        - 0 (default): Finds overlaps and boxes with no support(in air)
        - 1: Outputs gravity fixed csv file
        '''),
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        '-f', 
        '--file', 
        type=str, 
        default='sol.csv', 
        help=textwrap.dedent('''\
        Path to the input file.
        '''),
    )
    parser.add_argument(
        '-i', 
        '--input', 
        type=int, 
        default=0, 
        help=textwrap.dedent('''\
        Input parameter for gravity function:
          0 (default): Finds overlaps and boxes with no support(in air)
          1: Outputs gravity fixed csv file
        '''),
    )
    
    args = parser.parse_args()
    if(args.input==1):
        if(not check_intersections(args.file)):
            gravity(args.file, args.input)
        else:
            print("Intersections found, Terminating.")
    else:
        check_intersections(args.file)
        gravity(args.file,args.input)

if __name__ == "__main__":
    main()
