/*****************************************************************************

  NoximPower.cpp -- Power model

 *****************************************************************************/

#include <cassert>
#include "NoximMain.h"
#include "NoximPower.h"

using namespace std;

// ---------------------------------------------------------------------------

NoximPower::NoximPower()
{
  pwr = 0.0;

  initPowerTable();

  switch(NoximGlobalParams::error_coding)
  {
		case ECODING_NONE:
			pwr_forward  = power_table_forward[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, NoximGlobalParams::flit_size_bits)];
            pwr_incoming = power_table_incoming[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, NoximGlobalParams::flit_size_bits)];
			pwr_crossbar = power_table_xbar[pair<unsigned int, unsigned int>(NoximGlobalParams::number_virtual_channel, NoximGlobalParams::flit_size_bits)];
			break;

		case ECODING_CADEC:
		case ECODING_JTEC:
			pwr_forward  = power_table_forward[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, JTEC_PAYLOAD_LENGTH)];
            pwr_incoming = power_table_incoming[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, JTEC_PAYLOAD_LENGTH)];
			pwr_crossbar = power_table_xbar[pair<unsigned int, unsigned int>(NoximGlobalParams::number_virtual_channel, JTEC_PAYLOAD_LENGTH)];
			break;

		case ECODING_DAP:
			pwr_forward  = power_table_forward[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, DAP_PAYLOAD_LENGTH)];
            pwr_incoming = power_table_incoming[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, DAP_PAYLOAD_LENGTH)];
			pwr_crossbar = power_table_xbar[pair<unsigned int, unsigned int>(NoximGlobalParams::number_virtual_channel, DAP_PAYLOAD_LENGTH)];
			break;

		case ECODING_ED:
		case ECODING_SEC:
			pwr_forward  = power_table_forward[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, ED_SEC_PAYLOAD_LENGTH)];
            pwr_incoming = power_table_incoming[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, ED_SEC_PAYLOAD_LENGTH)];
			pwr_crossbar = power_table_xbar[pair<unsigned int, unsigned int>(NoximGlobalParams::number_virtual_channel, ED_SEC_PAYLOAD_LENGTH)];
			break;

		case ECODING_BCH:
			pwr_forward  = power_table_forward[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, BCH_PAYLOAD_LENGTH)];
            pwr_incoming = power_table_incoming[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, BCH_PAYLOAD_LENGTH)];
			pwr_crossbar = power_table_xbar[pair<unsigned int, unsigned int>(NoximGlobalParams::number_virtual_channel, BCH_PAYLOAD_LENGTH)];
			break;

		case ECODING_JTEC_SQED:
			pwr_forward  = power_table_forward[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, JTEC_SQED_PAYLOAD_LENGTH)];
            pwr_incoming = power_table_incoming[pair<unsigned int, unsigned int>(NoximGlobalParams::buffer_depth, JTEC_SQED_PAYLOAD_LENGTH)];
			pwr_crossbar = power_table_xbar[pair<unsigned int, unsigned int>(NoximGlobalParams::number_virtual_channel, JTEC_SQED_PAYLOAD_LENGTH)];
  }

  if (NoximGlobalParams::routing_algorithm == ROUTING_XYZ && NoximGlobalParams::topology == NET_MESH) pwr_routing = PWR_ROUTING_XYZ;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_LBDR && NoximGlobalParams::topology == NET_MESH) pwr_routing = PWR_ROUTING_LBDR;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_DQP_NETZ && NoximGlobalParams::topology == NET_MESH) pwr_routing = PWR_ROUTING_DQP_NETZ;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_ETW && NoximGlobalParams::topology == NET_MESH) pwr_routing = PWR_ROUTING_ETW;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_XYZ && NoximGlobalParams::topology == NET_TORUS) pwr_routing = PWR_ROUTING_XY_TORUS;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_XYZ && NoximGlobalParams::topology == NET_FOLDED_TORUS) pwr_routing = PWR_ROUTING_XY_FOLDED_TORUS;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_WEST_FIRST) pwr_routing = PWR_ROUTING_WEST_FIRST;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_NORTH_LAST) pwr_routing = PWR_ROUTING_NORTH_LAST;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_NEGATIVE_FIRST) pwr_routing = PWR_ROUTING_NEGATIVE_FIRST;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_ODD_EVEN) pwr_routing = PWR_ROUTING_ODD_EVEN;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_DYAD) pwr_routing = PWR_ROUTING_DYAD;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_FULLY_ADAPTIVE) pwr_routing = PWR_ROUTING_FULLY_ADAPTIVE;
  else if (NoximGlobalParams::routing_algorithm == ROUTING_TABLE_BASED) pwr_routing = PWR_ROUTING_TABLE_BASED;
  else assert(false);

  if (NoximGlobalParams::selection_strategy == SEL_RANDOM) pwr_selection = PWR_SEL_RANDOM;
  else if (NoximGlobalParams::selection_strategy == SEL_BUFFER_LEVEL) pwr_selection = PWR_SEL_BUFFER_LEVEL;
  else if (NoximGlobalParams::selection_strategy == SEL_NOP) pwr_selection = PWR_SEL_NOP;
  else if (NoximGlobalParams::selection_strategy == SEL_PATHAWARE_MINIMAL) pwr_selection = PWR_SEL_PATHAWARE_MINIMAL;
  else assert(false);

  if (NoximGlobalParams::routing_algorithm == ROUTING_XYZ) pwr_selection = 0.0;

  pwr_arbiter = power_table_arbiter[NoximGlobalParams::number_virtual_channel];
}

// ---------------------------------------------------------------------------

void NoximPower::initPowerTable()
{
  power_table_forward[pair<unsigned int, unsigned int>(4,32)] = PWR_FORWARD_FLIT_4_32;
  power_table_forward[pair<unsigned int, unsigned int>(4,38)] = PWR_FORWARD_FLIT_4_38;
  power_table_forward[pair<unsigned int, unsigned int>(4,65)] = PWR_FORWARD_FLIT_4_65;
  power_table_forward[pair<unsigned int, unsigned int>(4,77)] = PWR_FORWARD_FLIT_4_77;
  power_table_forward[pair<unsigned int, unsigned int>(4,78)] = PWR_FORWARD_FLIT_4_78;

  power_table_incoming[pair<unsigned int, unsigned int>(4,32)] = PWR_INCOMING_4_32;
  power_table_incoming[pair<unsigned int, unsigned int>(4,38)] = PWR_INCOMING_4_38;
  power_table_incoming[pair<unsigned int, unsigned int>(4,65)] = PWR_INCOMING_4_65;
  power_table_incoming[pair<unsigned int, unsigned int>(4,77)] = PWR_INCOMING_4_77;
  power_table_incoming[pair<unsigned int, unsigned int>(4,78)] = PWR_INCOMING_4_78;

  power_table_forward[pair<unsigned int, unsigned int>(8,32)] = PWR_FORWARD_FLIT_8_32;
  power_table_forward[pair<unsigned int, unsigned int>(8,38)] = PWR_FORWARD_FLIT_8_38;
  power_table_forward[pair<unsigned int, unsigned int>(8,65)] = PWR_FORWARD_FLIT_8_65;
  power_table_forward[pair<unsigned int, unsigned int>(8,77)] = PWR_FORWARD_FLIT_8_77;
  power_table_forward[pair<unsigned int, unsigned int>(8,78)] = PWR_FORWARD_FLIT_8_78;

  power_table_incoming[pair<unsigned int, unsigned int>(8,32)] = PWR_INCOMING_8_32;
  power_table_incoming[pair<unsigned int, unsigned int>(8,38)] = PWR_INCOMING_8_38;
  power_table_incoming[pair<unsigned int, unsigned int>(8,65)] = PWR_INCOMING_8_65;
  power_table_incoming[pair<unsigned int, unsigned int>(8,77)] = PWR_INCOMING_8_77;
  power_table_incoming[pair<unsigned int, unsigned int>(8,78)] = PWR_INCOMING_8_78;

  power_table_forward[pair<unsigned int, unsigned int>(2,32)] = PWR_FORWARD_FLIT_2_32;
  power_table_forward[pair<unsigned int, unsigned int>(2,38)] = PWR_FORWARD_FLIT_2_38;
  power_table_forward[pair<unsigned int, unsigned int>(2,65)] = PWR_FORWARD_FLIT_2_65;
  power_table_forward[pair<unsigned int, unsigned int>(2,77)] = PWR_FORWARD_FLIT_2_77;
  power_table_forward[pair<unsigned int, unsigned int>(2,78)] = PWR_FORWARD_FLIT_2_78;

  power_table_incoming[pair<unsigned int, unsigned int>(2,32)] = PWR_INCOMING_2_32;
  power_table_incoming[pair<unsigned int, unsigned int>(2,38)] = PWR_INCOMING_2_38;
  power_table_incoming[pair<unsigned int, unsigned int>(2,65)] = PWR_INCOMING_2_65;
  power_table_incoming[pair<unsigned int, unsigned int>(2,77)] = PWR_INCOMING_2_77;
  power_table_incoming[pair<unsigned int, unsigned int>(2,78)] = PWR_INCOMING_2_78;

  power_table_xbar[pair<unsigned int, unsigned int>(1,32)] = PWR_XBAR_32;
  power_table_xbar[pair<unsigned int, unsigned int>(1,38)] = PWR_XBAR_38;
  power_table_xbar[pair<unsigned int, unsigned int>(1,65)] = PWR_XBAR_65;
  power_table_xbar[pair<unsigned int, unsigned int>(1,77)] = PWR_XBAR_77;
  power_table_xbar[pair<unsigned int, unsigned int>(1,78)] = PWR_XBAR_78;

  power_table_xbar[pair<unsigned int, unsigned int>(2,32)] = PWR_XBAR_32_2VC;
  power_table_xbar[pair<unsigned int, unsigned int>(2,38)] = PWR_XBAR_38_2VC;
  power_table_xbar[pair<unsigned int, unsigned int>(2,65)] = PWR_XBAR_65_2VC;
  power_table_xbar[pair<unsigned int, unsigned int>(2,77)] = PWR_XBAR_77_2VC;
  power_table_xbar[pair<unsigned int, unsigned int>(2,78)] = PWR_XBAR_78_2VC;

  power_table_xbar[pair<unsigned int, unsigned int>(4,32)] = PWR_XBAR_32_4VC;
  power_table_xbar[pair<unsigned int, unsigned int>(4,38)] = PWR_XBAR_38_4VC;
  power_table_xbar[pair<unsigned int, unsigned int>(4,65)] = PWR_XBAR_65_4VC;
  power_table_xbar[pair<unsigned int, unsigned int>(4,77)] = PWR_XBAR_77_4VC;
  power_table_xbar[pair<unsigned int, unsigned int>(4,78)] = PWR_XBAR_78_4VC;

  power_table_arbiter[1] = PWR_ARBITER;
  power_table_arbiter[4] = PWR_ARBITER_4VC;
}

// ---------------------------------------------------------------------------

void NoximPower::Routing()
{
  pwr += pwr_routing;
}

// ---------------------------------------------------------------------------

void NoximPower::Selection()
{
  pwr += pwr_selection;
}

// ---------------------------------------------------------------------------

void NoximPower::Arbiter()
{
  pwr += pwr_arbiter;
}

// ---------------------------------------------------------------------------

void NoximPower::Forward()
{
  pwr += pwr_forward;
}

// ---------------------------------------------------------------------------

void NoximPower::Incoming()
{
  pwr += pwr_incoming;
}

// ---------------------------------------------------------------------------

void NoximPower::Crossbar()
{
  pwr += pwr_crossbar;
}

// ---------------------------------------------------------------------------