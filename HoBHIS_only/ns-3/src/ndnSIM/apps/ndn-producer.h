/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Ilya Moiseenko <iliamo@cs.ucla.edu>
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef NDN_PRODUCER_H
#define NDN_PRODUCER_H

#include "ndn-app.h"

#include "ns3/ptr.h"
#include "ns3/random-variable.h"
#include "ns3/ndn-name-components.h"
#include "ns3/ndn-content-object.h"

namespace ns3 {
namespace ndn {

/**
 * @brief A simple Interest-sink applia simple Interest-sink application
 *
 * A simple Interest-sink applia simple Interest-sink application,
 * which replying every incoming Interest with Data packet with a specified
 * size and name same as in Interest.cation, which replying every incoming Interest
 * with Data packet with a specified size and name same as in Interest.
 */
class Producer : public App
{
public: 
  static TypeId
  GetTypeId (void);
        
  Producer ();

  // inherited from NdnApp
  void OnInterest (const Ptr<const InterestHeader> &interest, Ptr<Packet> packet);

protected:
  // inherited from Application base class.
  virtual void
  StartApplication ();    // Called at time specified by Start

  virtual void
  StopApplication ();     // Called at time specified by Stop

private:
  NameComponents m_prefix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;

  // Random payload size
  uint32_t m_virtualRandPayloadSizeMin;
  uint32_t m_virtualRandPayloadSizeMax;
  
  //Random delay to generate heterogeneous RTT
  double m_virtualDelayMin;
  double m_virtualDelayMax;

  UniformVariable m_rand; ///< @brief random payload size
  UniformVariable m_rand_rtt;
  double mean_rtt;
  double sum_rtt;
  int num_rtt;
  double predict;
  double mov_av;
  bool first_Interest;

};

} // namespace ndn
} // namespace ns3

#endif // NDN_PRODUCER_H
