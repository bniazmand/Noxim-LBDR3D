#!/bin/bash
counter=0 
[[ -d results ]] || mkdir results
ResultFile=results/${PWD##*/}_latency_result.txt

for f in *.m
do
# echo $f
awk '{ print $2 }' $f > temp.txt

if [[ $counter -lt 1 ]]
then
awk '{ print $1 }' $f > temp1.txt
sed -n 8,77p temp1.txt > $ResultFile
rm temp1.txt
fi

if [[ $counter -lt 10 ]]
then
inputNo="0${counter}"
else
inputNo=$counter
fi
sed -n 8,77p temp.txt > latency_result_$inputNo.dot
rm temp.txt
counter=`expr $counter + 1`
done


for f in *.dot
do
# echo $f
paste $ResultFile $f  > latency_result_temp.txt
cp latency_result_temp.txt $ResultFile
rm latency_result_temp.txt
done


echo "pir	0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30	31	32	33	34	35	36	37	38	39	40	41	42	43	44	45	46	47	48	49" > new_file.txt
cat $ResultFile >> new_file.txt
cp new_file.txt $ResultFile

rm new_file.txt

rm -r *.dot
