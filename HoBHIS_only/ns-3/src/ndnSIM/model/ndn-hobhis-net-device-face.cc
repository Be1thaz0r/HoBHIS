/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
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
 * Authors: Rozhnova Natalya <natalya.rozhnova@lip6.fr>
 */

#include "ndn-hobhis-net-device-face.h"
#include "ndn-l3-protocol.h"

#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/pointer.h"
#include "ns3/random-variable.h"

#include "ns3/ndn-content-object.h"

#include "ns3/point-to-point-net-device.h"
#include "ns3/channel.h"
#include "ns3/ndn-name-components.h"
#include "ns3/ndn-header-helper.h"
#include "ns3/ndn-interest.h"
#include "ns3/simulator.h"

#include <map>
#include <utility>
#include "ns3/ndn_shr_entry.h"
#include "ns3/ndn_send_time_entry.h"

NS_LOG_COMPONENT_DEFINE ("ndn.HobhisNetDeviceFace");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (HobhisNetDeviceFace);

TypeId
HobhisNetDeviceFace::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::ndn::HobhisNetDeviceFace")
    		.SetParent<NetDeviceFace> ()
    		.SetGroupName ("Ndn")
    		.AddAttribute ("MaxInterest",
    					   "Size of the shaper interest queue.",
    					   UintegerValue (100),
    					   MakeUintegerAccessor (&HobhisNetDeviceFace::m_maxInterest),
    					   MakeUintegerChecker<uint32_t> ())
    		.AddAttribute ("Design",
    					   "Design parameter",
    					   DoubleValue (0.1),
    					   MakeDoubleAccessor (&HobhisNetDeviceFace::m_design),
    					   MakeDoubleChecker<double> ())
    		.AddAttribute ("QueueTarget",
    					   "Target for the shaper interest queue.",
    					   UintegerValue (50),
    					   MakeUintegerAccessor (&HobhisNetDeviceFace::m_target),
    					   MakeUintegerChecker<uint32_t> ())
    	    .AddAttribute ("HoBHISEnabled", "Enable shaper",
    	    			   BooleanValue(false),
    	    			   MakeBooleanAccessor(&HobhisNetDeviceFace::m_hobhisEnabled),
    	    			   MakeBooleanChecker())
    	    .AddAttribute ("ClientServer", "Is the node a client/server?",
    	    		 	   BooleanValue(true),
    	    		 	   MakeBooleanAccessor(&HobhisNetDeviceFace::m_client_server),
    	    		 	   MakeBooleanChecker())
    	    .AddAttribute ("DynamicDesign", "Use the dynamic adjustment of the design parameter",
    	     		 	   BooleanValue(false),
    	       		 	   MakeBooleanAccessor(&HobhisNetDeviceFace::m_dynamic_design),
    	     		 	   MakeBooleanChecker())
    		;
	return tid;
}

HobhisNetDeviceFace::HobhisNetDeviceFace (Ptr<Node> node, const Ptr<NetDevice> &netDevice)
: NetDeviceFace (node, netDevice)
, m_outContentFirst(true)
, m_outContentSize(1000)
, m_outInterestFirst(true)
, m_outInterestSize(40)
, m_inContentFirst(true)
, m_inContentSize(1000)
, m_shaperState(OPEN)
, m_InterestFirst()
//, m_InFaceBW()
{
	DataRateValue dataRate;
	netDevice->GetAttribute ("DataRate", dataRate);
	m_outBitRate = dataRate.Get().GetBitRate();
	m_shapingRate = m_outBitRate; // assume symmetric bandwidth
}

HobhisNetDeviceFace::~HobhisNetDeviceFace ()
{
	NS_LOG_FUNCTION_NOARGS ();
	//  delete SHR;
}

HobhisNetDeviceFace& HobhisNetDeviceFace::operator= (const HobhisNetDeviceFace &)
{
	return *this;
}

bool
HobhisNetDeviceFace::SendImpl (Ptr<Packet> p)
{
	NS_LOG_FUNCTION (this << p);

	HeaderHelper::Type type = HeaderHelper::GetNdnHeaderType (p);

	switch (type)
	{
	case HeaderHelper::INTEREST_NDNSIM:
	{

		uint8_t nack;
		Ptr<ndn::InterestHeader> header = Create<ndn::InterestHeader> ();
		Ptr<Packet> nack_p = p->Copy();
		nack_p->RemoveHeader (*header);
		nack = header->GetNack();

		if(this->HobhisEnabled() && ! this->ClientServer() && nack == 0)
		{
			Ptr<Packet> packet = p->Copy ();
			Ptr<InterestHeader> header = Create<InterestHeader> ();
			packet->RemoveHeader (*header);
			NS_LOG_DEBUG("Interest packet, router");
			NS_LOG_LOGIC(this << " shaper qlen: " << m_interestQueue.size());
			ndn::Name prefix = header->GetName ().cut(1);

			if(m_interestQueue.size() + 1 <= m_maxInterest)
			{
				// Enqueue success
				m_interestQueue.push(p);

				std::map<ndn::Name, uint32_t>::iterator
				iqit(m_nIntQueueSizePerFlow.find(prefix)),
				iqend(m_nIntQueueSizePerFlow.end());
				if (iqit == iqend)
				{
					m_nIntQueueSizePerFlow.insert(std::pair<ndn::Name, uint32_t>(prefix, 1));
				}
				else
				{
					uint32_t & queue_size = iqit->second;
					uint32_t qs = queue_size;
					if(qs + 1 <= m_maxInterest)
						queue_size++;
				}

				if (m_shaperState == OPEN)
				{
					if (m_outInterestFirst)
						{
							m_outInterestSize = p->GetSize(); // first sample
							m_outInterestFirst = false;
							ShaperSend();
						}
						else
						{
							m_outInterestSize += (p->GetSize() - m_outInterestSize) / 8.0; // smoothing
							ShaperDequeue();
						}

					//	return NetDeviceFace::SendImpl (p);
				}

				return true;
			}
			else
			{
				NS_LOG_LOGIC(this << " Tail drop");
				return false;
			}
		}
		else
		{
			return NetDeviceFace::SendImpl (p);
		}
	}
	case HeaderHelper::CONTENT_OBJECT_NDNSIM:
	{
		NS_LOG_DEBUG("Data packet, router");
		Ptr<Packet> packet = p->Copy ();
		Ptr<ContentObjectHeader> header = Create<ContentObjectHeader> ();
		packet->RemoveHeader (*header);
		ndn::Name prefix = header->GetName();
		if (m_outContentFirst)
		{
			m_outContentSize = p->GetSize(); // first sample
			m_outContentFirst = false;
		}
		else
		{
			m_outContentSize += (p->GetSize() - m_outContentSize) / 8.0; // smoothing
		}

		return NetDeviceFace::SendImpl (p); // no shaping for content packets
	}
	default:
		return false;
	}
}

void
HobhisNetDeviceFace::ShaperOpen ()
{
	NS_LOG_FUNCTION (this);

	if (m_interestQueue.size() > 0)// && m_shapingRate >=0.0)
	{
		ShaperDequeue();
	}
/*	else if(m_shapingRate < 0.0)
	{
		ShaperDequeue();
	}*/
	else
	{
		m_shaperState = OPEN;
	}
}

void
HobhisNetDeviceFace::ShaperDequeue ()
{
	NS_LOG_FUNCTION (this);
	NS_LOG_LOGIC(this << " shaper qlen: " << m_interestQueue.size());

	Time gap = ComputeGap();

	if(gap.GetSeconds() >= 0.0)
	{
		Simulator::Schedule (gap, &HobhisNetDeviceFace::ShaperSend, this);
	}
	else
	{
		Simulator::Schedule (Seconds(0.0001), &HobhisNetDeviceFace::ShaperOpen, this);
	}

}

void HobhisNetDeviceFace::ShaperSend()
{
	Ptr<Packet> p = m_interestQueue.front ();
	m_interestQueue.pop ();
	Ptr<Packet> packet = p->Copy();
	Ptr<InterestHeader> header = Create<InterestHeader> ();
	packet->RemoveHeader (*header);
	ndn::Name prefix = header->GetName ();

	std::map<ndn::Name, uint32_t>::iterator
	iqit(m_nIntQueueSizePerFlow.find(prefix.cut(1))),
	iqend(m_nIntQueueSizePerFlow.end());
	if (iqit != iqend)
	{
		uint32_t & queue_size = iqit->second;
		if(queue_size != 0)
			queue_size--;
	}

	std::map <ndn::Name, STimeEntry> & sendtable = GetSendingTable();

	std::map<ndn::Name, STimeEntry>::iterator
	fit(sendtable.find(prefix)),
	fend(sendtable.end());
	if (fit == fend)
	{
		sendtable.insert(std::pair<ndn::Name, STimeEntry>(prefix, STimeEntry(Simulator::Now().GetSeconds())));
	}
	// send out the interest
	NetDeviceFace::SendImpl (p);
	ShaperOpen();
}

Time HobhisNetDeviceFace::ComputeGap()
{

	Ptr<Packet> p = m_interestQueue.front ()->Copy();

	m_shaperState = BLOCKED;

	Ptr<Packet> packet = p->Copy ();
	Ptr<InterestHeader> header = Create<InterestHeader> ();
	packet->RemoveHeader (*header);
	ndn::Name prefix = header->GetName ();

	double rtt = -1.0;
	double buf_part = 0;
	uint64_t bw = GetInFaceBW(prefix.cut(1));
	double qlen = 0.0;
	double qlen_flow = 0.0;
	double queue_rel = 1.0;
	double fl_num = 1.0;
	uint32_t max_chunks;

	std::map <ndn::Name, ShrEntry> & shtable = GetShapingTable();
	/*
	std::cout<<"Shaping Table"<<std::endl;
	std::map<ndn::Name, ShrEntry>::const_iterator
		sit(shtable.begin()),
		send(shtable.end());
	for(;sit!=send;++sit) {
		std::cout <<this<<" at: "<<Simulator::Now().GetSeconds()<<'\t'<< sit->first << '\t' << sit->second << std::endl;
	}
	 */
	std::map<ndn::Name, ShrEntry>::iterator
	fit(shtable.find(prefix.cut(1))),
	fend(shtable.end());
	if (fit != fend) {
		ShrEntry & values = fit->second;
		rtt = values.get_rtt();
		qlen_flow = values.get_queue_length();
		qlen = values.get_total_queue_length();
		max_chunks = values.get_max_chunks();
	}
	double flows = GetFlowNumber();
	if(flows != 0.0)
		fl_num = flows;
//	double bw1 = double(bw);

	if(rtt != -1.0)
	{
		if(qlen!=0.0 && qlen_flow!=0.0)
			queue_rel = double (qlen_flow/qlen);

		// for traffic split
		/*  		double x = double(1.0 / queue_rel);

  	  	  	  	  	if(x > 1.0) {bw1 = double(bw/x);}
		 */
		buf_part = m_design * double(m_target*queue_rel - qlen_flow)/rtt;
		m_shapingRate = double(bw)/(8.0 * m_inContentSize) + buf_part;
	}
	else m_shapingRate = double(bw)/(8.0 * m_outInterestSize);

	double out_rate_in_interests = double(m_outBitRate)/(8.0 * m_outInterestSize);

	Time gap = Seconds(0.0);

	if(m_shapingRate >= out_rate_in_interests)
	{
		m_shapingRate = out_rate_in_interests/fl_num;

		gap = Seconds(0.0);
	}
	else
	{
		double del = 0.0;

		del = double(1.0 / m_shapingRate);

		gap = Seconds(del);

	}
/*
	std::cout<<"*************************"<<this<<"*************************"<<std::endl
			<<this<<"\t\t\tGetCapacity() = "<<GetCapacity()<<std::endl
			<<this<<"\t\t\tbw = "<<bw<<std::endl
			<<this<<"\t\t\tm_inContentSize = "<<m_inContentSize<<std::endl
			<<this<<"\t\t\tm_outBitRate = "<<m_outBitRate<<std::endl
			<<this<<"\t\t\tm_outInterestSize = "<<m_outInterestSize<<std::endl
			<<this<<"\t\t\tbuf_part = "<< buf_part<<std::endl
			<<this<<"\t\t\trtt = "<<rtt<<std::endl
			<<this<<"\t\t\t"<<Simulator::Now().GetSeconds()<<"\t\t\tshr = "<<m_shapingRate<<std::endl
			<<this<<"\t\t\tout_rate_in_interests = "<<out_rate_in_interests<<std::endl
			<<this<<"\t\t\tfl_num = "<<fl_num<<std::endl
			<<this<<"\t\t\tm_hobhisEnabled = "<<m_hobhisEnabled<<std::endl
			<<this<<"\t\t\tqueue_rel = "<<queue_rel<<std::endl
			<<this<<"\t\t\t"<<Simulator::Now().GetSeconds()<<"\tqlen_flow = "<<qlen_flow<<std::endl
			<<this<<"\t\t\tqlen = "<<qlen<<std::endl
			<<this<<"\t\t\t IQueue = "<<m_interestQueue.size()<<std::endl
			<<this<<"\t\t\tgap = "<<gap.GetSeconds()<<std::endl
			<<"*************************************************************"<<std::endl;
*/

	if (fit != fend) {
		ShrEntry & values = fit->second;
		if(m_shapingRate < 0.0) m_shapingRate = 0.0;
		values.set_sh_rate(m_shapingRate);//m_shapingRate);
	}

	NS_LOG_LOGIC("Actual shaping rate: " << m_shapingRate << "bps, Gap: " << gap);

	return gap;
}

void
HobhisNetDeviceFace::ReceiveFromNetDevice (Ptr<NetDevice> device,
		Ptr<const Packet> p,
		uint16_t protocol,
		const Address &from,
		const Address &to,
		NetDevice::PacketType packetType)
{
	NS_LOG_FUNCTION (device << p << protocol << from << to << packetType);

	HeaderHelper::Type type = HeaderHelper::GetNdnHeaderType (p);
	switch (type)
	{
	case HeaderHelper::CONTENT_OBJECT_NDNSIM:
	{
		NS_LOG_DEBUG("Receive Data from netdevice");
		if (m_inContentFirst)
		{
			m_inContentSize = p->GetSize(); // first sample
			m_inContentFirst = false;
		}
		else
		{
			m_inContentSize = p->GetSize();
			m_inContentSize += (p->GetSize() - m_inContentSize) / 8.0; // smoothing
		}
		break;
	}
	default:
		break;
	}

	Receive (p);
}

void
HobhisNetDeviceFace::SetInFaceBW(ndn::Name prefix, uint64_t bw)
{

// From inline functions Get in Face class
	std::map <ndn::Name, uint64_t> & bwTable = GetInFaceBWTable();
	std::map<ndn::Name, uint64_t>::iterator
			mit(bwTable.find(prefix)),
			mend(bwTable.end());
			if (mit == mend)
			{
				bwTable.insert(std::pair<ndn::Name, uint64_t>(prefix, bw));
			}
/*
			std::cout<<"Bandwidth Table in Set"<<std::endl;

						std::map<ndn::Name, uint64_t>::const_iterator
						fit(bwTable.begin()),
						fend(bwTable.end());
						for(;fit!=fend;++fit)
						{
							std::cout <<"at: "<<Simulator::Now().GetSeconds()<<'\t'<< fit->first << '\t' << fit->second << std::endl;
						}
*/
			//else {} for future work (for Multicast scenarios where we take a minimum capacity)
}

uint64_t
HobhisNetDeviceFace::GetInFaceBW(ndn::Name prefix)
{
	std::map <ndn::Name, uint64_t> & bwTable = GetInFaceBWTable();
	std::map<ndn::Name, uint64_t>::const_iterator
			mit(bwTable.find(prefix)),
			mend(bwTable.end());
			if (mit != mend)
			{
				uint64_t bw = mit->second;
				return bw;
			}
			//else {} for future work to keep more than one input faces
/*			std::cout<<"Bandwidth Table in Get"<<std::endl;

			std::map<ndn::Name, uint64_t>::const_iterator
			fit(bwTable.begin()),
			fend(bwTable.end());
			for(;fit!=fend;++fit)
			{
				std::cout <<"at: "<<Simulator::Now().GetSeconds()<<'\t'<< fit->first << '\t' << fit->second << std::endl;
			}
*/
	return -1; // something wrong
}

uint32_t HobhisNetDeviceFace::GetIntQueueSizePerFlow(ndn::Name prefix)
{
	std::map<ndn::Name, uint32_t>::iterator
	mit(m_nIntQueueSizePerFlow.find(prefix)),
	mend(m_nIntQueueSizePerFlow.end());
	if(mit!=mend)
	{
		uint32_t & queue_size = mit->second;
		uint32_t ql = queue_size;
		return ql;
	}
return 0;
}


} // namespace ndnsim
} // namespace ns3

