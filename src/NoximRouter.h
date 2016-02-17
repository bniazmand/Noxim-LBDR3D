/*****************************************************************************

  NoximRouter.h -- Router definition

 *****************************************************************************/

#ifndef __NoximRouter_H__
#define __NoximRouter_H__

//---------------------------------------------------------------------------

#include <systemc.h>
#include "NoximMain.h"
#include "NoximBuffer.h"
#include "NoximStats.h"
#include "NoximGlobalRoutingTable.h"
#include "NoximLocalRoutingTable.h"
#include "NoximReservationTable.h"

SC_MODULE(NoximRouter)
{

  // I/O Ports

  sc_in_clk          clock;        // The input clock for the router
  sc_in<bool>        reset;        // The reset signal for the router

  sc_in<NoximFlit>   flit_rx[DIRECTIONS+1];   // The input channels (including local one)
  sc_in  <bool     > req_rx       [DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];	  	// The requests associated with the input channels
  sc_in<bool>        req_local_rx;
  sc_out <bool     > ack_rx       [DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];	  	// The outgoing ack signals associated with the input channels
  sc_out<bool>       ack_local_rx;

  sc_out<NoximFlit>  flit_tx[DIRECTIONS+1];   // The output channels (including local one)
  sc_out <bool     > req_tx       [DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];	  	// The requests associated with the output channels
  sc_out<bool>       req_local_tx;
  sc_in  <bool     > ack_tx       [DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];	  	// The outgoing ack signals associated with the output channels
  sc_in<bool>        ack_local_tx;

  sc_out <int> free_slots          [DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
  sc_out<int>        free_local_slots;
  sc_in  <int> free_slots_neighbor [DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
  sc_in<int>         free_local_slots_neighbor;

  // Neighbor-on-Path related I/O

  sc_out<NoximNoP_data>  NoP_data_out[DIRECTIONS];
  sc_in<NoximNoP_data>   NoP_data_in[DIRECTIONS];

  // Registers

  /*
  NoximCoord             position;                        // Router position inside the mesh
  */
  int                    local_id;                        // Unique ID
  int                    routing_type;                    // Type of routing algorithm
  int                    selection_type;
  NoximBuffer            buffer[DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL]  ;	 // Buffer for each input channel
  NoximBuffer            local_buffer;
  bool                   current_level_rx[DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL] ;
  bool                   current_level_local_rx;
  bool                   current_level_tx[DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL] ;
  bool                   current_level_local_tx;
  NoximStats             stats;                           // Statistics
  NoximLocalRoutingTable routing_table;                   // Routing table
  NoximReservationTable  reservation_table;               // Switch reservation table
  int                    start_from_port;                 // Port from which to start the reservation cycle
  unsigned long          routed_flits;
  int				     start_from_channel[DIRECTIONS];
  int				     current_local_channel;

    // LBDR3D Input port Signals, Connectivitiy and Routing Bits
    bool                   ipN, ipE, ipW, ipS;
    bool                   C_n , C_e , C_w , C_s , C_u , C_d ;
    bool                   R_ne , R_nw , R_en , R_es , R_wn , R_ws , R_se , R_sw;
    bool                   R_nu , R_un , R_eu , R_ue , R_wu , R_uw , R_su , R_us;
    bool                   R_nd , R_dn , R_ed , R_de , R_wd , R_dw , R_sd , R_ds;
    
    // LBDR3D Signals
    bool                   N , E , W , S , U , D;
    bool                   N_prime , E_prime , W_prime , S_prime , U_prime , D_prime;
    bool                   Nu_prime, Eu_prime, Wu_prime, Su_prime, Nd_prime, Ed_prime, Wd_prime, Sd_prime, NEu_prime, NWu_prime, SEu_prime, SWu_prime, NEd_prime, NWd_prime, SEd_prime, SWd_prime;
    bool                   Nu, Eu, Wu, Su, Nd, Ed, Wd, Sd; // Up and Down Elevator bits
    bool                   N_zegond , E_zegond , W_zegond , S_zegond , U_zegond , D_zegond;
    bool                   N_tierse, E_tierse, W_tierse, S_tierse;
    
    // Elevator list for each router (list of up and down and up/down vertical nodes in the network) in the current layer
    vector <int> elevator_nodes_up, elevator_nodes_down;
    
  // Functions

  void               rxProcess();        // The receiving process
  void               txProcess();        // The transmitting process
  void               bufferMonitor();
  void               configure(const int _id, const double _warm_up_time, const unsigned int _max_buffer_size,
							   NoximGlobalRoutingTable& grt);
    
  unsigned long getRoutedFlits(); // Returns the number of routed flits

  unsigned int  getFlitsCount();  // Returns the number of flits into the router

  double        getPower();  // Returns the total power dissipated by the router

  double        getRouterPower(); // Returns the power dissipated by the router

  // Constructor

  SC_CTOR(NoximRouter)
  {
    SC_METHOD(rxProcess);
    sensitive << reset;
    sensitive << clock.pos();

    SC_METHOD(txProcess);
    sensitive << reset;
    sensitive << clock.pos();

//    SC_METHOD(bufferMonitor);
//    sensitive << reset;
//    sensitive << clock.pos();
  }

 private:

  // performs actual routing + selection
  //int route(const NoximRouteData& route_data);
  int route(const NoximRouteData& route_data, int channel);

  // wrappers
  //int selectionFunction(const vector<int>& directions,const NoximRouteData& route_data);
  int selectionFunction(const vector<int>& directions,const NoximRouteData& route_data, int channel);
  //vector<int> routingFunction(const NoximRouteData& route_data);
  vector<int> routingFunction(const NoximRouteData& route_data, int channel);

  // selection strategies
  int selectionRandom(const vector<int>& directions);
  //int selectionBufferLevel(const vector<int>& directions);
  int selectionBufferLevel(const vector<int>& directions, int channel);
  //int selectionNoP(const vector<int>& directions,const NoximRouteData& route_data);
  int selectionNoP(const vector<int>& directions,const NoximRouteData& route_data, int channel);
  int selectionPathAwareMinimal (const vector<int>& directions,const NoximRouteData& route_data, int channel);
    
  // routing functions
  vector<int> routingXYZ(const NoximCoord& current, const NoximCoord& destination);
  vector < int >routingLBDR(const NoximCoord & position, const NoximCoord & destination, int dir_in);
  vector < int >routing_DQP_NETZ(const NoximCoord & current, const NoximCoord & destination, int dir_in);
  vector < int >routing_ETW(NoximCoord & current, NoximCoord & destination, int dir_in);
  vector<int> routingWestFirst(const NoximCoord& current, const NoximCoord& destination);
  vector<int> routingNorthLast(const NoximCoord& current, const NoximCoord& destination);
  vector<int> routingNegativeFirst(const NoximCoord& current, const NoximCoord& destination);
  vector<int> routingOddEven(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination);
  vector<int> routingDyAD(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination);
  vector < int >routingOddEven_for_3D(const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
  vector < int >routingOddEven_3D(const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
  vector<int> routingLookAhead(const NoximCoord& current, const NoximCoord& destination);
  vector<int> routingFullyAdaptive(const NoximCoord& current, const NoximCoord& destination);
  vector<int> routingTableBased(const int dir_in, const NoximCoord& current, const NoximCoord& destination);
  NoximNoP_data getCurrentNoPData() const;
  void NoP_report() const;
  int NoPScore(const NoximNoP_data& nop_data, const vector<int>& nop_channels, int channel) const;
  int reflexDirection(int direction) const;
  int getNeighborId(int _id,int direction) const;
  bool inCongestion();
};

//---------------------------------------------------------------------------

#endif
