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

#ifndef NDN_CONSUMER_WINDOW_AIMD_H
#define NDN_CONSUMER_WINDOW_AIMD_H

#include "ndn-consumer-window-relentless.h"
#include "ns3/traced-value.h"

namespace ns3 {
namespace ndn {

class ConsumerWindowAIMD: public ConsumerWindowRelentless
{
public:
  static TypeId GetTypeId ();

  ConsumerWindowAIMD ();
  virtual ~ConsumerWindowAIMD ();

protected:
  virtual void AdjustWindowOnNack (const Ptr<const InterestHeader> &interest, Ptr<Packet> payload);

private:
  uint32_t m_recover;
};

} // namespace ndn
} // namespace ns3

#endif
