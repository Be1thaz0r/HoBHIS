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
 * Authors: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef NDN_FACE_H
#define NDN_FACE_H

#include <ostream>
#include <algorithm>
#include <map>

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/type-id.h"
#include "ns3/traced-callback.h"
#include "ns3/ndn-limits.h"
#include "ns3/ndn_shr_entry.h"
#include "ns3/ndn_send_time_entry.h"

namespace ns3 {

class Packet;
class Node;

namespace ndn {

/**
 * \ingroup ndn
 * \defgroup ndn-face Faces
 */
/**
 * \ingroup ndn-face
 * \brief Virtual class defining NDN face
 *
 * This class defines basic functionality of NDN face. Face is core
 * component responsible for actual delivery of data packet to and
 * from NDN stack
 *
 * \see ndn::LocalFace, ndn::NetDeviceFace, ndn::Ipv4Face, ndn::UdpFace
 */
class Face :
    public Object
{
public:
  static TypeId
  GetTypeId ();
  
  /**
   * \brief Ndn protocol handler
   *
   * \param face Face from which packet has been received
   * \param packet Original packet
   */
  typedef Callback<void,const Ptr<Face>&,const Ptr<const Packet>& > ProtocolHandler;

  /**
   * \brief Default constructor
   */
  Face (Ptr<Node> node);
  virtual ~Face();

  /**
   * @brief Get node to which this face is associated
   */
  Ptr<Node>
  GetNode () const;

  ////////////////////////////////////////////////////////////////////
  
  /**
   * \brief Register callback to call when new packet arrives on the face
   *
   * This method should call protocol-dependent registration function
   */
  virtual void
  RegisterProtocolHandler (ProtocolHandler handler);

  /**
   * \brief Send packet on a face
   *
   * This method will be called by lower layers to send data to device or application
   *
   * \param p smart pointer to a packet to send
   *
   * @return false if either limit is reached
   */ 
  bool
  Send (Ptr<Packet> p);

  /**
   * \brief Receive packet from application or another node and forward it to the Ndn stack
   *
   * \todo The only reason for this call is to handle tracing, if requested
   */
  bool
  Receive (const Ptr<const Packet> &p);
  ////////////////////////////////////////////////////////////////////

  /**
   * \brief Assign routing/forwarding metric with face
   *
   * \param metric configured routing metric (cost) of this face
   */
  virtual void SetMetric (uint16_t metric);

  /**
   * \brief Get routing/forwarding metric assigned to the face
   *
   * \returns configured routing/forwarding metric (cost) of this face
   */
  virtual uint16_t GetMetric (void) const;

  /**
   * These are face states and may be distinct from actual lower-layer
   * device states, such as found in real implementations (where the
   * device may be down but ndn face state is still up).
   */
  
  /**
   * \brief Enable or disable this face
   */
  virtual void
  SetUp (bool up = true);

  /**
   * \brief Returns true if this face is enabled, false otherwise.
   */
  virtual bool
  IsUp () const;

  /**
   * @brief Print information about the face into the stream
   * @param os stream to write information to
   */
  virtual std::ostream&
  Print (std::ostream &os) const;

  /**
   * \brief Set node Id
   *
   * Id is purely informative and should not be used for any other purpose
   *
   * \param id id to set
   */
  inline void
  SetId (uint32_t id);

  /**
   * \brief Get node Id
   *
   * Id is purely informative and should not be used for any other purpose
   *
   * \returns id id to set
   */
  inline uint32_t
  GetId () const;

  virtual bool CanEnqueueInterest(){return true;};

  void SetCapacity(uint64_t dataRate);
   uint64_t GetCapacity();

  /**
   * \brief Compare two faces. Only two faces on the same node could be compared.
   *
   * Internal index is used for comparison.
   */
  bool
  operator== (const Face &face) const;

  /**
   * \brief Compare two faces. Only two faces on the same node could be compared.
   *
   * Internal index is used for comparison.
   */
  inline bool
  operator!= (const Face &face) const;
  
  /**
   * \brief Compare two faces. Only two faces on the same node could be compared.
   *
   * Internal index is used for comparison.
   */
  bool
  operator< (const Face &face) const;

  inline const std::map<ndn::Name, ShrEntry> & GetShapingTable() const {return this->m_shaping_table;}
  inline std::map<ndn::Name, ShrEntry> & GetShapingTable() {return this->m_shaping_table;}

  inline const std::map<ndn::Name, STimeEntry> & GetSendingTable() const {return this->m_send_time_table;}
  inline std::map<ndn::Name, STimeEntry> & GetSendingTable() {return this->m_send_time_table;}

  virtual bool HobhisEnabled(){return false;};

  virtual bool ClientServer(){return false;};

  inline const std::map<ndn::Name, uint64_t> & GetInFaceBWTable() const {return this->m_InFaceBW;}
  inline std::map<ndn::Name, uint64_t> & GetInFaceBWTable() {return this->m_InFaceBW;}

  virtual void SetInFaceBW(ndn::Name prefix, uint64_t bw){};
  void SetFlowNumber(double nflows){ m_Nflows = nflows;};

  double GetFlowNumber(){return m_Nflows;};
protected:
  /**
   * \brief Send packet on a face (actual implementation)
   *
   * \param p smart pointer to a packet to send
   */
  virtual bool
  SendImpl (Ptr<Packet> p) = 0;  

  uint64_t DRate;

private:
  Face (const Face &); ///< \brief Disabled copy constructor
  Face& operator= (const Face &); ///< \brief Disabled copy operator
  
protected:
  Ptr<Node> m_node; ///< \brief Smart pointer to Node
  
private:
  ProtocolHandler m_protocolHandler; ///< Callback via which packets are getting send to Ndn stack
  bool m_ifup; ///< \brief flag indicating that the interface is UP 
  uint32_t m_id; ///< \brief id of the interface in Ndn stack (per-node uniqueness)
  uint32_t m_metric; ///< \brief metric of the face

  TracedCallback<Ptr<const Packet> > m_txTrace;
  TracedCallback<Ptr<const Packet> > m_rxTrace;
  TracedCallback<Ptr<const Packet> > m_dropTrace;

  std::map<ndn::Name, ShrEntry> m_shaping_table;
  std::map<ndn::Name, STimeEntry> m_send_time_table;
  std::map<ndn::Name, uint64_t> m_InFaceBW;

  double m_Nflows;
};

std::ostream& operator<< (std::ostream& os, const Face &face);

inline bool
operator < (const Ptr<Face> &lhs, const Ptr<Face> &rhs)
{
  return *lhs < *rhs;
}

void
Face::SetId (uint32_t id)
{
  m_id = id;
}

uint32_t
Face::GetId () const
{
  return m_id;
}

inline bool
Face::operator!= (const Face &face) const
{
  return !(*this == face);
}

} // namespace ndn
} // namespace ns3

#endif // NDN_FACE_H
