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

#include "ndn-consumer-window-aimd.h"
#include "ns3/ndn-interest.h"
#include "ns3/log.h"

#include <boost/lexical_cast.hpp>

NS_LOG_COMPONENT_DEFINE ("ndn.ConsumerWindowAIMD");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ConsumerWindowAIMD);

TypeId
ConsumerWindowAIMD::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::ConsumerWindowAIMD")
    .SetGroupName ("Ndn")
    .SetParent<ConsumerWindowRelentless> ()
    .AddConstructor<ConsumerWindowAIMD> ()
    ;

  return tid;
}

ConsumerWindowAIMD::ConsumerWindowAIMD ()
  : ConsumerWindowRelentless ()
  , m_recover (0)
{
}

ConsumerWindowAIMD::~ConsumerWindowAIMD ()
{
}

void
ConsumerWindowAIMD::AdjustWindowOnNack (const Ptr<const InterestHeader> &interest, Ptr<Packet> payload)
{
  // multiplicative decrease
  // but do it just once for every window of interests
  uint32_t seq = boost::lexical_cast<uint32_t> (interest->GetName ().GetComponents ().back ());
  if (seq > m_recover)
    {
      m_ssthresh = std::max<uint32_t> (2, m_inFlight / 2);
      m_window = m_ssthresh;
      m_window_cnt = 0;
      m_recover = m_seq;
    }

  NS_LOG_DEBUG ("Window: " << m_window << ", InFlight: " << m_inFlight << ", Ssthresh: " << m_ssthresh);
}

} // namespace ndn
} // namespace ns3
