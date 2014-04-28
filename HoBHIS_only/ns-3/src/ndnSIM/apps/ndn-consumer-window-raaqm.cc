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

#include "ndn-consumer-window-raaqm.h"
#include "ns3/log.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/simulator.h"
#include "ns3/random-variable.h"
#include <boost/lexical_cast.hpp>
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("ndn.ConsumerWindowRAAQM");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ConsumerWindowRAAQM);

TypeId
ConsumerWindowRAAQM::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::ConsumerWindowRAAQM")
    .SetGroupName ("Ndn")
    .SetParent<ConsumerWindow> ()
    .AddConstructor<ConsumerWindowRAAQM> ()

    .AddAttribute ("Beta", "Multiplicative decrease factor",
                   DoubleValue (0.1),
                   MakeDoubleAccessor (&ConsumerWindowRAAQM::m_beta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Pmin", "Minimum window decrease probability",
                   DoubleValue (0.00001),
                   MakeDoubleAccessor (&ConsumerWindowRAAQM::m_p_min),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Pmax", "Maximum window decrease probability",
                   DoubleValue (0.1),
                   MakeDoubleAccessor (&ConsumerWindowRAAQM::m_p_max),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RTTSampleSize", "RTT sample window size",
                   UintegerValue (30),
                   MakeUintegerAccessor (&ConsumerWindowRAAQM::m_rtt_sample_size),
                   MakeUintegerChecker<uint32_t> ())

    .AddTraceSource ("SsthreshTrace",
                     "Ssthresh that determines whether we are in slow start or congestion avoidance phase",
                     MakeTraceSourceAccessor (&ConsumerWindowRAAQM::m_ssthresh))
    ;

  return tid;
}

ConsumerWindowRAAQM::ConsumerWindowRAAQM ()
  : ConsumerWindow ()
  , m_ssthresh (std::numeric_limits<uint32_t>::max ())
  , m_window_cnt (0)
  , m_rtt_samples ()
{
}

ConsumerWindowRAAQM::~ConsumerWindowRAAQM ()
{
}

void
ConsumerWindowRAAQM::AdjustWindowOnContentObject (const Ptr<const ContentObjectHeader> &contentObject,
                                                       Ptr<Packet> payload)
{
  if (m_window < m_ssthresh)
    {
      // in slow start phase
      m_window++;
    }
  else
    {
      // in congestion avoidance phase
      if (m_window_cnt >= m_window)
        {
          m_window++;
          m_window_cnt = 0;
        }
      else
        {
          m_window_cnt++;
        }
    }

  // RTT
  uint32_t seq = boost::lexical_cast<uint32_t> (contentObject->GetName ().GetComponents ().back ());
  if (m_seqRetxCounts[seq] == 1) // ignore retransmitted interest/data pairs
    {
      SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find (seq);
      if (entry != m_seqLastDelay.end ())
        {
          Time cur_rtt = Simulator::Now () - entry->time;
          m_rtt_samples.push_back(cur_rtt);
          if (m_rtt_samples.size() >= m_rtt_sample_size)
            {
              while (m_rtt_samples.size() > m_rtt_sample_size)
                m_rtt_samples.pop_front();

              Time min_rtt = *std::min_element(m_rtt_samples.begin(), m_rtt_samples.end());
              Time max_rtt = *std::max_element(m_rtt_samples.begin(), m_rtt_samples.end());
              NS_LOG_DEBUG ("cur_rtt: " << cur_rtt << ", min_rtt: " << min_rtt << ", max_rtt: " << max_rtt);

              double p = m_p_min + (m_p_max - m_p_min) * ((cur_rtt - min_rtt).GetSeconds() / (max_rtt - min_rtt).GetSeconds());
              NS_LOG_DEBUG ("window decrease probability: " << p);

              UniformVariable r (0.0, 1.0);
              if (r.GetValue () < p)
                {
                  m_window = m_window * (1.0 - m_beta);
                  m_ssthresh = m_window;
                }
            }
        }
    }

  NS_LOG_DEBUG ("Window: " << m_window << ", InFlight: " << m_inFlight);
}

} // namespace ndn
} // namespace ns3
