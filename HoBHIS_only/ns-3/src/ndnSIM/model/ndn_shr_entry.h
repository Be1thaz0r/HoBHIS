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

#ifndef NDN_SHR_ENTRY_HPP
#define NDN_SHR_ENTRY_HPP



#include <ostream>

#include "ns3/ndn-name.h"


namespace ns3 {
    namespace ndn {

        class ShrEntry
        {
            private:

                double    m_sh_rate;     /**< The sh rate      */
                uint32_t  m_q_len;       /**< The queue length per flow */
                uint32_t  m_total_q_len; /**< Total queue length */
                double    m_rtt;         /**< The RTT          */
                double    m_bandwidth;   /**< The bandwidth    */
                bool	  m_fromIRC;	 /**< Indicates that tolrate in this table is the tolrate
                							  from IRC packet (the one of bottleneck node) */
                uint32_t  m_max_chunks;	 /**< Transmission buffer size to send Chunks */


            public:
                // Constructors

                /**
                 * \brief Constructor.
                 * \param prefix The prefix.
                 * \param shrate The sh rate.
                 * ... 
                 */

                ShrEntry(double shrate, uint32_t qlen_flow, uint32_t qlen, double rtt, double bw, bool irc, uint32_t max_chunks);

                // Getters/Setters

                inline double get_sh_rate() const {
                    return this->m_sh_rate;
                }

                inline void set_sh_rate(double sh_rate) {
                    this->m_sh_rate = sh_rate;
                }

                inline uint32_t get_queue_length() const {
                    return this->m_q_len;
                }

                inline uint32_t get_total_queue_length() const {
                    return this->m_total_q_len;
                }

                inline bool get_irc() const {
                	return this->m_fromIRC;
                }

                inline void set_queue_length(uint32_t qlen) {
                    this->m_q_len = qlen;
                }

                inline void set_total_queue_length(uint32_t qlen) {
                    this->m_total_q_len = qlen;
                }

                inline double get_rtt() const {
                    return this->m_rtt;
                }

                inline void set_rtt(double rtt) {
                    this->m_rtt = rtt;
                }


                inline double get_bandwith() const {
                    return this->m_bandwidth;
                }

                inline void set_bandwidth(double bw) {
                	this->m_bandwidth = bw;
                }

                inline void set_irc(bool irc) {
                	this->m_fromIRC = irc;
                }

                inline void set_max_chunks(uint32_t max_chunks) {
                	this->m_max_chunks = max_chunks;
                }

                inline uint32_t get_max_chunks() const {
                	return this->m_max_chunks;
                }
                // Operators

                ShrEntry & operator = (const ShrEntry & rhs);

                //bool operator < (const ShrEntry & rhs) const;

        };

        /**
         * \brief Write the content of an ShrEntry instance.
         * \param out The output stream.
         * \param entry The ShrEntry instance we want to write.
         * \param out The updated output stream.
         */

        std::ostream & operator << (std::ostream & out, const ShrEntry & entry);

        /**
         * \brief Compare two ShrEntry instances
         * \param x The left operand
         * \param y The right operand
         * \param return true if x is less than y
         */

        bool operator < (const ShrEntry & x, const ShrEntry & y);
    } // end namespace ndn
} // end namespace ns3

#endif
