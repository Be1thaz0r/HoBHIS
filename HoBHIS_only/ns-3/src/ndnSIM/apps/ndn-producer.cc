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

#include "ndn-producer.h"
#include "ns3/log.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-fib.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"

#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE ("ndn.Producer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (Producer);
    
TypeId
Producer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::Producer")
    .SetGroupName ("Ndn")
    .SetParent<App> ()
    .AddConstructor<Producer> ()
    .AddAttribute ("Prefix","Prefix, for which producer has the data",
                   StringValue ("/"),
                   MakeNameComponentsAccessor (&Producer::m_prefix),
                   MakeNameComponentsChecker ())
    .AddAttribute ("PayloadSize", "Virtual payload size for Content packets",
                   UintegerValue (1024),
                   MakeUintegerAccessor(&Producer::m_virtualPayloadSize),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("RandomPayloadSizeMin", "Virtual min random payload size for Content packets",
                   UintegerValue (0),
                   MakeUintegerAccessor(&Producer::m_virtualRandPayloadSizeMin),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("RandomPayloadSizeMax", "Virtual max random payload size for Content packets",
                   UintegerValue (0),
                   MakeUintegerAccessor(&Producer::m_virtualRandPayloadSizeMax),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&Producer::m_freshness),
                   MakeTimeChecker ())
	.AddAttribute ("RandomDelayMin", "Virtual min random delay for Content packets",
                   UintegerValue (0),
                   MakeUintegerAccessor(&Producer::m_virtualDelayMin),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("RandomDelayMax", "Virtual max random delay for Content packets",
                   DoubleValue (0),
                   MakeDoubleAccessor(&Producer::m_virtualDelayMax),
                   MakeDoubleChecker<double>())
    ;
        
  return tid;
}
    
Producer::Producer (): mean_rtt(0.0), sum_rtt(0.0), num_rtt(0), predict (0.0), mov_av(0.0), first_Interest(true)
{
  // NS_LOG_FUNCTION_NOARGS ();
}

// inherited from Application base class.
void
Producer::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (GetNode ()->GetObject<Fib> () != 0);

  App::StartApplication ();

  NS_LOG_DEBUG ("NodeID: " << GetNode ()->GetId ());
  
  Ptr<Fib> fib = GetNode ()->GetObject<Fib> ();
  
  Ptr<fib::Entry> fibEntry = fib->Add (m_prefix, m_face, 0);

  fibEntry->UpdateStatus (m_face, fib::FaceMetric::NDN_FIB_GREEN);
  
  // // make face green, so it will be used primarily
  // StaticCast<fib::FibImpl> (fib)->modify (fibEntry,
  //                                        ll::bind (&fib::Entry::UpdateStatus,
  //                                                  ll::_1, m_face, fib::FaceMetric::NDN_FIB_GREEN));
}

void
Producer::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (GetNode ()->GetObject<Fib> () != 0);

  App::StopApplication ();
}


void
Producer::OnInterest (const Ptr<const InterestHeader> &interest, Ptr<Packet> origPacket)
{
  App::OnInterest (interest, origPacket); // tracing inside
  int packet_size;

  NS_LOG_FUNCTION (this << interest);

  if (!m_active) return;
    
  static ContentObjectTail tail;
  Ptr<ContentObjectHeader> header = Create<ContentObjectHeader> ();
  header->SetName (Create<NameComponents> (interest->GetName ()));
  header->SetFreshness (m_freshness);

  NS_LOG_INFO ("node("<< GetNode()->GetId() <<") respodning with ContentObject:\n" << boost::cref(*header));

	if (m_virtualRandPayloadSizeMax) {
    packet_size = m_rand.GetInteger(m_virtualRandPayloadSizeMin, m_virtualRandPayloadSizeMax);
  } else {
    packet_size = m_virtualPayloadSize;
  }
  
  Ptr<Packet> packet = Create<Packet> (packet_size);

  packet->AddHeader (*header);
  packet->AddTrailer (tail);

  // Echo back FwHopCountTag if exists
  FwHopCountTag hopCountTag;
  if (origPacket->RemovePacketTag (hopCountTag))
    {
      packet->AddPacketTag (hopCountTag);
    }

Time rtt_del = Seconds(m_rand_rtt.GetValue(m_virtualDelayMin, m_virtualDelayMax));
  Simulator::Schedule (rtt_del,
                       &Producer::m_protocolHandler, this, packet);

//  m_protocolHandler (packet);
  
  m_transmittedContentObjects (header, packet, this, m_face);
}

} // namespace ndn
} // namespace ns3
