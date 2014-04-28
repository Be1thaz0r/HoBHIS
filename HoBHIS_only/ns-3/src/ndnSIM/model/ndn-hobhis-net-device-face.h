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

#ifndef NDN_HOBHIS_NET_DEVICE_FACE_H
#define NDN_HOBHIS_NET_DEVICE_FACE_H

#include <queue>
#include "ndn-net-device-face.h"
#include "ns3/net-device.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"

//#include "ns3/random-variable.h"

namespace ns3 {
namespace ndn {

/**
 * \ingroup ndn-face
 * \brief Implementation of layer-2 (Ethernet) Ndn face with interest shaping
 *
 * This class adds interest shaping to NdnNetDeviceFace
 *
 * \see NdnNetDeviceFace
 */
class HobhisNetDeviceFace  : public NetDeviceFace
{
public:
  static TypeId
  GetTypeId ();

  /**
   * \brief Constructor
   *
   * @param node Node associated with the face
   * @param netDevice a smart pointer to NetDevice object to which
   * this face will be associate
   */
  HobhisNetDeviceFace (Ptr<Node> node, const Ptr<NetDevice> &netDevice);

  /**
   * \brief Destructor
   *
   */
  virtual ~HobhisNetDeviceFace();

  uint32_t GetQueueLength() {return m_interestQueue.size();};

  bool SetSendingTime(ndn::NameComponents prefix,
		  	  	   	  double stime);

//  void SetBackBW(double bndwth) {m_backBW = bndwth;};

  bool HobhisEnabled() {return m_hobhisEnabled;};

  bool ClientServer() {return m_client_server;};

  void SetInFaceBW(ndn::Name prefix, uint64_t bw);

  uint64_t GetInFaceBW(ndn::Name prefix);

  uint32_t GetIntQueueSizePerFlow(ndn::Name prefix);

//  void SetFlowNumber(double nflows) {m_Nflows = nflows;};

protected:

  virtual bool
  SendImpl (Ptr<Packet> p);

  Ptr<Face> inFace;
private:
  HobhisNetDeviceFace (const HobhisNetDeviceFace &); ///< \brief Disabled copy constructor
  HobhisNetDeviceFace& operator= (const HobhisNetDeviceFace &); ///< \brief Disabled copy operator

  void ShaperOpen ();
  void ShaperDequeue ();

  virtual void ReceiveFromNetDevice (Ptr<NetDevice> device,
                             Ptr<const Packet> p,
                             uint16_t protocol,
                             const Address &from,
                             const Address &to,
                             NetDevice::PacketType packetType);

  void ShaperSend();
  Time ComputeGap();

  std::queue<Ptr<Packet> > m_interestQueue;
  uint32_t m_maxInterest;

  double m_shapingRate;
  uint64_t m_outBitRate;

//for shaping rate computation
  double m_design;
  uint32_t m_target;

  bool m_outContentFirst;
  double m_outContentSize;
  bool m_outInterestFirst;
  double m_outInterestSize;

  bool m_inContentFirst;
  double m_inContentSize;

//  uint64_t m_backBW;

   enum ShaperState
  {
    OPEN,
    BLOCKED
  };

  ShaperState m_shaperState;
  bool m_hobhisEnabled;
  bool m_client_server;
  std::map<ndn::Name, bool> m_InterestFirst;
  std::map <ndn::Name, uint32_t> m_nIntQueueSizePerFlow;
 // std::map<ndn::Name, uint64_t> m_InFaceBW;
  bool m_dynamic_design;
 };

} // namespace ndn
} // namespace ns3

#endif //NDN_HOBHIS_NET_DEVICE_FACE_H
