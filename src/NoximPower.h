/*****************************************************************************

  NoximPower.h -- Power model

 *****************************************************************************/

#ifndef __NoximPower_H__
#define __NoximPower_H__

#include <map>

using namespace std;

// ---------------------------------------------------------------------------
/*
We assumed the tile size to be 2mm x 2mm and that the tiles were
arranged in a regular fashion on the floorplan. The load wire
capacitance was set to 0.50fF per micron, so considering an average of
25% switching activity the amount of energy consumed by a flit for a
hop interconnect is 0.384nJ.
*/

#define PWR_ROUTING_XYZ            0.031e-12 // needs correction for 3D Mesh topology
#define PWR_ROUTING_WEST_FIRST     0.045e-12 // needs correction for 3D Mesh topology
#define PWR_ROUTING_NORTH_LAST     0.050e-12 // needs correction for 3D Mesh topology
#define PWR_ROUTING_LBDR           0.050e-12 // needs correction for 3D Mesh topology (Based on LBDR3D and using West-First in 3D)
#define PWR_ROUTING_DQP_NETZ       0.050e-12 // needs correction for 3D Mesh topology (Based on LBDR3D and using West-First in 3D)
#define PWR_ROUTING_ETW            0.050e-12 // needs correction for 3D Mesh topology (Based on LBDR3D and using West-First in 3D)
#define PWR_ROUTING_NEGATIVE_FIRST 0.050e-12 // needs correction for 3D Mesh topology
#define PWR_ROUTING_ODD_EVEN       0.136e-12 // needs correction for 3D Mesh topology
#define PWR_ROUTING_DYAD           0.256e-12
#define PWR_ROUTING_FULLY_ADAPTIVE 0.054e-12
#define PWR_ROUTING_TABLE_BASED    1.000e-12
#define PWR_SEL_RANDOM             0.009e-12
#define PWR_SEL_BUFFER_LEVEL       0.010e-12
#define PWR_SEL_NOP                0.020e-12
#define PWR_SEL_PATHAWARE_MINIMAL  0.010e-12 // needs correction for 2D and 3D Mesh topology

#define PWR_ROUTING_XY_TORUS        0.072e-12
#define PWR_ROUTING_XY_FOLDED_TORUS 0.052e-12

#define PWR_FORWARD_FLIT_4_32      2.766e-12
#define PWR_INCOMING_4_32          2.766e-12
#define PWR_FORWARD_FLIT_4_38      3.137e-12
#define PWR_INCOMING_4_38          3.137e-12
#define PWR_FORWARD_FLIT_4_65      4.824e-12
#define PWR_INCOMING_4_65          4.824e-12
#define PWR_FORWARD_FLIT_4_77      5.767e-12
#define PWR_INCOMING_4_77          5.767e-12
#define PWR_FORWARD_FLIT_4_78      5.715e-12
#define PWR_INCOMING_4_78          5.715e-12

#define PWR_FORWARD_FLIT_8_32      4.227e-12
#define PWR_INCOMING_8_32          4.227e-12
#define PWR_FORWARD_FLIT_8_38      4.884e-12
#define PWR_INCOMING_8_38          4.884e-12
#define PWR_FORWARD_FLIT_8_65      7.878e-12
#define PWR_INCOMING_8_65          7.878e-12
#define PWR_FORWARD_FLIT_8_77      9.581e-12
#define PWR_INCOMING_8_77          9.581e-12
#define PWR_FORWARD_FLIT_8_78      9.518e-12
#define PWR_INCOMING_8_78          9.518e-12

#define PWR_FORWARD_FLIT_2_32      1.950e-12
#define PWR_INCOMING_2_32          1.950e-12
#define PWR_FORWARD_FLIT_2_38      2.152e-12
#define PWR_INCOMING_2_38          2.152e-12
#define PWR_FORWARD_FLIT_2_65      3.241e-12
#define PWR_INCOMING_2_65          3.241e-12
#define PWR_FORWARD_FLIT_2_77      3.749e-12
#define PWR_INCOMING_2_77          3.749e-12
#define PWR_FORWARD_FLIT_2_78      3.813e-12
#define PWR_INCOMING_2_78          3.813e-12

#define PWR_XBAR_32                1.520e-12
#define PWR_XBAR_38                1.800e-12
#define PWR_XBAR_65                3.170e-12
#define PWR_XBAR_77                3.761e-12
#define PWR_XBAR_78                3.822e-12

#define PWR_XBAR_32_2VC            2.120e-12
#define PWR_XBAR_38_2VC            2.495e-12
#define PWR_XBAR_65_2VC            4.253e-12
#define PWR_XBAR_77_2VC            5.491e-12
#define PWR_XBAR_78_2VC            5.545e-12

#define PWR_XBAR_32_4VC            3.815e-12
#define PWR_XBAR_38_4VC            4.896e-12
#define PWR_XBAR_65_4VC            8.694e-12
#define PWR_XBAR_77_4VC            11.747e-12
#define PWR_XBAR_78_4VC            11.862e-12

#define PWR_ARBITER                0.441e-12
#define PWR_ARBITER_2VC            3.770e-12
#define PWR_ARBITER_4VC            3.770e-12

// ---------------------------------------------------------------------------

class NoximPower
{

 public:

  NoximPower();

  void Routing();
  void Selection();
  void Arbiter();
  void Forward();
  void Incoming();
  void Crossbar();

  double getPower() { return pwr; }

  double getPwrRouting() { return pwr_routing; }
  double getPwrSelection() { return pwr_selection; }
  double getPwrForward() { return pwr_forward; }
  double getPwrArbiter() { return pwr_arbiter; }
  double getPwrIncoming() { return pwr_incoming; }
  double getPwrCrossbar() { return pwr_crossbar; }

 private:
  void initPowerTable();

  double pwr_routing;
  double pwr_selection;
  double pwr_forward;
  double pwr_arbiter;
  double pwr_incoming;
  double pwr_crossbar;

  double pwr;

  map<pair<unsigned int, unsigned int>, double > power_table_forward;
  map<pair<unsigned int, unsigned int>, double > power_table_incoming;
  map<pair<unsigned int, unsigned int>, double > power_table_xbar;
  map<unsigned int, double >                     power_table_arbiter;
};

// ---------------------------------------------------------------------------

#endif