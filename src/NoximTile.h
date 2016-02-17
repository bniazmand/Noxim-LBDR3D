/*****************************************************************************

  NoximTile.h -- Tile definition

 *****************************************************************************/

#ifndef __NoximTile_H__
#define __NoximTile_H__

//---------------------------------------------------------------------------

#include <systemc.h>
#include "NoximRouter.h"
#include "NoximProcessingElement.h"

SC_MODULE(NoximTile)
{

  // I/O Ports

  sc_in_clk           clock;        // The input clock for the tile
  sc_in<bool>         reset;        // The reset signal for the tile

  sc_in<NoximFlit>   flit_rx[DIRECTIONS];   // The input channels
  sc_in  <bool>       req_rx [DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];	        // The requests associated with the input channels
  sc_out <bool>       ack_rx [DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];	        // The outgoing ack signals associated with the input channels

  sc_out<NoximFlit>  flit_tx[DIRECTIONS];   // The output channels
  sc_out <bool> req_tx[DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];	        // The requests associated with the output channels
  sc_in <bool> ack_tx[DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];	        // The outgoing ack signals associated with the output channels

  sc_out <int> free_slots[DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];
  sc_in <int> free_slots_neighbor[DIRECTIONS][DEFAULT_NUMBER_VIRTUAL_CHANNEL];

  // NoP related I/O
  sc_out<NoximNoP_data>  NoP_data_out[DIRECTIONS];
  sc_in<NoximNoP_data>   NoP_data_in[DIRECTIONS];

  // Signals

  sc_signal<NoximFlit> flit_rx_local;   // The input channels
  sc_signal<bool>       req_rx_local;    // The requests associated with the input channels
  sc_signal<bool>       ack_rx_local;    // The outgoing ack signals associated with the input channels

  sc_signal<NoximFlit> flit_tx_local;   // The output channels
  sc_signal<bool>       req_tx_local;    // The requests associated with the output channels
  sc_signal<bool>       ack_tx_local;    // The outgoing ack signals associated with the output channels
  
  sc_signal<int>        free_slots_local;
  sc_signal<int>        free_slots_neighbor_local;


  // Instances
  NoximRouter*            r;               // Router instance
  NoximProcessingElement* pe;              // Processing Element instance

  
  // Constructor

  SC_CTOR(NoximTile)
  {
    // Router pin assignments
    r = new NoximRouter("Router");
    r->clock(clock);
    r->reset(reset);
    for(int i=0; i<DIRECTIONS; i++)
    {
      r->flit_rx[i](flit_rx[i]);

      r->flit_tx[i](flit_tx[i]);

		for(int j=0; j<NoximGlobalParams::number_virtual_channel; j++)
		{
			r->req_rx[i][j](req_rx[i][j]);
			r->ack_rx[i][j](ack_rx[i][j]);

			r->req_tx[i][j](req_tx[i][j]);
			r->ack_tx[i][j](ack_tx[i][j]);

			r->free_slots[i][j](free_slots[i][j]);
			r->free_slots_neighbor[i][j](free_slots_neighbor[i][j]);
		}

      // NoP 
      r->NoP_data_out[i](NoP_data_out[i]);
      r->NoP_data_in[i](NoP_data_in[i]);
    }

    r->flit_rx[DIRECTION_LOCAL](flit_tx_local);
	r->req_local_rx(req_tx_local);
	r->ack_local_rx(ack_tx_local);

    r->flit_tx[DIRECTION_LOCAL](flit_rx_local);
	r->req_local_tx(req_rx_local);
	r->ack_local_tx(ack_rx_local);

	r->free_local_slots(free_slots_local);
	r->free_local_slots_neighbor(free_slots_neighbor_local);

    // Processing Element pin assignments
    pe = new NoximProcessingElement("ProcessingElement");
    pe->clock(clock);
    pe->reset(reset);

    pe->flit_rx(flit_rx_local);
    pe->req_rx(req_rx_local);
    pe->ack_rx(ack_rx_local);

    pe->flit_tx(flit_tx_local);
    pe->req_tx(req_tx_local);
    pe->ack_tx(ack_tx_local);

    pe->free_slots_neighbor(free_slots_neighbor_local);

  }

};

//---------------------------------------------------------------------------

#endif