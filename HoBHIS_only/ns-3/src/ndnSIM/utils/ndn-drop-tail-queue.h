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
 * Natalya Rozhnova, <natalya.rozhnova@lip6.fr>, UPMC, Paris, France
 */

#ifndef NDNDROPTAIL_H
#define NDNDROPTAIL_H

#include <queue>
#include <map>
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/ndn-name.h"

namespace ns3 {
namespace ndn{

class TraceContainer;

/**
 * \ingroup queue
 *
 * \brief A FIFO packet queue that drops tail-end packets on overflow
 */
class NDNDropTailQueue : public Queue {
public:
  static TypeId GetTypeId (void);
  /**
   * \brief NDNDropTailQueue Constructor
   *
   * Creates a droptail queue with a maximum size of 100 packets by default
   */
  NDNDropTailQueue ();

  virtual ~NDNDropTailQueue();

  /**
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   *
   */
  void SetMode (NDNDropTailQueue::QueueMode mode);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  NDNDropTailQueue::QueueMode GetMode (void);

  inline const std::map<ndn::Name, uint32_t> & GetQLengthPerFlow() const {return this->m_nQueueSizePerFlow;}
  inline std::map<ndn::Name, uint32_t> & GetQLengthPerFlow() {return this->m_nQueueSizePerFlow;}

  void PrintQueueSizePerFlow();
  uint32_t GetQueueSizePerFlow(ndn::Name prefix);

  uint32_t GetDataQueueLength() {return m_packets.size();};

  double GetFlowNumber();

  uint32_t GetMaxChunks() const {return this->m_maxPackets;};

private:
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;

  std::queue<Ptr<Packet> > m_packets;

  uint32_t m_maxPackets;
  uint32_t m_maxBytes;
  uint32_t m_bytesInQueue;
  QueueMode m_mode;
  std::map <ndn::Name, uint32_t> m_nQueueSizePerFlow;

};

} // namespace ns3
}
#endif /* DROPTAIL_H */
