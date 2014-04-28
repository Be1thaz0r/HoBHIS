/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013 University of California, Los Angeles
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef CCNX_APP_DELAY_TRACER_H
#define CCNX_APP_DELAY_TRACER_H

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include <ns3/nstime.h>
#include <ns3/event-id.h>

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <list>

namespace ns3 {

class Node;
class Packet;

namespace ndn {

class App;

/**
 * @ingroup ndn
 * @brief  network-layer tracer for aggregate packet counts
 */
class AppDelayTracer : public SimpleRefCount<AppDelayTracer>
{
public:
  /**
   * @brief Helper method to install tracers on all simulation nodes
   *
   * @param file File to which traces will be written
   * @param averagingPeriod How often data will be written into the trace file (default, every half second)
   *
   * @returns a tuple of reference to output stream and list of tracers. !!! Attention !!! This tuple needs to be preserved
   *          for the lifetime of simulation, otherwise SEGFAULTs are inevitable
   * 
   */
  static boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<AppDelayTracer> > >
  InstallAll (const std::string &file);

  /**
   * @brief Trace constructor that attaches to all applications on the node using node's pointer
   * @param os    reference to the output stream
   * @param node  pointer to the node
   */
  AppDelayTracer (boost::shared_ptr<std::ostream> os, Ptr<Node> node);

  /**
   * @brief Trace constructor that attaches to all applications on the node using node's name
   * @param os        reference to the output stream
   * @param nodeName  name of the node registered using Names::Add
   */
  AppDelayTracer (boost::shared_ptr<std::ostream> os, const std::string &node);

  /**
   * @brief Destructor
   */
  ~AppDelayTracer ();

  /**
   * @brief Print head of the trace (e.g., for post-processing)
   *
   * @param os reference to output stream
   */
  void
  PrintHeader (std::ostream &os) const;
  
private:
  void
  Connect ();

  void 
  LastRetransmittedInterestDataDelay (Ptr<App> app, uint32_t seqno, Time delay, int32_t hopCount);
  
  void 
  FirstInterestDataDelay (Ptr<App> app, uint32_t seqno, Time delay, uint32_t rextCount, int32_t hopCount);
  
private:
  std::string m_node;
  Ptr<Node> m_nodePtr;

  boost::shared_ptr<std::ostream> m_os;
};

} // namespace ndn
} // namespace ns3

#endif // CCNX_CS_TRACER_H
