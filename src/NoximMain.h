/*****************************************************************************

  NoximMain.h -- Common constants and structs definitions

 *****************************************************************************/

#ifndef __NoximMain_H__
#define __NoximMain_H__

//---------------------------------------------------------------------------

#include <cassert>
#include <systemc.h>
#include <vector>
#include <sstream>

using namespace std;


// Define the directions as numbers
#define DIRECTIONS             6
#define DIRECTION_NORTH        0
#define DIRECTION_EAST         1
#define DIRECTION_SOUTH        2
#define DIRECTION_WEST         3
#define DIRECTION_UP           4
#define DIRECTION_DOWN         5
#define DIRECTION_LOCAL        6

// Generic not reserved resource
#define NOT_RESERVED          -2

// To mark invalid or non exhistent values
#define NOT_VALID             -1

// Routing algorithms
#define ROUTING_XYZ            0
#define ROUTING_WEST_FIRST     1
#define ROUTING_NORTH_LAST     2
#define ROUTING_NEGATIVE_FIRST 3
#define ROUTING_ODD_EVEN       4
#define ROUTING_DYAD           5
#define ROUTING_FULLY_ADAPTIVE 8
#define ROUTING_TABLE_BASED    9
#define ROUTING_LBDR          10
#define ROUTING_DQP_NETZ      11
#define ROUTING_ETW           12
#define INVALID_ROUTING       -1

// Selection strategies
#define SEL_RANDOM             0
#define SEL_BUFFER_LEVEL       1
#define SEL_NOP                2
#define SEL_PATHAWARE_MINIMAL  3
#define INVALID_SELECTION     -1

// Traffic distribution
#define TRAFFIC_RANDOM         0
#define TRAFFIC_TRANSPOSE1     1
#define TRAFFIC_TRANSPOSE2     2
#define TRAFFIC_HOTSPOT        3
#define TRAFFIC_TABLE_BASED    4
#define TRAFFIC_BIT_REVERSAL   5
#define TRAFFIC_SHUFFLE        6
#define TRAFFIC_BUTTERFLY      7
#define INVALID_TRAFFIC       -1

// Verbosity levels
#define VERBOSE_OFF            0
#define VERBOSE_LOW            1
#define VERBOSE_MEDIUM         2
#define VERBOSE_HIGH           3

// Encoder types
#define DENC_TRANSPARENT       0
#define DENC_BI                1
#define DENC_CDBI              2
#define DENC_SC                3
#define DENC_SCS               4
#define DENC_FPC               5

#define ECODING_NONE           0
#define ECODING_JTEC           1
#define ECODING_JTEC_SQED      2
#define ECODING_DAP            3
#define ECODING_CADEC          4
#define ECODING_ED             5
#define ECODING_SEC            6
#define ECODING_BCH            7

#define NET_MESH               0
#define NET_TORUS              1
#define NET_FOLDED_TORUS       2

#define JTEC_PAYLOAD_LENGTH         77
#define JTEC_SQED_PAYLOAD_LENGTH    78
#define ED_SEC_PAYLOAD_LENGTH       38
#define BCH_PAYLOAD_LENGTH          44
#define DAP_PAYLOAD_LENGTH          65

//---------------------------------------------------------------------------

// Default configuration can be overridden with command-line arguments
#define DEFAULT_VERBOSE_MODE               VERBOSE_OFF
#define DEFAULT_TRACE_MODE                       false
#define DEFAULT_TRACE_FILENAME                      ""
#define DEFAULT_MESH_DIM_X                           4
#define DEFAULT_MESH_DIM_Y                           4
#define DEFAULT_MESH_DIM_Z                           4
#define DEFAULT_TOPOLOGY                      NET_MESH
#define DEFAULT_BUFFER_DEPTH                         4
#define DEFAULT_MAX_PACKET_SIZE                      8
#define DEFAULT_MIN_PACKET_SIZE                      8
#define DEFAULT_ROUTING_ALGORITHM          ROUTING_ETW
#define DEFAULT_NUMBER_VIRTUAL_CHANNEL               2
#define DEFAULT_ROUTING_TABLE_FILENAME              ""
#define DEFAULT_SELECTION_STRATEGY          SEL_RANDOM
#define DEFAULT_PACKET_INJECTION_RATE             0.018
#define DEFAULT_PROBABILITY_OF_RETRANSMISSION     0.018
#define DEFAULT_TRAFFIC_DISTRIBUTION    TRAFFIC_RANDOM
#define DEFAULT_TRAFFIC_TABLE_FILENAME              "table.txt"
#define DEFAULT_RESET_TIME                        1000
#define DEFAULT_SIMULATION_TIME                  10000
#define DEFAULT_STATS_WARM_UP_TIME  DEFAULT_RESET_TIME
#define DEFAULT_DETAILED                         false
#define DEFAULT_DYAD_THRESHOLD                     0.6
#define DEFAULT_MAX_VOLUME_TO_BE_DRAINED             0
#define DEFAULT_FLIT_SIZE_BITS                      32
#define DEFAULT_RND_TRAFFIC                       true
#define DEFAULT_RND_TRAFFIC_SIZE                8*1024
#define DEFAULT_TRAFFIC_FNAME                       ""
#define DEFAULT_DENC                  DENC_TRANSPARENT
#define DEFAULT_PARTITIONS                           1
#define DEFAULT_BIT_ERROR_RATE                     0.0
#define DEFAULT_ERROR_CODING              ECODING_NONE
#define DEFAULT_LINK_VDD                           1.6

// TODO by Fafa - this MUST be removed!!! Use only STL vectors instead!!!
#define MAX_STATIC_DIM 20

//---------------------------------------------------------------------------
// NoximGlobalParams -- used to forward configuration to every sub-block
struct NoximGlobalParams
{
  static int verbose_mode;
  static int trace_mode;
  static char trace_filename[128];
  static int mesh_dim_x;
  static int mesh_dim_y;
  static int mesh_dim_z;
  static int topology;
  static int buffer_depth;
  static int min_packet_size;
  static int max_packet_size;
  static int routing_algorithm;
  static int number_virtual_channel;
  static char routing_table_filename[128];
  static int selection_strategy;
  static float packet_injection_rate;
  static float probability_of_retransmission;
  static int traffic_distribution;
  static char traffic_table_filename[128];
  static int simulation_time;
  static int stats_warm_up_time;
  static int rnd_generator_seed;
  static int partitions;
  static bool detailed;
  static vector<pair<int,double> > hotspots;
  static float dyad_threshold;
  static unsigned int max_volume_to_be_drained;
  static unsigned int flit_size_bits;
  static char traffic_fname[128];
  static bool rnd_traffic;
  static unsigned int rnd_traffic_size;
  static unsigned int data_encoding;
  static float bit_error_rate;
  static unsigned int error_coding;
  static float link_vdd;
};


//---------------------------------------------------------------------------
// NoximCoord -- XY coordinates type of the Tile inside the Mesh
class NoximCoord
{
public:
    int x;			// X coordinate
    int y;			// Y coordinate
    int z;          // Z coordinate
    
    inline bool operator ==(const NoximCoord & coord) const
    {
        return (coord.x == x && coord.y == y && coord.z == z);
    }
};

//---------------------------------------------------------------------------
// NoximFlitType -- Flit type enumeration
enum NoximFlitType
{
  FLIT_TYPE_HEAD, FLIT_TYPE_BODY, FLIT_TYPE_TAIL
};

//---------------------------------------------------------------------------
// NoximPayload -- Payload definition
struct NoximPayload
{
    sc_uint<32> data;	// Bus for the data to be exchanged
    
    inline bool operator ==(const NoximPayload & payload) const {
        return (payload.data == data);
    }
};

//---------------------------------------------------------------------------
// NoximPacket -- Packet definition
struct NoximPacket
{
  int                src_id;
  int                dst_id;
  double             timestamp;    // SC timestamp at packet generation
  int                size;
  int                flit_left;    // Number of remaining flits inside the packet
  int                is_nack;

  NoximPacket() {;}
  NoximPacket(const int s, const int d, const double ts, const int sz) {
    make(s, d, ts, sz);
  }

  void make(const int s, const int d, const double ts, const int sz) {
    src_id = s; dst_id = d; timestamp = ts; size = sz; flit_left = sz;
  }
};

//---------------------------------------------------------------------------
// NoximRouteData -- data required to perform routing
struct NoximRouteData
{
    int current_id;
    int src_id;
    int dst_id;
    int dir_in; // direction from which the packet comes from
};

//---------------------------------------------------------------------------

struct NoximChannelStatus
{
    int free_slots;  // occupied buffer slots
    bool available; // 
    inline bool operator != (const NoximChannelStatus& bs) const
    {
	return (free_slots != bs.free_slots || available != bs.available);
    };
};

//---------------------------------------------------------------------------
// NoximNoP_data -- NoP Data definition
struct NoximNoP_data
{
    int sender_id;
	NoximChannelStatus* channel_status_neighbor;

    inline bool operator == (const NoximNoP_data& nop_data) const
    {
			return false;
    };
};

//---------------------------------------------------------------------------
// NoximFlit -- Flit definition
struct NoximFlit
{
  int               src_id;
  int               dst_id;
  int               channel;
  NoximFlitType     flit_type;    // The flit type (FLIT_TYPE_HEAD, FLIT_TYPE_BODY, FLIT_TYPE_TAIL)
  int               sequence_no;  // The sequence number of the flit inside the packet
  NoximPayload      payload;      // Payload
  NoximPayload      enc_payload;
  NoximPayload      original_payload;
  int               is_nack;
  double            timestamp;    // Unix timestamp at packet generation
  int               hop_no;       // Current number of hops from source to destination
    
  inline bool operator ==(const NoximFlit & flit) const
    {
        return (flit.src_id == src_id && flit.dst_id == dst_id
                && flit.flit_type == flit_type
                && flit.sequence_no == sequence_no
                && flit.timestamp == timestamp
                && flit.hop_no == hop_no);
    }
  
};

// output redefinitions *******************************************

inline ostream& operator << (ostream& os, const NoximFlit & flit)
{

  if (NoximGlobalParams::verbose_mode==VERBOSE_HIGH)
  {
      os << "### FLIT ###" << endl;
      os << "Source Tile[" << flit.src_id << "]" << endl;
      os << "Destination Tile[" << flit.dst_id << "]" << endl;
      switch(flit.flit_type)
      {
	case FLIT_TYPE_HEAD: os << "Flit Type is HEAD" << endl; break;
	case FLIT_TYPE_BODY: os << "Flit Type is BODY" << endl; break;
	case FLIT_TYPE_TAIL: os << "Flit Type is TAIL" << endl; break;
      }
      os << "Sequence no. " << flit.sequence_no << endl;
      os << "Payload printing not implemented (yet)." << endl;
      os << "Unix timestamp at packet generation " << flit.timestamp << endl;
      os << "Total number of hops from source to destination is " << flit.hop_no << endl;;
  }
  else
    {
      os << "[type: ";
      switch(flit.flit_type)
      {
	case FLIT_TYPE_HEAD: os << "H"; break;
	case FLIT_TYPE_BODY: os << "B"; break;
	case FLIT_TYPE_TAIL: os << "T"; break;
      }
      
      os << ", seq: " << flit.sequence_no << ", " << flit.src_id << "-->" << flit.dst_id << "]"; 
    }

  return os;
}

//---------------------------------------------------------------------------

inline ostream& operator << (ostream& os, const NoximChannelStatus& status)
{
  char msg;
  if (status.available) msg = 'A'; 
  else
      msg = 'N';
  os << msg << "(" << status.free_slots << ")"; 
  return os;
}

//---------------------------------------------------------------------------

inline ostream& operator << (ostream& os, const NoximNoP_data& NoP_data)
{
  os << "      NoP data from [" << NoP_data.sender_id << "] [ ";

  for (int j=0; j<DIRECTIONS*NoximGlobalParams::number_virtual_channel; j++)
      os << NoP_data.channel_status_neighbor[j] << " ";

  cout << "]" << endl;
  return os;
}

//---------------------------------------------------------------------------

inline ostream& operator << (ostream& os, const NoximCoord& coord)
{
  os << "(" << coord.x << "," << coord.y << ")";

  return os;
}


// trace redefinitions *******************************************
//
//---------------------------------------------------------------------------
inline void sc_trace(sc_trace_file*& tf, const NoximFlit& flit, string& name)
{
  sc_trace(tf, flit.src_id, name+".src_id");
  sc_trace(tf, flit.dst_id, name+".dst_id");
  sc_trace(tf, flit.sequence_no, name+".sequence_no");
  sc_trace(tf, flit.timestamp, name+".timestamp");
  sc_trace(tf, flit.hop_no, name+".hop_no");
}

//---------------------------------------------------------------------------

inline void sc_trace(sc_trace_file*& tf, const NoximNoP_data& NoP_data, string& name)
{
  sc_trace(tf, NoP_data.sender_id, name+".sender_id");
}

//---------------------------------------------------------------------------

inline void sc_trace(sc_trace_file*& tf, const NoximChannelStatus& bs, string& name)
{
  sc_trace(tf, bs.free_slots, name+".free_slots");
  sc_trace(tf, bs.available, name+".available");
}

// misc common functions **************************************
//---------------------------------------------------------------------------
inline NoximCoord id2Coord(int id)
{
    NoximCoord coord;
    
    coord.z = id / (NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y);////
    coord.y = (id-coord.z*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y) /NoximGlobalParams::mesh_dim_x;
    coord.x = (id-coord.z*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y)  % NoximGlobalParams::mesh_dim_x;
    
    assert(coord.x < NoximGlobalParams::mesh_dim_x);
    assert(coord.y < NoximGlobalParams::mesh_dim_y);
    assert(coord.z < NoximGlobalParams::mesh_dim_z);
    
    return coord;
}

//---------------------------------------------------------------------------

inline int coord2Id(const NoximCoord & coord)
{
    int id = coord.z*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y + (coord.y * NoximGlobalParams::mesh_dim_x) + coord.x;
    assert(id < NoximGlobalParams::mesh_dim_x * NoximGlobalParams::mesh_dim_y * NoximGlobalParams::mesh_dim_z);
    return id;
}

inline int coord2Id(int x, int y, int z)
{
    NoximCoord c = {x, y, z};
    return coord2Id(c);
}

inline int xyz2Id( int x, int y, int z)
{
    int id = z * NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y + y * NoximGlobalParams::mesh_dim_x + x;
    
    assert(id < NoximGlobalParams::mesh_dim_x * NoximGlobalParams::mesh_dim_y * NoximGlobalParams::mesh_dim_z);
    
    return id;
}

inline int xy2Id(int x, int y)
{
    int id = (y * NoximGlobalParams::mesh_dim_x) + x;
    
    assert(id < NoximGlobalParams::mesh_dim_x * NoximGlobalParams::mesh_dim_y);
    
    return id;
}

//---------------------------------------------------------------------------

#endif