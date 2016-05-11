#!/bin/bash
[[ -d ../collected_results ]] || mkdir ../collected_results

for i in ../Results/* ; do
  if [ -d "$i" ]; then
  		cp latency_script.sh $i
  		cp energy_script.sh $i
  		cp throughput_script.sh $i
  		cd $i
  		echo moved to ${PWD##*/} folder
      if [ -d results ]; then
          echo removed results folder!
          rm -r results
      fi
      echo running the scripts
  		sh latency_script.sh && sh energy_script.sh && sh throughput_script.sh
      echo copying results to collected_results folder
  		cp results/* ../../collected_results
      echo cleaning up...
  		rm -r results
  		rm latency_script.sh && rm energy_script.sh && rm throughput_script.sh
  		cd ../../data_collection_scripts
      echo -----------------
  fi
done

cp parser_Avg.py ../collected_results
cd ../collected_results
for i in *.txt ; do
  python parser_Avg.py $i
done




