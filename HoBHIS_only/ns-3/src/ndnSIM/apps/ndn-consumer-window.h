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

#ifndef NDN_CONSUMER_WINDOW_H
#define NDN_CONSUMER_WINDOW_H

#include "ndn-consumer.h"
#include "ns3/traced-value.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn
 * \brief Ndn application for sending out Interest packets (window-based, constant window size, default size: 1)
 *
 * !!! ATTENTION !!! This is highly experimental and relies on experimental features of the simulator.
 * Behavior may be unpredictable if used incorrectly.
 */
class ConsumerWindow: public Consumer
{
public:
  static TypeId GetTypeId ();

  /**
   * \brief Default constructor
   */
  ConsumerWindow ();

  virtual ~ConsumerWindow ();

  // From App
  // virtual void
  // OnInterest (const Ptr<const InterestHeader> &interest);

  virtual void
  OnNack (const Ptr<const InterestHeader> &interest, Ptr<Packet> payload);

  virtual void
  OnContentObject (const Ptr<const ContentObjectHeader> &contentObject,
                   Ptr<Packet> payload);

  virtual void
  OnTimeout (uint32_t sequenceNumber);

  virtual void
  WillSendOutInterest(uint32_t sequenceNumber);

protected:
  /**
   * \brief Constructs the Interest packet and sends it using a callback to the underlying NDN protocol
   */
  virtual void
  ScheduleNextPacket ();

  virtual void AdjustWindowOnNack (const Ptr<const InterestHeader> &interest, Ptr<Packet> payload);
  virtual void AdjustWindowOnContentObject (const Ptr<const ContentObjectHeader> &contentObject,
                                            Ptr<Packet> payload);
  virtual void AdjustWindowOnTimeout (uint32_t sequenceNumber);

  uint32_t m_initialWindow;
  TracedValue<uint32_t> m_window;
  TracedValue<uint32_t> m_inFlight;

private:
  virtual void
  SetWindow (uint32_t window);

  uint32_t
  GetWindow () const;
};

} // namespace ndn
} // namespace ns3

#endif
