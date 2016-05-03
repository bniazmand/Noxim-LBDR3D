#include "NoximLocalRoutingTable.h"
#include "NoximMain.h"

//---------------------------------------------------------------------------

void NoximLocalRoutingTable::configure(NoximGlobalRoutingTable& rtable, const int _node_id)
{
  rt_node = rtable.getNodeRoutingTable(_node_id);
  node_id = _node_id;
}

//---------------------------------------------------------------------------

NoximAdmissibleOutputs NoximLocalRoutingTable::getAdmissibleOutputs(const NoximLinkId& in_link, const int destination_id)
{
  return rt_node[in_link][destination_id];
}

//---------------------------------------------------------------------------

NoximAdmissibleOutputs NoximLocalRoutingTable::getAdmissibleOutputs(const int in_direction, const int destination_id)
{
  NoximLinkId lid = direction2ILinkId(node_id, in_direction);

  return getAdmissibleOutputs(lid, destination_id);
}

//---------------------------------------------------------------------------