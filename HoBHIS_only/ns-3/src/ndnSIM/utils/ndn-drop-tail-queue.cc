/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 * Author: Natalya Rozhnova <natalya.rozhnova@lip6.fr>, Université Pierre et Marie Curie, Paris, France
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ndn-drop-tail-queue.h"
#include "ns3/ndn-header-helper.h"
#include "ns3/ppp-header.h"
#include "ns3/ndn-content-object.h"
#include "ns3/ndn-interest.h"

NS_LOG_COMPONENT_DEFINE ("NDNDropTailQueue");

namespace ns3 {
namespace ndn{

NS_OBJECT_ENSURE_REGISTERED (NDNDropTailQueue);

TypeId NDNDropTailQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NDNDropTailQueue")
    .SetParent<Queue> ()
    .AddConstructor<NDNDropTailQueue> ()
    .AddAttribute ("Mode",
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&NDNDropTailQueue::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets accepted by this NDNDropTailQueue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&NDNDropTailQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes",
                   "The maximum number of bytes accepted by this NDNDropTailQueue.",
                   UintegerValue (100 * 65535),
                   MakeUintegerAccessor (&NDNDropTailQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}

NDNDropTailQueue::NDNDropTailQueue () :
  Queue (),
  m_packets (),
  m_bytesInQueue (0),
  m_nQueueSizePerFlow()
{
  NS_LOG_FUNCTION_NOARGS ();
}

NDNDropTailQueue::~NDNDropTailQueue ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
NDNDropTailQueue::SetMode (NDNDropTailQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (mode);
  m_mode = mode;
}

NDNDropTailQueue::QueueMode
NDNDropTailQueue::GetMode (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mode;
}

bool
NDNDropTailQueue::DoEnqueue (Ptr<Packet> p)
{
	NS_LOG_FUNCTION (this << p);

	PppHeader pppHeader;
	Ptr<Packet> copy = p->Copy ();
	copy->RemoveHeader(pppHeader);
	ndn::HeaderHelper::Type type = ndn::HeaderHelper::GetNdnHeaderType (copy);

	uint8_t nack;


	if(type ==ndn::HeaderHelper::INTEREST_NDNSIM ||
			type == ndn::HeaderHelper::INTEREST_CCNB)
	{
		Ptr<ndn::InterestHeader> header = Create<ndn::InterestHeader> ();
		Ptr<Packet> nack_p = copy->Copy();
		nack_p->RemoveHeader (*header);
		nack = header->GetNack();
	}

	if(type == ndn::HeaderHelper::CONTENT_OBJECT_NDNSIM ||
			type == ndn::HeaderHelper::CONTENT_OBJECT_CCNB ||
			((type ==ndn::HeaderHelper::INTEREST_NDNSIM ||
					type == ndn::HeaderHelper::INTEREST_CCNB) && nack == 0))
	{
		if (m_mode == QUEUE_MODE_PACKETS &&(m_packets.size () >= m_maxPackets))
		{
			NS_LOG_LOGIC ("Queue full (at max packets) -- droppping pkt");
			Drop (p);
			return false;
		}

		if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes))
		{
			NS_LOG_LOGIC ("Queue full (packet would exceed max bytes) -- droppping pkt");
			Drop (p);
			return false;
		}

		NS_LOG_LOGIC ("Number packets " << m_packets.size ());
		NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
	}

	if(type == ndn::HeaderHelper::CONTENT_OBJECT_NDNSIM || type == ndn::HeaderHelper::CONTENT_OBJECT_CCNB)
	{
		m_bytesInQueue += p->GetSize ();
		m_packets.push (p);

		Ptr<ndn::ContentObjectHeader> header = Create<ndn::ContentObjectHeader> ();
		copy->RemoveHeader (*header);
		ndn::Name prefix = header->GetName ().cut(1);

		std::map<ndn::Name, uint32_t>::iterator
		fit(m_nQueueSizePerFlow.find(prefix)),
		fend(m_nQueueSizePerFlow.end());
		if (fit == fend)
		{
			m_nQueueSizePerFlow.insert(std::pair<ndn::Name, uint32_t>(prefix, 1));
		}
		else
		{
			uint32_t & queue_size = fit->second;
			uint32_t qs = queue_size;
			if(qs + 1 <= m_maxPackets)
				queue_size++;
		}
	}

	return true;
}

Ptr<Packet>
NDNDropTailQueue::DoDequeue (void)
{
	  NS_LOG_FUNCTION (this);
	  Ptr<Packet> p;

		  if (m_packets.empty ())
		  {
			  NS_LOG_LOGIC ("Queue empty");
			  return 0;
		  }

		  p = m_packets.front ();
		  m_packets.pop ();
		  m_bytesInQueue -= p->GetSize ();

		  NS_LOG_LOGIC ("Popped " << p);

		  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
		  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);


		  std::map <ndn::Name, uint32_t> & QueueSizePerFlow = GetQLengthPerFlow ();

		  PppHeader pppHeader;
		  Ptr<Packet> copy = p->Copy ();
		  copy->RemoveHeader(pppHeader);

		  ndn::HeaderHelper::Type type = ndn::HeaderHelper::GetNdnHeaderType (copy);

		  if(type == ndn::HeaderHelper::CONTENT_OBJECT_NDNSIM || type == ndn::HeaderHelper::CONTENT_OBJECT_CCNB)
		  {
			  Ptr<ndn::ContentObjectHeader> header = Create<ndn::ContentObjectHeader> ();
			  copy->RemoveHeader (*header);
			  ndn::Name prefix = header->GetName ().cut(1);

			  std::map<ndn::Name, uint32_t>::iterator
			  fit(QueueSizePerFlow.find(prefix)),
			  fend(QueueSizePerFlow.end());
			  if (fit != fend)
			  {
				  uint32_t & queue_size = fit->second;
				  if(queue_size != 0)
					  queue_size--;
			  }
		  }

	  return p;
}

Ptr<const Packet>
NDNDropTailQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

void NDNDropTailQueue::PrintQueueSizePerFlow()
{
	const std::map <ndn::Name, uint32_t> & QueueSizePerFlow = GetQLengthPerFlow ();

	std::cout<<"Queue size per flow"<<std::endl;
	std::map<ndn::Name, uint32_t>::const_iterator
	mit(QueueSizePerFlow.begin()),
	mend(QueueSizePerFlow.end());
	for(;mit!=mend;++mit)
	{
		std::cout <<"prefix name : "<< mit->first << "\tqueue size : " << mit->second << std::endl;
	}

}

uint32_t NDNDropTailQueue::GetQueueSizePerFlow(ndn::Name prefix)
{

	std::map <ndn::Name, uint32_t> & QueueSizePerFlow = GetQLengthPerFlow ();
	std::map<ndn::Name, uint32_t>::iterator
	mit(QueueSizePerFlow.find(prefix)),
	mend(QueueSizePerFlow.end());
	if(mit!=mend)
	{
		uint32_t & queue_size = mit->second;
		uint32_t ql = queue_size;
		return ql;
	}
return 0;
}

double NDNDropTailQueue:: GetFlowNumber()
{
	std::map <ndn::Name, uint32_t> & QueueSizePerFlow = GetQLengthPerFlow ();
	double fl = double(QueueSizePerFlow.size());
//	PrintQueueSizePerFlow();
	return fl;
}

} // namespace ns3
}
