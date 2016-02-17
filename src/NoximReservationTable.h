/*****************************************************************************

  NoximReservationTable.h -- Switch reservation table definition

 *****************************************************************************/

#ifndef __NoximReservationTable_H__
#define __NoximReservationTable_H__

//---------------------------------------------------------------------------

#include <cassert>

using namespace std;

//---------------------------------------------------------------------------

class NoximReservationTable
{
 public:

  NoximReservationTable();

  // Clear reservation table
  void clear();

  // check if port_out is reservable
  bool isAvailable(const int port_out, const int channel) const;

  // Connects port_in with port_out. Asserts if port_out is reserved
  void reserve(const int port_in, const int port_out, const int channel);

  // Releases port_out connection. 
  // Asserts if port_out is not reserved or not valid
  void release(const int port_out, const int channel);

  // Returns the output port connected to port_in.
  int getOutputPort(const int port_in, const int channel) const;

  // Makes output port no longer available for reservation/release
  void invalidate(const int port_out, const int channel);

private:
  
	vector<int> rtable; // reservation vector: rtable[i] gives the input
		      // port whose output port 'i' is connected to

};

//---------------------------------------------------------------------------

#endif