#ifndef __NoximGlobalRoutingTable_H__
#define __NoximGlobalRoutingTable_H__

//---------------------------------------------------------------------------

#include <vector>
#include <map>
#include <set>

using namespace std;

//---------------------------------------------------------------------------

typedef pair<int,int> NoximLinkId; // source, destination node

// Routing table
typedef set<NoximLinkId> NoximAdmissibleOutputs;

// Map a destination to a set of admissible outputs
typedef map<int,NoximAdmissibleOutputs> NoximRoutingTableLink; 

// Map an input link to its routing table
typedef map<NoximLinkId,NoximRoutingTableLink> NoximRoutingTableNode;

// Map a node of the network to its routing table
typedef map<int,NoximRoutingTableNode> NoximRoutingTableNoC;

//---------------------------------------------------------------------------

// Converts an input direction to a link 
NoximLinkId direction2ILinkId(const int node_id, const int dir);

// Converts an input direction to a link
int oLinkId2Direction(const NoximLinkId& out_link);

// Converts a set of output links to a set of directions
vector<int> admissibleOutputsSet2Vector(const NoximAdmissibleOutputs& ao);

//---------------------------------------------------------------------------

class NoximGlobalRoutingTable
{

public:

  NoximGlobalRoutingTable(); 

  // Load routing table from file. Returns true if ok, false otherwise
  bool load(const char* fname);

  NoximRoutingTableNode getNodeRoutingTable(const int node_id);

  bool isValid() { return valid; }


private:

  NoximRoutingTableNoC rt_noc;  
  bool                 valid;

};

//---------------------------------------------------------------------------

#endif
