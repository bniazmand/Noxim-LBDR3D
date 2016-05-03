/*****************************************************************************
 
 NoximRouter.cpp -- Router implementation
 
 *****************************************************************************/

#include "NoximRouter.h"

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
        int Manhattan_Distance;
        
        elevators *next_elevator;
    };
    
    elevators *start_elevator;
    
    elevator()
    {
        start_elevator = NULL;
    }
    
    void insert_node(int elevator_node_id , int Manhattan_Distance)
    {
        elevators *ptr = new elevators;
        if (!ptr) return;
        ptr->elevator_node_id = elevator_node_id;
        ptr->Manhattan_Distance = Manhattan_Distance;
        
        ptr->next_elevator = NULL;
        elevators *t=start_elevator , *s = NULL;
        
        while (t && (t->Manhattan_Distance < Manhattan_Distance))
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
                if( temp1->Manhattan_Distance > temp2->Manhattan_Distance )
                {
                    temp_1 = temp1->elevator_node_id;
                    temp_2 = temp1->Manhattan_Distance;
                    temp1->elevator_node_id = temp2->elevator_node_id;
                    temp1->Manhattan_Distance = temp2->Manhattan_Distance;
                    temp2->elevator_node_id = temp_1;
                    temp2->Manhattan_Distance = temp_1;
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
            cout << "Manhattan Distance of Elevator from current and destination node : " << t->elevator_node_id << " : \n";
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

void NoximRouter::rxProcess()
{
    if(reset.read())
    {
        // Clear outputs and indexes of receiving protocol
        for(int i=0; i<DIRECTIONS; i++)
        {
            for(int j=0; j<NoximGlobalParams::number_virtual_channel; j++)
            {
                ack_rx[i][j].write(0);
                current_level_rx[i][j] = 0;
            }
        }
        
        ack_local_rx.write(0);
        current_level_local_rx = 0;
        
        reservation_table.clear();
        routed_flits = 0;
    }
    else
    {
        // For each channel decide if a new flit can be accepted
        //
        // This process simply sees a flow of incoming flits. All arbitration
        // and wormhole related issues are addressed in the txProcess()
        
        for(int i=0; i<DIRECTIONS; i++)
        {
            for(int j=0; j<NoximGlobalParams::number_virtual_channel; j++)
            {
                // To accept a new flit, the following conditions must match:
                //
                // 1) there is an incoming request
                // 2) there is a free slot in the input buffer of direction i
                
                if (sc_time_stamp().to_double()/1000 == 1073 && local_id == 13)
                {
                    int xxhs = 0;
                }
                
                if ( (req_rx[i][j].read() == 1 - current_level_rx[i][j]) && !buffer[i][j].IsFull() )
                {
                    NoximFlit received_flit = flit_rx[i].read();
                    
                    if(NoximGlobalParams::verbose_mode > VERBOSE_OFF)
                    {
                        cout << sc_time_stamp().to_double()/1000 << ": Router[" << local_id <<"], Input[" << i
                        << "] VChannel[" << j << "], Received flit: " << received_flit << endl;
                    }
                    
                    // Store the incoming flit in the circular buffer
                    buffer[i][j].Push(received_flit);
                    
                    // Negate the old value for Alternating Bit Protocol (ABP)
                    current_level_rx[i][j] = 1 - current_level_rx[i][j];
                    
                    // Incoming flit
                    stats.power.Incoming();
                }
                ack_rx[i][j].write(current_level_rx[i][j]);
            }
        }
        
        if ( (req_local_rx.read() == 1 - current_level_local_rx) && !local_buffer.IsFull() )
        {
            NoximFlit received_flit = flit_rx[DIRECTION_LOCAL].read();
            
            if(NoximGlobalParams::verbose_mode > VERBOSE_OFF)
            {
                cout << sc_time_stamp().to_double()/1000 << ": Router[" << local_id <<"], Input[" << DIRECTION_LOCAL << "], Received flit: " << received_flit << endl;
            }
            
            // Store the incoming flit in the circular buffer
            local_buffer.Push(received_flit);
            
            // Negate the old value for Alternating Bit Protocol (ABP)
            current_level_local_rx = 1 - current_level_local_rx;
            
            // Incoming flit
            stats.power.Incoming();
        }
        ack_local_rx.write(current_level_local_rx);
        
    }
    bufferMonitor();
}

//---------------------------------------------------------------------------

void NoximRouter::txProcess()
{
    if (reset.read())
    {
        // Clear outputs and indexes of transmitting protocol
        for(int i=0; i<DIRECTIONS; i++)
        {
            for(int j=0; j<NoximGlobalParams::number_virtual_channel; j++)
            {
                req_tx[i][j].write(0);
                current_level_tx[i][j] = 0;
            }
        }
        req_local_tx.write(0);
        current_level_local_tx = 0;
        
        current_local_channel = 0;
    }
    else
    {
        int use_crossbar = 0;
        int use_arbiter = 0;
        
        int out_busy[DIRECTIONS+1];
        for(int j=0; j<DIRECTIONS+1; j++)
            out_busy[j] = 0;
        
        // 1st phase: Reservation
        for(int j=0; j<DIRECTIONS+1; j++)
        {
            int i = (start_from_port + j) % (DIRECTIONS + 1);
            if (i != DIRECTION_LOCAL)
            {
                for(int k=0; k<NoximGlobalParams::number_virtual_channel; k++)
                {
                    int v = (start_from_channel[i] + k) % NoximGlobalParams::number_virtual_channel;
                    
                    if (!buffer[i][v].IsEmpty())
                    {
                        NoximFlit flit = buffer[i][v].Front();
                        if (flit.flit_type==FLIT_TYPE_HEAD)
                        {
                            // prepare data for routing
                            NoximRouteData route_data;
                            route_data.current_id = local_id;
                            route_data.src_id = flit.src_id;
                            route_data.dst_id = flit.dst_id;
                            route_data.dir_in = i;
                            
                            v = flit.channel; // once a flit is assigned a VC, during the routing, it should remain on the same VC (VN)
                            
                            int o = route(route_data, v);
                            
                            //                            if ( (route_data.src_id == route_data.dst_id) && (o == DIRECTION_LOCAL) )
                            //                                cout << "Traffic generator has generated a packet is sent from a node to itself !!! \n";
                            //                            else if ( (route_data.src_id != route_data.dst_id) && (o == DIRECTION_LOCAL) )
                            //                                cout << "Traffic : " << route_data.src_id << " -> " << route_data.dst_id << " has reached its destination!\n";
                            
                            if (reservation_table.isAvailable(o, v))
                            {
                                reservation_table.reserve(i, o, v);
                                use_arbiter = 1;
                                if(NoximGlobalParams::verbose_mode > VERBOSE_OFF)
                                {
                                    cout << sc_time_stamp().to_double()/1000
                                    << ": Router[" << local_id
                                    << "], Input[" << i << "], VChannel[" << v << "] ("
                                    << buffer[i][v].Size() << " flits)"
                                    << ", reserved Output[" << o << "], flit: " << flit << endl;
                                }
                            }
                        }
                    }
                }
            }
            else  // if (i == DIRECTION_LOCAL)
                // Here we can describe what the router does when the packet is at source node, that means the current router is the source node
            {
                if (!local_buffer.IsEmpty())
                {
                    NoximFlit flit = local_buffer.Front();
                    
                    if (flit.flit_type==FLIT_TYPE_HEAD)
                    {
                        // prepare data for routing
                        NoximRouteData route_data;
                        route_data.current_id = local_id;
                        route_data.src_id = flit.src_id;
                        route_data.dst_id = flit.dst_id;
                        route_data.dir_in = i;
                        
                        //						if (NoximGlobalParams::number_virtual_channel > 1)
                        //							current_local_channel = rand() % NoximGlobalParams::number_virtual_channel;
                        
                        // New Virtual Channel Allocation Algorithm for LBDR3D (dead-lock freeness) ???
                        
                        if (NoximGlobalParams::number_virtual_channel > 1)  // This is one of the places we can implement VC Allocation policy (Default : Choose Randomly).
                        {
                            NoximCoord src_coord, dst_coord;
                            src_coord = id2Coord(flit.src_id);
                            dst_coord = id2Coord(flit.dst_id);
                            if (src_coord.z < dst_coord.z) // destination layer is below source layer
                                current_local_channel = 0;
                            else if (src_coord.z > dst_coord.z) // destination layer is above source layer
                                current_local_channel = 1;
                            //                                                      else    // destination layer is the same as source layer (keep the previous VC)
                            //                                                            current_local_channel = 0;
                        }
                        
                        // End of New Virtual Channel Allocation Algorithm for LBDR3D (dead-lock freeness)
                        
                        int o = route(route_data, current_local_channel);
                        
                        if (reservation_table.isAvailable(o, current_local_channel))
                        {
                            reservation_table.reserve(i, o, current_local_channel);
                            use_arbiter = 1;
                            if(NoximGlobalParams::verbose_mode > VERBOSE_OFF)
                            {
                                cout << sc_time_stamp().to_double()/1000
                                << ": Router[" << local_id
                                << "], Input[" << i << "] (" << local_buffer.Size() << " flits)"
                                << ", reserved Output[" << o << "] VChannel[" << current_local_channel
                                << "], flit: " << flit << endl;
                            }
                        }
                    }
                }
            }
        }
        
        // 2nd phase: Forwarding
        for(int h=0; h<DIRECTIONS+1; h++)
        {
            int i = (start_from_port + h) % (DIRECTIONS + 1);
            
            if (i != DIRECTION_LOCAL)
            {
                for(int k=0; k<NoximGlobalParams::number_virtual_channel; k++)
                {
                    int j = (start_from_channel[i] + k) % NoximGlobalParams::number_virtual_channel;
                    
                    if (!buffer[i][j].IsEmpty())
                    {
                        NoximFlit flit = buffer[i][j].Front();
                        
                        int o = reservation_table.getOutputPort(i,j);
                        
                        if (o != NOT_RESERVED)
                        {
                            if (o != DIRECTION_LOCAL)
                            {
                                if ( current_level_tx[o][j] == ack_tx[o][j].read() && out_busy[o] == 0 &&
                                    (free_slots_neighbor[o][j].read() > 0 || NoximGlobalParams::number_virtual_channel == 1) )
                                {
                                    if(NoximGlobalParams::verbose_mode > VERBOSE_OFF)
                                    {
                                        cout << sc_time_stamp().to_double()/1000
                                        << ": Router[" << local_id
                                        << "], Input[" << i << "] forward to Output[" << o
                                        << "] VChannel[" << j
                                        << "], flit: " << flit << endl;
                                    }
                                    
                                    flit_tx[o].write(flit);
                                    current_level_tx[o][j] = 1 - current_level_tx[o][j];
                                    req_tx[o][j].write(current_level_tx[o][j]);
                                    buffer[i][j].Pop();
                                    
                                    stats.power.Forward();
                                    
                                    if (flit.flit_type == FLIT_TYPE_TAIL)
                                        reservation_table.release(o,j);
                                    else if (flit.flit_type == FLIT_TYPE_HEAD)
                                        use_crossbar = 1;
                                    
                                    // Increment routed flits counter
                                    routed_flits++;
                                    
                                    flit.hop_no = flit.hop_no + 1;
                                    
                                    out_busy[o] = 1;
                                }
                            }
                            else
                            {
                                if ( current_level_local_tx == ack_local_tx.read() && out_busy[o] == 0 )
                                {
                                    if(NoximGlobalParams::verbose_mode > VERBOSE_OFF)
                                    {
                                        cout << sc_time_stamp().to_double()/1000
                                        << ": Router[" << local_id
                                        << "], Input[" << i << "] forward to Output[" << o
                                        << "] VChannel[" << j << "], flit: " << flit << endl;
                                    }
                                    
                                    flit_tx[o].write(flit);
                                    current_level_local_tx = 1 - current_level_local_tx;
                                    req_local_tx.write(current_level_local_tx);
                                    buffer[i][j].Pop();
                                    
                                    stats.power.Forward();
                                    
                                    if (flit.flit_type == FLIT_TYPE_TAIL)
                                        reservation_table.release(o,j);
                                    else if (flit.flit_type == FLIT_TYPE_HEAD)
                                        use_crossbar = 1;
                                    
                                    // Update stats
                                    stats.receivedFlit(sc_time_stamp().to_double()/1000, flit);
                                    
                                    out_busy[o] = 1;
                                }
                            }
                        }
                    }
                }
                
                start_from_channel[i] = (start_from_channel[i] + 1) % NoximGlobalParams::number_virtual_channel;
            }
            else // if (i == DIRECTION_LOCAL)
            {
                if (!local_buffer.IsEmpty())
                {
                    NoximFlit flit = local_buffer.Front();
                    
                    // Is this arbitration ?
                    int o = reservation_table.getOutputPort(i,current_local_channel);
                    
                    if (o != NOT_RESERVED)
                    {
                        if (o != DIRECTION_LOCAL)
                        {
                            if ( current_level_tx[o][current_local_channel] == ack_tx[o][current_local_channel].read() &&
                                out_busy[o] == 0 && (free_slots_neighbor[o][current_local_channel].read() > 0 ||
                                                     NoximGlobalParams::number_virtual_channel == 1) )
                            {
                                if(NoximGlobalParams::verbose_mode > VERBOSE_OFF)
                                {
                                    cout << sc_time_stamp().to_double()/1000
                                    << ": Router[" << local_id
                                    << "], Input[" << i << "] forward to Output[" << o
                                    << "] VChannel[" << current_local_channel << "], flit: " << flit << endl;
                                }
                                
                                flit.channel = current_local_channel;
                                
                                flit_tx[o].write(flit);
                                current_level_tx[o][current_local_channel] = 1 - current_level_tx[o][current_local_channel];
                                req_tx[o][current_local_channel].write(current_level_tx[o][current_local_channel]);
                                local_buffer.Pop();
                                
                                stats.power.Forward();
                                
                                if (flit.flit_type == FLIT_TYPE_TAIL)
                                    reservation_table.release(o,current_local_channel);
                                else if (flit.flit_type == FLIT_TYPE_HEAD)
                                    use_crossbar = 1;
                                
                                flit.hop_no = flit.hop_no + 1;
                                
                                out_busy[o] = 1;
                            }
                        }
                    }
                }
            }
        }
        
        start_from_port++;
        
        if (use_crossbar) stats.power.Crossbar();
        if (use_arbiter)  stats.power.Arbiter();
    } // else
}

//---------------------------------------------------------------------------

NoximNoP_data NoximRouter::getCurrentNoPData() const
{
    NoximNoP_data NoP_data;
    NoP_data.channel_status_neighbor = new NoximChannelStatus[DIRECTIONS*NoximGlobalParams::number_virtual_channel];
    
    for (int j=0; j<DIRECTIONS; j++)
    {
        for (int i=0; i<NoximGlobalParams::number_virtual_channel; i++)
        {
            NoP_data.channel_status_neighbor[j*NoximGlobalParams::number_virtual_channel+i].free_slots = free_slots_neighbor[j][i].read();
            NoP_data.channel_status_neighbor[j*NoximGlobalParams::number_virtual_channel+i].available = (reservation_table.isAvailable(j,i));
        }
    }
    
    NoP_data.sender_id = local_id;
    
    return NoP_data;
}

//---------------------------------------------------------------------------

void NoximRouter::bufferMonitor()
{
    if (reset.read())
    {
        for (int i=0; i<DIRECTIONS; i++)
        {
            for (int j=0; j<NoximGlobalParams::number_virtual_channel; j++)
            {
                free_slots[i][j].write(buffer[i][j].GetMaxBufferSize());
            }
        }
        
        free_local_slots.write(local_buffer.GetMaxBufferSize());
    }
    else
    {
        
        // update current input buffers level to neighbors
        for (int i=0; i<DIRECTIONS; i++)
        {
            for (int j=0; j<NoximGlobalParams::number_virtual_channel; j++)
            {
                free_slots[i][j].write(buffer[i][j].getCurrentFreeSlots());
            }
        }
        
        free_local_slots.write(local_buffer.getCurrentFreeSlots());
        
        // NoP selection: send neighbor info to each direction 'i'
        NoximNoP_data current_NoP_data = getCurrentNoPData();
        
        for (int i=0; i<DIRECTIONS; i++)
            NoP_data_out[i].write(current_NoP_data);
        //}
    }
}

//---------------------------------------------------------------------------

vector<int> NoximRouter::routingFunction(const NoximRouteData& route_data, int channel)
{
    NoximCoord position  = id2Coord(route_data.current_id);
    NoximCoord src_coord = id2Coord(route_data.src_id);
    NoximCoord dst_coord = id2Coord(route_data.dst_id);
    int dir_in = route_data.dir_in;
    
    switch (NoximGlobalParams::routing_algorithm)
    {
        case ROUTING_XYZ:
            return routingLBDR(position, dst_coord, dir_in);
            
        case ROUTING_LBDR:
            return routingLBDR(position, dst_coord, dir_in);
            
        case ROUTING_DQP_NETZ:
            return routing_DQP_NETZ(position, dst_coord, dir_in);
            
        case ROUTING_ETW:
            return routing_ETW(position, dst_coord, dir_in);
            
        case ROUTING_WEST_FIRST:
            return routingLBDR(position, dst_coord, dir_in);
            
        case ROUTING_NORTH_LAST:
            return routingLBDR(position, dst_coord, dir_in);
            
        case ROUTING_NEGATIVE_FIRST:
            return routingLBDR(position, dst_coord, dir_in);
            
        case ROUTING_ODD_EVEN:
            return routingLBDR(position, dst_coord, dir_in);
            
        case ROUTING_DYAD:
            return routingDyAD(position, src_coord, dst_coord);
            
        case ROUTING_FULLY_ADAPTIVE:
            return routingFullyAdaptive(position, dst_coord);
            
        case ROUTING_TABLE_BASED:
            return routingTableBased(dir_in, position, dst_coord);
            
        default:
            assert(false);
    }
    
    // something weird happened, you shouldn't be here
    return (vector<int>)(0);
}

//---------------------------------------------------------------------------

int NoximRouter::route(const NoximRouteData& route_data, int channel)
{
    stats.power.Routing();
    
//    cout << route_data.current_id << " -> ";
//    
//    if (route_data.current_id == route_data.dst_id)
//        cout << endl;
    
    if (route_data.dst_id == local_id)
    {
        //    cout << "Flit " << route_data.src_id << " -> " << route_data.dst_id << " has reached its destination! \n";
        return DIRECTION_LOCAL;
    }
    
    vector<int> candidate_channels = routingFunction(route_data, channel);
    
    return selectionFunction(candidate_channels, route_data, channel);
}

//---------------------------------------------------------------------------

void NoximRouter::NoP_report() const
{
    NoximNoP_data NoP_tmp;
    
    cout << sc_time_stamp().to_double()/1000 << ": Router[" << local_id << "] NoP report: " << endl;
    
    for (int i=0;i<DIRECTIONS*NoximGlobalParams::number_virtual_channel; i++)
    {
        NoP_tmp = NoP_data_in[i].read();
        if (NoP_tmp.sender_id!=NOT_VALID)
            cout << NoP_tmp;
    }
}
//---------------------------------------------------------------------------

int NoximRouter::NoPScore(const NoximNoP_data& nop_data, const vector<int>& nop_channels, int channel) const
{
    int score = 0;
    
    for (unsigned int i=0;i<nop_channels.size();i++)
    {
        int available;
        
        if (nop_data.channel_status_neighbor[nop_channels[i]*NoximGlobalParams::number_virtual_channel+channel].available)
            available = 1;
        else available = 0;
        
        int free_slots = nop_data.channel_status_neighbor[nop_channels[i]*NoximGlobalParams::number_virtual_channel+channel].free_slots;
        
        score += available*free_slots;
    }
    
    return score;
}
//---------------------------------------------------------------------------

int NoximRouter::selectionNoP(const vector<int>& directions, const NoximRouteData& route_data, int channel)
{
    vector<int> neighbors_on_path;
    vector<int> score;
    int direction_selected = NOT_VALID;
    
    int current_id = route_data.current_id;
    
    for (unsigned int i=0; i<directions.size(); i++)
    {
        // get id of adjacent candidate
        int candidate_id = getNeighborId(current_id,directions[i]);
        
        // apply routing function to the adjacent candidate node
        NoximRouteData tmp_route_data;
        tmp_route_data.current_id = candidate_id;
        tmp_route_data.src_id = route_data.src_id;
        tmp_route_data.dst_id = route_data.dst_id;
        tmp_route_data.dir_in = reflexDirection(directions[i]);
        
        
        vector<int> next_candidate_channels = routingFunction(tmp_route_data, channel);
        
        // select useful data from Neighbor-on-Path input
        NoximNoP_data nop_tmp = NoP_data_in[directions[i]].read();
        
        // store the score of node in the direction[i]
        score.push_back(NoPScore(nop_tmp, next_candidate_channels, channel));
    }
    
    // check for direction with higher score
    int max_direction = directions[0];
    int max = score[0];
    for (unsigned int i = 0;i<directions.size();i++)
    {
        if (score[i]>max)
        {
            max_direction = directions[i];
            max = score[i];
        }
    }
    
    // if multiple direction have the same score = max, choose randomly.
    
    vector<int> equivalent_directions;
    
    for (unsigned int i = 0;i<directions.size();i++)
        if (score[i]==max)
            equivalent_directions.push_back(directions[i]);
    
    direction_selected =  equivalent_directions[rand() % equivalent_directions.size()];
    
    return direction_selected;
}

//---------------------------------------------------------------------------

int NoximRouter::selectionBufferLevel(const vector<int>& directions, int channel)
{
    vector<int>  best_dirs;
    int          max_free_slots = 0;
    for (unsigned int i=0; i<directions.size(); i++)
    {
        int free_slots = free_slots_neighbor[directions[i]][channel].read();
        bool available = reservation_table.isAvailable(directions[i], channel);
        if (available)
        {
            if (free_slots > max_free_slots)
            {
                max_free_slots = free_slots;
                best_dirs.clear();
                best_dirs.push_back(directions[i]);
            }
            else if (free_slots == max_free_slots)
                best_dirs.push_back(directions[i]);
        }
    }
    
    if (best_dirs.size())
        return(best_dirs[rand() % best_dirs.size()]);
    else
        return(directions[rand() % directions.size()]);
}

//---------------------------------------------------------------------------
int NoximRouter::selectionRandom(const vector<int>& directions)
{
    return directions[rand() % directions.size()];
}

//---------------------------------------------------------------------------

// Our proposed Selection Functions (Path-based Contention Aware)

int NoximRouter::selectionPathAwareMinimal(const vector<int>& directions,const NoximRouteData& route_data, int channel)
{
    // NE Quadrant
    
    if (((directions.size() == 2) && N_prime && E_prime))
    {
        if (reservation_table.isAvailable(directions[0], channel) && reservation_table.isAvailable(directions[1], channel))
        {
            //            srand(time(NULL));
            if (route_data.dir_in == DIRECTION_WEST)
                return DIRECTION_EAST;
            else if (route_data.dir_in == DIRECTION_SOUTH)
                return DIRECTION_NORTH;
            else
            {
                //                int i = rand() % directions.size();
                //                return directions[i];
                return selectionNoP(directions, route_data, channel);
            }
        }
        
        if (reservation_table.isAvailable(directions[0], channel) && !reservation_table.isAvailable(directions[1], channel)) return directions[0];
        if (!reservation_table.isAvailable(directions[0], channel) && reservation_table.isAvailable(directions[1], channel)) return directions[1];
        if (!reservation_table.isAvailable(directions[0], channel) && !reservation_table.isAvailable(directions[1], channel))
        {
            return directions[0];
        }
    }
    
    // SE Quadrant
    
    if (((directions.size() == 2) && S_prime && E_prime))
    {
        if (reservation_table.isAvailable(directions[0], channel) && reservation_table.isAvailable(directions[1], channel))
        {
            //              srand(time(NULL));
            if (route_data.dir_in == DIRECTION_WEST)
                return DIRECTION_EAST;
            else if (route_data.dir_in == DIRECTION_NORTH)
                return DIRECTION_SOUTH;
            else
            {
                //                int i = rand() % directions.size();
                //                return directions[i];
                return selectionNoP(directions, route_data, channel);
            }
        }
        
        if (reservation_table.isAvailable(directions[0], channel) && !reservation_table.isAvailable(directions[1], channel)) return directions[0];
        if (!reservation_table.isAvailable(directions[0], channel) && reservation_table.isAvailable(directions[1], channel)) return directions[1];
        if (!reservation_table.isAvailable(directions[0], channel) && !reservation_table.isAvailable(directions[1], channel))
        {
            return directions[0];
        }
    }
    
    
    // NW Quadrant
    
    if (N_prime && W_prime)
    {
        bool available = reservation_table.isAvailable(directions[0], channel);
        if (available)
            return directions[0];
        else return directions[0];
    }
    
    // SW Quadrant
    
    if (S_prime && W_prime)
    {
        bool available = reservation_table.isAvailable(directions[0], channel);
        if (available)
            return directions[0];
        else return directions[0];
    }
    
    // Only E selected
    
    if (E_prime && !W_prime && !N_prime && !S_prime)
    {
        bool available = reservation_table.isAvailable(directions[0], channel);
        if (available)
            return directions[0];
        else if (reservation_table.isAvailable(DIRECTION_NORTH, channel) && C_n && reservation_table.isAvailable(DIRECTION_SOUTH, channel) && C_s)
        {
            //          srand(time(NULL));
            vector < int > temp_directions;
            temp_directions.push_back(DIRECTION_NORTH);
            temp_directions.push_back(DIRECTION_SOUTH);
            int i = rand() % temp_directions.size();
            return temp_directions[i];
        }
        else if (reservation_table.isAvailable(DIRECTION_NORTH, channel) && C_n)
            return DIRECTION_NORTH;
        else if (reservation_table.isAvailable(DIRECTION_SOUTH, channel) && C_s)
            return DIRECTION_SOUTH;
        else return directions[0];
    }
    
    // Only W selected
    
    if (!E_prime && W_prime && !N_prime && !S_prime)
    {
        bool available = reservation_table.isAvailable(directions[0], channel);
        if (available)
            return directions[0];
        else return directions[0];
    }
    
    // Only S selected
    
    if (!E_prime && !W_prime && !N_prime && S_prime)
    {
        bool available = reservation_table.isAvailable(directions[0], channel);
        if (available)
            return directions[0];
        else if (reservation_table.isAvailable(DIRECTION_WEST, channel) && C_w)
            return DIRECTION_WEST;
        else return directions[0];
    }
    
    // Only N selected
    
    if (!E_prime && !W_prime && N_prime && !S_prime)
    {
        bool available = reservation_table.isAvailable(directions[0], channel);
        if (available)
            return directions[0];
        else if (reservation_table.isAvailable(DIRECTION_WEST, channel) && C_w)
            return DIRECTION_WEST;
        else return directions[0];
    }
    
    return 0;
}

//---------------------------------------------------------------------------

int NoximRouter::selectionFunction(const vector<int>& directions, const NoximRouteData& route_data, int channel)
{
    if (directions.size()==1) return directions[0];
    
    stats.power.Selection();
    switch (NoximGlobalParams::selection_strategy)
    {
        case SEL_RANDOM:
            return selectionRandom(directions);
        case SEL_BUFFER_LEVEL:
            return selectionBufferLevel(directions, channel);
        case SEL_NOP:
            return selectionNoP(directions, route_data, channel);
        case SEL_PATHAWARE_MINIMAL:
            return selectionPathAwareMinimal(directions, route_data, channel);
        default:
            assert(false);
    }
    
    return 0;
}

//---------------------------------------------------------------------------

vector<int> NoximRouter::routingXYZ(const NoximCoord& current, const NoximCoord& destination) // XY
{
    vector<int> directions;
    
    if (NoximGlobalParams::topology == NET_MESH)
    {
        if      (destination.x > current.x)	directions.push_back(DIRECTION_EAST );
        else if (destination.x < current.x)	directions.push_back(DIRECTION_WEST );
        else if (destination.y > current.y)	directions.push_back(DIRECTION_SOUTH);
        else if (destination.y < current.y)	directions.push_back(DIRECTION_NORTH);
        //      else if (destination.z > current.z)	directions.push_back(DIRECTION_DOWN );
        //      else if (destination.z < current.z)	directions.push_back(DIRECTION_UP   );
        return directions;
    }
    /*
     else if (NoximGlobalParams::topology == NET_TORUS)
     {
	    float rowCheck = NoximGlobalParams::mesh_dim_y/2;
	    float colCheck = NoximGlobalParams::mesh_dim_x/2;
     
     if (destination.y == current.y)
     {
     if ((destination.x < current.x) && ((current.x - destination.x) >= colCheck))
     directions.push_back(DIRECTION_EAST);
     else if ((destination.x < current.x) && ((current.x - destination.x) < colCheck))
     directions.push_back(DIRECTION_WEST);
     else if ((destination.x > current.x) && ((destination.x - current.x) >= colCheck))
     directions.push_back(DIRECTION_WEST);
     else if ((destination.x > current.x) && ((destination.x - current.x) < colCheck))
     directions.push_back(DIRECTION_EAST);
     }
     
     else if ((destination.y < current.y) && ((current.y - destination.y) >= rowCheck))
     directions.push_back(DIRECTION_SOUTH);
     else if ((destination.y < current.y) && ((current.y - destination.y) < rowCheck))
     directions.push_back(DIRECTION_NORTH);
     
     else if ((destination.y > current.y) && ((destination.y - current.y) >= rowCheck))
     directions.push_back(DIRECTION_NORTH);
     else if ((destination.y > current.y) && ((destination.y - current.y) < rowCheck))
     directions.push_back(DIRECTION_SOUTH);
     }
     */
    /*
     else if (NoximGlobalParams::topology == NET_FOLDED_TORUS)
     {
     if (current.y < destination.y)
     {
     if ((current.y + destination.y) % 2 == 0)
			  directions.push_back(DIRECTION_SOUTH);
     else if (current.y + destination.y < NoximGlobalParams::mesh_dim_y)
			  directions.push_back(DIRECTION_NORTH);
     else
			  directions.push_back(DIRECTION_SOUTH);
     }
     else if (current.y > destination.y)
     {
     if ((current.y + destination.y) % 2 == 0 || current.y + destination.y < NoximGlobalParams::mesh_dim_y)
			  directions.push_back(DIRECTION_NORTH);
     else
			  directions.push_back(DIRECTION_SOUTH);
     }
     else if (current.x < destination.x)
     {
     if ((current.x + destination.x) % 2 == 0)
			  directions.push_back(DIRECTION_EAST);
     else if (current.x + destination.x < NoximGlobalParams::mesh_dim_x)
			  directions.push_back(DIRECTION_WEST);
     else
			  directions.push_back(DIRECTION_EAST);
     }
     else if (current.x > destination.x)
     {
     if ((current.x + destination.x) % 2 == 0 || current.x + destination.x < NoximGlobalParams::mesh_dim_x)
			  directions.push_back(DIRECTION_WEST);
     else
			  directions.push_back(DIRECTION_EAST);
     }
     }
     */
    
    return directions;
}

//---------------------------------------------------------------------------

vector < int >NoximRouter::routingLBDR(const NoximCoord & current,const NoximCoord & destination, int dir_in)
{
    vector < int > directions;
    
    // Checking the values of elevator bits at every router (for DEBUGGING)
    /*    int current_id = coord2Id(current);
     if (C_u == true)
     cout << "Node [" << current_id << "] is an up elevator\n";
     if (NEu == true)
     cout << "Node [" << current_id << "] has an up elevator on NE quadrant\n";
     if (NWu == true)
     cout << "Node [" << current_id << "] has an up elevator on NW quadrant\n";
     if (SEu == true)
     cout << "Node [" << current_id << "] has an up elevator on SE quadrant\n";
     if (SWu == true)
     cout << "Node [" << current_id << "] has an up elevator on SW quadrant\n";
     if (Nu == true)
     cout << "Node [" << current_id << "] has an up elevator on N direction\n";
     if (Eu == true)
     cout << "Node [" << current_id << "] has an up elevator on E direction\n";
     if (Wu == true)
     cout << "Node [" << current_id << "] has an up elevator on W direction\n";
     if (Su == true)
     cout << "Node [" << current_id << "] has an up elevator on S direction\n";
     
     if (C_d == true)
     cout << "Node [" << current_id << "] is a down elevator\n";
     if (NEd == true)
     cout << "Node [" << current_id << "] has a down elevator on NE quadrant\n";
     if (NWd == true)
     cout << "Node [" << current_id << "] has a down elevator on NW quadrant\n";
     if (SEd == true)
     cout << "Node [" << current_id << "] has a down elevator on SE quadrant\n";
     if (SWd == true)
     cout << "Node [" << current_id << "] has a down elevator on SW quadrant\n";
     if (Nd == true)
     cout << "Node [" << current_id << "] has a down elevator on N direction\n";
     if (Ed == true)
     cout << "Node [" << current_id << "] has a down elevator on E direction\n";
     if (Wd == true)
     cout << "Node [" << current_id << "] has a down elevator on W direction\n";
     if (Sd == true)
     cout << "Node [" << current_id << "] has a down elevator on S direction\n";
     
     cout << "\n";
     */
    //First part of Routing Logic
    
    if (destination.x > current.x)
    {
        E_prime = true;
        W_prime = false;
    }
    
    else if (destination.x < current.x)
    {
        E_prime = false;
        W_prime = true;
    }
    
    else if (destination.x == current.x)
    {
        E_prime = false;
        W_prime = false;
    }
    
    if (destination.y > current.y)
    {
        N_prime = false;
        S_prime = true;
    }
    
    else if (destination.y < current.y)
    {
        N_prime = true;
        S_prime = false;
    }
    
    else if (destination.y == current.y)
    {
        N_prime = false;
        S_prime = false;
    }
    
    if (destination.z > current.z)
    {
        D_prime = true;
        U_prime = false;
    }
    
    else if (destination.z < current.z)
    {
        D_prime = false;
        U_prime = true;
    }
    
    else if (destination.z == current.z)
    {
        D_prime = false;
        U_prime = false;
    }
    
    ipN = false;
    ipE = false;
    ipW = false;
    ipS = false;
    
    if      (dir_in == DIRECTION_NORTH)      ipN = true;
    else if (dir_in == DIRECTION_EAST)       ipE = true;
    else if (dir_in == DIRECTION_WEST)       ipW = true;
    else if (dir_in == DIRECTION_SOUTH)      ipS = true;
    
    //Second part of Routing Logic
    
    /* Newly added logic for LBDR3D */
    
    NEu_prime = Nu  & Eu  & !ipN & !ipE;
    NWu_prime = Nu  & Wu  & !ipN & !ipW;
    SEu_prime = Su  & Eu  & !ipS & !ipE;
    SWu_prime = Su  & Wu  & !ipS & !ipW;
    NEd_prime = Nd  & Ed  & !ipN & !ipE;
    NWd_prime = Nd  & Wd  & !ipN & !ipW;
    SEd_prime = Sd  & Ed  & !ipS & !ipE;
    SWd_prime = Sd  & Wd  & !ipS & !ipW;
    Nu_prime  = Nu  & !Eu & !Wu  & !ipN;
    Eu_prime  = Eu  & !Nu & !Su  & !ipE;
    Wu_prime  = Wu  & !Nu & !Su  & !ipW;
    Su_prime  = Su  & !Eu & !Wu  & !ipS;
    Nd_prime  = Nd  & !Ed & !Wd  & !ipN;
    Ed_prime  = Ed  & !Nd & !Sd  & !ipE;
    Wd_prime  = Wd  & !Nd & !Sd  & !ipW;
    Sd_prime  = Sd  & !Ed & !Wd  & !ipS;
    
    N_zegond = (N_prime & !U_prime & !D_prime) | (U_prime & (Nu_prime | NEu_prime | NWu_prime)) | (D_prime & (Nd_prime | NEd_prime | NWd_prime));
    E_zegond = (E_prime & !U_prime & !D_prime) | (U_prime & (Eu_prime | NEu_prime | SEu_prime)) | (D_prime & (Ed_prime | NEd_prime | SEd_prime));
    W_zegond = (W_prime & !U_prime & !D_prime) | (U_prime & (Wu_prime | NWu_prime | SWu_prime)) | (D_prime & (Wd_prime | NWd_prime | SWd_prime));
    S_zegond = (S_prime & !U_prime & !D_prime) | (U_prime & (Su_prime | SEu_prime | SWu_prime)) | (D_prime & (Sd_prime | SEd_prime | SWd_prime));
    
    N_tierse = (N_zegond & !E_zegond & !W_zegond) | (N_zegond & E_zegond & R_ne) | (N_zegond & W_zegond & R_nw);
    E_tierse = (E_zegond & !N_zegond & !S_zegond) | (E_zegond & N_zegond & R_en) | (E_zegond & S_zegond & R_es);
    W_tierse = (W_zegond & !N_zegond & !S_zegond) | (W_zegond & N_zegond & R_wn) | (W_zegond & S_zegond & R_ws);
    S_tierse = (S_zegond & !E_zegond & !W_zegond) | (S_zegond & E_zegond & R_se) | (S_zegond & W_zegond & R_sw);
    
    N = N_tierse & C_n;
    E = E_tierse & C_e;
    W = W_tierse & C_w;
    S = S_tierse & C_s;
    U = U_prime  & C_u;
    D = D_prime  & C_d;
    
    /* Newly added logic for LBDR3D */
    if (U == true)
    {
        directions.push_back(DIRECTION_UP);
        return directions;
    }
    
    if (D == true)
    {
        directions.push_back(DIRECTION_DOWN);
        return directions;
    }
    /*  if ((N == true && W == true) || (S == true && W == true))
     {
     directions.push_back(DIRECTION_WEST);
     return directions;
     }  */
    if (E == true)
        directions.push_back(DIRECTION_EAST);
    if (W == true)
        directions.push_back(DIRECTION_WEST);
    if (N == true)
        directions.push_back(DIRECTION_NORTH);
    if (S == true)
        directions.push_back(DIRECTION_SOUTH);
    
    if (directions.size() == 0)
    {
        int dummy = 2;
        dummy++;
    }
    
    return directions;
    
}

//---------------------------------------------------------------------------

vector< int > NoximRouter::routing_DQP_NETZ(const NoximCoord & current, const NoximCoord & destination, int dir_in)
{
    vector<int> directions;
    
    //            For DEBUG
    //    if ( (coord2Id(current.x, current.y, current.z) == 47) && (Nu == true) )
    //        cout << "\n\nAt node 47 and going upwards !\n\n";
    
    if (current.z > destination.z) // destination layer is above current layer
    {
        if (current.x <= destination.x && current.y >= destination.y) // destination node is located on the NE quadrant, but on a different layer
        {
            
            if ((C_u == true) && (Eu == true) && (Nu == true)) // current node is up vertical node, but there is at least one up vertical node in N and E directions
            {
                directions.push_back(DIRECTION_UP);
                directions.push_back(DIRECTION_NORTH);
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_u == true) && (Eu == true) && (Nu == false))
            {
                directions.push_back(DIRECTION_UP);
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_u == true) && (Eu == false) && (Nu == true))
            {
                directions.push_back(DIRECTION_UP);
                directions.push_back(DIRECTION_NORTH);
            }
            
            else if ((C_u == true) && (Eu == false) && (Nu == false))
            {
                directions.push_back(DIRECTION_UP);
            }
            
            else if ((C_u == false) && (Eu == true) && (Nu == false))
            {
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_u == false) && (Eu == false) && (Nu == true))
            {
                directions.push_back(DIRECTION_NORTH);
            }
            
            else if ((C_u == false) && (Eu == true) && (Nu == true)) // Not in the paper, but should be there ??
            {
                directions.push_back(DIRECTION_NORTH);
                directions.push_back(DIRECTION_EAST);
            }
        }
        
        else if (current.x > destination.x && current.y < destination.y) // destination node is located on the SW quadrant, but on a different layer
            // The assumption is that there is at least one node on the NE quadrant with vertial link for transmitting the packet to the next layer
        {
            if (C_u == true)
                directions.push_back(DIRECTION_UP);
            
            else if ((C_u == false) && (Eu == true) && (Nu == true))
            {
                directions.push_back(DIRECTION_NORTH);
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_u == false) && (Eu == true) && (Nu == false))
            {
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_u == false) && (Eu == false) && (Nu == true))
            {
                directions.push_back(DIRECTION_NORTH);
            }
        }
        
        else if (current.x > destination.x && current.y >= destination.y) // destination node is located on the NW quadrant, but on a different layer
            // The assumption is that there is at least one node on the NE quadrant with vertial link for transmitting the packet to the next layer
        {
            if ((C_u == true) && (Nu == true))
            {
                directions.push_back(DIRECTION_UP);
                directions.push_back(DIRECTION_NORTH);
            }
            
            else if ((C_u == true) && (Nu == false))
            {
                directions.push_back(DIRECTION_UP);
            }
            
            else if ((C_u == false) && (Nu == true))
            {
                directions.push_back(DIRECTION_NORTH);
            }
            
            else if ((C_u == false) && (Nu == false) && (Eu == true))
            {
                directions.push_back(DIRECTION_EAST);
            }
        }
        
        else if (current.x <= destination.x && current.y < destination.y) // destination node is located on the SE quadrant, but on a different layer
            // The assumption is that there is at least one node on the NE quadrant with vertial link for transmitting the packet to the next layer
        {
            if ((C_u == true) && (Eu == true))
            {
                directions.push_back(DIRECTION_UP);
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_u == true) && (Eu == false))
            {
                directions.push_back(DIRECTION_UP);
            }
            
            else if ((C_u == false) && (Eu == true))
            {
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_u == false) && (Eu == false) && (Nu == true))
            {
                directions.push_back(DIRECTION_NORTH);
            }
        }
    }
    
    else if (current.z < destination.z) // destination layer is below current layer
    {
        if (current.x <= destination.x && current.y >= destination.y) // destination node is located on the NE quadrant, but on a different layer
        {
            if ((C_d == true) && (Ed == true) && (Nd == true)) // current node is up vertical node, but there is at least one up vertical node in N and E directions
            {
                directions.push_back(DIRECTION_DOWN);
                directions.push_back(DIRECTION_NORTH);
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_d == true) && (Ed == true) && (Nd == false))
            {
                directions.push_back(DIRECTION_DOWN);
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_d == true) && (Ed == false) && (Nd == true))
            {
                directions.push_back(DIRECTION_DOWN);
                directions.push_back(DIRECTION_NORTH);
            }
            
            else if ((C_d == true) && (Ed == false) && (Nd == false))
            {
                directions.push_back(DIRECTION_DOWN);
            }
            
            else if ((C_d == false) && (Ed == true) && (Nd == false))
            {
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_d == false) && (Ed == false) && (Nd == true))
            {
                directions.push_back(DIRECTION_NORTH);
            }
            
            else if ((C_d == false) && (Ed == true) && (Nd == true)) // Not in the paper, but should be there ??
            {
                directions.push_back(DIRECTION_NORTH);
                directions.push_back(DIRECTION_EAST);
            }
        }
        
        else if (current.x > destination.x && current.y < destination.y) // destination node is located on the SW quadrant, but on a different layer
            // The assumption is that there is at least one node on the NE quadrant with vertial link for transmitting the packet to the next layer
        {
            if (C_d == true)
                directions.push_back(DIRECTION_DOWN);
            
            else if ((C_d == false) && (Ed == true) && (Nd == true))
            {
                directions.push_back(DIRECTION_NORTH);
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_d == false) && (Ed == true) && (Nd == false))
            {
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_d == false) && (Ed == false) && (Nd == true))
            {
                directions.push_back(DIRECTION_NORTH);
            }
        }
        
        else if (current.x > destination.x && current.y >= destination.y) // destination node is located on the NW quadrant, but on a different layer
            // The assumption is that there is at least one node on the NE quadrant with vertial link for transmitting the packet to the next layer
        {
            if ((C_d == true) && (Nd == true))
            {
                directions.push_back(DIRECTION_DOWN);
                directions.push_back(DIRECTION_NORTH);
            }
            
            else if ((C_d == true) && (Nd == false))
            {
                directions.push_back(DIRECTION_DOWN);
            }
            
            else if ((C_d == false) && (Nd == true))
            {
                directions.push_back(DIRECTION_NORTH);
            }
            
            else if ((C_d == false) && (Nd == false) && (Ed == true))
            {
                directions.push_back(DIRECTION_EAST);
            }
        }
        
        else if (current.x <= destination.x && current.y < destination.y) // destination node is located on the SE quadrant, but on a different layer
            // The assumption is that there is at least one node on the NE quadrant with vertial link for transmitting the packet to the next layer
        {
            if ((C_d == true) && (Ed == true))
            {
                directions.push_back(DIRECTION_DOWN);
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_d == true) && (Ed == false))
            {
                directions.push_back(DIRECTION_DOWN);
            }
            
            else if ((C_d == false) && (Ed == true))
            {
                directions.push_back(DIRECTION_EAST);
            }
            
            else if ((C_d == false) && (Ed == false) && (Nd == true))
            {
                directions.push_back(DIRECTION_NORTH);
            }
        }
    }
    
    else if ((current.z == destination.z)) // && (current.x != destination.x) && (current.y != destination.y)) // destination layer is the same as current layer, then us XY routing
    {
        if      (destination.x > current.x)	directions.push_back(DIRECTION_EAST );
        else if (destination.x < current.x)	directions.push_back(DIRECTION_WEST );
        else if (destination.y > current.y)	directions.push_back(DIRECTION_SOUTH);
        else if (destination.y < current.y)	directions.push_back(DIRECTION_NORTH);
    }
    
    if (directions.size() == 0)
        cout << "\n\nDirections size is zero!\n\n";
    
    return directions;
}


//---------------------------------------------------------------------------
struct elevator_node
{
    int elevator_node_id;
    int Manhattan_Distance;
};

vector< int > NoximRouter::routing_ETW(NoximCoord & current, NoximCoord & destination, int dir_in)
{
    vector<int> directions;
    
    // Shortest Manhattan Distance Elevator Assignment Algorithm (for ETW Routing)
    
    ifstream inputFile;
    inputFile.open("elevator_nodes.txt"); // Open the file including all up and down elevators
    
    int no_of_elevator_nodes;
    inputFile >> no_of_elevator_nodes;
    
    elevator eligible_up_elevator_nodes, eligible_down_elevator_nodes; // List of eligible up and down elevators chosen by SMD algorithm (for ETW Routing)
    
    elevator_nodes elevator_list[no_of_elevator_nodes];
    
    for (int count = 0; count < no_of_elevator_nodes; count++)
    {
        inputFile >> elevator_list[count].elevator_node_id >> elevator_list[count].up >> elevator_list[count].down;
    }
    
    inputFile.close();
    
    
    //    cout << "\n\ Down Elevator Nodes: \n";
    
    for (int i=0; i < elevator_nodes_down.size(); i++)
    {
        cout << elevator_nodes_down[i] << " ";
    }
    
    // If destination is on the upper layer(s) (U) or
    // If destination is on the lower layer(s) and on the East side of current node(DE)
    // (Packets stay in Set1 or they can change from Set1 to Set2 directions)
    // Eligible Elevator is assigned during run-time
    if ( (destination.z < current.z) || ( (destination.z > current.z) && (destination.x >= current.x) ) )
    {
        // Perform this for all elevators in the current layer
        if (destination.z < current.z) // If going upwards
        {
            for (int p1 = 0; p1 < no_of_elevator_nodes; p1++)  // Check the up elevator nodes in elevator nodes array
            {
                NoximCoord elevator_coord;
                elevator_coord = id2Coord(elevator_list[p1].elevator_node_id);
                
                if (elevator_list[p1].elevator_node_id <= xyz2Id(NoximGlobalParams::mesh_dim_x-1,NoximGlobalParams::mesh_dim_y-1,current.z) && elevator_list[p1].elevator_node_id >= xyz2Id(0,0,current.z) && elevator_list[p1].up == true && elevator_coord.z == current.z) // focusing on the up elevator node belonging to the current layer
                {
                    // If Elevator node is on East side of Destination node
                    if (elevator_coord.x >= destination.x)
                    {
                        // Elevator is eligible, calculate the Manhattan Distance and add it to the list of eligible up elevators
                        
                        int Manhattan_Distance = abs(elevator_coord.x - current.x) + abs(elevator_coord.y - current.y) + abs(destination.x - elevator_coord.x) + abs(destination.y - elevator_coord.y);
                        
                        eligible_up_elevator_nodes.insert_node(elevator_list[p1].elevator_node_id, Manhattan_Distance);
                    }
                }
            }
            
            eligible_up_elevator_nodes.sort_nodes();
            //            NoximCoord nearest_elevator;
            
            //            nearest_elevator = id2Coord(eligible_up_elevator_nodes.nearest_elevators());
            //
            //            if (eligible_up_elevator_nodes.nearest_elevators() != -1) // if there is at least one eligible up elevator node chosen by the SMD algorithm
            //            {
            //                cout << "Elevator found : for node [" << coord2Id(current) << "] , the chosen up elevator is located at node [" << coord2Id(nearest_elevator) << "] \n";
            //            }
            
        }
        
        else if (destination.z > current.z) // If going downwards
        {
            for (int p1 = 0; p1 < no_of_elevator_nodes; p1++)  // Check the down elevator nodes in elevators array
            {
                NoximCoord elevator_coord;
                elevator_coord = id2Coord(elevator_list[p1].elevator_node_id);
                
                if (elevator_list[p1].elevator_node_id <= xyz2Id(NoximGlobalParams::mesh_dim_x-1,NoximGlobalParams::mesh_dim_y-1,current.z) && elevator_list[p1].elevator_node_id >= xyz2Id(0,0,current.z) && elevator_list[p1].down == true && elevator_coord.z == current.z) // focusing on the down elevator node belonging to the current layer
                {
                    // If Elevator node is on East side of Destination node
                    if (elevator_coord.x >= destination.x)
                    {
                        // Elevator is eligible, calculate the Manhattan Distance and add it to the list of eligible up elevators
                        
                        int Manhattan_Distance = abs(elevator_coord.x - current.x) + abs(elevator_coord.y - current.y) + abs(destination.x - elevator_coord.x) + abs(destination.y - elevator_coord.y);
                        
                        eligible_down_elevator_nodes.insert_node(elevator_list[p1].elevator_node_id, Manhattan_Distance);
                    }
                }
            }
            
            eligible_down_elevator_nodes.sort_nodes();
            //            NoximCoord nearest_elevator;
            
            //            nearest_elevator = id2Coord(eligible_down_elevator_nodes.nearest_elevators());
            //
            //            if (eligible_down_elevator_nodes.nearest_elevators() != -1) // if there is at least one eligible down elevator node chosen by the SMD algorithm
            //            {
            //                cout << "Elevator found : for node [" << coord2Id(current) << "] , the chosen down elevator is located at node [" << coord2Id(nearest_elevator) << "] \n";
            //            }
        }
    }
    
    // If destination is on the lower layer(s) and on the West side of current node(DW)
    // Eligible Elevator is assigned during run-time
    // Definitely going downwards
    
    else if ( (destination.z > current.z) && (destination.x < current.x) )
    {
        for (int p1 = 0; p1 < no_of_elevator_nodes; p1++)  // Check the down elevator nodes in elevators array
        {
            NoximCoord elevator_coord;
            elevator_coord = id2Coord(elevator_list[p1].elevator_node_id);
            
            if (elevator_list[p1].elevator_node_id <= xyz2Id(NoximGlobalParams::mesh_dim_x-1,NoximGlobalParams::mesh_dim_y-1,current.z) && elevator_list[p1].elevator_node_id >= xyz2Id(0,0,current.z) && elevator_list[p1].down == true && elevator_coord.z == current.z) // focusing on the down elevator node belonging to the current layer
            {
                // If Elevator node is between current node and destination node, or
                // If Elevator node is on the East side of the current node
                if ( ( (destination.x <= elevator_coord.x) && (elevator_coord.x << current.x) ) || (elevator_coord.x >= current.x) )
                {
                    // Elevator is eligible, calculate the Manhattan Distance and add it to the list of eligible up elevators
                    
                    int Manhattan_Distance = abs(elevator_coord.x - current.x) + abs(elevator_coord.y - current.y) + abs(destination.x - elevator_coord.x) + abs(destination.y - elevator_coord.y);
                    
                    eligible_down_elevator_nodes.insert_node(elevator_list[p1].elevator_node_id, Manhattan_Distance);
                }
            }
        }
        
        eligible_down_elevator_nodes.sort_nodes();
        //        NoximCoord nearest_elevator;
        //
        //        nearest_elevator = id2Coord(eligible_down_elevator_nodes.nearest_elevators());
        //
        //        if (eligible_down_elevator_nodes.nearest_elevators() != -1) // if there is at least one eligible down elevator node chosen by the SMD algorithm
        //        {
        //            cout << "Elevator found : for node [" << coord2Id(current) << "] , the chosen down elevator is located at node [" << coord2Id(nearest_elevator) << "] \n";
        //        }
    }
    
    /*
     cout << "\nEligible Up Elevator Nodes: \n";
     
     for (int i=0; i < eligible_up_elevator_nodes.size(); i++)
     {
     cout << eligible_up_elevator_nodes[i].Manhattan_Distance << " ";
     }
     
     cout << "\n\nEligible Down Elevator Nodes: \n";
     
     for (int i=0; i < eligible_down_elevator_nodes.size(); i++)
     {
     cout << eligible_down_elevator_nodes[i].elevator_node_id << " ";
     }
     
     cout << "\n\n";
     */
    
    // End of Shortest Manhattan Distance Elevator Assignment Algorithm (for ETW Routing)
    
    // Routing Algorithm
    // Something is not right regarding the routing algorithm, because the elevators assigned to a node must be used
    
    if (destination.z == current.z) // Source and destination are on the same layer
    {
        if (destination.x >= current.x) // Use West-First routing turns
        {
            if (destination.x == current.x ||
                destination.y == current.y)
                return routingXYZ(current, destination);
            
            if (destination.y < current.y)
            {
                directions.push_back(DIRECTION_NORTH);
                directions.push_back(DIRECTION_EAST);
            }
            else
            {
                directions.push_back(DIRECTION_SOUTH);
                directions.push_back(DIRECTION_EAST);
            }
        }
        
        else if (destination.x < current.x)
        {
            if (destination.y < current.y) // NW quadrant
            {
                directions.push_back(DIRECTION_NORTH);
                directions.push_back(DIRECTION_WEST);
            }
            
            else if (destination.y == current.y) // West direction
            {
                directions.push_back(DIRECTION_WEST);
            }
            
            else if (destination.y > current.y) // SW quadrant
            {
                directions.push_back(DIRECTION_SOUTH);
                directions.push_back(DIRECTION_WEST);
            }
        }
        
        else // if (destination.x == current.x)
        {
            if (destination.y < current.y)
                directions.push_back(DIRECTION_NORTH);
            
            else if (destination.y > current.y)
                directions.push_back(DIRECTION_SOUTH);
        }
    }
    
    else if (destination.z < current.z) // Destination is on the upper layer
    {
        int elevator_node_id = eligible_up_elevator_nodes.nearest_elevators();
        NoximCoord elevator_id = id2Coord(elevator_node_id);
        
        if (local_id == elevator_node_id) // if current node is an up elevator, send to UP
            directions.push_back(DIRECTION_UP);
        
        else  // if current node is not an up elevator, send to eligible elevator node on current layer (with least Manhattan Distance), use directions only from Set1 (East, Up, North and South (via VC0))
        {
            if (elevator_id.x <= current.x ||
                elevator_id.y == current.y)
                return routingXYZ(current, elevator_id);
            
            if (elevator_id.y < current.y)
            {
                directions.push_back(DIRECTION_NORTH);
                directions.push_back(DIRECTION_EAST);
            }
            else
            {
                directions.push_back(DIRECTION_SOUTH);
                directions.push_back(DIRECTION_EAST);
            }
        }
    }
    
    else if (destination.z > current.z) // Destination is on the lower layer
    {
        int elevator_node_id = eligible_down_elevator_nodes.nearest_elevators();
        NoximCoord elevator_id = id2Coord(elevator_node_id);
        
        if (local_id == elevator_node_id) // if current node is a down elevator, send to DOWN
            directions.push_back(DIRECTION_DOWN);
        
        if ( (elevator_id.x == current.x) && (elevator_id.y > current.y) ) // S
            directions.push_back(DIRECTION_SOUTH);
        
        else if ( (elevator_id.x == current.x) && (elevator_id.y < current.y) ) // N
            directions.push_back(DIRECTION_NORTH);
        
        else if ( (elevator_id.x > current.x) && (elevator_id.y < current.y) ) // NE
        {
            directions.push_back(DIRECTION_EAST);
            directions.push_back(DIRECTION_NORTH);
        }
        else if ( (elevator_id.x < current.x) && (elevator_id.y < current.y) ) // NW
        {
            directions.push_back(DIRECTION_WEST);
            directions.push_back(DIRECTION_NORTH);
        }
        
        else if ( (elevator_id.x > current.x) && (elevator_id.y > current.y) ) // SE
        {
            directions.push_back(DIRECTION_EAST);
            directions.push_back(DIRECTION_SOUTH);
        }
        
        else if ( (elevator_id.x < current.x) && (elevator_id.y > current.y) ) // SW
        {
            directions.push_back(DIRECTION_WEST);
            directions.push_back(DIRECTION_SOUTH);
        }
        
        else if ((elevator_id.y == current.y) && (elevator_id.x < current.x)) // W
            directions.push_back(DIRECTION_WEST);
        
        else if ((elevator_id.y == current.y) && (elevator_id.x > current.x)) // E
            directions.push_back(DIRECTION_EAST);
    }
    
    return directions;
}


//---------------------------------------------------------------------------

vector<int> NoximRouter::routingWestFirst(const NoximCoord& current, const NoximCoord& destination)
{
    vector<int> directions;
    
    if (destination.x <= current.x ||
        destination.y == current.y)
        return routingXYZ(current, destination);
    
    if (destination.y < current.y)
    {
        directions.push_back(DIRECTION_NORTH);
        directions.push_back(DIRECTION_EAST);
    }
    else
    {
        directions.push_back(DIRECTION_SOUTH);
        directions.push_back(DIRECTION_EAST);
    }
    
    return directions;
}

//---------------------------------------------------------------------------

vector<int> NoximRouter::routingNorthLast(const NoximCoord& current, const NoximCoord& destination)
{
    vector<int> directions;
    
    if (destination.x == current.x ||
        destination.y <= current.y)
        return routingXYZ(current, destination);
    
    if (destination.x < current.x)
    {
        directions.push_back(DIRECTION_SOUTH);
        directions.push_back(DIRECTION_WEST);
    }
    else
    {
        directions.push_back(DIRECTION_SOUTH);
        directions.push_back(DIRECTION_EAST);
    }
    
    return directions;
}

//---------------------------------------------------------------------------

vector<int> NoximRouter::routingNegativeFirst(const NoximCoord& current, const NoximCoord& destination)
{
    vector<int> directions;
    
    if ( (destination.x <= current.x && destination.y <= current.y) ||
        (destination.x >= current.x && destination.y >= current.y) )
        return routingXYZ(current, destination);
    
    if (destination.x > current.x && 
        destination.y < current.y)
    {
        directions.push_back(DIRECTION_NORTH);
        directions.push_back(DIRECTION_EAST);
    }
    else
    {
        directions.push_back(DIRECTION_SOUTH);
        directions.push_back(DIRECTION_WEST);
    }
    
    return directions;
}

//---------------------------------------------------------------------------

vector<int> NoximRouter::routingOddEven(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination)
{
    vector<int> directions;
    
    int c0 = current.x;
    int c1 = current.y;
    int s0 = source.x;
    int d0 = destination.x;
    int d1 = destination.y;
    int e0, e1;
    
    e0 = d0 - c0;
    e1 = -(d1 - c1);
    
    if (e0 == 0)
    {
        if (e1 > 0)
            directions.push_back(DIRECTION_NORTH);
        else
            directions.push_back(DIRECTION_SOUTH);
    }
    else
    {
        if (e0 > 0)
        {
            if (e1 == 0)
                directions.push_back(DIRECTION_EAST);
            else
            {
                if ( (c0 % 2 == 1) || (c0 == s0) )
                {
                    if (e1 > 0)
                        directions.push_back(DIRECTION_NORTH);
                    else
                        directions.push_back(DIRECTION_SOUTH);
                }
                if ( (d0 % 2 == 1) || (e0 != 1) )
                    directions.push_back(DIRECTION_EAST);
            }
        }
        else
        {
            directions.push_back(DIRECTION_WEST);
            if (c0 % 2 == 0)
            {
                if (e1 > 0)
                    directions.push_back(DIRECTION_NORTH);
                if (e1 < 0) 
                    directions.push_back(DIRECTION_SOUTH);
            }
        }
    }
    
    if (!(directions.size() > 0 && directions.size() <= 2))
    {
        cout << "\nRouting OddEven errore:";
        cout << source << endl;
        cout << destination << endl;
        cout << current << endl;
        
    }
    assert(directions.size() > 0 && directions.size() <= 2);
    
    return directions;
}

//---------------------------------------------------------------------------

vector<int> NoximRouter::routingDyAD(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination)
{
    vector<int> directions;
    
    directions = routingOddEven(current, source, destination);
    
    if (!inCongestion())
        directions.resize(1);
    
    return directions;
}

//---------------------------------------------------------------------------

vector<int> NoximRouter::routingFullyAdaptive(const NoximCoord& current, const NoximCoord& destination)
{
    vector<int> directions;
    
    if (destination.x == current.x || destination.y == current.y)
        return routingXYZ(current, destination);
    
    if (destination.x > current.x && destination.y < current.y)
    {
        directions.push_back(DIRECTION_NORTH);
        directions.push_back(DIRECTION_EAST);
    }
    else if (destination.x > current.x && destination.y > current.y)
    {
        directions.push_back(DIRECTION_SOUTH);
        directions.push_back(DIRECTION_EAST);
    }
    else if (destination.x < current.x && destination.y > current.y)
    {
        directions.push_back(DIRECTION_SOUTH);
        directions.push_back(DIRECTION_WEST);
    }
    else
    {
        directions.push_back(DIRECTION_NORTH);
        directions.push_back(DIRECTION_WEST);
    }
    
    return directions;
}

//---------------------------------------------------------------------------

vector<int> NoximRouter::routingTableBased(const int dir_in, const NoximCoord& current, const NoximCoord& destination)
{
    NoximAdmissibleOutputs ao = routing_table.getAdmissibleOutputs(dir_in, coord2Id(destination));
    
    if (ao.size() == 0)
    {
        cout << "dir: " << dir_in << ", (" << current.x << "," << current.y << ") --> "
        << "(" << destination.x << "," << destination.y << ")" << endl
        << coord2Id(current) << "->" << coord2Id(destination) << endl;
    }
    
    assert(ao.size() > 0);
    
    return admissibleOutputsSet2Vector(ao);
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------


void NoximRouter::configure(const int _id, const double _warm_up_time, const unsigned int _max_buffer_size,
                            NoximGlobalRoutingTable& grt)
{
    local_id = _id;
    stats.configure(_id, _warm_up_time);
    
    start_from_port = DIRECTION_LOCAL;
    
    for (int i=0; i<DIRECTIONS; i++)
        start_from_channel[i] = 0;
    
    if (grt.isValid())
        routing_table.configure(grt, _id);
    
    for (int i=0; i<DIRECTIONS; i++)
        for (int j=0; j<NoximGlobalParams::number_virtual_channel; j++)
            buffer[i][j].SetMaxBufferSize(_max_buffer_size);
    
    local_buffer.SetMaxBufferSize(_max_buffer_size);
}

//---------------------------------------------------------------------------

unsigned long NoximRouter::getRoutedFlits()
{ 
    return routed_flits;
}

//---------------------------------------------------------------------------

unsigned int NoximRouter::getFlitsCount()
{
    unsigned count = 0;
    
    for (int i=0; i<DIRECTIONS; i++)
    {
        for (int j=0; j<NoximGlobalParams::number_virtual_channel; j++)
            count += buffer[i][j].Size();
    }
    count += local_buffer.Size();
    
    return count;
}

//---------------------------------------------------------------------------

double NoximRouter::getPower()
{
    return stats.power.getPower();
}

//---------------------------------------------------------------------------

int NoximRouter::reflexDirection(int direction) const
{
    if (direction == DIRECTION_NORTH) return DIRECTION_SOUTH;
    if (direction == DIRECTION_EAST) return DIRECTION_WEST;
    if (direction == DIRECTION_WEST) return DIRECTION_EAST;
    if (direction == DIRECTION_SOUTH) return DIRECTION_NORTH;
    
    // you shouldn't be here
    assert(false);
    return NOT_VALID;
}

//---------------------------------------------------------------------------

int NoximRouter::getNeighborId(int _id, int direction) const
{
    NoximCoord my_coord = id2Coord(_id);
    
    switch (direction)
    {
        case DIRECTION_NORTH:
            if (my_coord.y==0) return NOT_VALID;
            my_coord.y--;
            break;
        case DIRECTION_SOUTH:
            if (my_coord.y==NoximGlobalParams::mesh_dim_y-1) return NOT_VALID;
            my_coord.y++;
            break;
        case DIRECTION_EAST:
            if (my_coord.x==NoximGlobalParams::mesh_dim_x-1) return NOT_VALID;
            my_coord.x++;
            break;
        case DIRECTION_WEST:
            if (my_coord.x==0) return NOT_VALID;
            my_coord.x--;
            break;
        default:
            cout << "direction not valid : " << direction;
            assert(false);
    }
    
    int neighbor_id = coord2Id(my_coord);
    
    return neighbor_id;
}

//---------------------------------------------------------------------------

bool NoximRouter::inCongestion()
{
    for (int i=0; i<DIRECTIONS; i++)
    {
        int flits = 0;
        for (int j=0; j<NoximGlobalParams::number_virtual_channel; j++)
            flits += NoximGlobalParams::buffer_depth - free_slots_neighbor[i][j];
        if (flits > (int)(NoximGlobalParams::number_virtual_channel * NoximGlobalParams::buffer_depth * NoximGlobalParams::dyad_threshold))
            return true;
    }
    
    return false;
}

//---------------------------------------------------------------------------