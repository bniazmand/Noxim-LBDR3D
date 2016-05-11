#!/bin/bash    
       mkdir Results
       cd Results/
       mkdir bitreversal
       mkdir butterfly
       mkdir random
       mkdir shuffle  
       mkdir transpose1
       mkdir transpose2
       cd ..

       ./noxim_explorer sim_random_20_and_40_percent_fault.cfg
       ./noxim_explorer sim_random_84_percent_fault.cfg
       ./noxim_explorer sim_transpose_20_and_40_percent_fault.cfg
       ./noxim_explorer sim_transpose_84_percent_fault.cfg
       ./noxim_explorer sim_bitreversal_20_and_40_percent_fault.cfg
       ./noxim_explorer sim_bitreversal_84_percent_fault.cfg

       cp elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_20_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_random__.m  ./Results/random/r_random__20_percent_fault.m
       cp elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_20_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_transpose1__.m  ./Results/transpose1/r_transpose1__20_percent_fault.m
       cp elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_20_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_bitreversal__.m  ./Results/bitreversal/r_bitreversal_20_percent_fault.m

       cp elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_40_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_random__.m  ./Results/random/r_random_40_percent_fault.m
       cp elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_40_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_transpose1__.m  ./Results/transpose1/r_transpose1_40_percent_fault.m
       cp elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_40_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_bitreversal__.m  ./Results/bitreversal/r_bitreversal_40_percent_fault.m

       cp elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_84_percent_with_pillar_txt__routing_xyz__sel_random__topology_4x4x4__traffic_random__.m  ./Results/random/r_random_84_percent_fault_with_pillar.m
       cp elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_84_percent_with_pillar_txt__routing_xyz__sel_random__topology_4x4x4__traffic_transpose1__.m  ./Results/transpose1/r_transpose1_84_percent_fault_with_pillar.m
       cp elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_84_percent_with_pillar_txt__routing_xyz__sel_random__topology_4x4x4__traffic_bitreversal__.m  ./Results/bitreversal/r_bitreversal_84_percent_fault_with_pillar.m

       rm elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_20_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_random__.m
       rm elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_20_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_transpose1__.m
       rm elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_20_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_bitreversal__.m
       rm elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_40_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_random__.m
       rm elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_40_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_transpose1__.m
       rm elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_40_percent_txt__routing_xyz__sel_random__topology_4x4x4__traffic_bitreversal__.m
       rm elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_84_percent_with_pillar_txt__routing_xyz__sel_random__topology_4x4x4__traffic_random__.m
       rm elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_84_percent_with_pillar_txt__routing_xyz__sel_random__topology_4x4x4__traffic_transpose1__.m
       rm elevator_nodes_Elevator_nodes_scenarios_elevator_nodes_84_percent_with_pillar_txt__routing_xyz__sel_random__topology_4x4x4__traffic_bitreversal__.m