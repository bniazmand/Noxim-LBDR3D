/*****************************************************************************

  NoximGlobalStats.cpp -- Global Statistics implementation

 *****************************************************************************/

#include <iomanip>
#include "NoximGlobalStats.h"

template <typename T>
NoximGlobalStats::Matrix<T> NoximGlobalStats::mkMatrix (size_t xdim, size_t ydim, size_t zdim)
{
    Matrix<T> mtx;
    
    mtx.resize(xdim);
    for (int i = 0; i < xdim; ++i)
    {
        mtx[i].resize (ydim);
        for (int j = 0; j < ydim; ++j)
            mtx[i][j].resize (zdim);
    }
    
    return mtx;
}

//---------------------------------------------------------------------------

NoximGlobalStats::NoximGlobalStats(const NoximNoC* _noc)
{
  noc = _noc;
}

//---------------------------------------------------------------------------

double NoximGlobalStats::getAverageDelay()
{
    unsigned int total_packets = 0;
    double avg_delay = 0.0;
    
    for (int z=0; z<NoximGlobalParams::mesh_dim_z; z++)
        for (int y=0; y<NoximGlobalParams::mesh_dim_y; y++)
            for (int x=0; x<NoximGlobalParams::mesh_dim_x; x++)
            {
                unsigned int received_packets = noc->t[x][y][z]->r->stats.getReceivedPackets();
                
                if (received_packets)
                {
                    avg_delay +=
                    received_packets * noc->t[x][y][z]->r->stats.getAverageDelay();
                    total_packets += received_packets;
                }
            }
    
    avg_delay /= (double) total_packets;
    
    return avg_delay;
}

//---------------------------------------------------------------------------

double NoximGlobalStats::getAverageDelay(const int src_id, const int dst_id)
{
  NoximTile* tile = noc->searchNode(dst_id);
  
  assert(tile != NULL);

  return tile->r->stats.getAverageDelay(src_id);
}

//---------------------------------------------------------------------------

double NoximGlobalStats::getMaxDelay()
{
    double maxd = -1.0;
    int node_id;
    double d;
    for (int z=0; z<NoximGlobalParams::mesh_dim_z; z++)
        for (int y=0; y<NoximGlobalParams::mesh_dim_y; y++)
            for (int x=0; x<NoximGlobalParams::mesh_dim_x; x++)
            {
                NoximCoord coord;
                coord.x = x;
                coord.y = y;
                coord.z = z;
                node_id = coord2Id(coord);
                d       = getMaxDelay(node_id);
                if (d > maxd)
                    maxd = d;
            }
    return maxd;
}

//---------------------------------------------------------------------------

double NoximGlobalStats::getMaxDelay(const int node_id)
{
    NoximCoord coord = id2Coord(node_id);
    
    unsigned int received_packets =
    noc->t[coord.x][coord.y][coord.z]->r->stats.getReceivedPackets();
    
    if (received_packets)
        return noc->t[coord.x][coord.y][coord.z]->r->stats.getMaxDelay();
    else
        return -1.0;
}

//---------------------------------------------------------------------------

double NoximGlobalStats::getMaxDelay(const int src_id, const int dst_id)
{
  NoximTile* tile = noc->searchNode(dst_id);
  
  assert(tile != NULL);

  return tile->r->stats.getMaxDelay(src_id);
}

//---------------------------------------------------------------------------

NoximGlobalStats::Matrix<double> NoximGlobalStats::getMaxDelayMtx ()
{
    Matrix<double> mtx = mkMatrixGlobalDim<double> ();
    
    for (int x = 0; x < NoximGlobalParams::mesh_dim_x; x++)
        for (int y = 0; y < NoximGlobalParams::mesh_dim_y; y++)
            for (int z = 0; z < NoximGlobalParams::mesh_dim_z; ++z)
            {
                int id = coord2Id(x, y, z);
                mtx[x][y][z] = getMaxDelay(id);
            }
    
    return mtx;
}

//---------------------------------------------------------------------------

double NoximGlobalStats::getAverageThroughput(const int src_id, const int dst_id)
{
  NoximTile* tile = noc->searchNode(dst_id);
  
  assert(tile != NULL);

  return tile->r->stats.getAverageThroughput(src_id);
}

//---------------------------------------------------------------------------

vector<unsigned int> NoximGlobalStats::getReceivedFlitsForChannel()
{
	vector<unsigned int> r;
	for (int i=0; i<NoximGlobalParams::number_virtual_channel; i++)
		r.push_back(0);
    
    for (int z=0; z<NoximGlobalParams::mesh_dim_z; z++)
        for (int y=0; y<NoximGlobalParams::mesh_dim_y; y++)
            for (int x=0; x<NoximGlobalParams::mesh_dim_x; x++)
                for (int i=0; i<NoximGlobalParams::number_virtual_channel; i++)
                    r[i] += noc->t[x][y][z]->r->stats.total_received_flits_for_channel[i];

	return r;
}

//---------------------------------------------------------------------------

double NoximGlobalStats::getAverageThroughput()
{
    unsigned int total_comms = 0;
    double avg_throughput = 0.0;
    
    for (int x = 0; x < NoximGlobalParams::mesh_dim_x; x++)
        for (int y = 0; y < NoximGlobalParams::mesh_dim_y; y++)
            for (int z = 0; z < NoximGlobalParams::mesh_dim_z; ++z)
            {
                unsigned int ncomms =
                noc->t[x][y][z]->r->stats.getTotalCommunications();
                
                if (ncomms) {
                    avg_throughput +=
                    ncomms * noc->t[x][y][z]->r->stats.getAverageThroughput();
                    total_comms += ncomms;
                }
            }
    
    avg_throughput /= (double) total_comms;
    
    return avg_throughput;
}

//---------------------------------------------------------------------------

unsigned int NoximGlobalStats::getReceivedPackets()
{
  unsigned int n = 0;

    for (int x = 0; x < NoximGlobalParams::mesh_dim_x; x++)
        for (int y = 0; y < NoximGlobalParams::mesh_dim_y; y++)
            for (int z = 0; z < NoximGlobalParams::mesh_dim_z; z++)
                n += noc->t[x][y][z]->r->stats.getReceivedPackets();

  return n;
}

//---------------------------------------------------------------------------

unsigned int NoximGlobalStats::getReceivedFlits()
{
  unsigned int n = 0;

    for (int z = 0; z < NoximGlobalParams::mesh_dim_z; z++)
        for (int y=0; y<NoximGlobalParams::mesh_dim_y; y++)
            for (int x=0; x<NoximGlobalParams::mesh_dim_x; x++)
            {
                n += noc->t[x][y][z]->r->stats.getReceivedFlits();
            }

  return n;
}

//---------------------------------------------------------------------------

double NoximGlobalStats::getThroughput()
{
    int total_cycles =
    NoximGlobalParams::simulation_time -
    NoximGlobalParams::stats_warm_up_time;
    
    //  int number_of_ip = NoximGlobalParams::mesh_dim_x * NoximGlobalParams::mesh_dim_y;
    //  return (double)getReceivedFlits()/(double)(total_cycles * number_of_ip);
    
    unsigned int n = 0;
    unsigned int trf = 0;
    for (int x = 0; x < NoximGlobalParams::mesh_dim_x; x++)
        for (int y = 0; y < NoximGlobalParams::mesh_dim_y; y++)
            for (int z = 0; z < NoximGlobalParams::mesh_dim_z; ++z)
            {
                unsigned int rf = noc->t[x][y][z]->r->stats.getReceivedFlits();
                
                if (rf != 0)
                    n++;
                
                trf += rf;
            }
    return (double) trf / (double) (total_cycles * n);
    
}

//---------------------------------------------------------------------------

NoximGlobalStats::Matrix<unsigned long> NoximGlobalStats::getRoutedFlitsMtx()
{
    Matrix  <unsigned long> mtx = mkMatrixGlobalDim<unsigned long> ();
    
    for (int x = 0; x < NoximGlobalParams::mesh_dim_x; x++)
        for (int y = 0; y < NoximGlobalParams::mesh_dim_y; y++)
            for (int z = 0; z < NoximGlobalParams::mesh_dim_z; z++)
                mtx[x][y][z] = noc->t[x][y][z]->r->getRoutedFlits();
    
    return mtx;
}

//---------------------------------------------------------------------------

double NoximGlobalStats::getPower()
{
    double power = 0.0;
    
    for (int x = 0; x < NoximGlobalParams::mesh_dim_x; x++)
        for (int y = 0; y < NoximGlobalParams::mesh_dim_y; y++)
            for (int z = 0; z < NoximGlobalParams::mesh_dim_z; z++)
                power += noc->t[x][y][z]->r->getPower();
    
    return power;
}

//---------------------------------------------------------------------------

void NoximGlobalStats::showStats(std::ostream& out, bool detailed)
{
  out << "% Total received packets: " << getReceivedPackets() << endl;
  out << "% Total received flits: " << getReceivedFlits() << endl;
  vector<unsigned int> r = getReceivedFlitsForChannel();
  if (NoximGlobalParams::number_virtual_channel > 1)
  {
	for (unsigned int i=0; i<NoximGlobalParams::number_virtual_channel; i++)
	{
		out << "% Total received flits from channel " << i << ": " << r[i] << endl;
	}
  }
  out << "% Global average delay (cycles): " << getAverageDelay() << endl;
  out << "% Global average throughput (flits/cycle): " << getAverageThroughput() << endl;
  out << "% Throughput (flits/cycle/IP): " << getThroughput() << endl;
  out << "% Max delay (cycles): " << getMaxDelay() << endl;
  out << "% Total energy (J): " << getPower() << endl;

  if (detailed)
    {
      out << endl << "detailed = [" << endl;
        for(int z=0; z<NoximGlobalParams::mesh_dim_z; z++)
            for(int  y=0; y<NoximGlobalParams::mesh_dim_y; y++)
                for(int  x=0; x<NoximGlobalParams::mesh_dim_x; x++)
                    noc->t[x][y][z]->r->stats.showStats(coord2Id (x, y, z), out, true);
      out << "];" << endl;

      // show MaxDelay matrix
      // Can be corrected later based on AccessNoximHotspot
        {
            Matrix<double> mtx = getMaxDelayMtx();
            
            out << endl << "max_delay = [" << endl;
            for (unsigned int x = 0; x < NoximGlobalParams::mesh_dim_x; x++) {
                for (unsigned int y = 0; y < NoximGlobalParams::mesh_dim_y; y++) {
                    out << "[";
                    out << "   ";
                    for (unsigned int z = 0; z < NoximGlobalParams::mesh_dim_z; ++z) {
                        out << setw(6) << mtx[x][y][z];
                    }
                    out << "]";
                }
                out << endl;
            }
            out << "];" << endl;
        }
        
      // show RoutedFlits matrix
      // Can be corrected later based on AccessNoximHotspot
        {
            Matrix <unsigned long> mtx = getRoutedFlitsMtx();
            
            out << endl << "routed_flits = [" << endl;
            for (unsigned int x = 0; x < NoximGlobalParams::mesh_dim_x; x++) {
                for (unsigned int y = 0; y < NoximGlobalParams::mesh_dim_y; y++) {
                    out << "[";
                    out << "   ";
                    for (unsigned int z = 0; z < NoximGlobalParams::mesh_dim_z; ++z) {
                        out << setw(6) << mtx[x][y][z];
                    }
                    out << "]";
                }
                out << endl;
            }
        }
        out << "];" << endl;
    }
}

//---------------------------------------------------------------------------