/*****************************************************************************

  NoximNoC.cpp -- Network-on-Chip (NoC) implementation

 *****************************************************************************/

#include "NoximNoC.h"
#include "NoximGlobalRoutingTable.h"
#include "NoximGlobalTrafficTable.h"

//---------------------------------------------------------------------------

 struct elevator_nodes
{
    int elevator_node_id;
    bool up,down;
};


class elevator
{
public:
    struct elevators
    {
        int elevator_node_id;
        int hop_to_elevator;
        
        elevators *next_elevator;
    };
    
    elevators *start_elevator;
    
    elevator()
    {
        start_elevator = NULL;
    }
    
    void insert_node(int elevator_node_id , int hop_to_elevator)
    {
        elevators *ptr = new elevators;
        if (!ptr) return;
        ptr->elevator_node_id = elevator_node_id;
        ptr->hop_to_elevator = hop_to_elevator;
        
        ptr->next_elevator = NULL;
        elevators *t=start_elevator , *s = NULL;
        
        while (t && (t->hop_to_elevator < hop_to_elevator))
        {
            s = t;
            t = t->next_elevator;
        }
        ptr->next_elevator = t;
        if (s == NULL)
            start_elevator = ptr;
        else s->next_elevator = ptr;
        return;
    }
    
    void sort_nodes()
    {
        elevators *temp1;                         // create a temporary node
        temp1 = new elevators; // allocate space for node
        
        elevators *temp2;                         // create a temporary node
        temp2 = new elevators; // allocate space for node
        
        int temp_1 = 0;                        // store temporary data value
        int temp_2 = 0;                        // store temporary data value
        
        for( temp1 = start_elevator ; temp1!=NULL ; temp1 = temp1->next_elevator )
        {
            for( temp2 = temp1->next_elevator ; temp2!=NULL ; temp2 = temp2->next_elevator )
            {
                if( temp1->hop_to_elevator > temp2->hop_to_elevator )
                {
                    temp_1 = temp1->elevator_node_id;
                    temp_2 = temp1->hop_to_elevator;
                    temp1->elevator_node_id = temp2->elevator_node_id;
                    temp1->hop_to_elevator = temp2->hop_to_elevator;
                    temp2->elevator_node_id = temp_1;
                    temp2->hop_to_elevator = temp_1;
                }
            }
        }
    }
    
    void print_nodes(int elevator_node_id , int hop_to_elevator)
    {
        elevators *t = start_elevator;
        while (t)
        {
            cout << "\n";
            cout << "Elevator Node ID : " << t->elevator_node_id << " : \n";
            cout << "Hop from current router to Elevator Node : " << t->elevator_node_id << " : \n";
            cout << "\n";
            
            t = t->next_elevator;
        }
    }
    
    int nearest_elevators()
    {
        elevators *t = start_elevator;
        while (t)
        {
            return t->elevator_node_id;
        }
        
        return -1;
    }
};

//---------------------------------------------------------------------------

void NoximNoC::buildMesh()
{
  // Check for routing table availability
  if (NoximGlobalParams::routing_algorithm == ROUTING_TABLE_BASED)
    assert(grtable.load(NoximGlobalParams::routing_table_filename));

  // Check for traffic table availability
  if (NoximGlobalParams::traffic_distribution == TRAFFIC_TABLE_BASED)
    assert(gttable.load(NoximGlobalParams::traffic_table_filename));

  // Create the mesh as a matrix of tiles
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++) {
        for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++) {
            for (int k = 0; k < NoximGlobalParams::mesh_dim_z; k++) {
	  // Create the single Tile with a proper name
      int tile_id = coord2Id (i, j, k);
      char tile_name[20];
      sprintf(tile_name, "Tile[%02d][%02d][%02d]", i, j, k);
      t[i][j][k] = new NoximTile(tile_name);

	  // Tell to the router its coordinates
	  t[i][j][k]->r->configure(tile_id,
				NoximGlobalParams::stats_warm_up_time,
				NoximGlobalParams::buffer_depth,
				grtable);

	  // Tell to the PE its coordinates
	  t[i][j][k]->pe->local_id = tile_id;
      t[i][j][k]->pe->traffic_table = &gttable;  // Needed to choose destination
      t[i][j][k]->pe->never_transmit = (gttable.occurrencesAsSource(t[i][j][k]->pe->local_id) == 0);

	  // Map clock and reset
	  t[i][j][k]->clock(clock);
	  t[i][j][k]->reset(reset);
/*	  if (NoximGlobalParams::topology == NET_FOLDED_TORUS)
	  {
		  if (j == 0)
		  {
			  t[i][0]->flit_rx[DIRECTION_NORTH](flit_to_north[i][1]);
			  t[i][0]->flit_tx[DIRECTION_NORTH](flit_to_south[i][1]);
			  t[i][0]->NoP_data_out[DIRECTION_NORTH](NoP_data_to_south[i][1]);
			  t[i][0]->NoP_data_in[DIRECTION_NORTH](NoP_data_to_north[i][1]);
		  }
		  else
		  {
			  t[i][j][k]->flit_rx[DIRECTION_NORTH](flit_to_south[i][j]);
			  t[i][j][k]->flit_tx[DIRECTION_NORTH](flit_to_north[i][j]);
			  t[i][j][k]->NoP_data_out[DIRECTION_NORTH](NoP_data_to_north[i][j]);
			  t[i][j][k]->NoP_data_in[DIRECTION_NORTH](NoP_data_to_south[i][j]);
		  }

		  if (i == NoximGlobalParams::mesh_dim_x-1)
		  {
		      t[NoximGlobalParams::mesh_dim_x-1][j]->flit_rx[DIRECTION_EAST](flit_to_east[NoximGlobalParams::mesh_dim_x][j]);
			  t[NoximGlobalParams::mesh_dim_x-1][j]->flit_tx[DIRECTION_EAST](flit_to_west[NoximGlobalParams::mesh_dim_x][j]);
			  t[NoximGlobalParams::mesh_dim_x-1][j]->NoP_data_out[DIRECTION_EAST](NoP_data_to_west[NoximGlobalParams::mesh_dim_x][j]);
		      t[NoximGlobalParams::mesh_dim_x-1][j]->NoP_data_in[DIRECTION_EAST](NoP_data_to_east[NoximGlobalParams::mesh_dim_x][j]);
		  }
		  else
		  {
			  t[i][j][k]->flit_rx[DIRECTION_EAST](flit_to_west[i+2][j]);
			  t[i][j][k]->flit_tx[DIRECTION_EAST](flit_to_east[i+2][j]);
			  t[i][j][k]->NoP_data_out[DIRECTION_EAST](NoP_data_to_east[i+2][j]);
		      t[i][j][k]->NoP_data_in[DIRECTION_EAST](NoP_data_to_west[i+2][j]);
		  }

		  if (j == NoximGlobalParams::mesh_dim_y-1)
		  {
		      t[i][NoximGlobalParams::mesh_dim_y-1]->flit_rx[DIRECTION_SOUTH](flit_to_south[i][NoximGlobalParams::mesh_dim_y]);
			  t[i][NoximGlobalParams::mesh_dim_y-1]->flit_tx[DIRECTION_SOUTH](flit_to_north[i][NoximGlobalParams::mesh_dim_y]);
			  t[i][NoximGlobalParams::mesh_dim_y-1]->NoP_data_out[DIRECTION_SOUTH](NoP_data_to_north[i][NoximGlobalParams::mesh_dim_y]);
			  t[i][NoximGlobalParams::mesh_dim_y-1]->NoP_data_in[DIRECTION_SOUTH](NoP_data_to_south[i][NoximGlobalParams::mesh_dim_y]);
		  }
		  else
		  {
			  t[i][j][k]->flit_rx[DIRECTION_SOUTH](flit_to_north[i][j+2]);
			  t[i][j][k]->flit_tx[DIRECTION_SOUTH](flit_to_south[i][j+2]);
			  t[i][j][k]->NoP_data_out[DIRECTION_SOUTH](NoP_data_to_south[i][j+2]);
			  t[i][j][k]->NoP_data_in[DIRECTION_SOUTH](NoP_data_to_north[i][j+2]);
		  }

		  if (i == 0)
		  {
		      t[0][j]->flit_rx[DIRECTION_WEST](flit_to_west[1][j]);
			  t[0][j]->flit_tx[DIRECTION_WEST](flit_to_east[1][j]);
			  t[0][j]->NoP_data_out[DIRECTION_WEST](NoP_data_to_east[1][j]);
			  t[0][j]->NoP_data_in[DIRECTION_WEST](NoP_data_to_west[1][j]);
		  }
		  else
		  {
			  t[i][j][k]->flit_rx[DIRECTION_WEST](flit_to_east[i][j]);
			  t[i][j][k]->flit_tx[DIRECTION_WEST](flit_to_west[i][j]);
			  t[i][j][k]->NoP_data_out[DIRECTION_WEST](NoP_data_to_west[i][j]);
			  t[i][j][k]->NoP_data_in[DIRECTION_WEST](NoP_data_to_east[i][j]);
		  }

		  for(int k=0; k<NoximGlobalParams::number_virtual_channel; k++)
		  {
			  if (j == 0)
		      {
				  t[i][0]->req_rx[DIRECTION_NORTH][k](req_to_north[i][1][k]);
			      t[i][0]->ack_rx[DIRECTION_NORTH][k](ack_to_south[i][1][k]);
				  t[i][0]->req_tx[DIRECTION_NORTH][k](req_to_south[i][1][k]);
			      t[i][0]->ack_tx[DIRECTION_NORTH][k](ack_to_north[i][1][k]);
				  t[i][0]->free_slots[DIRECTION_NORTH][k](free_slots_to_south[i][1][k]);
				  t[i][0]->free_slots_neighbor[DIRECTION_NORTH][k](free_slots_to_north[i][1][k]);
			  }
			  else
			  {
				  t[i][j][k]->req_rx[DIRECTION_NORTH][k](req_to_south[i][j][k]);
			      t[i][j][k]->ack_rx[DIRECTION_NORTH][k](ack_to_north[i][j][k]);
				  t[i][j][k]->req_tx[DIRECTION_NORTH][k](req_to_north[i][j][k]);
			      t[i][j][k]->ack_tx[DIRECTION_NORTH][k](ack_to_south[i][j][k]);
				  t[i][j][k]->free_slots[DIRECTION_NORTH][k](free_slots_to_north[i][j][k]);
				  t[i][j][k]->free_slots_neighbor[DIRECTION_NORTH][k](free_slots_to_south[i][j][k]);
			  }

			  if (i == NoximGlobalParams::mesh_dim_x-1)
		      {
			      t[NoximGlobalParams::mesh_dim_x-1][j]->req_rx[DIRECTION_EAST][k](req_to_east[NoximGlobalParams::mesh_dim_x][j][k]);
			      t[NoximGlobalParams::mesh_dim_x-1][j]->ack_rx[DIRECTION_EAST][k](ack_to_west[NoximGlobalParams::mesh_dim_x][j][k]);
			      t[NoximGlobalParams::mesh_dim_x-1][j]->req_tx[DIRECTION_EAST][k](req_to_west[NoximGlobalParams::mesh_dim_x][j][k]);
			      t[NoximGlobalParams::mesh_dim_x-1][j]->ack_tx[DIRECTION_EAST][k](ack_to_east[NoximGlobalParams::mesh_dim_x][j][k]);
				  t[NoximGlobalParams::mesh_dim_x-1][j]->free_slots[DIRECTION_EAST][k](free_slots_to_west[NoximGlobalParams::mesh_dim_x][j][k]);
			      t[NoximGlobalParams::mesh_dim_x-1][j]->free_slots_neighbor[DIRECTION_EAST][k](free_slots_to_east[NoximGlobalParams::mesh_dim_x][j][k]);
			  }
			  else
			  {
				  t[i][j][k]->req_rx[DIRECTION_EAST][k](req_to_west[i+2][j][k]);
			      t[i][j][k]->ack_rx[DIRECTION_EAST][k](ack_to_east[i+2][j][k]);
			      t[i][j][k]->req_tx[DIRECTION_EAST][k](req_to_east[i+2][j][k]);
			      t[i][j][k]->ack_tx[DIRECTION_EAST][k](ack_to_west[i+2][j][k]);
				  t[i][j][k]->free_slots[DIRECTION_EAST][k](free_slots_to_east[i+2][j][k]);
			      t[i][j][k]->free_slots_neighbor[DIRECTION_EAST][k](free_slots_to_west[i+2][j][k]);
			  }

			  if (j == NoximGlobalParams::mesh_dim_y-1)
		      {
			      t[i][NoximGlobalParams::mesh_dim_y-1]->req_rx[DIRECTION_SOUTH][k](req_to_south[i][NoximGlobalParams::mesh_dim_y][k]);
			      t[i][NoximGlobalParams::mesh_dim_y-1]->ack_rx[DIRECTION_SOUTH][k](ack_to_north[i][NoximGlobalParams::mesh_dim_y][k]);
			      t[i][NoximGlobalParams::mesh_dim_y-1]->req_tx[DIRECTION_SOUTH][k](req_to_north[i][NoximGlobalParams::mesh_dim_y][k]);
			      t[i][NoximGlobalParams::mesh_dim_y-1]->ack_tx[DIRECTION_SOUTH][k](ack_to_south[i][NoximGlobalParams::mesh_dim_y][k]);
				  t[i][NoximGlobalParams::mesh_dim_y-1]->free_slots[DIRECTION_SOUTH][k](free_slots_to_north[i][NoximGlobalParams::mesh_dim_y][k]);
				  t[i][NoximGlobalParams::mesh_dim_y-1]->free_slots_neighbor[DIRECTION_SOUTH][k](free_slots_to_south[i][NoximGlobalParams::mesh_dim_y][k]);
			  }
			  else
			  {
				  t[i][j][k]->req_rx[DIRECTION_SOUTH][k](req_to_north[i][j+2][k]);
			      t[i][j][k]->ack_rx[DIRECTION_SOUTH][k](ack_to_south[i][j+2][k]);
			      t[i][j][k]->req_tx[DIRECTION_SOUTH][k](req_to_south[i][j+2][k]);
			      t[i][j][k]->ack_tx[DIRECTION_SOUTH][k](ack_to_north[i][j+2][k]);
				  t[i][j][k]->free_slots[DIRECTION_SOUTH][k](free_slots_to_south[i][j+2][k]);
				  t[i][j][k]->free_slots_neighbor[DIRECTION_SOUTH][k](free_slots_to_north[i][j+2][k]);
			  }

			  if (i == 0)
		      {
			      t[0][j]->req_rx[DIRECTION_WEST][k](req_to_west[1][j][k]);
			      t[0][j]->ack_rx[DIRECTION_WEST][k](ack_to_east[1][j][k]);
				  t[0][j]->req_tx[DIRECTION_WEST][k](req_to_east[1][j][k]);
			      t[0][j]->ack_tx[DIRECTION_WEST][k](ack_to_west[1][j][k]);
				  t[0][j]->free_slots[DIRECTION_WEST][k](free_slots_to_east[1][j][k]);
				  t[0][j]->free_slots_neighbor[DIRECTION_WEST][k](free_slots_to_west[1][j][k]);
			  }
			  else
			  {
				  t[i][j][k]->req_rx[DIRECTION_WEST][k](req_to_east[i][j][k]);
			      t[i][j][k]->ack_rx[DIRECTION_WEST][k](ack_to_west[i][j][k]);
				  t[i][j][k]->req_tx[DIRECTION_WEST][k](req_to_west[i][j][k]);
			      t[i][j][k]->ack_tx[DIRECTION_WEST][k](ack_to_east[i][j][k]);
				  t[i][j][k]->free_slots[DIRECTION_WEST][k](free_slots_to_west[i][j][k]);
				  t[i][j][k]->free_slots_neighbor[DIRECTION_WEST][k](free_slots_to_east[i][j][k]);
			  }
		  }
	  }
 */
                
  /*
		  // Map Rx signals
		  t[i][j][k]->flit_rx[DIRECTION_NORTH](flit_to_south[i][j][k]);
		  t[i][j][k]->flit_rx[DIRECTION_EAST](flit_to_west[i+1][j][k]);
		  t[i][j][k]->flit_rx[DIRECTION_SOUTH](flit_to_north[i][j+1][k]);
		  t[i][j][k]->flit_rx[DIRECTION_WEST](flit_to_east[i][j][k]);
          t[i][j][k]->flit_rx[DIRECTION_UP](flit_to_down[i][j][k+1]);
          t[i][j][k]->flit_rx[DIRECTION_DOWN](flit_to_up[i][j][k]);

		  for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
			{
			  t[i][j][k]->req_rx[DIRECTION_NORTH][k](req_to_south[i][j][k][l]);
			  t[i][j][k]->ack_rx[DIRECTION_NORTH][k](ack_to_north[i][j][k][l]);
			  t[i][j][k]->req_rx[DIRECTION_EAST][k](req_to_west[i+1][j][k][l]);
			  t[i][j][k]->ack_rx[DIRECTION_EAST][k](ack_to_east[i+1][j][k][l]);
			  t[i][j][k]->req_rx[DIRECTION_SOUTH][k](req_to_north[i][j+1][k][l]);
			  t[i][j][k]->ack_rx[DIRECTION_SOUTH][k](ack_to_south[i][j+1][k][l]);
			  t[i][j][k]->req_rx[DIRECTION_WEST][k](req_to_east[i][j][k][l]);
			  t[i][j][k]->ack_rx[DIRECTION_WEST][k](ack_to_west[i][j][k][l]);
              t[i][j][k]->req_rx[DIRECTION_UP][k](req_to_down[i][j][k+1][l]);
              t[i][j][k]->ack_rx[DIRECTION_UP][k](ack_to_up[i][j][k+1][l]);
              t[i][j][k]->req_rx[DIRECTION_DOWN][k](req_to_up[i][j][k][l]);
              t[i][j][k]->ack_rx[DIRECTION_DOWN][k](ack_to_down[i][j][k][l]);
			}

		  // Map Tx signals
		  t[i][j][k]->flit_tx[DIRECTION_NORTH](flit_to_north[i][j][k]);
		  t[i][j][k]->flit_tx[DIRECTION_EAST](flit_to_east[i+1][j][k]);
		  t[i][j][k]->flit_tx[DIRECTION_SOUTH](flit_to_south[i][j+1][k]);
		  t[i][j][k]->flit_tx[DIRECTION_WEST](flit_to_west[i][j][k]);
          t[i][j][k]->flit_tx[DIRECTION_UP](flit_to_up[i][j][k+1]);
          t[i][j][k]->flit_tx[DIRECTION_DOWN](flit_to_down[i][j][k]);

		  for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
		  {
			  t[i][j][k]->req_tx[DIRECTION_NORTH][k](req_to_north[i][j][k][l]);
			  t[i][j][k]->ack_tx[DIRECTION_NORTH][k](ack_to_south[i][j][k][l]);
			  t[i][j][k]->req_tx[DIRECTION_EAST][k](req_to_east[i+1][j][k][l]);
			  t[i][j][k]->ack_tx[DIRECTION_EAST][k](ack_to_west[i+1][j][k][l]);
			  t[i][j][k]->req_tx[DIRECTION_SOUTH][k](req_to_south[i][j+1][k][l]);
			  t[i][j][k]->ack_tx[DIRECTION_SOUTH][k](ack_to_north[i][j+1][k][l]);
			  t[i][j][k]->req_tx[DIRECTION_WEST][k](req_to_west[i][j][k][l]);
			  t[i][j][k]->ack_tx[DIRECTION_WEST][k](ack_to_east[i][j][k][l]);
              t[i][j][k]->req_tx[DIRECTION_UP][k](req_to_up[i][j][k+1][l]);
              t[i][j][k]->ack_tx[DIRECTION_UP][k](ack_to_down[i][j][k+1][l]);
              t[i][j][k]->req_tx[DIRECTION_DOWN][k](req_to_down[i][j][k][l]);
              t[i][j][k]->ack_tx[DIRECTION_DOWN][k](ack_to_up[i][j][k][l]);
		  }

		  // Map buffer level signals (analogy with req_tx/rx port mapping)
		  for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
		  {
			  t[i][j][k]->free_slots[DIRECTION_NORTH][k](free_slots_to_north[i][j][k][l]);
			  t[i][j][k]->free_slots[DIRECTION_EAST][k](free_slots_to_east[i+1][j][k][l]);
			  t[i][j][k]->free_slots[DIRECTION_SOUTH][k](free_slots_to_south[i][j+1][k][l]);
			  t[i][j][k]->free_slots[DIRECTION_WEST][k](free_slots_to_west[i][j][k][l]);
              t[i][j][k]->free_slots[DIRECTION_UP][k](free_slots_to_up[i][j][k+1][l]);
              t[i][j][k]->free_slots[DIRECTION_DOWN][k](free_slots_to_down[i][j][k][l]);
		  }

		  for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
		  {
			  t[i][j][k]->free_slots_neighbor[DIRECTION_NORTH][k](free_slots_to_south[i][j][k][l]);
			  t[i][j][k]->free_slots_neighbor[DIRECTION_EAST][k](free_slots_to_west[i+1][j][k][l]);
			  t[i][j][k]->free_slots_neighbor[DIRECTION_SOUTH][k](free_slots_to_north[i][j+1][k][l]);
			  t[i][j][k]->free_slots_neighbor[DIRECTION_WEST][k](free_slots_to_east[i][j][k][l]);
              t[i][j][k]->free_slots_neighbor[DIRECTION_UP][k](free_slots_to_down[i][j][k+1][l]);
              t[i][j][k]->free_slots_neighbor[DIRECTION_DOWN][k](free_slots_to_up[i][j][k][l]);
		  }

		  // NoP

		  t[i][j][k]->NoP_data_out[DIRECTION_NORTH](NoP_data_to_north[i][j][k]);
		  t[i][j][k]->NoP_data_out[DIRECTION_EAST](NoP_data_to_east[i+1][j][k]);
		  t[i][j][k]->NoP_data_out[DIRECTION_SOUTH](NoP_data_to_south[i][j+1][k]);
		  t[i][j][k]->NoP_data_out[DIRECTION_WEST](NoP_data_to_west[i][j][k]);
          t[i][j][k]->NoP_data_out[DIRECTION_UP](NoP_data_to_up[i][j][k+1]);
          t[i][j][k]->NoP_data_out[DIRECTION_DOWN](NoP_data_to_down[i][j][k]);

		  t[i][j][k]->NoP_data_in[DIRECTION_NORTH](NoP_data_to_south[i][j][k]);
		  t[i][j][k]->NoP_data_in[DIRECTION_EAST](NoP_data_to_west[i+1][j][k]);
		  t[i][j][k]->NoP_data_in[DIRECTION_SOUTH](NoP_data_to_north[i][j+1][k]);
		  t[i][j][k]->NoP_data_in[DIRECTION_WEST](NoP_data_to_east[i][j][k]);
          t[i][j][k]->NoP_data_in[DIRECTION_UP](NoP_data_to_down[i][j][k+1]);
          t[i][j][k]->NoP_data_in[DIRECTION_DOWN](NoP_data_to_up[i][j][k]);
                
                */
                
                
                // Map Rx signals
                
                for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
                {
                    t[i][j][k]->req_rx             [DIRECTION_NORTH][l] (req_to_south       [i  ][j  ][k  ][l]);
                    t[i][j][k]->ack_rx             [DIRECTION_NORTH][l] (ack_to_north       [i  ][j  ][k  ][l]);
                    t[i][j][k]->req_rx             [DIRECTION_EAST ][l] (req_to_west        [i+1][j  ][k  ][l]);
                    t[i][j][k]->ack_rx             [DIRECTION_EAST ][l] (ack_to_east        [i+1][j  ][k  ][l]);
                    t[i][j][k]->req_rx             [DIRECTION_SOUTH][l] (req_to_north       [i  ][j+1][k  ][l]);
                    t[i][j][k]->ack_rx             [DIRECTION_SOUTH][l] (ack_to_south       [i  ][j+1][k  ][l]);
                    t[i][j][k]->req_rx             [DIRECTION_WEST ][l] (req_to_east        [i  ][j  ][k  ][l]);
                    t[i][j][k]->ack_rx             [DIRECTION_WEST ][l] (ack_to_west        [i  ][j  ][k  ][l]);
                    
//                    if( k < NoximGlobalParams::mesh_dim_z - 1)
                    {
                        t[i][j][k]->req_rx             [DIRECTION_UP   ][l] (req_to_down       [i  ][j  ][k  ][l]);
                        t[i][j][k]->ack_rx             [DIRECTION_UP   ][l] (ack_to_up         [i  ][j  ][k  ][l]);
                        t[i][j][k]->req_rx             [DIRECTION_DOWN ][l] (req_to_up         [i  ][j  ][k+1][l]);
                        t[i][j][k]->ack_rx             [DIRECTION_DOWN ][l] (ack_to_down       [i  ][j  ][k+1][l]);
                        }
                }
                
                t[i][j][k]->flit_rx            [DIRECTION_NORTH] (flit_to_south      [i  ][j  ][k  ]);
                t[i][j][k]->flit_rx            [DIRECTION_EAST ] (flit_to_west       [i+1][j  ][k  ]);
                t[i][j][k]->flit_rx            [DIRECTION_SOUTH] (flit_to_north      [i  ][j+1][k  ]);
                t[i][j][k]->flit_rx            [DIRECTION_WEST ] (flit_to_east       [i  ][j  ][k  ]);
                
//                if( k < NoximGlobalParams::mesh_dim_z - 1)
                {
                    t[i][j][k]->flit_rx            [DIRECTION_UP   ] (flit_to_down      [i  ][j  ][k  ]);
                    t[i][j][k]->flit_rx            [DIRECTION_DOWN ] (flit_to_up        [i  ][j  ][k+1]);
                }
                
                // Map Tx signals
                
                for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
                {
                    t[i][j][k]->req_tx             [DIRECTION_NORTH][l] (req_to_north       [i  ][j  ][k  ][l]);
                    t[i][j][k]->ack_tx             [DIRECTION_NORTH][l] (ack_to_south       [i  ][j  ][k  ][l]);
                    t[i][j][k]->req_tx             [DIRECTION_EAST ][l] (req_to_east        [i+1][j  ][k  ][l]);
                    t[i][j][k]->ack_tx             [DIRECTION_EAST ][l] (ack_to_west        [i+1][j  ][k  ][l]);
                    t[i][j][k]->req_tx             [DIRECTION_SOUTH][l] (req_to_south       [i  ][j+1][k  ][l]);
                    t[i][j][k]->ack_tx             [DIRECTION_SOUTH][l] (ack_to_north       [i  ][j+1][k  ][l]);
                    t[i][j][k]->req_tx             [DIRECTION_WEST ][l] (req_to_west        [i  ][j  ][k  ][l]);
                    t[i][j][k]->ack_tx             [DIRECTION_WEST ][l] (ack_to_east        [i  ][j  ][k  ][l]);
                    
//                    if( k < NoximGlobalParams::mesh_dim_z - 1)
                    {
                        t[i][j][k]->req_tx             [DIRECTION_UP   ][l] (req_to_up         [i  ][j  ][k  ][l]);
                        t[i][j][k]->ack_tx             [DIRECTION_UP   ][l] (ack_to_down       [i  ][j  ][k  ][l]);
                        t[i][j][k]->req_tx             [DIRECTION_DOWN ][l] (req_to_down       [i  ][j  ][k+1][l]);
                        t[i][j][k]->ack_tx             [DIRECTION_DOWN ][l] (ack_to_up         [i  ][j  ][k+1][l]);
                    }
                }
                
                t[i][j][k]->flit_tx            [DIRECTION_NORTH] (flit_to_north      [i  ][j  ][k  ]);
                t[i][j][k]->flit_tx            [DIRECTION_EAST ] (flit_to_east       [i+1][j  ][k  ]);
                t[i][j][k]->flit_tx            [DIRECTION_SOUTH] (flit_to_south      [i  ][j+1][k  ]);
                t[i][j][k]->flit_tx            [DIRECTION_WEST ] (flit_to_west       [i  ][j  ][k  ]);
                
//                if( k < NoximGlobalParams::mesh_dim_z - 1)
                {
                    t[i][j][k]->flit_tx            [DIRECTION_UP   ] (flit_to_up        [i  ][j  ][k  ]);
                    t[i][j][k]->flit_tx            [DIRECTION_DOWN ] (flit_to_down      [i  ][j  ][k+1]);
                }
                
                // Map buffer level signals (analogy with req_tx/rx port mapping)
                
                for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
                {
                    t[i][j][k]->free_slots         [DIRECTION_NORTH][l] (free_slots_to_north[i  ][j  ][k  ][l]);
                    t[i][j][k]->free_slots         [DIRECTION_EAST ][l] (free_slots_to_east [i+1][j  ][k  ][l]);
                    t[i][j][k]->free_slots         [DIRECTION_SOUTH][l] (free_slots_to_south[i  ][j+1][k  ][l]);
                    t[i][j][k]->free_slots         [DIRECTION_WEST ][l] (free_slots_to_west [i  ][j  ][k  ][l]);
                    t[i][j][k]->free_slots         [DIRECTION_UP   ][l] (free_slots_to_up   [i  ][j  ][k  ][l]);
                    t[i][j][k]->free_slots         [DIRECTION_DOWN ][l] (free_slots_to_down [i  ][j  ][k+1][l]);
                    
                    t[i][j][k]->free_slots_neighbor[DIRECTION_NORTH][l] (free_slots_to_south[i  ][j  ][k  ][l]);
                    t[i][j][k]->free_slots_neighbor[DIRECTION_EAST ][l] (free_slots_to_west [i+1][j  ][k  ][l]);
                    t[i][j][k]->free_slots_neighbor[DIRECTION_SOUTH][l] (free_slots_to_north[i  ][j+1][k  ][l]);
                    t[i][j][k]->free_slots_neighbor[DIRECTION_WEST ][l] (free_slots_to_east [i  ][j  ][k  ][l]);
                    t[i][j][k]->free_slots_neighbor[DIRECTION_UP   ][l] (free_slots_to_down [i  ][j  ][k  ][l]);
                    t[i][j][k]->free_slots_neighbor[DIRECTION_DOWN ][l] (free_slots_to_up   [i  ][j  ][k+1][l]);
                }
                
                // NoP
                t[i][j][k]->NoP_data_out       [DIRECTION_NORTH] (NoP_data_to_north  [i  ][j  ][k  ]);
                t[i][j][k]->NoP_data_out       [DIRECTION_EAST ] (NoP_data_to_east   [i+1][j  ][k  ]);
                t[i][j][k]->NoP_data_out       [DIRECTION_SOUTH] (NoP_data_to_south  [i  ][j+1][k  ]);
                t[i][j][k]->NoP_data_out       [DIRECTION_WEST ] (NoP_data_to_west   [i  ][j  ][k  ]);
                t[i][j][k]->NoP_data_out       [DIRECTION_UP   ] (NoP_data_to_up     [i  ][j  ][k  ]);
                t[i][j][k]->NoP_data_out       [DIRECTION_DOWN ] (NoP_data_to_down   [i  ][j  ][k+1]);
                
                t[i][j][k]->NoP_data_in        [DIRECTION_NORTH] (NoP_data_to_south  [i  ][j  ][k  ]);
                t[i][j][k]->NoP_data_in        [DIRECTION_EAST ] (NoP_data_to_west   [i+1][j  ][k  ]);
                t[i][j][k]->NoP_data_in        [DIRECTION_SOUTH] (NoP_data_to_north  [i  ][j+1][k  ]);
                t[i][j][k]->NoP_data_in        [DIRECTION_WEST ] (NoP_data_to_east   [i  ][j  ][k  ]);
                t[i][j][k]->NoP_data_in        [DIRECTION_UP   ] (NoP_data_to_down   [i  ][j  ][k  ]);
                t[i][j][k]->NoP_data_in        [DIRECTION_DOWN ] (NoP_data_to_up     [i  ][j  ][k+1]);
    }
    }
    }
    
    // LBDR Connectivity Bits Initialization (C_n , C_e , C_w , C_s)
    
    // Setting Connectivity Bits (excluding UP and DOWN bits) for all nodes
    
    for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
        for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
            for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
            {
                t[i1][j1][k1] -> r -> C_n = true;
                t[i1][j1][k1] -> r -> C_e = true;
                t[i1][j1][k1] -> r -> C_w = true;
                t[i1][j1][k1] -> r -> C_s = true;
            }
    
    // Setting Connectivity Bits (UP and DOWN) for all planes
    
    for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
        for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
            for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
            {
                t[i1][j1][k1] -> r -> C_u = false;
                t[i1][j1][k1] -> r -> C_d = false;
            }
    
    // Setting Connectivity Bits for Boundary planes
    
    for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
        for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
        {
            t[0][j1][k1] -> r -> C_w = false;
            t[NoximGlobalParams::mesh_dim_x-1][j1][k1] -> r -> C_e = false;
        }
    
    for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
        for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
        {
            t[i1][0][k1] -> r -> C_n = false;
            t[i1][NoximGlobalParams::mesh_dim_y-1][k1] -> r -> C_s = false;
        }

    // Vertical bit initialization (Nu, Eu, Wu, Su, Nd, Ed, Wd and Sd) for UP and DOWN elevator nodes (or both)
    
    ifstream inputFile;
    inputFile.open("elevator_nodes.txt");
    
    int no_of_elevator_nodes;
    inputFile >> no_of_elevator_nodes;
    
    elevator_nodes elevator_list[no_of_elevator_nodes];
    
    for (int count = 0; count < no_of_elevator_nodes; count++)
    {
        inputFile >> elevator_list[count].elevator_node_id >> elevator_list[count].up >> elevator_list[count].down;
    }
    
    inputFile.close();
    
    // Setting Connectivity Bits (UP and DOWN bits) for elevator nodes in all planes
    
    for (int p1 = 0; p1 < no_of_elevator_nodes; p1++)
    {
        NoximCoord coordinate;
        coordinate = id2Coord(elevator_list[p1].elevator_node_id);
        if (elevator_list[p1].up == true)
            t[coordinate.x][coordinate.y][coordinate.z] -> r -> C_u = true;
        if (elevator_list[p1].down == true)
            t[coordinate.x][coordinate.y][coordinate.z] -> r -> C_d = true;
    }
    
    // New code (Making the topology full 3D Mesh)
/*
     for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z - 1; k1++)
     for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
     for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
     {
     t[i1][j1][k1] -> r -> C_d = true;
     }
     
     for (int k1 = 1; k1 < NoximGlobalParams::mesh_dim_z; k1++)
     for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
     for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
     {
     t[i1][j1][k1] -> r -> C_u = true;
     }
*/
    
    // Resetting of elevator bits for routers
    
    for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
        for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
            for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
            {
                t[i1][j1][k1] -> r -> Nu = false;
                t[i1][j1][k1] -> r -> Eu = false;
                t[i1][j1][k1] -> r -> Wu = false;
                t[i1][j1][k1] -> r -> Su = false;
                
                t[i1][j1][k1] -> r -> Nd = false;
                t[i1][j1][k1] -> r -> Ed = false;
                t[i1][j1][k1] -> r -> Wd = false;
                t[i1][j1][k1] -> r -> Sd = false;
            }
    
    // End of Initialization of elevator bits for routers
    
    // New algorithm for setting elevator bits
    
    elevator up_elevator_array[4][4][4];
    elevator down_elevator_array[4][4][4];
    
    if (NoximGlobalParams::routing_algorithm == ROUTING_XYZ || NoximGlobalParams::routing_algorithm == ROUTING_WEST_FIRST || NoximGlobalParams::routing_algorithm == ROUTING_LBDR || NoximGlobalParams::routing_algorithm == ROUTING_NORTH_LAST || NoximGlobalParams::routing_algorithm == ROUTING_NEGATIVE_FIRST || NoximGlobalParams::routing_algorithm == ROUTING_ODD_EVEN)
    {
    // Setting "up" elevator bits

    for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)  // for each layer
    {
//                cout << "\nStatus of nodes having information of elevator nodes on layer " << k1 << " : \n";
        
        for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)      // for each node on the plane
            for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
            {
                //                cout << "\n\nWe are now at node: " << xyz2Id(i1, j1, k1) << endl;
                
                if (t[i1][j1][k1] -> r -> C_u == false) // If current node is not an up elevator itself
                {
                    for (int p1 = 0; p1 < no_of_elevator_nodes; p1++)  // Check the up elevator nodes in elevator nodes array
                    {
                        NoximCoord elevator_coord;
                        elevator_coord = id2Coord(elevator_list[p1].elevator_node_id);
                        
                        if (elevator_list[p1].elevator_node_id <= xyz2Id(NoximGlobalParams::mesh_dim_x-1,NoximGlobalParams::mesh_dim_y-1,k1) && elevator_list[p1].elevator_node_id >= xyz2Id(0,0,k1) && elevator_list[p1].up == true && elevator_coord.z == k1) // focusing on the up elevator node belonging to the current layer
                        {
                            int x_offset = abs(elevator_coord.x - i1);
                            int y_offset = abs(elevator_coord.y - j1);
                            
                            // Creating up elevator array
                            up_elevator_array[i1][j1][k1].insert_node(elevator_list[p1].elevator_node_id, x_offset + y_offset);
                            
                            // add the up vertical node to the array of up vertical nodes at current router
                            t[i1][j1][k1] -> r -> elevator_nodes_up.push_back(elevator_list[p1].elevator_node_id);
                            
                            /*
                            cout << "\nUp Elevator Nodes: \n";
                            
                            for (int i=0; i < t[i1][j1][k1] -> r -> elevator_nodes_up.size(); i++)
                            {
                                cout << t[i1][j1][k1] -> r -> elevator_nodes_up[i] << " ";
                            }
                            */
                            
                        }
                    }
                    
                    up_elevator_array[i1][j1][k1].sort_nodes();
                    NoximCoord nearest_elevator;
                    
                    nearest_elevator = id2Coord(up_elevator_array[i1][j1][k1].nearest_elevators());
                    
                    if (up_elevator_array[i1][j1][k1].nearest_elevators() != -1) // if there is a near elevator node (there is at least one elevator)
                    {
                        int x_offset = nearest_elevator.x - i1;
                        int y_offset = nearest_elevator.y - j1;
                        
                        if (x_offset > 0 && y_offset < 0) // NE quadrant
                        {
                            t[i1][j1][k1] -> r -> Nu = true;
                            t[i1][j1][k1] -> r -> Eu = true;
                        }
                        if (x_offset < 0 && y_offset < 0) // NW quadrant
                        {
                            t[i1][j1][k1] -> r -> Nu = true;
                            t[i1][j1][k1] -> r -> Wu = true;
                        }
                        if (x_offset > 0 && y_offset > 0) // SE quadrant
                        {
                            t[i1][j1][k1] -> r -> Su = true;
                            t[i1][j1][k1] -> r -> Eu = true;
                        }
                        if (x_offset < 0 && y_offset > 0) // SW quadrant
                        {
                            t[i1][j1][k1] -> r -> Su = true;
                            t[i1][j1][k1] -> r -> Wu = true;
                        }
                        if (x_offset == 0 && y_offset < 0) // N direction
                            t[i1][j1][k1] -> r -> Nu = true;
                        if (x_offset == 0 && y_offset > 0) // S direction
                            t[i1][j1][k1] -> r -> Su = true;
                        if (x_offset > 0 && y_offset == 0) // E direction
                            t[i1][j1][k1] -> r -> Eu = true;
                        if (x_offset < 0 && y_offset == 0) // W direction
                            t[i1][j1][k1] -> r -> Wu = true;
                    }
                    
                    if (k1 == 0)
                    {
                        t[i1][j1][k1] -> r -> Nu = false;
                        t[i1][j1][k1] -> r -> Su = false;
                        t[i1][j1][k1] -> r -> Eu = false;
                        t[i1][j1][k1] -> r -> Wu = false;
                    }
                }
                
                // For Debug
                //                    cout << "\n\n";
        /*
                if (t[i1][j1][k1] -> r -> C_u == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] is an up elevator\n";
                if (t[i1][j1][k1] -> r -> Nu == true && t[i1][j1][k1] -> r -> Eu == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on NE quadrant\n";
                if (t[i1][j1][k1] -> r -> Nu == true && t[i1][j1][k1] -> r -> Wu == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on NW quadrant\n";
                if (t[i1][j1][k1] -> r -> Su == true && t[i1][j1][k1] -> r -> Eu == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on SE quadrant\n";
                if (t[i1][j1][k1] -> r -> Su == true && t[i1][j1][k1] -> r -> Wu == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on SW quadrant\n";
                if (t[i1][j1][k1] -> r -> Nu == true && t[i1][j1][k1] -> r -> Eu == false && t[i1][j1][k1] -> r -> Wu == false)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on N direction\n";
                if (t[i1][j1][k1] -> r -> Eu == true && t[i1][j1][k1] -> r -> Nu == false && t[i1][j1][k1] -> r -> Su == false)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on E direction\n";
                if (t[i1][j1][k1] -> r -> Wu == true && t[i1][j1][k1] -> r -> Nu == false && t[i1][j1][k1] -> r -> Su == false)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on W direction\n";
                if (t[i1][j1][k1] -> r -> Su == true && t[i1][j1][k1] -> r -> Eu == false && t[i1][j1][k1] -> r -> Wu == false)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on S direction\n";
                */
            }
    }

    
    // Setting "down" elevator bits

    for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)  // for each layer
    {
//                cout << "\nStatus of nodes having information of elevator nodes on layer " << k1 << " : \n";
        
        for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)      // for each node on the plane
            for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
            {
                if (t[i1][j1][k1] -> r -> C_d == false) // If current node is not an up elevator itself
                {
                    for (int p1 = 0; p1 < no_of_elevator_nodes; p1++)  // Check the up elevator nodes in elevator nodes array
                    {
                        NoximCoord elevator_coord;
                        elevator_coord = id2Coord(elevator_list[p1].elevator_node_id);
                        
                        if (elevator_list[p1].elevator_node_id <= xyz2Id(NoximGlobalParams::mesh_dim_x-1,NoximGlobalParams::mesh_dim_y-1,k1) && elevator_list[p1].elevator_node_id >= xyz2Id(0,0,k1) && elevator_list[p1].down == true && elevator_coord.z == k1) // focusing on the up elevator node belonging to the current layer
                        {
                            
                            int x_offset = abs(elevator_coord.x - i1);
                            int y_offset = abs(elevator_coord.y - j1);
                            
                            // Creating up elevator array
                            down_elevator_array[i1][j1][k1].insert_node(elevator_list[p1].elevator_node_id, x_offset + y_offset);

                            // add the down vertical node to the array of up vertical nodes at current router
                            t[i1][j1][k1] -> r -> elevator_nodes_down.push_back(elevator_list[p1].elevator_node_id);
                        }
                    }
                    
                    down_elevator_array[i1][j1][k1].sort_nodes();
                    NoximCoord nearest_elevator;
                    
                    nearest_elevator = id2Coord(down_elevator_array[i1][j1][k1].nearest_elevators());
                    
                    if (down_elevator_array[i1][j1][k1].nearest_elevators() != -1) // if there is a near elevator node (there is at least one elevator)
                    {
                        int x_offset = nearest_elevator.x - i1;
                        int y_offset = nearest_elevator.y - j1;
                        
                        if (x_offset > 0 && y_offset < 0) // NE quadrant
                        {
                            t[i1][j1][k1] -> r -> Nd = true;
                            t[i1][j1][k1] -> r -> Ed = true;
                        }
                        if (x_offset < 0 && y_offset < 0) // NW quadrant
                        {
                            t[i1][j1][k1] -> r -> Nd = true;
                            t[i1][j1][k1] -> r -> Wd = true;
                        }
                        if (x_offset > 0 && y_offset > 0) // SE quadrant
                        {
                            t[i1][j1][k1] -> r -> Sd = true;
                            t[i1][j1][k1] -> r -> Ed = true;
                        }
                        if (x_offset < 0 && y_offset > 0) // SW quadrant
                        {
                            t[i1][j1][k1] -> r -> Sd = true;
                            t[i1][j1][k1] -> r -> Wd = true;
                        }
                        if (x_offset == 0 && y_offset < 0) // N direction
                            t[i1][j1][k1] -> r -> Nd = true;
                        if (x_offset == 0 && y_offset > 0) // S direction
                            t[i1][j1][k1] -> r -> Sd = true;
                        if (x_offset > 0 && y_offset == 0) // E direction
                            t[i1][j1][k1] -> r -> Ed = true;
                        if (x_offset < 0 && y_offset == 0) // W direction
                            t[i1][j1][k1] -> r -> Wd = true;
                    }
                    
                    if (k1 == NoximGlobalParams::mesh_dim_z - 1)
                    {
                        t[i1][j1][k1] -> r -> Nd = false;
                        t[i1][j1][k1] -> r -> Sd = false;
                        t[i1][j1][k1] -> r -> Ed = false;
                        t[i1][j1][k1] -> r -> Wd = false;
                    }
                }
                
                // For Debug
                //                    cout << "\n\n";
        /*
                if (t[i1][j1][k1] -> r -> C_d == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] is a down elevator\n";
                if (t[i1][j1][k1] -> r -> Nd == true && t[i1][j1][k1] -> r -> Ed == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on NE quadrant\n";
                if (t[i1][j1][k1] -> r -> Nd == true && t[i1][j1][k1] -> r -> Wd == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on NW quadrant\n";
                if (t[i1][j1][k1] -> r -> Sd == true && t[i1][j1][k1] -> r -> Ed == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on SE quadrant\n";
                if (t[i1][j1][k1] -> r -> Sd == true && t[i1][j1][k1] -> r -> Wd == true)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on SW quadrant\n";
                if (t[i1][j1][k1] -> r -> Nd == true && t[i1][j1][k1] -> r -> Ed == false && t[i1][j1][k1] -> r -> Wd == false)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on N direction\n";
                if (t[i1][j1][k1] -> r -> Ed == true && t[i1][j1][k1] -> r -> Nd == false && t[i1][j1][k1] -> r -> Sd == false)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on E direction\n";
                if (t[i1][j1][k1] -> r -> Wd == true && t[i1][j1][k1] -> r -> Nd == false && t[i1][j1][k1] -> r -> Sd == false)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on W direction\n";
                if (t[i1][j1][k1] -> r -> Sd == true && t[i1][j1][k1] -> r -> Ed == false && t[i1][j1][k1] -> r -> Wd == false)
                    cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on S direction\n";
         */
            }
    }

    }
    
    // New algorithm for Setting "up" and "down" elevator bits (for DQP-NETZ)
    
    if (NoximGlobalParams::routing_algorithm == ROUTING_DQP_NETZ)
    {
        // For all nodes in the network (UP)
        for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)  // for each layer
        {
//            cout << "\nStatus of nodes having information of up elevator nodes on layer " << k1 << " : \n";

            for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++) // for each node on the plane
                for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
                {
                    // For each node other than current node (tile) t[i2][j2][k2]
                    // setting up elevator bits (Nu, Eu, Wu and Su)
                    // Check the up elevator nodes in elevator nodes array
                    for (int i2 = 0 ; i2 < no_of_elevator_nodes ; i2++)
                    {
                        NoximCoord elevator_coord;
                        elevator_coord = id2Coord(elevator_list[i2].elevator_node_id);
                        
                        // focusing on the up elevator node belonging to the current layer
                        if (elevator_list[i2].elevator_node_id <= coord2Id(NoximGlobalParams::mesh_dim_x-1,NoximGlobalParams::mesh_dim_y-1,k1) && elevator_list[i2].elevator_node_id >= xyz2Id(0,0,k1) && elevator_list[i2].up == true && elevator_coord.z == k1 && elevator_list[i2].elevator_node_id != coord2Id(i1,j1,k1))
                        {
                            int x_offset = elevator_coord.x - i1;
                            int y_offset = elevator_coord.y - j1;
                            
                            if (y_offset < 0) // N direction
                                t[i1][j1][k1] -> r -> Nu = true;
                            if (x_offset > 0) // E direction
                                t[i1][j1][k1] -> r -> Eu = true;
                            if (x_offset > 0 && y_offset > 0)  // NE quadrant
                            {
                                t[i1][j1][k1] -> r -> Nu = true;
                                t[i1][j1][k1] -> r -> Eu = true;
                            }
                        }
                    }
                    
                    
                    // For Debug
                    //                    cout << "\n\n";
          /*
                    if (t[i1][j1][k1] -> r -> C_u == true)
                        cout << "Node [" << coord2Id(i1,j1,k1) << "] is an up elevator\n";
                    if (t[i1][j1][k1] -> r -> Nu == true && t[i1][j1][k1] -> r -> Eu == false && t[i1][j1][k1] -> r -> Wu == false)
                        cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on N direction\n";
                    if (t[i1][j1][k1] -> r -> Eu == true && t[i1][j1][k1] -> r -> Nu == false && t[i1][j1][k1] -> r -> Su == false)
                        cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on E direction\n";
                    if (t[i1][j1][k1] -> r -> Nu == true && t[i1][j1][k1] -> r -> Eu == true)
                        cout << "Node [" << coord2Id(i1,j1,k1) << "] has an up elevator on NE quadrant\n";
           */
                    
//                    if (coord2Id(i1,j1,k1) == 44)
//                        if (t[i1][j1][k1] -> r -> Nu == true)
//                            cout << "\n\nAt node 44 !\n\n";
                }
        }
        
        
        // For all nodes in the network (DOWN)
        for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)  // for each layer
        {
//            cout << "\nStatus of nodes having information of down elevator nodes on layer " << k1 << " : \n";
            
            for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++) // for each node on the plane
                for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
                {

                    // For each node other than current node (tile) t[i2][j2][k2]
                    // setting down elevator bits (Nd, Ed, Wd and Sd)
                    // Check the down elevator nodes in elevator nodes array
                    for (int i2 = 0 ; i2 < no_of_elevator_nodes ; i2++)
                    {
                        NoximCoord elevator_coord;
                        elevator_coord = id2Coord(elevator_list[i2].elevator_node_id);
                        
                        // focusing on the up elevator node belonging to the current layer
                        if (elevator_list[i2].elevator_node_id <= coord2Id(NoximGlobalParams::mesh_dim_x-1,NoximGlobalParams::mesh_dim_y-1,k1) && elevator_list[i2].elevator_node_id >= xyz2Id(0,0,k1) && elevator_list[i2].down == true && elevator_coord.z == k1 && elevator_list[i2].elevator_node_id != coord2Id(i1,j1,k1))
                        {
                            int x_offset = elevator_coord.x - i1;
                            int y_offset = elevator_coord.y - j1;
                            
                            if (x_offset == 0 && y_offset < 0) // N direction
                                t[i1][j1][k1] -> r -> Nd = true;
                            if (x_offset > 0 && y_offset == 0) // E direction
                                t[i1][j1][k1] -> r -> Ed = true;
                            if (x_offset > 0 && y_offset < 0)  // NE quadrant
                            {
                                t[i1][j1][k1] -> r -> Nd = true;
                                t[i1][j1][k1] -> r -> Ed = true;
                            }
                        }
                    }

                    
                    // For Debug
                    //                    cout << "\n\n";
           /*
                    if (t[i1][j1][k1] -> r -> C_d == true)
                        cout << "Node [" << coord2Id(i1,j1,k1) << "] is a down elevator\n";
                    if (t[i1][j1][k1] -> r -> Nd == true && t[i1][j1][k1] -> r -> Ed == false && t[i1][j1][k1] -> r -> Wd == false)
                        cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on N direction\n";
                    if (t[i1][j1][k1] -> r -> Ed == true && t[i1][j1][k1] -> r -> Nd == false && t[i1][j1][k1] -> r -> Sd == false)
                        cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on E direction\n";
                    if (t[i1][j1][k1] -> r -> Nd == true && t[i1][j1][k1] -> r -> Ed == true)
                        cout << "Node [" << coord2Id(i1,j1,k1) << "] has a down elevator on NE quadrant\n";
              */      
//                    if (coord2Id(i1,j1,k1) == 44)
//                        if (t[i1][j1][k1] -> r -> Nu == true)
//                            cout << "\n\nAt node 44 !\n\n";
                }
        }
        
        cout << "\n";
    }
    
    // LBDR Routing Bits Initialization (R_ne , R_nw , R_en , R_es , R_wn , R_ws , R_se , R_sw)
    
    // Setting Routing Bits for all nodes
    
    if (NoximGlobalParams::routing_algorithm == ROUTING_XYZ) // XY
    {
        // XY Routing Algorithm
        
        for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
            for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
                for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
                {
                    t[i1][j1][k1] -> r -> R_ne = false;
                    t[i1][j1][k1] -> r -> R_nw = false;
                    t[i1][j1][k1] -> r -> R_nu = true;
                    t[i1][j1][k1] -> r -> R_nd = true;
                    t[i1][j1][k1] -> r -> R_en = true;
                    t[i1][j1][k1] -> r -> R_es = true;
                    t[i1][j1][k1] -> r -> R_eu = true;
                    t[i1][j1][k1] -> r -> R_ed = true;
                    t[i1][j1][k1] -> r -> R_wn = true;
                    t[i1][j1][k1] -> r -> R_ws = true;
                    t[i1][j1][k1] -> r -> R_wu = true;
                    t[i1][j1][k1] -> r -> R_wd = true;
                    t[i1][j1][k1] -> r -> R_se = false;
                    t[i1][j1][k1] -> r -> R_sw = false;
                    t[i1][j1][k1] -> r -> R_su = true;
                    t[i1][j1][k1] -> r -> R_sd = true;
                    t[i1][j1][k1] -> r -> R_un = true;
                    t[i1][j1][k1] -> r -> R_ue = true;
                    t[i1][j1][k1] -> r -> R_us = true;
                    t[i1][j1][k1] -> r -> R_uw = true;
                    t[i1][j1][k1] -> r -> R_dn = true;
                    t[i1][j1][k1] -> r -> R_de = true;
                    t[i1][j1][k1] -> r -> R_ds = true;
                    t[i1][j1][k1] -> r -> R_dw = true;
                }
    }
    
    if (NoximGlobalParams::routing_algorithm == ROUTING_NORTH_LAST)
    {
    // North-Last Routing Algorithm
 
     for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
     for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
     for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
     {
     t[i1][j1][k1] -> r -> R_ne = false;
     t[i1][j1][k1] -> r -> R_nw = false;
     t[i1][j1][k1] -> r -> R_nu = true;
     t[i1][j1][k1] -> r -> R_nd = true;
     t[i1][j1][k1] -> r -> R_en = true;
     t[i1][j1][k1] -> r -> R_es = true;
     t[i1][j1][k1] -> r -> R_eu = true;
     t[i1][j1][k1] -> r -> R_ed = true;
     t[i1][j1][k1] -> r -> R_wn = true;
     t[i1][j1][k1] -> r -> R_ws = true;
     t[i1][j1][k1] -> r -> R_wu = true;
     t[i1][j1][k1] -> r -> R_wd = true;
     t[i1][j1][k1] -> r -> R_se = true;
     t[i1][j1][k1] -> r -> R_sw = true;
     t[i1][j1][k1] -> r -> R_su = true;
     t[i1][j1][k1] -> r -> R_sd = true;
     t[i1][j1][k1] -> r -> R_un = true;
     t[i1][j1][k1] -> r -> R_ue = true;
     t[i1][j1][k1] -> r -> R_us = true;
     t[i1][j1][k1] -> r -> R_uw = true;
     t[i1][j1][k1] -> r -> R_dn = true;
     t[i1][j1][k1] -> r -> R_de = true;
     t[i1][j1][k1] -> r -> R_ds = true;
     t[i1][j1][k1] -> r -> R_dw = true;
     }
  
    }
    
    if (NoximGlobalParams::routing_algorithm == ROUTING_WEST_FIRST)
    {
    // West-First Routing Algorithm
     for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
     for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
     for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
     {
     t[i1][j1][k1] -> r -> R_ne = true;
     t[i1][j1][k1] -> r -> R_nw = false;
     t[i1][j1][k1] -> r -> R_nu = true;
     t[i1][j1][k1] -> r -> R_nd = true;
     t[i1][j1][k1] -> r -> R_en = true;
     t[i1][j1][k1] -> r -> R_es = true;
     t[i1][j1][k1] -> r -> R_eu = true;
     t[i1][j1][k1] -> r -> R_ed = true;
     t[i1][j1][k1] -> r -> R_wn = true;
     t[i1][j1][k1] -> r -> R_ws = true;
     t[i1][j1][k1] -> r -> R_wu = true;
     t[i1][j1][k1] -> r -> R_wd = true;
     t[i1][j1][k1] -> r -> R_se = true;
     t[i1][j1][k1] -> r -> R_sw = false;
     t[i1][j1][k1] -> r -> R_su = true;
     t[i1][j1][k1] -> r -> R_sd = true;
     t[i1][j1][k1] -> r -> R_un = true;
     t[i1][j1][k1] -> r -> R_ue = true;
     t[i1][j1][k1] -> r -> R_us = true;
     t[i1][j1][k1] -> r -> R_uw = true;
     t[i1][j1][k1] -> r -> R_dn = true;
     t[i1][j1][k1] -> r -> R_de = true;
     t[i1][j1][k1] -> r -> R_ds = true;
     t[i1][j1][k1] -> r -> R_dw = true;
     }
    }
    
    if (NoximGlobalParams::routing_algorithm == ROUTING_NEGATIVE_FIRST)
    {
    // Negative-Last Routing Algorithm
     for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
     for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
     for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
     {
     t[i1][j1][k1] -> r -> R_ne = true;
     t[i1][j1][k1] -> r -> R_nw = false;
     t[i1][j1][k1] -> r -> R_nu = true;
     t[i1][j1][k1] -> r -> R_nd = true;
     t[i1][j1][k1] -> r -> R_en = true;
     t[i1][j1][k1] -> r -> R_es = false;
     t[i1][j1][k1] -> r -> R_eu = true;
     t[i1][j1][k1] -> r -> R_ed = true;
     t[i1][j1][k1] -> r -> R_wn = true;
     t[i1][j1][k1] -> r -> R_ws = true;
     t[i1][j1][k1] -> r -> R_wu = true;
     t[i1][j1][k1] -> r -> R_wd = true;
     t[i1][j1][k1] -> r -> R_se = true;
     t[i1][j1][k1] -> r -> R_sw = true;
     t[i1][j1][k1] -> r -> R_su = true;
     t[i1][j1][k1] -> r -> R_sd = true;
     t[i1][j1][k1] -> r -> R_un = true;
     t[i1][j1][k1] -> r -> R_ue = true;
     t[i1][j1][k1] -> r -> R_us = true;
     t[i1][j1][k1] -> r -> R_uw = true;
     t[i1][j1][k1] -> r -> R_dn = true;
     t[i1][j1][k1] -> r -> R_de = true;
     t[i1][j1][k1] -> r -> R_ds = true;
     t[i1][j1][k1] -> r -> R_dw = true;
     }
    }
    
    if (NoximGlobalParams::routing_algorithm == ROUTING_ODD_EVEN)
    {
    // Odd-Even Routing Algorithm
    for (int k1 = 0; k1 < NoximGlobalParams::mesh_dim_z; k1++)
        for (int i1 = 0; i1 < NoximGlobalParams::mesh_dim_x; i1++)
            for (int j1 = 0; j1 < NoximGlobalParams::mesh_dim_y; j1++)
            {
                if (i1 % 2 == 0) // even column
                {
                    t[i1][j1][k1] -> r -> R_ne = true;
                    t[i1][j1][k1] -> r -> R_nw = true;
                    t[i1][j1][k1] -> r -> R_nu = true;
                    t[i1][j1][k1] -> r -> R_nd = true;
                    t[i1][j1][k1] -> r -> R_en = true;
                    t[i1][j1][k1] -> r -> R_es = true;
                    t[i1][j1][k1] -> r -> R_eu = true;
                    t[i1][j1][k1] -> r -> R_ed = true;
                    t[i1][j1][k1] -> r -> R_wn = true;
                    t[i1][j1][k1] -> r -> R_ws = true;
                    t[i1][j1][k1] -> r -> R_wu = true;
                    t[i1][j1][k1] -> r -> R_wd = true;
                    t[i1][j1][k1] -> r -> R_se = true;
                    t[i1][j1][k1] -> r -> R_sw = true;
                    t[i1][j1][k1] -> r -> R_su = true;
                    t[i1][j1][k1] -> r -> R_sd = true;
                    t[i1][j1][k1] -> r -> R_un = true;
                    t[i1][j1][k1] -> r -> R_ue = true;
                    t[i1][j1][k1] -> r -> R_us = true;
                    t[i1][j1][k1] -> r -> R_uw = true;
                    t[i1][j1][k1] -> r -> R_dn = true;
                    t[i1][j1][k1] -> r -> R_de = true;
                    t[i1][j1][k1] -> r -> R_ds = true;
                    t[i1][j1][k1] -> r -> R_dw = true;
                }
                
                else  // odd column
                {
                    t[i1][j1][k1] -> r -> R_ne = true;
                    t[i1][j1][k1] -> r -> R_nw = false;
                    t[i1][j1][k1] -> r -> R_nu = true;
                    t[i1][j1][k1] -> r -> R_nd = true;
                    t[i1][j1][k1] -> r -> R_en = false;
                    t[i1][j1][k1] -> r -> R_es = false;
                    t[i1][j1][k1] -> r -> R_eu = true;
                    t[i1][j1][k1] -> r -> R_ed = true;
                    t[i1][j1][k1] -> r -> R_wn = true;
                    t[i1][j1][k1] -> r -> R_ws = true;
                    t[i1][j1][k1] -> r -> R_wu = true;
                    t[i1][j1][k1] -> r -> R_wd = true;
                    t[i1][j1][k1] -> r -> R_se = true;
                    t[i1][j1][k1] -> r -> R_sw = false;
                    t[i1][j1][k1] -> r -> R_su = true;
                    t[i1][j1][k1] -> r -> R_sd = true;
                    t[i1][j1][k1] -> r -> R_un = true;
                    t[i1][j1][k1] -> r -> R_ue = true;
                    t[i1][j1][k1] -> r -> R_us = true;
                    t[i1][j1][k1] -> r -> R_uw = true;
                    t[i1][j1][k1] -> r -> R_dn = true;
                    t[i1][j1][k1] -> r -> R_de = true;
                    t[i1][j1][k1] -> r -> R_ds = true;
                    t[i1][j1][k1] -> r -> R_dw = true;
                }
            }
    }
	  // dummy NoximNoP_data structure
	  NoximNoP_data tmp_NoP;
	  tmp_NoP.channel_status_neighbor = new NoximChannelStatus[DIRECTIONS*NoximGlobalParams::number_virtual_channel];

	  tmp_NoP.sender_id = NOT_VALID;

	  for (int i=0; i<DIRECTIONS*NoximGlobalParams::number_virtual_channel; i++)
	  {
		  tmp_NoP.channel_status_neighbor[i].free_slots = NOT_VALID;
		  tmp_NoP.channel_status_neighbor[i].available = false;
	  }
    
    for(int i=0; i<=NoximGlobalParams::mesh_dim_x; i++){ // xz plane
        for(int k=0; k<=NoximGlobalParams::mesh_dim_z; k++){
            int j = NoximGlobalParams::mesh_dim_y;
            for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
            {
                req_to_south       [i][0][k][l] = 0;
                ack_to_north       [i][0][k][l] = 0;
                req_to_north       [i][j][k][l] = 0;
                ack_to_south       [i][j][k][l] = 0;
            
                free_slots_to_south[i][0][k][l].write(NOT_VALID);
                free_slots_to_north[i][j][k][l].write(NOT_VALID);
            }
            
            NoP_data_to_south  [i][0][k].write(tmp_NoP);
            NoP_data_to_north  [i][j][k].write(tmp_NoP);
        }
    }
    
    
    for(int j=0; j<=NoximGlobalParams::mesh_dim_y; j++) // yz plane
        for(int k=0; k<=NoximGlobalParams::mesh_dim_z; k++){
            int i = NoximGlobalParams::mesh_dim_x;
            for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
            {
                req_to_east       [0][j][k][l] = 0;
                ack_to_west       [0][j][k][l] = 0;
                req_to_west       [i][j][k][l] = 0;
                ack_to_east       [i][j][k][l] = 0;
                
                free_slots_to_east[0][j][k][l].write(NOT_VALID);
                free_slots_to_west[i][j][k][l].write(NOT_VALID);
            }
            
            NoP_data_to_east  [0][j][k].write(tmp_NoP);
            NoP_data_to_west  [i][j][k].write(tmp_NoP);
        }

    
    for(int i=0; i<=NoximGlobalParams::mesh_dim_x; i++){ // xy plane
        for(int j=0; j<=NoximGlobalParams::mesh_dim_y; j++){
            int k = NoximGlobalParams::mesh_dim_z;
            for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
            {
                req_to_up          [i][j][k][l] = 0; // ? k-1
                ack_to_up          [i][j][k][l] = 0; // ? k-1
                req_to_down        [i][j][0][l] = 0;
                ack_to_down        [i][j][0][l] = 0;
            
                free_slots_to_up   [i][j][k][l].write(NOT_VALID); // ? k-1
                free_slots_to_down [i][j][0][l].write(NOT_VALID);
            }
            
            NoP_data_to_down  [i][j][0].write(tmp_NoP);
            NoP_data_to_up    [i][j][k].write(tmp_NoP);
        }
    }
    
	  // invalidate reservation table entries for non-exhistent channels
    for(int i=0; i<NoximGlobalParams::mesh_dim_x; i++) // xz plane
        for(int k=0; k<NoximGlobalParams::mesh_dim_z; k++){
            int j = NoximGlobalParams::mesh_dim_y;
            
            for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
            {
                t[i][0  ][k]->r->reservation_table.invalidate(DIRECTION_NORTH,l);
                t[i][j-1][k]->r->reservation_table.invalidate(DIRECTION_SOUTH,l);
            }
        }

    for(int j=0; j<NoximGlobalParams::mesh_dim_y; j++) // yz plane
        for(int k=0; k<NoximGlobalParams::mesh_dim_z; k++){
            int i = NoximGlobalParams::mesh_dim_x;
            
            for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
            {
                t[0  ][j][k]->r->reservation_table.invalidate(DIRECTION_WEST,l);
                t[i-1][j][k]->r->reservation_table.invalidate(DIRECTION_EAST,l);
            }
        }
    
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++) {
        for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++) {
            int k = NoximGlobalParams::mesh_dim_z;

            for(int l=0; l<NoximGlobalParams::number_virtual_channel; l++)
            {
                t[i][j][0]->r->reservation_table.invalidate(DIRECTION_UP,l);
                t[i][j][k-1]->r->reservation_table.invalidate(DIRECTION_DOWN,l);
            }
        }
    }
}

//---------------------------------------------------------------------------

NoximTile* NoximNoC::searchNode(const int id) const
{
    NoximCoord node = id2Coord(id);
    return t[node.x][node.y][node.z];
    
    return t[0][0][0];
}

//---------------------------------------------------------------------------
