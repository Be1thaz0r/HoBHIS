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

#include "ndn-consumer-window-relentless.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("ndn.ConsumerWindowRelentless");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ConsumerWindowRelentless);

TypeId
ConsumerWindowRelentless::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::ConsumerWindowRelentless")
    .SetGroupName ("Ndn")
    .SetParent<ConsumerWindow> ()
    .AddConstructor<ConsumerWindowRelentless> ()

    .AddTraceSource ("SsthreshTrace",
                     "Ssthresh that determines whether we are in slow start or congestion avoidance phase",
                     MakeTraceSourceAccessor (&ConsumerWindowRelentless::m_ssthresh))
    ;

  return tid;
}

ConsumerWindowRelentless::ConsumerWindowRelentless ()
  : ConsumerWindow ()
  , m_ssthresh (std::numeric_limits<uint32_t>::max ())
  , m_window_cnt (0)
{
}

ConsumerWindowRelentless::~ConsumerWindowRelentless ()
{
}

void
ConsumerWindowRelentless::AdjustWindowOnContentObject (const Ptr<const ContentObjectHeader> &contentObject,
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

  NS_LOG_DEBUG ("Window: " << m_window << ", InFlight: " << m_inFlight << ", Ssthresh: " << m_ssthresh);
}

void
ConsumerWindowRelentless::AdjustWindowOnNack (const Ptr<const InterestHeader> &interest, Ptr<Packet> payload)
{
  m_window = std::max<uint32_t> (0, m_window - 1);
  m_ssthresh = m_window;

  NS_LOG_DEBUG ("Window: " << m_window << ", InFlight: " << m_inFlight << ", Ssthresh: " << m_ssthresh);
}

} // namespace ndn
} // namespace ns3
