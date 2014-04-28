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

#ifndef _CCNB_PARSER_BASE_TAG_H_
#define _CCNB_PARSER_BASE_TAG_H_

#include "block.h"
#include <list>

namespace ns3 {
namespace ndn {
namespace CcnbParser {

/**
 * \ingroup ccnx-ccnb
 * \brief Virtual base class providing a common storage for TAG
 * and DTAG ccnb-encoded blocks
 *
 * \see http://www.ccnx.org/releases/latest/doc/technical/BinaryEncoding.html
 */
class BaseTag : public Block
{
public:
  std::list<Ptr<Block> > m_attrs;      ///< \brief List of attributes, associated with this tag
  std::list<Ptr<Block> > m_nestedTags; ///< \brief List of nested tags
  
protected:
  /**
   * \brief Default constructor
   */
  BaseTag() { }
};

}
}
}

#endif // _CCNB_PARSER_BASE_TAG_H_

