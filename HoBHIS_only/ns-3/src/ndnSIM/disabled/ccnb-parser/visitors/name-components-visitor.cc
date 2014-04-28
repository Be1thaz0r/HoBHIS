/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "name-components-visitor.h"

#include "string-visitor.h"
#include "../syntax-tree/dtag.h"

#include "ns3/ndn-name-components.h"

namespace ns3 {
namespace ndn {
namespace CcnbParser {

void
NameComponentsVisitor::visit (Dtag &n, boost::any param/*should be NameComponents* */)
{
  // uint32_t n.m_dtag;
  // std::list<Ptr<Block> > n.m_nestedBlocks;
  static StringVisitor stringVisitor; 
 
  NameComponents &components = *(boost::any_cast<NameComponents*> (param));

  switch (n.m_dtag)
    {
    case CCN_DTAG_Component:
      if (n.m_nestedTags.size()!=1) // should be exactly one UDATA inside this tag
        throw CcnbDecodingException ();
      components.Add (
                      boost::any_cast<std::string> ((*n.m_nestedTags.begin())->accept(
                                                                                        stringVisitor
                                                                                        )));
      break;
    default:
      // ignore any other components
      // when parsing Exclude, there could be <Any /> and <Bloom /> tags
      break;
    }
}

}
}
}
