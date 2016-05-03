/*****************************************************************************

  NoximReservationTable.cpp -- NoximReservationTable implementation

 *****************************************************************************/

#include "NoximMain.h"
#include "NoximReservationTable.h"

//---------------------------------------------------------------------------

NoximReservationTable::NoximReservationTable()
{
  clear();
}

//---------------------------------------------------------------------------

void NoximReservationTable::clear()
{
  // note that NOT_VALID entries should remain untouched

	rtable.resize(DIRECTIONS*NoximGlobalParams::number_virtual_channel+1);

	// note that NOT_VALID entries should remain untouched
	for (int i=0; i<DIRECTIONS*NoximGlobalParams::number_virtual_channel+1; i++)
		if (rtable[i] != NOT_VALID) rtable[i] = NOT_RESERVED;
}

//---------------------------------------------------------------------------

bool NoximReservationTable::isAvailable(const int port_out, const int channel) const
{
  assert(port_out >= 0 && port_out < DIRECTIONS+1);

  if (port_out == DIRECTION_LOCAL)
	return ((rtable[port_out*NoximGlobalParams::number_virtual_channel] == NOT_RESERVED));

  return ((rtable[port_out*NoximGlobalParams::number_virtual_channel+channel] == NOT_RESERVED));
}

//---------------------------------------------------------------------------

void NoximReservationTable::reserve(const int port_in, const int port_out, const int channel)
{
    // reservation of reserved/not valid ports is illegal. Correctness
    // should be assured by NoximReservationTable users
	assert(isAvailable(port_out, channel));


    // check for previous reservation to be released
  int port = getOutputPort(port_in, channel);

  if (port!=NOT_RESERVED)
    release(port, channel);

  if (port_out == DIRECTION_LOCAL)
  {
	  if (port_in == DIRECTION_LOCAL)
		  rtable[port_out*NoximGlobalParams::number_virtual_channel] = port_in*NoximGlobalParams::number_virtual_channel;
	  else
		  rtable[port_out*NoximGlobalParams::number_virtual_channel] = port_in*NoximGlobalParams::number_virtual_channel+channel;
  }
  else
  {
	  if (port_in == DIRECTION_LOCAL)
		  rtable[port_out*NoximGlobalParams::number_virtual_channel+channel] = port_in*NoximGlobalParams::number_virtual_channel;
	  else
		  rtable[port_out*NoximGlobalParams::number_virtual_channel+channel] = port_in*NoximGlobalParams::number_virtual_channel+channel;
  }
}

//---------------------------------------------------------------------------

void NoximReservationTable::release(const int port_out, const int channel)
{
  assert(port_out >= 0 && port_out < DIRECTIONS+1);
    // there is a valid reservation on port_out
  if (port_out == DIRECTION_LOCAL)
	  assert(rtable[port_out*NoximGlobalParams::number_virtual_channel] >= 0 && rtable[port_out*NoximGlobalParams::number_virtual_channel] < DIRECTIONS*NoximGlobalParams::number_virtual_channel+1);
  else
	  assert(rtable[port_out*NoximGlobalParams::number_virtual_channel+channel] >= 0 && rtable[port_out*NoximGlobalParams::number_virtual_channel+channel] < DIRECTIONS*NoximGlobalParams::number_virtual_channel+1);

  if (port_out == DIRECTION_LOCAL)
	rtable[port_out*NoximGlobalParams::number_virtual_channel] = NOT_RESERVED;
  else
	rtable[port_out*NoximGlobalParams::number_virtual_channel+channel] = NOT_RESERVED;
}

//---------------------------------------------------------------------------

int NoximReservationTable::getOutputPort(const int port_in, const int channel) const
{
  assert(port_in >= 0 && port_in < DIRECTIONS+1);

  for (int i=0; i<DIRECTIONS+1; i++)
	if (i == DIRECTION_LOCAL)
	{
		if (port_in == DIRECTION_LOCAL)
		{
			if (rtable[i*NoximGlobalParams::number_virtual_channel] == port_in*NoximGlobalParams::number_virtual_channel)
				return i; // port_in reserved outport i
		}
		else
		{
			if (rtable[i*NoximGlobalParams::number_virtual_channel] == port_in*NoximGlobalParams::number_virtual_channel+channel)
				return i; // port_in reserved outport i
		}
	}
	else
	{
		if (port_in == DIRECTION_LOCAL)
		{
			if (rtable[i*NoximGlobalParams::number_virtual_channel+channel] == port_in*NoximGlobalParams::number_virtual_channel)
				return i; // port_in reserved outport i
		}
		else
		{
			if (rtable[i*NoximGlobalParams::number_virtual_channel+channel] == port_in*NoximGlobalParams::number_virtual_channel+channel)
				return i; // port_in reserved outport i
		}
	}

  // semantic: port_in currently doesn't reserve any out port
  return NOT_RESERVED;
}
//---------------------------------------------------------------------------

// makes port_out no longer available for reservation/release
void NoximReservationTable::invalidate(const int port_out, const int channel)
{
	if (port_out == DIRECTION_LOCAL)
		rtable[port_out*NoximGlobalParams::number_virtual_channel] = NOT_VALID;
	else
		rtable[port_out*NoximGlobalParams::number_virtual_channel+channel] = NOT_VALID;
}