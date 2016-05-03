/*****************************************************************************

  NoximGlobalStats.h -- Global Statistics definition

 *****************************************************************************/

#ifndef __NoximGlobalStats_H__
#define __NoximGlobalStats_H__

//---------------------------------------------------------------------------

#include <iostream>
#include <vector>
#include "NoximNoC.h"
#include "NoximTile.h"

//---------------------------------------------------------------------------

class NoximGlobalStats
{
public:
    
    template <typename T>
    struct Matrix : public vector<vector<vector<T> > > {};
    
    template <typename T>
    Matrix<T> mkMatrix (size_t xdim, size_t ydim, size_t zdim);
    
    template <typename T>
    Matrix<T> mkMatrixGlobalDim () {
        return mkMatrix<T> (NoximGlobalParams::mesh_dim_x,
                            NoximGlobalParams::mesh_dim_y,
                            NoximGlobalParams::mesh_dim_z);
    }
    
  NoximGlobalStats(const NoximNoC* _noc);
  
  // Returns the aggragated average delay (cycles)
  double getAverageDelay();
  
  // Returns the aggragated average delay (cycles) for communication
  // src_id->dst_id
  double getAverageDelay(const int src_id, const int dst_id);

  // Returns the max delay
  double getMaxDelay();

  // Returns the max delay (cycles) experimented by destination
  // node_id. Returns -1 if node_id is not destination of any
  // communication
  double getMaxDelay(const int node_id);

  // Returns the max delay (cycles) for communication src_id->dst_id
  double getMaxDelay(const int src_id, const int dst_id);
  
  // Returns tha matrix of max delay for any node of the network
  Matrix<double> getMaxDelayMtx();

  // Returns the aggragated average throughput (flits/cycles)
  double getAverageThroughput();

  // Returns the aggragated average throughput (flits/cycles) for
  // communication src_id->dst_id
  double getAverageThroughput(const int src_id, const int dst_id);

  // Returns the total number of received packets
  unsigned int getReceivedPackets();

  // Returns the total number of received flits
  unsigned int getReceivedFlits();

  // Returns the total number of received flits for each channel
  vector<unsigned int> getReceivedFlitsForChannel();

  // Returns the maximum value of the accepted traffic
  double getThroughput();

  // Returns the number of routed flits for each router
  Matrix <unsigned long> getRoutedFlitsMtx();

  // Returns the total power (routers+links)
  double getPower();

  // Returns the total power dissipated by the routers
  double getRoutersPower();

  // Shows global statistics
  void showStats(std::ostream& out = std::cout, bool detailed = false);

private:
  const NoximNoC* noc;
};

//---------------------------------------------------------------------------

#endif