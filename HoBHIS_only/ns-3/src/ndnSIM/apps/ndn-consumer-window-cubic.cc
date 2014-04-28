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
 * Author: Yaogong Wang <ywang15@ncsu.edu>
 */

#include "ndn-consumer-window-cubic.h"
#include "ns3/log.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/simulator.h"
#include <boost/lexical_cast.hpp>

NS_LOG_COMPONENT_DEFINE ("ndn.ConsumerWindowCUBIC");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ConsumerWindowCUBIC);

TypeId
ConsumerWindowCUBIC::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::ConsumerWindowCUBIC")
    .SetGroupName ("Ndn")
    .SetParent<ConsumerWindow> ()
    .AddConstructor<ConsumerWindowCUBIC> ()

    .AddAttribute ("Beta", "CUBIC multiplicative decrease factor",
                 DoubleValue (0.2),
                 MakeDoubleAccessor (&ConsumerWindowCUBIC::m_beta),
                 MakeDoubleChecker<double> ())
    .AddAttribute ("C", "CUBIC parameter",
                 DoubleValue (0.4),
                 MakeDoubleAccessor (&ConsumerWindowCUBIC::m_c),
                 MakeDoubleChecker<double> ())
    .AddAttribute ("FastConvergence", "turn on/off fast convergence",
                 BooleanValue (true),
                 MakeBooleanAccessor (&ConsumerWindowCUBIC::m_fast_convergence),
                 MakeBooleanChecker ())

    .AddTraceSource ("SsthreshTrace",
                     "Ssthresh that determines whether we are in slow start or congestion avoidance phase",
                     MakeTraceSourceAccessor (&ConsumerWindowCUBIC::m_ssthresh))
    ;

  return tid;
}

ConsumerWindowCUBIC::ConsumerWindowCUBIC ()
  : ConsumerWindow ()
  , m_ssthresh (std::numeric_limits<uint32_t>::max ())
  , m_window_cnt (0)
  , m_last_decrease (0.0)
  , m_epoch_start (0.0)
  , m_dMin (0.0)
  , m_last_window (0)
  , m_k (0.0)
  , m_origin_point (0)
{
}

ConsumerWindowCUBIC::~ConsumerWindowCUBIC ()
{
}

void
ConsumerWindowCUBIC::AdjustWindowOnContentObject (const Ptr<const ContentObjectHeader> &contentObject,
                                                       Ptr<Packet> payload)
{
  // record minimum RTT in m_dMin
  uint32_t seq = boost::lexical_cast<uint32_t> (contentObject->GetName ().GetComponents ().back ());
  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find (seq);
  if (entry != m_seqLastDelay.end ())
    {
      Time rtt = Simulator::Now () - entry->time;
      if (m_dMin == Seconds(0.0))
        m_dMin = rtt;
      else
        m_dMin = m_dMin<rtt ? m_dMin : rtt;
    }

  if (m_window < m_ssthresh)
    {
      // in slow start phase
      m_window++;
    }
  else
    {
      // in congestion avoidance phase
      if (m_epoch_start == Seconds(0.0))
        {
          // new epoch
          m_epoch_start = Simulator::Now();
          m_k = pow((m_last_window - m_window) / m_c, 1/3);
          m_origin_point = m_last_window;
        }

      double t = (Simulator::Now() + m_dMin - m_epoch_start).GetSeconds();
      uint32_t target = m_origin_point + m_c * pow(t - m_k, 3);
      uint32_t cnt;
      if (target > m_window)
        cnt = m_window / (target - m_window);
      else
        cnt = 100 * m_window;

      if (m_window_cnt >= cnt)
        {
          m_window++;
          m_window_cnt = 0;
        }
      else
        {
          m_window_cnt++;
        }
    }

  NS_LOG_DEBUG ("Window: " << m_window << ", InFlight: " << m_inFlight << ", Ssthresh: " << m_ssthresh);
}

void
ConsumerWindowCUBIC::AdjustWindowOnNack (const Ptr<const InterestHeader> &interest, Ptr<Packet> payload)
{
  uint32_t seq = boost::lexical_cast<uint32_t> (interest->GetName ().GetComponents ().back ());
  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find (seq);
  if (entry != m_seqLastDelay.end () && entry->time > m_last_decrease)
    {
      m_epoch_start = Seconds(0.0);
      m_last_decrease = Simulator::Now();

      if (m_window < m_last_window && m_fast_convergence)
        m_last_window = m_window * (2.0 - m_beta) / 2.0;
      else
        m_last_window = m_window;

      m_window = m_window * (1.0 - m_beta);
      m_ssthresh = m_window;
    }

  NS_LOG_DEBUG ("Window: " << m_window << ", InFlight: " << m_inFlight << ", Ssthresh: " << m_ssthresh);
}

} // namespace ndn
} // namespace ns3
