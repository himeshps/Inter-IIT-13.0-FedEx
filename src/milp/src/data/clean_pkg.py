import csv
data = []
data_p = []
data_e = []
with open('raw_pkg.txt', mode='r') as file:
    reader = csv.reader(file)
    for row in reader:
        tp = row
        tp[0] = int(tp[0][2:])
        if (tp[-2][0] == 'E'):
            tp[-2] = 0
        else:
            tp[-2] = 1

        if (tp[-1] == '-'):
            tp[-1] = -1
        data.append(tp)

        if (tp[-2] == 1):
            data_p.append(tp)
        else:
            data_e.append(tp)

with open('pkg.csv', mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(data)
with open('pri_pkg.csv', mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(data_p)
with open('eco_pkg.csv', mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(data_e)
        
