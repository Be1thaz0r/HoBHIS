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

#ifndef NDN_CONSUMER_WINDOW_CUBIC_H
#define NDN_CONSUMER_WINDOW_CUBIC_H

#include "ndn-consumer-window.h"
#include "ns3/traced-value.h"

namespace ns3 {
namespace ndn {

class ConsumerWindowCUBIC: public ConsumerWindow
{
public:
  static TypeId GetTypeId ();

  ConsumerWindowCUBIC ();
  virtual ~ConsumerWindowCUBIC ();

protected:
  virtual void AdjustWindowOnNack (const Ptr<const InterestHeader> &interest, Ptr<Packet> payload);
  virtual void AdjustWindowOnContentObject (const Ptr<const ContentObjectHeader> &contentObject,
                                            Ptr<Packet> payload);

private:
  TracedValue<uint32_t> m_ssthresh;
  uint32_t m_window_cnt;

  double m_beta;
  double m_c;
  bool m_fast_convergence;

  Time m_last_decrease;
  Time m_epoch_start;
  Time m_dMin;
  uint32_t m_last_window;
  double m_k;
  uint32_t m_origin_point;
};

} // namespace ndn
} // namespace ns3

#endif
