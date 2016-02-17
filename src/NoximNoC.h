/*****************************************************************************

  NoximNoC.h -- Network-on-Chip (NoC) definition

 *****************************************************************************/

#ifndef __NoximNoC_H__
#define __NoximNoC_H__

//---------------------------------------------------------------------------

#include <systemc.h>
#include "NoximTile.h"
#include "NoximGlobalTrafficTable.h"
#include "NoximGlobalRoutingTable.h"

SC_MODULE(NoximNoC)
{

  // I/O Ports

  sc_in_clk        clock;        // The input clock for the NoC
  sc_in<bool>      reset;        // The reset signal for the NoC

  // Signals

    sc_signal <bool> req_to_east                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> req_to_west                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> req_to_south                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> req_to_north                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> req_to_up                     [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> req_to_down                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];

    sc_signal <bool> ack_to_east                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> ack_to_west                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> ack_to_south                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> ack_to_north                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> ack_to_up                     [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <bool> ack_to_down                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];

    sc_signal <NoximFlit> flit_to_east             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_west             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_south            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_north            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_up               [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_down             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

    sc_signal <int> free_slots_to_east             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <int> free_slots_to_west             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <int> free_slots_to_south            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <int> free_slots_to_north            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <int> free_slots_to_up               [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
    sc_signal <int> free_slots_to_down             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUMBER_VIRTUAL_CHANNEL];

  // NoP
    sc_signal <NoximNoP_data> NoP_data_to_east     [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximNoP_data> NoP_data_to_west     [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximNoP_data> NoP_data_to_south    [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximNoP_data> NoP_data_to_north    [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximNoP_data> NoP_data_to_up       [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximNoP_data> NoP_data_to_down     [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

  // Matrix of tiles

    NoximTile  *t[MAX_STATIC_DIM][MAX_STATIC_DIM][MAX_STATIC_DIM];


  // Global tables
  NoximGlobalRoutingTable grtable;
  NoximGlobalTrafficTable gttable;

  void flitsMonitor()
  {
    if (!reset.read())
      {

	unsigned int count = 0;
	for(int i=0; i<NoximGlobalParams::mesh_dim_x; i++)
	  for(int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
          for(int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
              count += t[i][j][k]->r->getFlitsCount();
	cout << count << endl;
      }
  }

  // Constructor

  SC_CTOR(NoximNoC)
  {
    // Build the Mesh
    buildMesh();

  }

  // Support methods
  NoximTile* searchNode(const int id) const;


 private:
  void buildMesh();
};

//---------------------------------------------------------------------------

#endif