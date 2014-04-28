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

#include "ndn_shr_entry.h"
#include <ostream>

namespace ns3 {
    namespace ndn {


    	ShrEntry::ShrEntry(double shrate, uint32_t qlen_flow, uint32_t qlen, double rtt, double bw, bool irc, uint32_t max_chunks):
            m_sh_rate(shrate),
            m_q_len(qlen_flow),
            m_total_q_len (qlen),
            m_rtt(rtt),
            m_bandwidth(bw),
            m_fromIRC(irc),
    		m_max_chunks(max_chunks)
        {}

        std::ostream & operator << (std::ostream & out, const ShrEntry & shr_entry)
        {
            out << "\tsh rate: "      << shr_entry.get_sh_rate()
                << "\tqueue length: " << shr_entry.get_queue_length()
                << "\ttotal queue length"<< shr_entry.get_total_queue_length()
                << "\trtt: "          << shr_entry.get_rtt()
                << "\tbandwidth: "    << shr_entry.get_bandwith()
                << "\tirc: "		  << shr_entry.get_irc()
                << "\tmax chunks"	  << shr_entry.get_max_chunks()
                <<std::endl;
                return out;
        }

        ShrEntry & ShrEntry::operator = (const ShrEntry & rhs)
        {
            this->set_sh_rate(rhs.get_sh_rate());
            this->set_queue_length(rhs.get_queue_length());
            this->set_total_queue_length(rhs.get_total_queue_length());
            this->set_rtt(rhs.get_rtt());
            this->set_bandwidth(rhs.get_bandwith());
            this->set_irc(rhs.get_irc());
            this->set_max_chunks(rhs.get_max_chunks());
            return *this;
        }

        bool operator < (const ShrEntry & x, const ShrEntry & y) {

        	if (x.get_sh_rate() < y.get_sh_rate()) return true;
            if (y.get_sh_rate() < x.get_sh_rate()) return false;

            if (x.get_queue_length() < y.get_queue_length()) return true;
            if (y.get_queue_length() < x.get_queue_length()) return false;

            if (x.get_total_queue_length() < y.get_total_queue_length()) return true;
            if (y.get_total_queue_length() < x.get_total_queue_length()) return false;


            if (x.get_rtt() < y.get_rtt()) return true;
            if (y.get_rtt() < x.get_rtt()) return false;

            if (x.get_bandwith() < y.get_bandwith()) return true;
            if (y.get_bandwith() < x.get_bandwith()) return false;

            if (x.get_max_chunks() < y.get_max_chunks()) return true;
            if (y.get_max_chunks() < x.get_max_chunks()) return false;

            // x == y
            return false;
        }
    }
}
