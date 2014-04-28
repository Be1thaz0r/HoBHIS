/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: Rajib Bhattacharjea<raj.b@gatech.edu>
//

// Georgia Tech Network Simulator - Round Trip Time Estimation Class
// George F. Riley.  Georgia Tech, Spring 2002

// THIS IS A COPY OF rtt-estimator.h from internet module with minor modifications

#ifndef NDN_RTT_ESTIMATOR_H
#define NDN_RTT_ESTIMATOR_H

#include <deque>
#include "ns3/sequence-number.h"
#include "ns3/nstime.h"
#include "ns3/object.h"

namespace ns3 {

namespace ndn {

/**
 * \ingroup tcp
 *
 * \brief Helper class to store RTT measurements
 */
class RttHistory {
public:
  RttHistory (SequenceNumber32 s, uint32_t c, Time t);
  RttHistory (const RttHistory& h); // Copy constructor
public:
  SequenceNumber32  seq;  // First sequence number in packet sent
  uint32_t        count;  // Number of bytes sent
  Time            time;   // Time this one was sent
  bool            retx;   // True if this has been retransmitted
};

typedef std::deque<RttHistory> RttHistory_t;

/**
 * \ingroup tcp
 *
 * \brief Base class for all RTT Estimators
 */
class RttEstimator : public Object {
public:
  static TypeId GetTypeId (void);

  RttEstimator();
  RttEstimator (const RttEstimator&);

  virtual ~RttEstimator();

  /**
   * \brief Note that a particular sequence has been sent
   * \param seq the packet sequence number.
   * \param size the packet size.
   */
  virtual void SentSeq (SequenceNumber32 seq, uint32_t size);

  /**
   * \brief Note that a particular ack sequence has been received
   * \param ackSeq the ack sequence number.
   * \return The measured RTT for this ack.
   */
  virtual Time AckSeq (SequenceNumber32 ackSeq);

  /**
   * \brief Clear all history entries
   */
  virtual void ClearSent ();

  /**
   * \brief Add a new measurement to the estimator. Pure virtual function.
   * \param t the new RTT measure.
   */
  virtual void  Measurement (Time t) = 0;

  /**
   * \brief Returns the estimated RTO. Pure virtual function.
   * \return the estimated RTO.
   */
  virtual Time RetransmitTimeout () = 0;

  virtual Ptr<RttEstimator> Copy () const = 0;

  /**
   * \brief Increase the estimation multiplier up to MaxMultiplier.
   */
  virtual void IncreaseMultiplier ();

  /**
   * \brief Resets the estimation multiplier to 1.
   */
  virtual void ResetMultiplier ();

  /**
   * \brief Resets the estimation to its initial state.
   */
  virtual void Reset ();

  /**
   * \brief Sets the Minimum RTO.
   * \param minRto The minimum RTO returned by the estimator.
   */
  void SetMinRto (Time minRto);

  /**
   * \brief Get the Minimum RTO.
   * \return The minimum RTO returned by the estimator.
   */
  Time GetMinRto (void) const;

  /**
   * \brief Sets the Maximum RTO.
   * \param minRto The maximum RTO returned by the estimator.
   */
  void SetMaxRto (Time maxRto);

  /**
   * \brief Get the Maximum RTO.
   * \return The maximum RTO returned by the estimator.
   */
  Time GetMaxRto (void) const;

  /**
   * \brief Sets the current RTT estimate (forcefully).
   * \param estimate The current RTT estimate.
   */
  void SetCurrentEstimate (Time estimate);

  /**
   * \brief gets the current RTT estimate.
   * \return The current RTT estimate.
   */
  Time GetCurrentEstimate (void) const;

private:
  SequenceNumber32 m_next;    // Next expected sequence to be sent
  uint16_t m_maxMultiplier;
  Time m_initialEstimatedRtt;

protected:
  Time         m_currentEstimatedRtt;     // Current estimate
  Time         m_minRto;                  // minimum value of the timeout
  Time         m_maxRto;                  // maximum value of the timeout
  uint32_t     m_nSamples;                // Number of samples
  uint16_t     m_multiplier;              // RTO Multiplier
  RttHistory_t m_history;     // List of sent packet
};

} // namespace ndn

} // namespace ns3

#endif /* RTT_ESTIMATOR_H */
