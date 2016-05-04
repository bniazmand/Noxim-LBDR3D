/*****************************************************************************

  main.cpp -- The testbench

 *****************************************************************************/

#include <systemc.h>
#include "NoximMain.h"
#include "NoximNoC.h"
#include "NoximGlobalStats.h"
#include "NoximCmdLineParser.h"
#include <time.h>

using namespace std;

// need to be globally visible to allow "-volume" simulation stop
unsigned int drained_volume;

//---------------------------------------------------------------------------

// Initialize global configuration parameters (can be overridden with command-line arguments)
int   NoximGlobalParams::verbose_mode                     = DEFAULT_VERBOSE_MODE;
int   NoximGlobalParams::trace_mode                       = DEFAULT_TRACE_MODE;
char  NoximGlobalParams::trace_filename[128]              = DEFAULT_TRACE_FILENAME;
int   NoximGlobalParams::mesh_dim_x                       = DEFAULT_MESH_DIM_X;
int   NoximGlobalParams::mesh_dim_y                       = DEFAULT_MESH_DIM_Y;
int NoximGlobalParams::mesh_dim_z                         = DEFAULT_MESH_DIM_Z;
int   NoximGlobalParams::topology                         = DEFAULT_TOPOLOGY;
int   NoximGlobalParams::buffer_depth                     = DEFAULT_BUFFER_DEPTH;
int   NoximGlobalParams::min_packet_size                  = DEFAULT_MIN_PACKET_SIZE;
int   NoximGlobalParams::max_packet_size                  = DEFAULT_MAX_PACKET_SIZE;
int   NoximGlobalParams::routing_algorithm                = DEFAULT_ROUTING_ALGORITHM;
int   NoximGlobalParams::number_virtual_channel           = DEFAULT_NUMBER_VIRTUAL_CHANNEL;
char  NoximGlobalParams::routing_table_filename[128]      = DEFAULT_ROUTING_TABLE_FILENAME;
int   NoximGlobalParams::selection_strategy               = DEFAULT_SELECTION_STRATEGY;
float NoximGlobalParams::packet_injection_rate            = DEFAULT_PACKET_INJECTION_RATE;
float NoximGlobalParams::probability_of_retransmission    = DEFAULT_PROBABILITY_OF_RETRANSMISSION;
int   NoximGlobalParams::traffic_distribution             = DEFAULT_TRAFFIC_DISTRIBUTION;
char  NoximGlobalParams::traffic_table_filename[128]      = DEFAULT_TRAFFIC_TABLE_FILENAME;
int   NoximGlobalParams::simulation_time                  = DEFAULT_SIMULATION_TIME;
int   NoximGlobalParams::stats_warm_up_time               = DEFAULT_STATS_WARM_UP_TIME;
int   NoximGlobalParams::partitions                       = DEFAULT_PARTITIONS;
int   NoximGlobalParams::rnd_generator_seed               = time(NULL);
bool  NoximGlobalParams::detailed                         = DEFAULT_DETAILED;
float NoximGlobalParams::dyad_threshold                   = DEFAULT_DYAD_THRESHOLD;
char  NoximGlobalParams::elevator_nodes_filename[256]     = DEFAULT_ELEVATOR_NODES_FILENAME;
char  NoximGlobalParams::traffic_fname[128]               = DEFAULT_TRAFFIC_FNAME;
bool  NoximGlobalParams::rnd_traffic                      = DEFAULT_RND_TRAFFIC;
unsigned int NoximGlobalParams::data_encoding             = DEFAULT_DENC;
unsigned int NoximGlobalParams::max_volume_to_be_drained  = DEFAULT_MAX_VOLUME_TO_BE_DRAINED;
unsigned int NoximGlobalParams::flit_size_bits            = DEFAULT_FLIT_SIZE_BITS;
unsigned int NoximGlobalParams::rnd_traffic_size          = DEFAULT_RND_TRAFFIC_SIZE;
float NoximGlobalParams::bit_error_rate                   = DEFAULT_BIT_ERROR_RATE;
float NoximGlobalParams::link_vdd                         = DEFAULT_LINK_VDD;
unsigned int NoximGlobalParams::error_coding              = DEFAULT_ERROR_CODING;
vector<pair<int,double> > NoximGlobalParams::hotspots;

//---------------------------------------------------------------------------

int sc_main(int arg_num, char* arg_vet[])
{
    // TEMP
    drained_volume = 0;

  // Handle command-line arguments
  cout << endl << "\t\tNoxim - the NoC Simulator" << endl;
  cout << "\t\t(C) University of Catania" << endl << endl;

  parseCmdLine(arg_num, arg_vet);

  // Signals
  sc_clock        clock("clock", 1, SC_NS);
  sc_signal<bool> reset;

  // NoC instance
  NoximNoC* n = new NoximNoC("NoC");
  n->clock(clock);
  n->reset(reset);

  // Trace signals
  sc_trace_file* tf = NULL;
  if(NoximGlobalParams::trace_mode)
  {
    tf = sc_create_vcd_trace_file(NoximGlobalParams::trace_filename);
    sc_trace(tf, reset, "reset");
    sc_trace(tf, clock, "clock");

    for(int i=0; i<NoximGlobalParams::mesh_dim_x; i++)
    {
      for(int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
      {
        char label[30];

        sprintf(label, "req_to_east(%02d)(%02d)", i, j);
        sc_trace(tf, n->req_to_east[i][j], label);
        sprintf(label, "req_to_west(%02d)(%02d)", i, j);
        sc_trace(tf, n->req_to_west[i][j], label);
        sprintf(label, "req_to_south(%02d)(%02d)", i, j);
        sc_trace(tf, n->req_to_south[i][j], label);
        sprintf(label, "req_to_north(%02d)(%02d)", i, j);
        sc_trace(tf, n->req_to_north[i][j], label);

        sprintf(label, "ack_to_east(%02d)(%02d)", i, j);
        sc_trace(tf, n->ack_to_east[i][j], label);
        sprintf(label, "ack_to_west(%02d)(%02d)", i, j);
        sc_trace(tf, n->ack_to_west[i][j], label);
        sprintf(label, "ack_to_south(%02d)(%02d)", i, j);
        sc_trace(tf, n->ack_to_south[i][j], label);
        sprintf(label, "ack_to_north(%02d)(%02d)", i, j);
        sc_trace(tf, n->ack_to_north[i][j], label);
      }
    }
  }


  // Reset the chip and run the simulation
  reset.write(1);
  cout << "Reset...";
  srand(NoximGlobalParams::rnd_generator_seed); // time(NULL));
  sc_start(DEFAULT_RESET_TIME, SC_NS);
  reset.write(0);
  cout << " done! Now running for " << NoximGlobalParams::simulation_time << " cycles..." << endl;
  sc_start(NoximGlobalParams::simulation_time, SC_NS);

  // Close the simulation
  if(NoximGlobalParams::trace_mode) sc_close_vcd_trace_file(tf);
  cout << "Noxim simulation completed." << endl;
  cout << " ( " << sc_time_stamp().to_double()/1000 << " cycles executed)" << endl << endl;

  // Show statistics
  NoximGlobalStats gs(n);
  gs.showStats(std::cout, NoximGlobalParams::detailed);

  if ((NoximGlobalParams::max_volume_to_be_drained>0) &&
      (sc_time_stamp().to_double()/1000 >= NoximGlobalParams::simulation_time))
      {
	  cout << "\nWARNING! the number of kbytes specified with -volume option"<<endl;
	  cout << "has not been reached. ( " << drained_volume/8/1024 << " KB instead of " << NoximGlobalParams::max_volume_to_be_drained << " KB)" <<endl;
	  cout << "You might want to try an higher value of simulation cycles" << endl;
	  cout << "using -sim option." << endl;
      }

  return 0;
}
