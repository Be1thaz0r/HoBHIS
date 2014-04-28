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

#ifndef NDN_CONSUMER_WINDOW_RAAQM_H
#define NDN_CONSUMER_WINDOW_RAAQM_H

#include <list>
#include "ndn-consumer-window.h"
#include "ns3/traced-value.h"

namespace ns3 {
namespace ndn {

class ConsumerWindowRAAQM: public ConsumerWindow
{
public:
  static TypeId GetTypeId ();

  ConsumerWindowRAAQM ();
  virtual ~ConsumerWindowRAAQM ();

protected:
  virtual void AdjustWindowOnContentObject (const Ptr<const ContentObjectHeader> &contentObject,
                                            Ptr<Packet> payload);

private:
  TracedValue<uint32_t> m_ssthresh;
  uint32_t m_window_cnt;
  std::list<Time> m_rtt_samples;

  double m_beta;
  double m_p_min;
  double m_p_max;
  uint32_t m_rtt_sample_size;
};

} // namespace ndn
} // namespace ns3

#endif
