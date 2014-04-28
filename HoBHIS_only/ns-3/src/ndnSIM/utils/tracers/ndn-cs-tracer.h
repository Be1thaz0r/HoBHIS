/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2011-2012 University of California, Los Angeles
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
 * Author: Xiaoyan Hu <x......u@gmail.com>
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef CCNX_CS_TRACER_H
#define CCNX_CS_TRACER_H

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include <ns3/nstime.h>
#include <ns3/event-id.h>

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <list>

namespace ns3 {

class Node;
class Packet;

namespace ndn {

class InterestHeader;
class ContentObjectHeader;

namespace cs {

struct Stats
{
  inline void Reset ()
  {
    m_cacheHits   = 0;
    m_cacheMisses = 0;
  }
  double m_cacheHits;
  double m_cacheMisses;
};

}  

/**
 * @ingroup ndn
 * @brief NDN tracer for cache performance (hits and misses)
 */
class CsTracer : public SimpleRefCount<CsTracer>
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
  static boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<CsTracer> > >
  InstallAll (const std::string &file, Time averagingPeriod = Seconds (0.5));

  /**
   * @brief Trace constructor that attaches to the node using node pointer
   * @param os    reference to the output stream
   * @param node  pointer to the node
   */
  CsTracer (boost::shared_ptr<std::ostream> os, Ptr<Node> node);

  /**
   * @brief Trace constructor that attaches to the node using node name
   * @param os        reference to the output stream
   * @param nodeName  name of the node registered using Names::Add
   */
  CsTracer (boost::shared_ptr<std::ostream> os, const std::string &node);

  /**
   * @brief Destructor
   */
  ~CsTracer ();

  /**
   * @brief Print head of the trace (e.g., for post-processing)
   *
   * @param os reference to output stream
   */
  void
  PrintHeader (std::ostream &os) const;

  /**
   * @brief Print current trace data
   *
   * @param os reference to output stream
   */
  void
  Print (std::ostream &os) const;
  
private:
  void
  Connect ();

  void 
  CacheHits (Ptr<const InterestHeader>, Ptr<const ContentObjectHeader>);
  
  void 
  CacheMisses (Ptr<const InterestHeader>);

private:
  void
  SetAveragingPeriod (const Time &period);

  void
  Reset ();

  void
  PeriodicPrinter ();
  
private:
  std::string m_node;
  Ptr<Node> m_nodePtr;

  boost::shared_ptr<std::ostream> m_os;

  Time m_period;
  EventId m_printEvent;
  cs::Stats m_stats;  
};

/**
 * @brief Helper to dump the trace to an output stream
 */
inline std::ostream&
operator << (std::ostream &os, const CsTracer &tracer)
{
  os << "# ";
  tracer.PrintHeader (os);
  os << "\n";
  tracer.Print (os);
  return os;
}

} // namespace ndn
} // namespace ns3

#endif // CCNX_CS_TRACER_H
