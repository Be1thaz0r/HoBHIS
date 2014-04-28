#ifndef NDN_REROUTE_TAG_H
#define NDN_REROUTE_TAG_H

#include "ns3/tag.h"

namespace ns3 {
namespace ndn {

class ReroutingTag : public Tag
{
public:
	ReroutingTag (): m_reroute_label (0) {}

  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::ndn::ReroutingTag")
      .SetParent<Tag> ()
      .AddConstructor<ReroutingTag> ()
    ;
    return tid;
  }

  virtual TypeId GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  virtual uint32_t GetSerializedSize (void) const
  {
    return sizeof(uint8_t);
  }

  virtual void Serialize (TagBuffer i) const
  {
    i.WriteU8 (m_reroute_label);
  }

  virtual void Deserialize (TagBuffer i)
  {
    m_reroute_label = i.ReadU8 ();
  }

  virtual void Print (std::ostream &os) const
  {
    os << "Label= " << m_reroute_label;
  }

  void SetLabel (uint8_t label)
  {
	  m_reroute_label = label;
  }

  uint8_t GetLabel (void) const
  {
    return m_reroute_label;
  }

private:
  uint8_t m_reroute_label;
};

} //ndn
} //ns3
#endif // NDN_REROUTE_TAG_H
