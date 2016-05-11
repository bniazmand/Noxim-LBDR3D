import sys
import os


result_file = open(sys.argv[1], 'r')

temp_file = sys.argv[1]
temp_file = temp_file[:-4]
temp_file = temp_file+"_final.dot"

output_file = open(temp_file, 'w')

line = result_file.readline()

pir = None
old_pir = None

first_counter = True
counter = 1
values_list = []
while line != "":
    if 'pir' not in line:
        values = line.split()
        pir = values[0]

        if pir != old_pir:
            if not first_counter:
                output_file.write(str(old_pir)+"\t")
                for i in range(0, len(values_list)):
                    average = values_list[i]/counter
                    if average < 0.01:
                        output_file.write(str(average)+"\t")
                    else:
                        output_file.write(str("{0:.2f}".format(values_list[i]/counter))+"\t")
                output_file.write("\n")
                counter = 1
                values_list = []
            else:
                first_counter = False
                counter = 1
                values_list = []
            old_pir = pir
            for i in range(1, len(values)):
                values_list.append(float(values[i]))
        else:
            counter += 1
            for i in range(1, len(values)):
                values_list[i-1] += float(values[i])
    else:
        output_file.write(line)
    line = result_file.readline()

output_file.write(str(old_pir)+"\t")
for i in range(0, len(values_list)):
    average = values_list[i]/counter
    if average < 0.01:
        output_file.write(str(average)+"\t")
    else:
        output_file.write(str("{0:.2f}".format(values_list[i]/counter))+"\t")
output_file.write("\n")

result_file.close()
output_file.close()