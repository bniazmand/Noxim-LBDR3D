#ifndef __NoximLocalRoutingTable_H__
#define __NoximLocalRoutingTable_H__

//---------------------------------------------------------------------------

#include "NoximGlobalRoutingTable.h"

//---------------------------------------------------------------------------

class NoximLocalRoutingTable
{

public:

  NoximLocalRoutingTable() {};

  // Extracts the routing table of node _node_id from the global
  // routing table rtable
  void configure(NoximGlobalRoutingTable& rtable, const int _node_id);

  // Returns the set of admissible output channels for destination
  // destination_id and input channel in_link
  NoximAdmissibleOutputs getAdmissibleOutputs(const NoximLinkId& in_link, const int destination_id);

  // Returns the set of admissible output channels for a destination
  // destination_id and a given input direction
  NoximAdmissibleOutputs getAdmissibleOutputs(const int in_direction, const int destination_id);


private:

  NoximRoutingTableNode rt_node;
  int                   node_id;
};

//---------------------------------------------------------------------------

#endif
