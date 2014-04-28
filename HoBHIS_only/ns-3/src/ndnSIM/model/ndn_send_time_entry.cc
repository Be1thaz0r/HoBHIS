/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
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
 * Authors: Rozhnova Natalya <natalya.rozhnova@lip6.fr>
 */

#include "ndn_send_time_entry.h"
#include <ostream>

namespace ns3 {
    namespace ndn {


    	STimeEntry::STimeEntry(double stime):
    			m_stime(stime)
        	{}

        std::ostream & operator << (std::ostream & out, const STimeEntry & stime_entry)
        {
            out << "stime "      << stime_entry.get_send_time()
            	<<std::endl;
                return out;
        }

        STimeEntry & STimeEntry::operator = (const STimeEntry & rhs)
        {
            this->set_send_time(rhs.get_send_time());
            return *this;
        }

        bool operator < (const STimeEntry & x, const STimeEntry & y) {

        	if (x.get_send_time() < y.get_send_time()) return true;
            if (y.get_send_time() < x.get_send_time()) return false;

            // x == y
            return false;
        }
    }
}
