import numpy as np
import pandas as pd
import warnings
warnings.filterwarnings('ignore')
df = pd.read_csv('gravity_fix.csv', delimiter=None)
package_data=pd.read_csv('wt_and_gravity_fix/pkg.csv', delimiter=None)
sorted_df = df.sort_values(by="Package-ID")
merged_df = pd.merge(sorted_df, package_data, on='Package-ID', how="inner")
total_cost=29233
elements=[]

for x in merged_df['Delay_cost']:
    if x!="-":
        total_cost-=int(x)
number_of_packages=df['Package-ID'].size

unique_count_column1 = merged_df["ULD-ID"][merged_df["Delay_cost"] == "-"].nunique()

with open('solution.txt', 'w') as file:
    pass

line_to_remove = 'Package-ID,ULD-ID,x0,y0,z0,x1,y1,z1'
with open('full.csv', 'r') as infile, open('input.txt', 'w') as outfile:
    for line in infile:
        stripped_line = line.strip()
        if stripped_line == line_to_remove:
            continue
        split_line = stripped_line.split(',')

        if len(split_line) >= 2:
            split_line[0] = 'P-' + split_line[0]
            split_line[1] = 'U' + split_line[1]

        modified_line = ','.join(split_line) + '\n'

        outfile.write(modified_line)


with open('input.txt', 'r') as infile:
    input_content = infile.read()

total_cost+=unique_count_column1*5000

values = [total_cost, number_of_packages, unique_count_column1]

result = ','.join(map(str, values))
result += '\n'

with open('solution.txt', 'a') as file:
  file.write(result)
  file.write(input_content)

with open('solution.txt', 'a') as file: 
    ctr = 0
    for i in range(1, 401):
        if i not in merged_df['Package-ID'].values:
            ctr+=1
            file.write(f"P-{i},NONE,-1,-1,-1,-1,-1,-1\n")

with open('solution.txt', 'r') as file:
    content = file.read() 
    
z=0
with open('solution.txt', 'r') as file:
    lines = file.readlines()
    for line in lines[1:]:
        x = line.split(',')
        if(x[6] == '-1'):
            z+=1
