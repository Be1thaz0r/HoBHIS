/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 */

#ifndef NDN_CONSUMER_RATE_H
#define NDN_CONSUMER_RATE_H

#include "ndn-consumer.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn
 * \brief Ndn application for sending out Interest packets at a "constant" rate (Poisson process)
 */
class ConsumerRate: public Consumer
{
public:
  static TypeId GetTypeId ();

  /**
   * \brief Default constructor
   */
  ConsumerRate ();
  virtual ~ConsumerRate ();

protected:
  /**
   * \brief Constructs the Interest packet and sends it using a callback to the underlying NDN protocol
   */
  virtual void
  ScheduleNextPacket ();

  virtual void
  OnContentObject (const Ptr<const ContentObjectHeader> &contentObject,
                   Ptr<Packet> payload);

  double m_frequency; // interest packet sending frequency (in hertz)
  bool m_firstTime;
  double m_incomingDataFrequency;
  Time m_prevData;

  double m_probeFactor;
  bool m_inSlowStart;
};

} // namespace ndn
} // namespace ns3

#endif
