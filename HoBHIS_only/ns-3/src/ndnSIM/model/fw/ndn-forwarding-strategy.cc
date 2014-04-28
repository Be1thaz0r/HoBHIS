/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
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
 * Author:  Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *          Ilya Moiseenko <iliamo@cs.ucla.edu>
 *
 * Modifications to support HoBHIS : Natalya Rozhnova <natalya.rozhnova@lip6.fr>
 */

#include "ndn-forwarding-strategy.h"
#include <map>
#include <utility>
#include <iostream>

#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-fib.h"
#include "ns3/ndn-content-store.h"
#include "ns3/ndn-face.h"

#include "ns3/assert.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/string.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/reroute_tag.h"
#include "ns3/ndn-net-device-face.h"
#include "ns3/data-rate.h"

#include <ns3/ndn-name.h>
#include <ns3/ndn-hobhis-net-device-face.h>
#include <ns3/point-to-point-net-device.h>
#include "ns3/ndn-drop-tail-queue.h"
#include <ns3/queue.h>

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/tuple/tuple.hpp>
namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ForwardingStrategy);

NS_LOG_COMPONENT_DEFINE (ForwardingStrategy::GetLogName ().c_str ());

std::string
ForwardingStrategy::GetLogName ()
{
  return "ndn.fw";
}

TypeId ForwardingStrategy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::ForwardingStrategy")
    .SetGroupName ("Ndn")
    .SetParent<Object> ()

    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////

    .AddTraceSource ("OutInterests",  "OutInterests",  MakeTraceSourceAccessor (&ForwardingStrategy::m_outInterests))
    .AddTraceSource ("InInterests",   "InInterests",   MakeTraceSourceAccessor (&ForwardingStrategy::m_inInterests))
    .AddTraceSource ("DropInterests", "DropInterests", MakeTraceSourceAccessor (&ForwardingStrategy::m_dropInterests))

    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////

    .AddTraceSource ("OutData",  "OutData",  MakeTraceSourceAccessor (&ForwardingStrategy::m_outData))
    .AddTraceSource ("InData",   "InData",   MakeTraceSourceAccessor (&ForwardingStrategy::m_inData))
    .AddTraceSource ("DropData", "DropData", MakeTraceSourceAccessor (&ForwardingStrategy::m_dropData))

    .AddAttribute ("CacheUnsolicitedData", "Cache overheard data that have not been requested",
                   BooleanValue (false),
                   MakeBooleanAccessor (&ForwardingStrategy::m_cacheUnsolicitedData),
                   MakeBooleanChecker ())

    .AddAttribute ("DetectRetransmissions", "If non-duplicate interest is received on the same face more than once, "
                                            "it is considered a retransmission",
                   BooleanValue (true),
                   MakeBooleanAccessor (&ForwardingStrategy::m_detectRetransmissions),
                   MakeBooleanChecker ())
    ;
  return tid;
}

ForwardingStrategy::ForwardingStrategy (): error(0.0), A_prev(0.0), A_max(0.0)//: m_irc()
{
}

ForwardingStrategy::~ForwardingStrategy ()
{
}

void
ForwardingStrategy::NotifyNewAggregate ()
{
  if (m_pit == 0)
    {
      m_pit = GetObject<Pit> ();
    }
  if (m_fib == 0)
    {
      m_fib = GetObject<Fib> ();
    }
  if (m_contentStore == 0)
    {
      m_contentStore = GetObject<ContentStore> ();
    }

  Object::NotifyNewAggregate ();
}

void
ForwardingStrategy::DoDispose ()
{
  m_pit = 0;
  m_contentStore = 0;
  m_fib = 0;

  Object::DoDispose ();
}

void
ForwardingStrategy::OnInterest (Ptr<Face> inFace,
                                Ptr<const InterestHeader> header,
                                Ptr<const Packet> origPacket)
{
  m_inInterests (header, inFace);

  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*header);
  bool similarInterest = true;
  if (pitEntry == 0)
    {
      similarInterest = false;
      pitEntry = m_pit->Create (header);
      if (pitEntry != 0)
        {
          DidCreatePitEntry (inFace, header, origPacket, pitEntry);
        }
      else
        {
          FailedToCreatePitEntry (inFace, header, origPacket);
          return;
        }
    }

  bool isDuplicated = true;
  if (!pitEntry->IsNonceSeen (header->GetNonce ()))
    {
      pitEntry->AddSeenNonce (header->GetNonce ());
      isDuplicated = false;
    }

  if (isDuplicated)
    {
      DidReceiveDuplicateInterest (inFace, header, origPacket, pitEntry);
      return;
    }

  Ptr<Packet> contentObject;
  Ptr<const ContentObjectHeader> contentObjectHeader; // used for tracing
  Ptr<const Packet> payload; // used for tracing
  boost::tie (contentObject, contentObjectHeader, payload) = m_contentStore->Lookup (header);
  if (contentObject != 0)
    {
      NS_ASSERT (contentObjectHeader != 0);

      FwHopCountTag hopCountTag;
      if (origPacket->PeekPacketTag (hopCountTag))
        {
          contentObject->AddPacketTag (hopCountTag);
        }

      pitEntry->AddIncoming (inFace/*, Seconds (1.0)*/);

      // Do data plane performance measurements
      WillSatisfyPendingInterest (0, pitEntry);

      // Actually satisfy pending interest
      SatisfyPendingInterest (0, contentObjectHeader, payload, contentObject, pitEntry);
      return;
    }

  if (similarInterest && ShouldSuppressIncomingInterest (inFace, header, origPacket, pitEntry))
    {
      pitEntry->AddIncoming (inFace/*, header->GetInterestLifetime ()*/);
      // update PIT entry lifetime
      pitEntry->UpdateLifetime (header->GetInterestLifetime ());

      // Suppress this interest if we're still expecting data from some other face
      NS_LOG_DEBUG ("Suppress interests");
      m_dropInterests (header, inFace);

      DidSuppressSimilarInterest (inFace, header, origPacket, pitEntry);
      return;
    }

  if (similarInterest)
    {
      DidForwardSimilarInterest (inFace, header, origPacket, pitEntry);
    }

  PropagateInterest (inFace, header, origPacket, pitEntry);
}

void
ForwardingStrategy::OnData (Ptr<Face> inFace,
                            Ptr<const ContentObjectHeader> header,
                            Ptr<Packet> payload,
                            Ptr<const Packet> origPacket)
{
	NS_LOG_FUNCTION (inFace << header->GetName () << payload << origPacket);
	m_inData (header, payload, inFace);

	// Lookup PIT entry
	Ptr<pit::Entry> pitEntry = m_pit->Lookup (*header);
	if (pitEntry == 0)
	{
		DidReceiveUnsolicitedData (inFace, header, payload, origPacket);
		return;
	}
	else
	{
		FwHopCountTag hopCountTag;
		if (payload->PeekPacketTag (hopCountTag))
		{
			Ptr<Packet> payloadCopy = payload->Copy ();
			payloadCopy->RemovePacketTag (hopCountTag);

			// Add or update entry in the content store
			m_contentStore->Add (header, payloadCopy);
		}
		else
		{
			// Add or update entry in the content store
			m_contentStore->Add (header, payload); // no need for extra copy
		}
	}

	while (pitEntry != 0)
	{
		// Do data plane performance measurements
		WillSatisfyPendingInterest (inFace, pitEntry);

		// Actually satisfy pending interest
		SatisfyPendingInterest (inFace, header, payload, origPacket, pitEntry);

		// Lookup another PIT entry
		pitEntry = m_pit->Lookup (*header);
	}

	Ptr<NetDeviceFace> ndf_in = StaticCast<NetDeviceFace>(inFace);
	if(ndf_in->HobhisEnabled() && ! ndf_in->ClientServer())
		NDNComputeRTT(inFace, header, payload, origPacket);
}

void
ForwardingStrategy::NDNComputeRTT(Ptr<Face> inFace,
							   Ptr<const ContentObjectHeader> header,
							   Ptr<Packet> payload,
							   Ptr<const Packet> origPacket)
{
	double rtt_old, rtt_next, rtt_curr, sendtime;

	ndn::Name prefix = header->GetName ();

	std::map <ndn::Name, STimeEntry> & sendtable = inFace->GetSendingTable();

	std::map<ndn::Name, STimeEntry>::iterator
	stit(sendtable.find(prefix)),
	stend(sendtable.end());
	if (stit !=  stend)
	{
		STimeEntry & values = stit->second;
		sendtime = values.get_send_time();
	}
	else return;

	//+++++++++++++++++++++++++++++++++++++++++++++
	std::map <ndn::Name, ShrEntry> & shtable = inFace->GetShapingTable();

	std::map<ndn::Name, ShrEntry>::iterator
	fit(shtable.find(prefix.cut(1))),
	fend(shtable.end());
	if (fit != fend)
	{
		ShrEntry & values = fit->second;
		rtt_old = values.get_rtt();
	}
	//++++++++++++++++++++++++++++++++++++++++++++

	rtt_curr = Simulator::Now().GetSeconds() - sendtime;
	error = double(rtt_curr)/double(rtt_old); //0.8* error + 0.2 * rtt_curr/rtt_old;
	if(rtt_old < rtt_curr) A_max = rtt_curr;
	if(rtt_old != -1.0)
	{
		rtt_next = (0.8 * rtt_curr + 0.2 * rtt_old);
		//if(rtt_next > A_max && error < 1.0) rtt_next = A_max;
	}
	else
		rtt_next = rtt_curr;

	//Delete corresponding entry from Sending table
	//
	if (stit !=  stend) {
		sendtable.erase(stit);
	}

	// put rtt to SHR Table

	if (fit != fend) {
		ShrEntry & values = fit->second;
		values.set_rtt(rtt_next);
	}

	//	      PrintShapingTable(inFace);
	//		  PrintSendingTable(inFace);
}

void
ForwardingStrategy::PrintShapingTable(Ptr<Face> inFace)
{
	std::cout<<"Shaping Table"<<std::endl;

	std::map <ndn::Name, ShrEntry> & shtable = inFace->GetShapingTable();
	std::map<ndn::Name, ShrEntry>::const_iterator
	mit(shtable.begin()),
	mend(shtable.end());
	for(;mit!=mend;++mit)
	{
		std::cout <<"at: "<<Simulator::Now().GetSeconds()<<'\t'<< mit->first << '\t' << mit->second << std::endl;
	}
}

void
ForwardingStrategy::PrintSendingTable(Ptr<Face> inFace)
{
	std::cout<<inFace<<" Begin Sending Table"<<std::endl;

	std::map <ndn::Name, STimeEntry> & sendtable = inFace->GetSendingTable();
	std::map<ndn::Name, STimeEntry>::const_iterator
	mit(sendtable.begin()),
	mend(sendtable.end());
	for(;mit!=mend;++mit)
	{
		std::cout <<inFace<<'\t'<< mit->first << '\t' << mit->second << std::endl;
	}
	std::cout<<inFace<<" End Sending Table"<<std::endl<<std::endl;
}

void
ForwardingStrategy::DidCreatePitEntry (Ptr<Face> inFace,
                                       Ptr<const InterestHeader> header,
                                       Ptr<const Packet> origPacket,
                                       Ptr<pit::Entry> pitEntrypitEntry)
{
}

void
ForwardingStrategy::FailedToCreatePitEntry (Ptr<Face> inFace,
                                            Ptr<const InterestHeader> header,
                                            Ptr<const Packet> origPacket)
{
  m_dropInterests (header, inFace);
}

void
ForwardingStrategy::DidReceiveDuplicateInterest (Ptr<Face> inFace,
                                                 Ptr<const InterestHeader> header,
                                                 Ptr<const Packet> origPacket,
                                                 Ptr<pit::Entry> pitEntry)
{
  /////////////////////////////////////////////////////////////////////////////////////////
  //                                                                                     //
  // !!!! IMPORTANT CHANGE !!!! Duplicate interests will create incoming face entry !!!! //
  //                                                                                     //
  /////////////////////////////////////////////////////////////////////////////////////////
  pitEntry->AddIncoming (inFace);
  m_dropInterests (header, inFace);
}

void
ForwardingStrategy::DidSuppressSimilarInterest (Ptr<Face> face,
                                                Ptr<const InterestHeader> header,
                                                Ptr<const Packet> origPacket,
                                                Ptr<pit::Entry> pitEntry)
{
}

void
ForwardingStrategy::DidForwardSimilarInterest (Ptr<Face> inFace,
                                               Ptr<const InterestHeader> header,
                                               Ptr<const Packet> origPacket,
                                               Ptr<pit::Entry> pitEntry)
{
}

void
ForwardingStrategy::DidExhaustForwardingOptions (Ptr<Face> inFace,
                                                 Ptr<const InterestHeader> header,
                                                 Ptr<const Packet> origPacket,
                                                 Ptr<pit::Entry> pitEntry)
{
  NS_LOG_FUNCTION (this << boost::cref (*inFace));
  if (pitEntry->AreAllOutgoingInVain ())
    {
      m_dropInterests (header, inFace);

      // All incoming interests cannot be satisfied. Remove them
      pitEntry->ClearIncoming ();

      // Remove also outgoing
      pitEntry->ClearOutgoing ();

      // Set pruning timout on PIT entry (instead of deleting the record)
      m_pit->MarkErased (pitEntry);
    }
}



bool
ForwardingStrategy::DetectRetransmittedInterest (Ptr<Face> inFace,
                                                 Ptr<const InterestHeader> header,
                                                 Ptr<const Packet> packet,
                                                 Ptr<pit::Entry> pitEntry)
{
  pit::Entry::in_iterator existingInFace = pitEntry->GetIncoming ().find (inFace);

  bool isRetransmitted = false;

  if (existingInFace != pitEntry->GetIncoming ().end ())
    {
      // this is almost definitely a retransmission. But should we trust the user on that?
      isRetransmitted = true;
    }

  return isRetransmitted;
}

void
ForwardingStrategy::SatisfyPendingInterest (Ptr<Face> inFace,
                                            Ptr<const ContentObjectHeader> header,
                                            Ptr<const Packet> payload,
                                            Ptr<const Packet> origPacket,
                                            Ptr<pit::Entry> pitEntry)
{
  if (inFace != 0)
    pitEntry->RemoveIncoming (inFace);

  //satisfy all pending incoming Interests
  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
      bool ok = incoming.m_face->Send (origPacket->Copy ());
      if (ok)
        {
          m_outData (header, payload, inFace == 0, incoming.m_face);
          DidSendOutData (inFace, incoming.m_face, header, payload, origPacket, pitEntry);

          NS_LOG_DEBUG ("Satisfy " << *incoming.m_face);
        }
      else
        {
          m_dropData (header, payload, incoming.m_face);
          NS_LOG_DEBUG ("Cannot satisfy data to " << *incoming.m_face);
        }
      // successfull forwarded data trace
    }

  // All incoming interests are satisfied. Remove them
  pitEntry->ClearIncoming ();

  // Remove all outgoing faces
  pitEntry->ClearOutgoing ();

  // Set pruning timout on PIT entry (instead of deleting the record)
  m_pit->MarkErased (pitEntry);
}

void
ForwardingStrategy::DidReceiveUnsolicitedData (Ptr<Face> inFace,
                                               Ptr<const ContentObjectHeader> header,
                                               Ptr<const Packet> payload,
                                               Ptr<const Packet> origPacket)
{
  if (m_cacheUnsolicitedData)
    {
      // Optimistically add or update entry in the content store
      m_contentStore->Add (header, payload);
    }
  else
    {
      // Drop data packet if PIT entry is not found
      // (unsolicited data packets should not "poison" content store)

      //drop dulicated or not requested data packet
      m_dropData (header, payload, inFace);
    }
}

void
ForwardingStrategy::WillSatisfyPendingInterest (Ptr<Face> inFace,
                                                Ptr<pit::Entry> pitEntry)
{
  pit::Entry::out_iterator out = pitEntry->GetOutgoing ().find (inFace);

  // If we have sent interest for this data via this face, then update stats.
  if (out != pitEntry->GetOutgoing ().end ())
    {
      pitEntry->GetFibEntry ()->UpdateFaceRtt (inFace, Simulator::Now () - out->m_sendTime);
    }
}

bool
ForwardingStrategy::ShouldSuppressIncomingInterest (Ptr<Face> inFace,
                                                    Ptr<const InterestHeader> header,
                                                    Ptr<const Packet> origPacket,
                                                    Ptr<pit::Entry> pitEntry)
{
  bool isNew = pitEntry->GetIncoming ().size () == 0 && pitEntry->GetOutgoing ().size () == 0;

  if (isNew) return false; // never suppress new interests

  bool isRetransmitted = m_detectRetransmissions && // a small guard
                         DetectRetransmittedInterest (inFace, header, origPacket, pitEntry);

  if (pitEntry->GetOutgoing ().find (inFace) != pitEntry->GetOutgoing ().end ())
    {
      NS_LOG_DEBUG ("Non duplicate interests from the face we have sent interest to. Don't suppress");
      // got a non-duplicate interest from the face we have sent interest to
      // Probably, there is no point in waiting data from that face... Not sure yet

      // If we're expecting data from the interface we got the interest from ("producer" asks us for "his own" data)
      // Mark interface YELLOW, but keep a small hope that data will come eventually.

      // ?? not sure if we need to do that ?? ...

      // pitEntry->GetFibEntry ()->UpdateStatus (inFace, fib::FaceMetric::NDN_FIB_YELLOW);
    }
  else
    if (!isNew && !isRetransmitted)
      {
        return true;
      }

  return false;
}

void
ForwardingStrategy::PropagateInterest (Ptr<Face> inFace,
                                       Ptr<const InterestHeader> header,
                                       Ptr<const Packet> origPacket,
                                       Ptr<pit::Entry> pitEntry)
{
  bool isRetransmitted = m_detectRetransmissions && // a small guard
                         DetectRetransmittedInterest (inFace, header, origPacket, pitEntry);

  pitEntry->AddIncoming (inFace/*, header->GetInterestLifetime ()*/);
  /// @todo Make lifetime per incoming interface
  pitEntry->UpdateLifetime (header->GetInterestLifetime ());

  bool propagated = DoPropagateInterest (inFace, header, origPacket, pitEntry);

  if (!propagated && isRetransmitted) //give another chance if retransmitted
    {
      // increase max number of allowed retransmissions
      pitEntry->IncreaseAllowedRetxCount ();

      // try again
      propagated = DoPropagateInterest (inFace, header, origPacket, pitEntry);
    }

  // if (!propagated)
  //   {
  //     NS_LOG_DEBUG ("++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  //     NS_LOG_DEBUG ("+++ Not propagated ["<< header->GetName () <<"], but number of outgoing faces: " << pitEntry->GetOutgoing ().size ());
  //     NS_LOG_DEBUG ("++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  //   }

  // ForwardingStrategy will try its best to forward packet to at least one interface.
  // If no interests was propagated, then there is not other option for forwarding or
  // ForwardingStrategy failed to find it.
  if (!propagated && pitEntry->AreAllOutgoingInVain ())
    {
      DidExhaustForwardingOptions (inFace, header, origPacket, pitEntry);
    }
}

bool
ForwardingStrategy::CanSendOutInterest (Ptr<Face> inFace,
                                        Ptr<Face> outFace,
                                        Ptr<const InterestHeader> header,
                                        Ptr<const Packet> origPacket,
                                        Ptr<pit::Entry> pitEntry)
{
  if (outFace == inFace)
    {
      // NS_LOG_DEBUG ("Same as incoming");
      return false; // same face as incoming, don't forward
    }

  pit::Entry::out_iterator outgoing =
    pitEntry->GetOutgoing ().find (outFace);

  if (outgoing != pitEntry->GetOutgoing ().end ())
    {
      if (!m_detectRetransmissions)
        return false; // suppress
      else if (outgoing->m_retxCount >= pitEntry->GetMaxRetxCount ())
        {
          // NS_LOG_DEBUG ("Already forwarded before during this retransmission cycle (" <<outgoing->m_retxCount << " >= " << pitEntry->GetMaxRetxCount () << ")");
          return false; // already forwarded before during this retransmission cycle
        }
   }

  return true;
}


bool
ForwardingStrategy::TrySendOutInterest (Ptr<Face> inFace,
                                        Ptr<Face> outFace,
                                        Ptr<const InterestHeader> header,
                                        Ptr<const Packet> origPacket,
                                        Ptr<pit::Entry> pitEntry)
{
	if (!CanSendOutInterest (inFace, outFace, header, origPacket, pitEntry))
	{
		return false;
	}

	//transmission
	Ptr<Packet> packetToSend = origPacket->Copy ();

	Ptr<Packet> copy = origPacket->Copy ();

	Ptr<InterestHeader> interest = Create<InterestHeader> (*header);
	ndn::Name prefix = interest->GetName ().cut(1);

	Ptr<NetDeviceFace> ndf_in = StaticCast<NetDeviceFace>(inFace);
	Ptr<NetDevice> nd_in = ndf_in->GetNetDevice();
	Ptr<PointToPointNetDevice> p2pnd_in = StaticCast<PointToPointNetDevice> (nd_in);
	Ptr<Queue> queue = p2pnd_in->GetQueue();
	uint32_t qlen = queue->GetNPackets();
	uint32_t qlen_flow=0;
	uint32_t max_chunks = queue->GetNPackets();

	if(ndf_in->HobhisEnabled()==true && ndf_in->ClientServer() == false)
	{
		outFace->SetInFaceBW(prefix, inFace->GetCapacity());
	}

	if (!outFace->Send (packetToSend))
	{
		return false;
	}

	pitEntry->AddOutgoing (outFace);

	uint64_t dRate = inFace->GetCapacity();

	if(ndf_in->HobhisEnabled()==true && ndf_in->ClientServer() == false)
	{
		Ptr<NDNDropTailQueue> ndnqueue = StaticCast<NDNDropTailQueue> (queue);
		if(ndnqueue != NULL)
		{
			uint32_t ql = ndnqueue->GetQueueSizePerFlow(prefix);
			qlen_flow = ql;
			max_chunks = ndnqueue->GetMaxChunks();
			double nfl = ndnqueue->GetFlowNumber();
			outFace->SetFlowNumber(nfl);
		}


		std::map <ndn::Name, ShrEntry> & shtable = outFace->GetShapingTable();
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		std::map<ndn::Name, ShrEntry>::iterator
		fit(shtable.find(prefix)),
		fend(shtable.end());
		if (fit != fend)
		{
			ShrEntry & values = fit->second;

			if(double(dRate)!=0.0)
			{
				values.set_queue_length(qlen_flow);
				values.set_total_queue_length(qlen);
				values.set_bandwidth(double(dRate));
				values.set_max_chunks(max_chunks);
			}
			else
			{
				values.set_bandwidth(double(outFace->GetCapacity()));
				values.set_queue_length(0);
				values.set_total_queue_length(0);
				values.set_max_chunks(max_chunks);
			}
		}
		else
		{
			if(double(dRate) != 0.0)
				shtable.insert(std::pair<ndn::Name, ShrEntry>(prefix, ShrEntry(-1.0, qlen_flow, qlen, -1.0, double(dRate), false, max_chunks)));
			else
				shtable.insert(std::pair<ndn::Name, ShrEntry>(prefix, ShrEntry(-1.0, qlen_flow, qlen, -1.0, double(outFace->GetCapacity()), false, max_chunks)));

			UpdateShRQLen(inFace, outFace, prefix);
		}
	}
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// PrintShapingTable(outFace);

	DidSendOutInterest (inFace, outFace, header, origPacket, pitEntry);

	return true;
}

void ForwardingStrategy::UpdateShRQLen(Ptr<Face> inFace, Ptr<Face> outFace, ndn::Name prefix)
{
	Ptr<NetDeviceFace> ndf_in = StaticCast<NetDeviceFace>(inFace);
	Ptr<NetDevice> nd_in = ndf_in->GetNetDevice();
	Ptr<PointToPointNetDevice> p2pnd_in = StaticCast<PointToPointNetDevice> (nd_in);
	Ptr<Queue> queue = p2pnd_in->GetQueue();
	uint32_t qlen = queue->GetNPackets();
	uint32_t qlen_flow = qlen;

	if(ndf_in->HobhisEnabled()==true && ndf_in->ClientServer() == false){
		Ptr<NDNDropTailQueue> ndnqueue = StaticCast<NDNDropTailQueue> (queue);
		if(ndnqueue != NULL)
		{
			qlen_flow = ndnqueue->GetQueueSizePerFlow(prefix);
			qlen = ndnqueue->GetDataQueueLength();
		}
	}

	std::map <ndn::Name, ShrEntry> & shtable = outFace->GetShapingTable();

	std::map<ndn::Name, ShrEntry>::iterator
	fit(shtable.find(prefix)),
	fend(shtable.end());
	if (fit != fend)
	{
		ShrEntry & values = fit->second;
		values.set_queue_length(qlen_flow);
		values.set_total_queue_length(qlen);
	}
	/*
	 * PrintShapingTable(outFace);
	 */

	Simulator::Schedule (Seconds(0.0001), &ForwardingStrategy::UpdateShRQLen, this, inFace, outFace, prefix);
}

void
ForwardingStrategy::DidSendOutInterest (Ptr<Face> inFace,
                                        Ptr<Face> outFace,
                                        Ptr<const InterestHeader> header,
                                        Ptr<const Packet> origPacket,
                                        Ptr<pit::Entry> pitEntry)
{
  m_outInterests (header, outFace);
}

void
ForwardingStrategy::DidSendOutData (Ptr<Face> inFace,
                                    Ptr<Face> outFace,
                                    Ptr<const ContentObjectHeader> header,
                                    Ptr<const Packet> payload,
                                    Ptr<const Packet> origPacket,
                                    Ptr<pit::Entry> pitEntry)
{
}

void
ForwardingStrategy::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
  // do nothing for now. may be need to do some logging
}

void
ForwardingStrategy::AddFace (Ptr<Face> face)
{
  // do nothing here
}

void
ForwardingStrategy::RemoveFace (Ptr<Face> face)
{
  // do nothing here
}

void
ForwardingStrategy::DidAddFibEntry (Ptr<fib::Entry> fibEntry)
{
  // do nothing here
}

void
ForwardingStrategy::WillRemoveFibEntry (Ptr<fib::Entry> fibEntry)
{
  // do nothing here
}


} // namespace ndn
} // namespace ns3
