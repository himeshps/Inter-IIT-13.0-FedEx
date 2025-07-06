import csv
data = []
with open('raw_uld.txt', mode='r') as file:
    reader = csv.reader(file)
    for row in reader:
        tp = row
        tp[0] = int(tp[0][1:])
        data.append(tp)

with open('uld.csv', mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(data)
        
